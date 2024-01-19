/*
 * Copyright (C) 2008-2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"

#if ENABLE(JIT)

#include "JIT.h"

#include "BaselineJITRegisters.h"
#include "BytecodeOperandsForCheckpoint.h"
#include "CacheableIdentifierInlines.h"
#include "CallFrameShuffler.h"
#include "CodeBlock.h"
#include "JITInlines.h"
#include "JITThunks.h"
#include "ScratchRegisterAllocator.h"
#include "SetupVarargsFrame.h"
#include "SlowPathCall.h"
#include "StackAlignment.h"
#include "ThunkGenerators.h"

namespace JSC {

void JIT::emit_op_ret(const JSInstruction* currentInstruction)
{
    static_assert(noOverlap(returnValueJSR, callFrameRegister));

    // Return the result in returnValueGPR (returnValueGPR2/returnValueGPR on 32-bit).
    auto bytecode = currentInstruction->as<OpRet>();
    emitGetVirtualRegister(bytecode.m_value, returnValueJSR);
    jumpThunk(CodeLocationLabel { vm().getCTIStub(CommonJITThunkID::ReturnFromBaseline).retaggedCode<NoPtrTag>() });
}

template<typename Op>
void JIT::emitPutCallResult(const Op& bytecode)
{
    emitValueProfilingSite(bytecode, returnValueJSR);
    emitPutVirtualRegister(destinationFor(bytecode, m_bytecodeIndex.checkpoint()).virtualRegister(), returnValueJSR);
}

template<typename Op>
void JIT::compileSetupFrame(const Op& bytecode)
{
    constexpr auto opcodeID = Op::opcodeID;

    if constexpr (opcodeID == op_call_varargs || opcodeID == op_construct_varargs || opcodeID == op_tail_call_varargs || opcodeID == op_tail_call_forward_arguments) {
        VirtualRegister thisValue = bytecode.m_thisValue;
        VirtualRegister arguments = bytecode.m_arguments;
        int firstFreeRegister = bytecode.m_firstFree.offset(); // FIXME: Why is this a virtual register if we never use it as one...
        int firstVarArgOffset = bytecode.m_firstVarArg;

        {
            constexpr GPRReg globalObjectGPR = preferredArgumentGPR<Z_JITOperation_GJZZ, 0>();
            constexpr JSValueRegs argumentsJSR = preferredArgumentJSR<Z_JITOperation_GJZZ, 1>();

            Z_JITOperation_GJZZ sizeOperation;
            if constexpr (opcodeID == op_tail_call_forward_arguments)
                sizeOperation = operationSizeFrameForForwardArguments;
            else
                sizeOperation = operationSizeFrameForVarargs;

            loadGlobalObject(globalObjectGPR);
            emitGetVirtualRegister(arguments, argumentsJSR);
            callOperation(sizeOperation, globalObjectGPR, argumentsJSR, -firstFreeRegister, firstVarArgOffset);
            move(TrustedImm32(-firstFreeRegister), regT1);
            emitSetVarargsFrame(*this, returnValueGPR, false, regT1, regT1);
        }

#if USE(JSVALUE64)
        addPtr(TrustedImm32(-static_cast<int32_t>(sizeof(CallerFrameAndPC) + WTF::roundUpToMultipleOf(stackAlignmentBytes(), 5 * sizeof(void*)))), regT1, stackPointerRegister);
#elif USE(JSVALUE32_64)
        addPtr(TrustedImm32(-(sizeof(CallerFrameAndPC) + WTF::roundUpToMultipleOf(stackAlignmentBytes(), 6 * sizeof(void*)))), regT1, stackPointerRegister);
#endif

        {
            emitGetVirtualRegister(arguments, jsRegT32);
            F_JITOperation_GFJZZ setupOperation;
            if constexpr (opcodeID == op_tail_call_forward_arguments)
                setupOperation = operationSetupForwardArgumentsFrame;
            else
                setupOperation = operationSetupVarargsFrame;
            loadGlobalObject(regT4);
            callOperation(setupOperation, regT4, regT1, jsRegT32, firstVarArgOffset, regT0);
            move(returnValueGPR, regT5);
        }

        // Profile the argument count.
        load32(Address(regT5, CallFrameSlot::argumentCountIncludingThis * static_cast<int>(sizeof(Register)) + PayloadOffset), regT2);
        move(TrustedImm32(CallLinkInfo::maxProfiledArgumentCountIncludingThisForVarargs), regT0);
#if CPU(ARM64) || CPU(X86_64)
        moveConditionally32(Above, regT2, regT0, regT0, regT2, regT2);
#else
        auto lower = branch32(BelowOrEqual, regT2, regT0);
        move(regT0, regT2);
        lower.link(this);
#endif
        materializePointerIntoMetadata(bytecode, Op::Metadata::offsetOfCallLinkInfo(), regT0);
        Jump notBiggest = branch32(Above, Address(regT0, CallLinkInfo::offsetOfMaxArgumentCountIncludingThisForVarargs()), regT2);
        store8(regT2, Address(regT0, CallLinkInfo::offsetOfMaxArgumentCountIncludingThisForVarargs()));
        notBiggest.link(this);

        // Initialize 'this'.
        constexpr JSValueRegs thisJSR = jsRegT10;
        emitGetVirtualRegister(thisValue, thisJSR);
        storeValue(thisJSR, Address(regT5, CallFrame::thisArgumentOffset() * static_cast<int>(sizeof(Register))));

        addPtr(TrustedImm32(sizeof(CallerFrameAndPC)), regT5, stackPointerRegister);
    } else {
        unsigned checkpoint = m_bytecodeIndex.checkpoint();
        int argCountIncludingThis = argumentCountIncludingThisFor(bytecode, checkpoint);
        int registerOffset = -static_cast<int>(stackOffsetInRegistersForCall(bytecode, checkpoint));


        if constexpr (opcodeID == op_call || opcodeID == op_tail_call || opcodeID == op_iterator_open || opcodeID == op_call_ignore_result) {
            if (shouldEmitProfiling()) {
                constexpr JSValueRegs tmpJSR = returnValueJSR;
                constexpr GPRReg tmpGPR = tmpJSR.payloadGPR();
                emitGetVirtualRegister(VirtualRegister(registerOffset + CallFrame::argumentOffsetIncludingThis(0)), tmpJSR);
                Jump done = branchIfNotCell(tmpJSR);
                load32(Address(tmpJSR.payloadGPR(), JSCell::structureIDOffset()), tmpGPR);
                store32ToMetadata(tmpGPR, bytecode, Op::Metadata::offsetOfArrayProfile() + ArrayProfile::offsetOfLastSeenStructureID());
                done.link(this);
            }
        }

        addPtr(TrustedImm32(registerOffset * sizeof(Register) + sizeof(CallerFrameAndPC)), callFrameRegister, stackPointerRegister);
        store32(TrustedImm32(argCountIncludingThis), Address(stackPointerRegister, CallFrameSlot::argumentCountIncludingThis * static_cast<int>(sizeof(Register)) + PayloadOffset - sizeof(CallerFrameAndPC)));
    }
}

template<typename Op>
void JIT::compileCallDirectEval(const Op&)
{
}

template<>
void JIT::compileCallDirectEval(const OpCallDirectEval& bytecode)
{
    using BaselineJITRegisters::CallDirectEval::SlowPath::calleeFrameGPR;
    using BaselineJITRegisters::CallDirectEval::SlowPath::thisValueJSR;
    using BaselineJITRegisters::CallDirectEval::SlowPath::scopeGPR;

    addPtr(TrustedImm32(-static_cast<ptrdiff_t>(sizeof(CallerFrameAndPC))), stackPointerRegister, calleeFrameGPR);
    storePtr(callFrameRegister, Address(calleeFrameGPR, CallFrame::callerFrameOffset()));

    resetSP();

    emitGetVirtualRegister(bytecode.m_thisValue, thisValueJSR);
    emitGetVirtualRegisterPayload(bytecode.m_scope, scopeGPR);
    callOperation(bytecode.m_ecmaMode.isStrict() ? operationCallDirectEvalStrict : operationCallDirectEvalSloppy, calleeFrameGPR, scopeGPR, thisValueJSR);
    addSlowCase(branchIfEmpty(returnValueJSR));

    setFastPathResumePoint();
    emitPutCallResult(bytecode);
}

void JIT::compileCallDirectEvalSlowCase(const JSInstruction* instruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCases(iter);

    auto bytecode = instruction->as<OpCallDirectEval>();
    int registerOffset = -bytecode.m_argv;

    addPtr(TrustedImm32(registerOffset * sizeof(Register) + sizeof(CallerFrameAndPC)), callFrameRegister, stackPointerRegister);

    static_assert(noOverlap(BaselineJITRegisters::Call::calleeJSR, BaselineJITRegisters::Call::callLinkInfoGPR, regT3));
    loadValue(Address(stackPointerRegister, sizeof(Register) * CallFrameSlot::callee - sizeof(CallerFrameAndPC)), BaselineJITRegisters::Call::calleeJSR);
    loadGlobalObject(regT3);
    materializePointerIntoMetadata(bytecode, OpCallDirectEval::Metadata::offsetOfCallLinkInfo(), BaselineJITRegisters::Call::callLinkInfoGPR);
    emitVirtualCallWithoutMovingGlobalObject(*m_vm, BaselineJITRegisters::Call::callLinkInfoGPR, CallMode::Regular);
    resetSP();
}

template<typename Op>
bool JIT::compileTailCall(const Op&, BaselineUnlinkedCallLinkInfo*, unsigned)
{
    return false;
}

template<>
bool JIT::compileTailCall(const OpTailCall& bytecode, BaselineUnlinkedCallLinkInfo* callLinkInfo, unsigned callLinkInfoIndex)
{
    auto [slowPaths, dispatchLabel] = CallLinkInfo::emitTailCallFastPath(*this, callLinkInfo, BaselineJITRegisters::Call::calleeJSR.payloadGPR(), BaselineJITRegisters::Call::callLinkInfoGPR, scopedLambda<void()>([&] {
        CallFrameShuffleData shuffleData = CallFrameShuffleData::createForBaselineOrLLIntTailCall(bytecode, m_unlinkedCodeBlock->numParameters());
        CallFrameShuffler shuffler { *this, shuffleData };
        shuffler.setCalleeJSValueRegs(BaselineJITRegisters::Call::calleeJSR);
        shuffler.lockGPR(BaselineJITRegisters::Call::callLinkInfoGPR);
        shuffler.lockGPR(BaselineJITRegisters::Call::globalObjectGPR);
        shuffler.lockGPR(BaselineJITRegisters::Call::callTargetGPR);
        shuffler.prepareForTailCall();
    }));
    addSlowCase(slowPaths);

    auto doneLocation = label();
    m_callCompilationInfo[callLinkInfoIndex].doneLocation = doneLocation;
    m_callCompilationInfo[callLinkInfoIndex].dispatchLabel = dispatchLabel;

    return true;
}

template<typename Op>
void JIT::compileOpCall(const JSInstruction* instruction, unsigned callLinkInfoIndex)
{
    auto bytecode = instruction->as<Op>();
    VirtualRegister callee = calleeFor(bytecode, m_bytecodeIndex.checkpoint());

    /* Caller always:
        - Updates callFrameRegister to callee callFrame.
        - Initializes ArgumentCount; CallerFrame; Callee.

       For a JS call:
        - Callee initializes ReturnPC; CodeBlock.
        - Callee restores callFrameRegister before return.

       For a non-JS call:
        - Caller initializes ReturnPC; CodeBlock.
        - Caller restores callFrameRegister after return.
    */

    BaselineUnlinkedCallLinkInfo* callLinkInfo = nullptr;
    if constexpr (Op::opcodeID != op_call_direct_eval) {
        callLinkInfo = addUnlinkedCallLinkInfo();
        callLinkInfo->bytecodeIndex = m_bytecodeIndex;
        ASSERT(m_callCompilationInfo.size() == callLinkInfoIndex);
        m_callCompilationInfo.append(CallCompilationInfo());
        m_callCompilationInfo[callLinkInfoIndex].unlinkedCallLinkInfo = callLinkInfo;
    }
    compileSetupFrame(bytecode);

    // SP holds newCallFrame + sizeof(CallerFrameAndPC), with ArgumentCount initialized.
    uint32_t locationBits = CallSiteIndex(m_bytecodeIndex).bits();
    store32(TrustedImm32(locationBits), Address(callFrameRegister, CallFrameSlot::argumentCountIncludingThis * static_cast<int>(sizeof(Register)) + TagOffset));

    emitGetVirtualRegister(callee, BaselineJITRegisters::Call::calleeJSR);
    storeValue(BaselineJITRegisters::Call::calleeJSR, Address(stackPointerRegister, CallFrameSlot::callee * static_cast<int>(sizeof(Register)) - sizeof(CallerFrameAndPC)));

    if constexpr (Op::opcodeID == op_call_direct_eval) {
        compileCallDirectEval(bytecode);
        return;
    }

    materializePointerIntoMetadata(bytecode, Op::Metadata::offsetOfCallLinkInfo(), BaselineJITRegisters::Call::callLinkInfoGPR);

