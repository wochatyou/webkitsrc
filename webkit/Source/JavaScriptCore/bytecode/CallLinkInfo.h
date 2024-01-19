/*
 * Copyright (C) 2012-2018 Apple Inc. All rights reserved.
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

#pragma once

#include "BaselineJITRegisters.h"
#include "CallFrameShuffleData.h"
#include "CallLinkInfoBase.h"
#include "CallMode.h"
#include "CodeLocation.h"
#include "CodeOrigin.h"
#include "CodeSpecializationKind.h"
#include "PolymorphicCallStubRoutine.h"
#include "WriteBarrier.h"
#include <wtf/ScopedLambda.h>
#include <wtf/SentinelLinkedList.h>

namespace JSC {
namespace DFG {
struct UnlinkedCallLinkInfo;
}

class CCallHelpers;
class ExecutableBase;
class FunctionCodeBlock;
class JSFunction;
class OptimizingCallLinkInfo;
class PolymorphicCallStubRoutine;
enum OpcodeID : unsigned;

struct CallFrameShuffleData;
struct UnlinkedCallLinkInfo;
struct BaselineUnlinkedCallLinkInfo;


using CompileTimeCallLinkInfo = std::variant<OptimizingCallLinkInfo*, BaselineUnlinkedCallLinkInfo*, DFG::UnlinkedCallLinkInfo*>;

class CallLinkInfo : public CallLinkInfoBase {
public:
    friend class LLIntOffsetsExtractor;

    static constexpr uint8_t maxProfiledArgumentCountIncludingThisForVarargs = UINT8_MAX;

    enum class Type : uint8_t {
        Baseline,
        Optimizing,
    };

    enum CallType : uint8_t {
        None,
        Call,
        CallVarargs,
        Construct,
        ConstructVarargs,
        TailCall,
        TailCallVarargs,
        DirectCall,
        DirectConstruct,
        DirectTailCall
    };

    enum class Mode : uint8_t {
        Init,
        Monomorphic,
        Polymorphic,
        Virtual,
        LinkedDirect,
    };

    static constexpr uintptr_t polymorphicCalleeMask = 1;
    
    static CallType callTypeFor(OpcodeID opcodeID);

    static bool isVarargsCallType(CallType callType)
    {
        switch (callType) {
        case CallVarargs:
        case ConstructVarargs:
        case TailCallVarargs:
            return true;

        default:
            return false;
        }
    }

    ~CallLinkInfo();
    
    static CodeSpecializationKind specializationKindFor(CallType callType)
    {
        return specializationFromIsConstruct(callType == Construct || callType == ConstructVarargs || callType == DirectConstruct);
    }
    CodeSpecializationKind specializationKind() const
    {
        return specializationKindFor(static_cast<CallType>(m_callType));
    }
    
    static CallMode callModeFor(CallType callType)
    {
        switch (callType) {
        case Call:
        case CallVarargs:
        case DirectCall:
            return CallMode::Regular;
        case TailCall:
        case TailCallVarargs:
        case DirectTailCall:
            return CallMode::Tail;
        case Construct:
        case ConstructVarargs:
        case DirectConstruct:
            return CallMode::Construct;
        case None:
            RELEASE_ASSERT_NOT_REACHED();
        }

        RELEASE_ASSERT_NOT_REACHED();
    }
    
    static bool isDirect(CallType callType)
    {
        switch (callType) {
        case DirectCall:
        case DirectTailCall:
        case DirectConstruct:
            return true;
        case Call:
        case CallVarargs:
        case TailCall:
        case TailCallVarargs:
        case Construct:
        case ConstructVarargs:
            return false;
        case None:
            RELEASE_ASSERT_NOT_REACHED();
            return false;
        }

        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }
    
    CallMode callMode() const
    {
        return callModeFor(static_cast<CallType>(m_callType));
    }

    bool isDirect() const
    {
        return isDirect(static_cast<CallType>(m_callType));
    }

    bool isTailCall() const
    {
        return callMode() == CallMode::Tail;
    }
    
    NearCallMode nearCallMode() const
    {
        return isTailCall() ? NearCallMode::Tail : NearCallMode::Regular;
    }

    bool isVarargs() const
    {
        return isVarargsCallType(static_cast<CallType>(m_callType));
    }

    bool isLinked() const { return mode() != Mode::Init && mode() != Mode::Virtual; }
    void unlinkOrUpgradeImpl(VM&, CodeBlock* oldCodeBlock, CodeBlock* newCodeBlock);

    enum class UseDataIC : bool { No, Yes };

#if ENABLE(JIT)
protected:
    static std::tuple<MacroAssembler::JumpList, MacroAssembler::Label> emitFastPathImpl(CallLinkInfo*, CCallHelpers&, GPRReg calleeGPR, GPRReg callLinkInfoGPR, UseDataIC, bool isTailCall, ScopedLambda<void()>&& prepareForTailCall) WARN_UNUSED_RETURN;
    static std::tuple<MacroAssembler::JumpList, MacroAssembler::Label> emitDataICFastPath(CCallHelpers&, GPRReg calleeGPR, GPRReg callLinkInfoGPR) WARN_UNUSED_RETURN;
    static std::tuple<MacroAssembler::JumpList, MacroAssembler::Label> emitTailCallDataICFastPath(CCallHelpers&, GPRReg calleeGPR, GPRReg callLinkInfoGPR, ScopedLambda<void()>&& prepareForTailCall) WARN_UNUSED_RETURN;

    static void emitSlowPathImpl(VM&, CCallHelpers&, GPRReg callLinkInfoGPR, UseDataIC, bool isTailCall, MacroAssembler::Label);
    static void emitDataICSlowPath(VM&, CCallHelpers&, GPRReg callLinkInfoGPR, bool isTailCall, MacroAssembler::Label);

public:
    static std::tuple<MacroAssembler::JumpList, MacroAssembler::Label> emitFastPath(CCallHelpers&, CompileTimeCallLinkInfo, GPRReg calleeGPR, GPRReg callLinkInfoGPR) WARN_UNUSED_RETURN;
    static std::tuple<MacroAssembler::JumpList, MacroAssembler::Label> emitTailCallFastPath(CCallHelpers&, CompileTimeCallLinkInfo, GPRReg calleeGPR, GPRReg callLinkInfoGPR, ScopedLambda<void()>&& prepareForTailCall) WARN_UNUSED_RETURN;
    static void emitSlowPath(VM&, CCallHelpers&, CompileTimeCallLinkInfo, GPRReg callLinkInfoGPR);
    static void emitTailCallSlowPath(VM&, CCallHelpers&, CompileTimeCallLinkInfo, GPRReg callLinkInfoGPR, MacroAssembler::Label);
#endif

    void revertCallToStub();

    bool isDataIC() const { return useDataIC() == UseDataIC::Yes; }
    UseDataIC useDataIC() const { return static_cast<UseDataIC>(m_useDataIC); }

    bool allowStubs() const { return m_allowStubs; }

    void disallowStubs()
    {
        m_allowStubs = false;
    }

    CodeLocationLabel<JSInternalPtrTag> doneLocation();

    void setMonomorphicCallee(VM&, JSCell*, JSObject* callee, CodeBlock*, CodePtr<JSEntryPtrTag>);
    void clearCallee();
    JSObject* callee();

    void setCodeBlock(VM&, JSCell*, FunctionCodeBlock*);
    void clearCodeBlock();
    FunctionCodeBlock* codeBlock();

    void setLastSeenCallee(VM&, const JSCell* owner, JSObject* callee);
    void clearLastSeenCallee();
    JSObject* lastSeenCallee() const;
    bool haveLastSeenCallee() const;
    
    void setExecutableDuringCompilation(ExecutableBase*);
    ExecutableBase* executable();
    
#if ENABLE(JIT)
    void setStub(JSCell* owner, Ref<PolymorphicCallStubRoutine>&&);
#endif
    void clearStub();

    void setVirtualCall(VM&, JSCell* owner);

    void revertCall(VM&);

    PolymorphicCallStubRoutine* stub() const
    {
#if ENABLE(JIT)
        return m_stub.get();
#else
        return nullptr;
#endif
    }

    bool seenOnce()
    {
        return m_hasSeenShouldRepatch;
    }

    void clearSeen()
    {
        m_hasSeenShouldRepatch = false;
    }

    void setSeen()
    {
        m_hasSeenShouldRepatch = true;
    }

    bool hasSeenClosure()
    {
        return m_hasSeenClosure;
    }

    void setHasSeenClosure()
    {
        m_hasSeenClosure = true;
    }

    bool clearedByGC()
    {
        return m_clearedByGC;
    }
    
    bool clearedByVirtual()
    {
        return m_clearedByVirtual;
    }

    void setClearedByVirtual()
    {
        m_clearedByVirtual = true;
    }
    
    void setCallType(CallType callType)
    {
        m_callType = callType;
    }

    CallType callType()
    {
        return static_cast<CallType>(m_callType);
    }

    static ptrdiff_t offsetOfMaxArgumentCountIncludingThisForVarargs()
    {
        return OBJECT_OFFSETOF(CallLinkInfo, m_maxArgumentCountIncludingThisForVarargs);
    }

    uint32_t maxArgumentCountIncludingThisForVarargs()
    {
        return m_maxArgumentCountIncludingThisForVarargs;
    }
    
    void updateMaxArgumentCountIncludingThisForVarargs(unsigned argumentCountIncludingThisForVarargs)
    {
        if (m_maxArgumentCountIncludingThisForVarargs < argumentCountIncludingThisForVarargs)
            m_maxArgumentCountIncludingThisForVarargs = std::min<unsigned>(argumentCountIncludingThisForVarargs, maxProfiledArgumentCountIncludingThisForVarargs);
    }

    static ptrdiff_t offsetOfSlowPathCount()
    {
        return OBJECT_OFFSETOF(CallLinkInfo, m_slowPathCount);
    }

    static ptrdiff_t offsetOfCallee()
    {
        return OBJECT_OFFSETOF(CallLinkInfo, m_calleeOrCodeBlock);
    }

    static ptrdiff_t offsetOfCodeBlock()
    {
        return OBJECT_OFFSETOF(CallLinkInfo, u) + OBJECT_OFFSETOF(UnionType, dataIC.m_codeBlock);
    }

    static ptrdiff_t offsetOfMonomorphicCallDestination()
    {
        return OBJECT_OFFSETOF(CallLinkInfo, u) + OBJECT_OFFSETOF(UnionType, dataIC.m_monomorphicCallDestination);
    }

#if ENABLE(JIT)
    GPRReg callLinkInfoGPR() const;

    static ptrdiff_t offsetOfStub()
    {
        return OBJECT_OFFSETOF(CallLinkInfo, m_stub);
    }
#endif

    uint32_t slowPathCount()
    {
        return m_slowPathCount;
    }

    CodeOrigin codeOrigin() const;

    template<typename Functor>
    void forEachDependentCell(const Functor& functor) const
    {
        if (isLinked()) {
            if (stub()) {
#if ENABLE(JIT)
                stub()->forEachDependentCell(functor);
#else
                RELEASE_ASSERT_NOT_REACHED();
#endif
            } else {
                functor(m_calleeOrCodeBlock.get());
                if (isDirect())
                    functor(m_lastSeenCalleeOrExecutable.get());
            }
        }
        if (!isDirect() && haveLastSeenCallee())
            functor(lastSeenCallee());
    }

    void visitWeak(VM&);

    Type type() const { return static_cast<Type>(m_type); }

    Mode mode() const { return static_cast<Mode>(m_mode); }

    JSCell* owner() const { return m_owner; }

    std::tuple<CodeBlock*, BytecodeIndex> retrieveCaller(JSCell* owner);

protected:
    CallLinkInfo(Type type, UseDataIC useDataIC, JSCell* owner)
        : CallLinkInfoBase(CallSiteType::CallLinkInfo)
        , m_useDataIC(static_cast<unsigned>(useDataIC))
        , m_type(static_cast<unsigned>(type))
        , m_owner(owner)
    {
        ASSERT(type == this->type());
        ASSERT(useDataIC == this->useDataIC());
    }

    void reset(VM&);

    bool m_hasSeenShouldRepatch : 1 { false };
    bool m_hasSeenClosure : 1 { false };
    bool m_clearedByGC : 1 { false };
    bool m_clearedByVirtual : 1 { false };
    bool m_allowStubs : 1 { true };
    unsigned m_callType : 4 { CallType::None }; // CallType
    unsigned m_useDataIC : 1; // UseDataIC
    unsigned m_type : 1; // Type
    unsigned m_mode : 3 { static_cast<unsigned>(Mode::Init) }; // Mode
    uint8_t m_maxArgumentCountIncludingThisForVarargs { 0 }; // For varargs: the profiled maximum number of arguments. For direct: the number of stack slots allocated for arguments.
    uint32_t m_slowPathCount { 0 };

    CodeLocationLabel<JSInternalPtrTag> m_doneLocation;
    union UnionType {
        UnionType()
            : dataIC { nullptr, nullptr }
        { }

        struct DataIC {
            CodeBlock* m_codeBlock; // This is weakly held. And cleared whenever m_monomorphicCallDestination is changed.
            CodePtr<JSEntryPtrTag> m_monomorphicCallDestination;
        } dataIC;

        struct {
            CodeLocationDataLabelPtr<JSInternalPtrTag> m_codeBlockLocation;
            CodeLocationDataLabelPtr<JSInternalPtrTag> m_calleeLocation;
        } codeIC;
    } u;

    WriteBarrier<JSCell> m_calleeOrCodeBlock;
    WriteBarrier<JSCell> m_lastSeenCalleeOrExecutable;
#if ENABLE(JIT)
    RefPtr<PolymorphicCallStubRoutine> m_stub;
#endif
    JSCell* m_owner { nullptr };
};

class BaselineCallLinkInfo final : public CallLinkInfo {
public:
    BaselineCallLinkInfo()
        : CallLinkInfo(Type::Baseline, UseDataIC::Yes, nullptr)
    {
    }

    void initialize(VM&, CodeBlock*, CallType, BytecodeIndex);

    void setCodeLocations(CodeLocationLabel<JSInternalPtrTag> doneLocation)
    {
        m_doneLocation = doneLocation;
    }

    CodeOrigin codeOrigin() const { return CodeOrigin { m_bytecodeIndex }; }

#if ENABLE(JIT)
    static constexpr GPRReg callLinkInfoGPR() { return BaselineJITRegisters::Call::callLinkInfoGPR; }
#endif

private:
    BytecodeIndex m_bytecodeIndex { };
};

inline CodeOrigin getCallLinkInfoCodeOrigin(CallLinkInfo& callLinkInfo)
{
    return callLinkInfo.codeOrigin();
}

struct UnlinkedCallLinkInfo {
    CodeLocationLabel<JSInternalPtrTag> doneLocation;

    void setCodeLocations(CodeLocationLabel<JSInternalPtrTag>, CodeLocationLabel<JSInternalPtrTag> doneLocation)
    {
        this->doneLocation = doneLocation;
    }
};

struct BaselineUnlinkedCallLinkInfo : public JSC::UnlinkedCallLinkInfo {
    BytecodeIndex bytecodeIndex; // Currently, only used by baseline, so this can trivially produce a CodeOrigin.

#if ENABLE(JIT)
    void setUpCall(CallLinkInfo::CallType) { }
#endif
};

#if ENABLE(JIT)

class OptimizingCallLinkInfo final : public CallLinkInfo {
public:
    friend class CallLinkInfo;

    OptimizingCallLinkInfo()
        : CallLinkInfo(Type::Optimizing, UseDataIC::Yes, nullptr)
    {
    }

    OptimizingCallLinkInfo(CodeOrigin codeOrigin, UseDataIC useDataIC, JSCell* owner)
        : CallLinkInfo(Type::Optimizing, useDataIC, owner)
        , m_codeOrigin(codeOrigin)
    {
    }

    void setUpCall(CallType callType)
    {
        m_callType = callType;
    }

    void setCodeLocations(
        CodeLocationLabel<JSInternalPtrTag> slowPathStart,
        CodeLocationLabel<JSInternalPtrTag> doneLocation)
    {
        if (!isDataIC())
            m_slowPathStart = slowPathStart;
        m_doneLocation = doneLocation;
    }

    CodeLocationLabel<JSInternalPtrTag> fastPathStart();
    CodeLocationLabel<JSInternalPtrTag> slowPathStart();

    GPRReg callLinkInfoGPR() const { return m_callLinkInfoGPR; }
    void setCallLinkInfoGPR(GPRReg callLinkInfoGPR) { m_callLinkInfoGPR = callLinkInfoGPR; }

    void emitDirectFastPath(CCallHelpers&);
    void emitDirectTailCallFastPath(CCallHelpers&, ScopedLambda<void()>&& prepareForTailCall);
    void initializeDirectCall();
    void setDirectCallTarget(CodeBlock*, CodeLocationLabel<JSEntryPtrTag>);
    void setDirectCallMaxArgumentCountIncludingThis(unsigned);
    unsigned maxArgumentCountIncludingThisForDirectCall() const { return m_maxArgumentCountIncludingThisForDirectCall; }

    void setSlowPathCallDestination(CodePtr<JSEntryPtrTag>);

    CodeOrigin codeOrigin() const { return m_codeOrigin; }

    void initializeFromDFGUnlinkedCallLinkInfo(VM&, const DFG::UnlinkedCallLinkInfo&, CodeBlock*);

    static ptrdiff_t offsetOfSlowPathCallDestination()
    {
        return OBJECT_OFFSETOF(OptimizingCallLinkInfo, m_slowPathCallDestination);
    }

private:
    void initializeDirectCallRepatch(CCallHelpers&);
    std::tuple<MacroAssembler::JumpList, MacroAssembler::Label> emitFastPath(CCallHelpers&, GPRReg calleeGPR, GPRReg callLinkInfoGPR) WARN_UNUSED_RETURN;
    std::tuple<MacroAssembler::JumpList, MacroAssembler::Label> emitTailCallFastPath(CCallHelpers&, GPRReg calleeGPR, GPRReg callLinkInfoGPR, ScopedLambda<void()>&& prepareForTailCall) WARN_UNUSED_RETURN;
    void emitSlowPath(VM&, CCallHelpers&, GPRReg callLinkInfoGPR);
    void emitTailCallSlowPath(VM&, CCallHelpers&, GPRReg callLinkInfoGPR, MacroAssembler::Label);

    CodeOrigin m_codeOrigin;
    CodePtr<JSEntryPtrTag> m_slowPathCallDestination;
    CodeLocationNearCall<JSInternalPtrTag> m_callLocation NO_UNIQUE_ADDRESS;
    GPRReg m_callLinkInfoGPR { InvalidGPRReg };
    unsigned m_maxArgumentCountIncludingThisForDirectCall { 0 };
    CodeLocationLabel<JSInternalPtrTag> m_slowPathStart;
    CodeLocationLabel<JSInternalPtrTag> m_fastPathStart;
};

inline GPRReg CallLinkInfo::callLinkInfoGPR() const
{
    switch (type()) {
    case Type::Baseline:
        return static_cast<const BaselineCallLinkInfo*>(this)->callLinkInfoGPR();
    case Type::Optimizing:
        return static_cast<const OptimizingCallLinkInfo*>(this)->callLinkInfoGPR();
    }
    return InvalidGPRReg;
}

#endif

inline CodeOrigin CallLinkInfo::codeOrigin() const
{
    switch (type()) {
    case Type::Baseline:
        return static_cast<const BaselineCallLinkInfo*>(this)->codeOrigin();
    case Type::Optimizing:
#if ENABLE(JIT)
        return static_cast<const OptimizingCallLinkInfo*>(this)->codeOrigin();
#else
        return { };
#endif
    }
    return { };
}

} // namespace JSC