#if USE(JSVALUE32_64)
    // We need this on JSVALUE32_64 only as on JSVALUE64 a pointer comparison in the DataIC fast
    // path catches this.
    addSlowCase(branchIfNotCell(BaselineJITRegisters::Call::calleeJSR));
#endif

    if constexpr (Op::opcodeID == op_tail_call)
        compileTailCall(bytecode, callLinkInfo, callLinkInfoIndex);
    else {
        if constexpr (Op::opcodeID == op_tail_call_varargs || Op::opcodeID == op_tail_call_forward_arguments) {
            auto [slowPaths, dispatchLabel] = CallLinkInfo::emitTailCallFastPath(*this, callLinkInfo, BaselineJITRegisters::Call::calleeJSR.payloadGPR(), BaselineJITRegisters::Call::callLinkInfoGPR, scopedLambda<void()>([&] {
                emitRestoreCalleeSaves();
                prepareForTailCallSlow(RegisterSet {
                    BaselineJITRegisters::Call::calleeJSR.payloadGPR(),
#if USE(JSVALUE32_64)
                    BaselineJITRegisters::Call::calleeJSR.tagGPR(),
#endif
                    BaselineJITRegisters::Call::callLinkInfoGPR,
                    BaselineJITRegisters::Call::globalObjectGPR,
                    BaselineJITRegisters::Call::callTargetGPR,
                });
            }));
            addSlowCase(slowPaths);
            auto doneLocation = label();
            m_callCompilationInfo[callLinkInfoIndex].doneLocation = doneLocation;
            m_callCompilationInfo[callLinkInfoIndex].dispatchLabel = dispatchLabel;
        } else {
            auto [slowPaths, dispatchLabel] = CallLinkInfo::emitFastPath(*this, callLinkInfo, BaselineJITRegisters::Call::calleeJSR.payloadGPR(), BaselineJITRegisters::Call::callLinkInfoGPR);
            addSlowCase(slowPaths);
            auto doneLocation = label();
            m_callCompilationInfo[callLinkInfoIndex].doneLocation = doneLocation;
            m_callCompilationInfo[callLinkInfoIndex].dispatchLabel = dispatchLabel;
            if constexpr (Op::opcodeID != op_iterator_open && Op::opcodeID != op_iterator_next)
                setFastPathResumePoint();
            resetSP();
            if constexpr (Op::opcodeID != op_call_ignore_result)
                emitPutCallResult(bytecode);
        }
    }
}

template<typename Op>
void JIT::compileOpCallSlowCase(const JSInstruction*, Vector<SlowCaseEntry>::iterator& iter, unsigned callLinkInfoIndex)
{
    constexpr OpcodeID opcodeID = Op::opcodeID;
    ASSERT(opcodeID != op_call_direct_eval);
    auto* callLinkInfo = m_callCompilationInfo[callLinkInfoIndex].unlinkedCallLinkInfo;

    linkAllSlowCases(iter);

    loadGlobalObject(BaselineJITRegisters::Call::globalObjectGPR);
    if constexpr (opcodeID == op_tail_call || opcodeID == op_tail_call_varargs || opcodeID == op_tail_call_forward_arguments) {
        auto dispatchLabel = m_callCompilationInfo[callLinkInfoIndex].dispatchLabel;
        CallLinkInfo::emitTailCallSlowPath(*m_vm, *this, callLinkInfo, BaselineJITRegisters::Call::callLinkInfoGPR, dispatchLabel);
    } else
        CallLinkInfo::emitSlowPath(*m_vm, *this, callLinkInfo, BaselineJITRegisters::Call::callLinkInfoGPR);
}

void JIT::emit_op_call(const JSInstruction* currentInstruction)
{
    compileOpCall<OpCall>(currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_call_ignore_result(const JSInstruction* currentInstruction)
{
    compileOpCall<OpCallIgnoreResult>(currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_tail_call(const JSInstruction* currentInstruction)
{
    compileOpCall<OpTailCall>(currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_call_direct_eval(const JSInstruction* currentInstruction)
{
    compileOpCall<OpCallDirectEval>(currentInstruction, m_callLinkInfoIndex);
}

void JIT::emit_op_call_varargs(const JSInstruction* currentInstruction)
{
    compileOpCall<OpCallVarargs>(currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_tail_call_varargs(const JSInstruction* currentInstruction)
{
    compileOpCall<OpTailCallVarargs>(currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_tail_call_forward_arguments(const JSInstruction* currentInstruction)
{
    compileOpCall<OpTailCallForwardArguments>(currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_construct_varargs(const JSInstruction* currentInstruction)
{
    compileOpCall<OpConstructVarargs>(currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_construct(const JSInstruction* currentInstruction)
{
    compileOpCall<OpConstruct>(currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_call(const JSInstruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase<OpCall>(currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_call_ignore_result(const JSInstruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase<OpCallIgnoreResult>(currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_tail_call(const JSInstruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase<OpTailCall>(currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_call_direct_eval(const JSInstruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileCallDirectEvalSlowCase(currentInstruction, iter);
}

void JIT::emitSlow_op_call_varargs(const JSInstruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase<OpCallVarargs>(currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_tail_call_varargs(const JSInstruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase<OpTailCallVarargs>(currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_tail_call_forward_arguments(const JSInstruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase<OpTailCallForwardArguments>(currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_construct_varargs(const JSInstruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase<OpConstructVarargs>(currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_construct(const JSInstruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase<OpConstruct>(currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emit_op_iterator_open(const JSInstruction* instruction)
{
    auto bytecode = instruction->as<OpIteratorOpen>();
    auto* tryFastFunction = ([&] () {
        switch (instruction->width()) {
        case Narrow: return iterator_open_try_fast_narrow;
        case Wide16: return iterator_open_try_fast_wide16;
        case Wide32: return iterator_open_try_fast_wide32;
        default: RELEASE_ASSERT_NOT_REACHED();
        }
    })();

    JITSlowPathCall slowPathCall(this, tryFastFunction);
    slowPathCall.call();
    Jump fastCase = branch32(NotEqual, GPRInfo::returnValueGPR2, TrustedImm32(static_cast<uint32_t>(IterationMode::Generic)));

    compileOpCall<OpIteratorOpen>(instruction, m_callLinkInfoIndex++);
    advanceToNextCheckpoint();

    // call result (iterator) is in returnValueJSR

    using BaselineJITRegisters::GetById::baseJSR;
    using BaselineJITRegisters::GetById::resultJSR;
    using BaselineJITRegisters::GetById::stubInfoGPR;

    moveValueRegs(returnValueJSR, baseJSR);
    auto [ stubInfo, stubInfoIndex ] = addUnlinkedStructureStubInfo();
    loadStructureStubInfo(stubInfoIndex, stubInfoGPR);

    emitJumpSlowCaseIfNotJSCell(baseJSR);

    static_assert(noOverlap(returnValueJSR, stubInfoGPR));

    const Identifier* ident = &vm().propertyNames->next;

    JITGetByIdGenerator gen(
        nullptr, stubInfo, JITType::BaselineJIT, CodeOrigin(m_bytecodeIndex), CallSiteIndex(BytecodeIndex(m_bytecodeIndex.offset())), RegisterSetBuilder::stubUnavailableRegisters(),
        CacheableIdentifier::createFromImmortalIdentifier(ident->impl()), baseJSR, resultJSR, stubInfoGPR, AccessType::GetById);

    gen.generateBaselineDataICFastPath(*this);
    resetSP(); // We might OSR exit here, so we need to conservatively reset SP
    addSlowCase();
    m_getByIds.append(gen);

    setFastPathResumePoint();
    emitValueProfilingSite(bytecode, resultJSR);
    emitPutVirtualRegister(bytecode.m_next, resultJSR);

    fastCase.link(this);
}

void JIT::emitSlow_op_iterator_open(const JSInstruction* instruction, Vector<SlowCaseEntry>::iterator& iter)
{
    auto bytecode = instruction->as<OpIteratorOpen>();

    linkAllSlowCases(iter);
    compileOpCallSlowCase<OpIteratorOpen>(instruction, iter, m_callLinkInfoIndex++);
    resetSP();
    emitPutCallResult(bytecode);
    emitJumpSlowToHotForCheckpoint(jump());

    linkAllSlowCases(iter);

    using BaselineJITRegisters::GetById::baseJSR;
    using BaselineJITRegisters::GetById::stubInfoGPR;
    using BaselineJITRegisters::GetById::globalObjectGPR;

    JumpList notObject;
    notObject.append(branchIfNotCell(baseJSR));
    notObject.append(branchIfNotObject(baseJSR.payloadGPR()));

    JITGetByIdGenerator& gen = m_getByIds[m_getByIdIndex++];
    gen.reportBaselineDataICSlowPathBegin(label());
    nearCallThunk(CodeLocationLabel { InlineCacheCompiler::generateSlowPathCode(vm(), gen.accessType()).retaggedCode<NoPtrTag>() });
    static_assert(BaselineJITRegisters::GetById::resultJSR == returnValueJSR);
    jump().linkTo(fastPathResumePoint(), this);

    notObject.link(this);
    loadGlobalObject(argumentGPR0);
    callOperation(operationThrowIteratorResultIsNotObject, argumentGPR0);
}

void JIT::emit_op_iterator_next(const JSInstruction* instruction)
{
    auto bytecode = instruction->as<OpIteratorNext>();
    auto* tryFastFunction = ([&] () {
        switch (instruction->width()) {
        case Narrow: return iterator_next_try_fast_narrow;
        case Wide16: return iterator_next_try_fast_wide16;
        case Wide32: return iterator_next_try_fast_wide32;
        default: RELEASE_ASSERT_NOT_REACHED();
        }
    })();

    using BaselineJITRegisters::GetById::baseJSR;
    using BaselineJITRegisters::GetById::resultJSR;
    using BaselineJITRegisters::GetById::stubInfoGPR;

    constexpr JSValueRegs nextJSR = baseJSR; // Used as temporary register
    emitGetVirtualRegister(bytecode.m_next, nextJSR);
    Jump genericCase = branchIfNotEmpty(nextJSR);

    JITSlowPathCall slowPathCall(this, tryFastFunction);
    slowPathCall.call();
    Jump fastCase = branch32(NotEqual, GPRInfo::returnValueGPR2, TrustedImm32(static_cast<uint32_t>(IterationMode::Generic)));

    genericCase.link(this);
    load8FromMetadata(bytecode, OpIteratorNext::Metadata::offsetOfIterationMetadata() + IterationModeMetadata::offsetOfSeenModes(), regT0);
    or32(TrustedImm32(static_cast<uint8_t>(IterationMode::Generic)), regT0);
    store8ToMetadata(regT0, bytecode, OpIteratorNext::Metadata::offsetOfIterationMetadata() + IterationModeMetadata::offsetOfSeenModes());
    compileOpCall<OpIteratorNext>(instruction, m_callLinkInfoIndex++);
    advanceToNextCheckpoint();

    // call result ({ done, value } JSObject) in regT0  (regT1/regT0 or 32-bit)
    static_assert(noOverlap(resultJSR, stubInfoGPR));

    moveValueRegs(returnValueJSR, baseJSR);

    addSlowCase(branchIfNotCell(baseJSR));
    addSlowCase(branchIfNotObject(baseJSR.payloadGPR()));
    {
        auto [ stubInfo, stubInfoIndex ] = addUnlinkedStructureStubInfo();
        loadStructureStubInfo(stubInfoIndex, stubInfoGPR);

        JITGetByIdGenerator gen(
            nullptr, stubInfo, JITType::BaselineJIT, CodeOrigin(m_bytecodeIndex), CallSiteIndex(BytecodeIndex(m_bytecodeIndex.offset())), RegisterSetBuilder::stubUnavailableRegisters(),
            CacheableIdentifier::createFromImmortalIdentifier(vm().propertyNames->done.impl()), baseJSR, resultJSR, stubInfoGPR, AccessType::GetById);

        gen.generateBaselineDataICFastPath(*this);
        resetSP(); // We might OSR exit here, so we need to conservatively reset SP
        addSlowCase();
        m_getByIds.append(gen);

        BytecodeIndex bytecodeIndex = m_bytecodeIndex;
        advanceToNextCheckpoint();
        emitValueProfilingSite(bytecode, bytecodeIndex, resultJSR);
        emitPutVirtualRegister(bytecode.m_done, resultJSR);
    }

    {
        auto usedRegisters = RegisterSetBuilder(resultJSR).buildAndValidate();
        ScratchRegisterAllocator scratchAllocator(usedRegisters);
        GPRReg scratch1 = scratchAllocator.allocateScratchGPR();
        GPRReg scratch2 = scratchAllocator.allocateScratchGPR();
        GPRReg globalGPR = scratchAllocator.allocateScratchGPR();
        const bool shouldCheckMasqueradesAsUndefined = false;
        loadGlobalObject(globalGPR);
        JumpList iterationDone = branchIfTruthy(vm(), resultJSR, scratch1, scratch2, fpRegT0, fpRegT1, shouldCheckMasqueradesAsUndefined, globalGPR);

        emitGetVirtualRegister(bytecode.m_value, baseJSR);
        auto [ stubInfo, stubInfoIndex ] = addUnlinkedStructureStubInfo();
        loadStructureStubInfo(stubInfoIndex, stubInfoGPR);

        JITGetByIdGenerator gen(
            nullptr, stubInfo, JITType::BaselineJIT, CodeOrigin(m_bytecodeIndex), CallSiteIndex(BytecodeIndex(m_bytecodeIndex.offset())), RegisterSetBuilder::stubUnavailableRegisters(),
            CacheableIdentifier::createFromImmortalIdentifier(vm().propertyNames->value.impl()), baseJSR, resultJSR, stubInfoGPR, AccessType::GetById);

        gen.generateBaselineDataICFastPath(*this);
        resetSP(); // We might OSR exit here, so we need to conservatively reset SP
        addSlowCase();
        m_getByIds.append(gen);

        setFastPathResumePoint();
        emitValueProfilingSite(bytecode, resultJSR);
        emitPutVirtualRegister(bytecode.m_value, resultJSR);

        iterationDone.link(this);
    }

    fastCase.link(this);
}

void JIT::emitSlow_op_iterator_next(const JSInstruction* instruction, Vector<SlowCaseEntry>::iterator& iter)
{
    auto bytecode = instruction->as<OpIteratorNext>();

    linkAllSlowCases(iter);
    compileOpCallSlowCase<OpIteratorNext>(instruction, iter, m_callLinkInfoIndex++);
    resetSP();
    emitPutCallResult(bytecode);
    emitJumpSlowToHotForCheckpoint(jump());

    using BaselineJITRegisters::GetById::baseJSR;
    using BaselineJITRegisters::GetById::resultJSR;
    using BaselineJITRegisters::GetById::globalObjectGPR;
    using BaselineJITRegisters::GetById::stubInfoGPR;

    linkAllSlowCases(iter);
    loadGlobalObject(argumentGPR0);
    callOperation(operationThrowIteratorResultIsNotObject, argumentGPR0);

    {
        JITGetByIdGenerator& gen = m_getByIds[m_getByIdIndex++];
        gen.reportBaselineDataICSlowPathBegin(label());
        nearCallThunk(CodeLocationLabel { InlineCacheCompiler::generateSlowPathCode(vm(), gen.accessType()).retaggedCode<NoPtrTag>() });
        static_assert(BaselineJITRegisters::GetById::resultJSR == returnValueJSR);
        emitJumpSlowToHotForCheckpoint(jump());
    }

    {
        linkAllSlowCases(iter);
        JITGetByIdGenerator& gen = m_getByIds[m_getByIdIndex++];
        gen.reportBaselineDataICSlowPathBegin(label());
        nearCallThunk(CodeLocationLabel { InlineCacheCompiler::generateSlowPathCode(vm(), gen.accessType()).retaggedCode<NoPtrTag>() });
        static_assert(BaselineJITRegisters::GetById::resultJSR == returnValueJSR);
    }
}

} // namespace JSC

#endif // ENABLE(JIT)
