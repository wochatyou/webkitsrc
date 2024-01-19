/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "JSTestNamedDeleterWithIdentifier.h"

#include "ActiveDOMObject.h"
#include "Document.h"
#include "ExtendedDOMClientIsoSubspaces.h"
#include "ExtendedDOMIsoSubspaces.h"
#include "IDLTypes.h"
#include "JSDOMAbstractOperations.h"
#include "JSDOMBinding.h"
#include "JSDOMConstructorNotConstructable.h"
#include "JSDOMConvertBase.h"
#include "JSDOMConvertStrings.h"
#include "JSDOMExceptionHandling.h"
#include "JSDOMGlobalObjectInlines.h"
#include "JSDOMOperation.h"
#include "JSDOMWrapperCache.h"
#include "Quirks.h"
#include "ScriptExecutionContext.h"
#include "WebCoreJSClientData.h"
#include <JavaScriptCore/FunctionPrototype.h>
#include <JavaScriptCore/HeapAnalyzer.h>
#include <JavaScriptCore/JSCInlines.h>
#include <JavaScriptCore/JSDestructibleObjectHeapCellType.h>
#include <JavaScriptCore/SlotVisitorMacros.h>
#include <JavaScriptCore/SubspaceInlines.h>
#include <wtf/GetPtr.h>
#include <wtf/PointerPreparations.h>
#include <wtf/URL.h>


namespace WebCore {
using namespace JSC;

// Functions

static JSC_DECLARE_HOST_FUNCTION(jsTestNamedDeleterWithIdentifierPrototypeFunction_namedDeleter);

// Attributes

static JSC_DECLARE_CUSTOM_GETTER(jsTestNamedDeleterWithIdentifierConstructor);

class JSTestNamedDeleterWithIdentifierPrototype final : public JSC::JSNonFinalObject {
public:
    using Base = JSC::JSNonFinalObject;
    static JSTestNamedDeleterWithIdentifierPrototype* create(JSC::VM& vm, JSDOMGlobalObject* globalObject, JSC::Structure* structure)
    {
        JSTestNamedDeleterWithIdentifierPrototype* ptr = new (NotNull, JSC::allocateCell<JSTestNamedDeleterWithIdentifierPrototype>(vm)) JSTestNamedDeleterWithIdentifierPrototype(vm, globalObject, structure);
        ptr->finishCreation(vm);
        return ptr;
    }

    DECLARE_INFO;
    template<typename CellType, JSC::SubspaceAccess>
    static JSC::GCClient::IsoSubspace* subspaceFor(JSC::VM& vm)
    {
        STATIC_ASSERT_ISO_SUBSPACE_SHARABLE(JSTestNamedDeleterWithIdentifierPrototype, Base);
        return &vm.plainObjectSpace();
    }
    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info());
    }

private:
    JSTestNamedDeleterWithIdentifierPrototype(JSC::VM& vm, JSC::JSGlobalObject*, JSC::Structure* structure)
        : JSC::JSNonFinalObject(vm, structure)
    {
    }

    void finishCreation(JSC::VM&);
};
STATIC_ASSERT_ISO_SUBSPACE_SHARABLE(JSTestNamedDeleterWithIdentifierPrototype, JSTestNamedDeleterWithIdentifierPrototype::Base);

using JSTestNamedDeleterWithIdentifierDOMConstructor = JSDOMConstructorNotConstructable<JSTestNamedDeleterWithIdentifier>;

template<> const ClassInfo JSTestNamedDeleterWithIdentifierDOMConstructor::s_info = { "TestNamedDeleterWithIdentifier"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestNamedDeleterWithIdentifierDOMConstructor) };

template<> JSValue JSTestNamedDeleterWithIdentifierDOMConstructor::prototypeForStructure(JSC::VM& vm, const JSDOMGlobalObject& globalObject)
{
    UNUSED_PARAM(vm);
    return globalObject.functionPrototype();
}

template<> void JSTestNamedDeleterWithIdentifierDOMConstructor::initializeProperties(VM& vm, JSDOMGlobalObject& globalObject)
{
    putDirect(vm, vm.propertyNames->length, jsNumber(0), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    JSString* nameString = jsNontrivialString(vm, "TestNamedDeleterWithIdentifier"_s);
    m_originalName.set(vm, this, nameString);
    putDirect(vm, vm.propertyNames->name, nameString, JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    putDirect(vm, vm.propertyNames->prototype, JSTestNamedDeleterWithIdentifier::prototype(vm, globalObject), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum | JSC::PropertyAttribute::DontDelete);
}

/* Hash table for prototype */

static const HashTableValue JSTestNamedDeleterWithIdentifierPrototypeTableValues[] =
{
    { "constructor"_s, static_cast<unsigned>(PropertyAttribute::DontEnum), NoIntrinsic, { HashTableValue::GetterSetterType, jsTestNamedDeleterWithIdentifierConstructor, 0 } },
    { "namedDeleter"_s, static_cast<unsigned>(JSC::PropertyAttribute::Function), NoIntrinsic, { HashTableValue::NativeFunctionType, jsTestNamedDeleterWithIdentifierPrototypeFunction_namedDeleter, 1 } },
};

const ClassInfo JSTestNamedDeleterWithIdentifierPrototype::s_info = { "TestNamedDeleterWithIdentifier"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestNamedDeleterWithIdentifierPrototype) };

void JSTestNamedDeleterWithIdentifierPrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    reifyStaticProperties(vm, JSTestNamedDeleterWithIdentifier::info(), JSTestNamedDeleterWithIdentifierPrototypeTableValues, *this);
    JSC_TO_STRING_TAG_WITHOUT_TRANSITION();
}

const ClassInfo JSTestNamedDeleterWithIdentifier::s_info = { "TestNamedDeleterWithIdentifier"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestNamedDeleterWithIdentifier) };

JSTestNamedDeleterWithIdentifier::JSTestNamedDeleterWithIdentifier(Structure* structure, JSDOMGlobalObject& globalObject, Ref<TestNamedDeleterWithIdentifier>&& impl)
    : JSDOMWrapper<TestNamedDeleterWithIdentifier>(structure, globalObject, WTFMove(impl))
{
}

static_assert(!std::is_base_of<ActiveDOMObject, TestNamedDeleterWithIdentifier>::value, "Interface is not marked as [ActiveDOMObject] even though implementation class subclasses ActiveDOMObject.");

JSObject* JSTestNamedDeleterWithIdentifier::createPrototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    auto* structure = JSTestNamedDeleterWithIdentifierPrototype::createStructure(vm, &globalObject, globalObject.objectPrototype());
    structure->setMayBePrototype(true);
    return JSTestNamedDeleterWithIdentifierPrototype::create(vm, &globalObject, structure);
}

JSObject* JSTestNamedDeleterWithIdentifier::prototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    return getDOMPrototype<JSTestNamedDeleterWithIdentifier>(vm, globalObject);
}

JSValue JSTestNamedDeleterWithIdentifier::getConstructor(VM& vm, const JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSTestNamedDeleterWithIdentifierDOMConstructor, DOMConstructorID::TestNamedDeleterWithIdentifier>(vm, *jsCast<const JSDOMGlobalObject*>(globalObject));
}

void JSTestNamedDeleterWithIdentifier::destroy(JSC::JSCell* cell)
{
    JSTestNamedDeleterWithIdentifier* thisObject = static_cast<JSTestNamedDeleterWithIdentifier*>(cell);
    thisObject->JSTestNamedDeleterWithIdentifier::~JSTestNamedDeleterWithIdentifier();
}

bool JSTestNamedDeleterWithIdentifier::legacyPlatformObjectGetOwnProperty(JSObject* object, JSGlobalObject* lexicalGlobalObject, PropertyName propertyName, PropertySlot& slot, bool ignoreNamedProperties)
{
    auto throwScope = DECLARE_THROW_SCOPE(JSC::getVM(lexicalGlobalObject));
    auto* thisObject = jsCast<JSTestNamedDeleterWithIdentifier*>(object);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    if (!ignoreNamedProperties) {
        using GetterIDLType = IDLDOMString;
        auto getterFunctor = visibleNamedPropertyItemAccessorFunctor<GetterIDLType, JSTestNamedDeleterWithIdentifier>([] (JSTestNamedDeleterWithIdentifier& thisObject, PropertyName propertyName) -> decltype(auto) {
            return thisObject.wrapped().namedItem(propertyNameToAtomString(propertyName));
        });
        if (auto namedProperty = accessVisibleNamedProperty<LegacyOverrideBuiltIns::No>(*lexicalGlobalObject, *thisObject, propertyName, getterFunctor)) {
            auto value = toJS<IDLDOMString>(*lexicalGlobalObject, throwScope, WTFMove(namedProperty.value()));
            RETURN_IF_EXCEPTION(throwScope, false);
            slot.setValue(thisObject, static_cast<unsigned>(JSC::PropertyAttribute::ReadOnly), value);
            return true;
        }
    }
    return JSObject::getOwnPropertySlot(object, lexicalGlobalObject, propertyName, slot);
}

bool JSTestNamedDeleterWithIdentifier::getOwnPropertySlot(JSObject* object, JSGlobalObject* lexicalGlobalObject, PropertyName propertyName, PropertySlot& slot)
{
    bool ignoreNamedProperties = false;
    return legacyPlatformObjectGetOwnProperty(object, lexicalGlobalObject, propertyName, slot, ignoreNamedProperties);
}

bool JSTestNamedDeleterWithIdentifier::getOwnPropertySlotByIndex(JSObject* object, JSGlobalObject* lexicalGlobalObject, unsigned index, PropertySlot& slot)
{
    VM& vm = JSC::getVM(lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto* thisObject = jsCast<JSTestNamedDeleterWithIdentifier*>(object);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    auto propertyName = Identifier::from(vm, index);
    using GetterIDLType = IDLDOMString;
    auto getterFunctor = visibleNamedPropertyItemAccessorFunctor<GetterIDLType, JSTestNamedDeleterWithIdentifier>([] (JSTestNamedDeleterWithIdentifier& thisObject, PropertyName propertyName) -> decltype(auto) {
        return thisObject.wrapped().namedItem(propertyNameToAtomString(propertyName));
    });
    if (auto namedProperty = accessVisibleNamedProperty<LegacyOverrideBuiltIns::No>(*lexicalGlobalObject, *thisObject, propertyName, getterFunctor)) {
        auto value = toJS<IDLDOMString>(*lexicalGlobalObject, throwScope, WTFMove(namedProperty.value()));
        RETURN_IF_EXCEPTION(throwScope, false);
        slot.setValue(thisObject, static_cast<unsigned>(JSC::PropertyAttribute::ReadOnly), value);
        return true;
    }
    return JSObject::getOwnPropertySlotByIndex(object, lexicalGlobalObject, index, slot);
}

void JSTestNamedDeleterWithIdentifier::getOwnPropertyNames(JSObject* object, JSGlobalObject* lexicalGlobalObject, PropertyNameArray& propertyNames, DontEnumPropertiesMode mode)
{
    VM& vm = JSC::getVM(lexicalGlobalObject);
    auto* thisObject = jsCast<JSTestNamedDeleterWithIdentifier*>(object);
    ASSERT_GC_OBJECT_INHERITS(object, info());
    for (auto& propertyName : thisObject->wrapped().supportedPropertyNames())
        propertyNames.add(Identifier::fromString(vm, propertyName));
    JSObject::getOwnPropertyNames(object, lexicalGlobalObject, propertyNames, mode);
}

bool JSTestNamedDeleterWithIdentifier::put(JSCell* cell, JSGlobalObject* lexicalGlobalObject, PropertyName propertyName, JSValue value, PutPropertySlot& putPropertySlot)
{
    auto* thisObject = jsCast<JSTestNamedDeleterWithIdentifier*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    if (UNLIKELY(thisObject != putPropertySlot.thisValue()))
        return JSObject::put(thisObject, lexicalGlobalObject, propertyName, value, putPropertySlot);

    // Temporary quirk for ungap/@custom-elements polyfill (rdar://problem/111008826), consider removing in 2025.
    if (auto* document = dynamicDowncast<Document>(jsDynamicCast<JSDOMGlobalObject*>(lexicalGlobalObject)->scriptExecutionContext())) {
        if (UNLIKELY(document->quirks().needsConfigurableIndexedPropertiesQuirk()))
            return JSObject::put(thisObject, lexicalGlobalObject, propertyName, value, putPropertySlot);
    }

    auto throwScope = DECLARE_THROW_SCOPE(lexicalGlobalObject->vm());

    throwScope.assertNoException();
    PropertyDescriptor ownDescriptor;
    PropertySlot slot(thisObject, PropertySlot::InternalMethodType::GetOwnProperty);;
    bool ignoreNamedProperties = true;
    bool hasOwnProperty = legacyPlatformObjectGetOwnProperty(thisObject, lexicalGlobalObject, propertyName, slot, ignoreNamedProperties);
    RETURN_IF_EXCEPTION(throwScope, false);
    if (hasOwnProperty) {
        ownDescriptor.setPropertySlot(lexicalGlobalObject, propertyName, slot);
        RETURN_IF_EXCEPTION(throwScope, false);
    }
    RELEASE_AND_RETURN(throwScope, ordinarySetWithOwnDescriptor(lexicalGlobalObject, thisObject, propertyName, value, putPropertySlot.thisValue(), WTFMove(ownDescriptor), putPropertySlot.isStrictMode()));
}

bool JSTestNamedDeleterWithIdentifier::putByIndex(JSCell* cell, JSGlobalObject* lexicalGlobalObject, unsigned index, JSValue value, bool shouldThrow)
{

    // Temporary quirk for ungap/@custom-elements polyfill (rdar://problem/111008826), consider removing in 2025.
    if (auto* document = dynamicDowncast<Document>(jsDynamicCast<JSDOMGlobalObject*>(lexicalGlobalObject)->scriptExecutionContext())) {
        if (UNLIKELY(document->quirks().needsConfigurableIndexedPropertiesQuirk()))
            return JSObject::putByIndex(cell, lexicalGlobalObject, index, value, shouldThrow);
    }

    auto* thisObject = jsCast<JSTestNamedDeleterWithIdentifier*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    VM& vm = JSC::getVM(lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    throwScope.assertNoException();
    auto propertyName = Identifier::from(vm, index);
    PutPropertySlot putPropertySlot(thisObject, shouldThrow);
    RELEASE_AND_RETURN(throwScope, ordinarySetSlow(lexicalGlobalObject, thisObject, propertyName, value, putPropertySlot.thisValue(), shouldThrow));
}

bool JSTestNamedDeleterWithIdentifier::defineOwnProperty(JSObject* object, JSGlobalObject* lexicalGlobalObject, PropertyName propertyName, const PropertyDescriptor& propertyDescriptor, bool shouldThrow)
{
    auto* thisObject = jsCast<JSTestNamedDeleterWithIdentifier*>(object);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    auto throwScope = DECLARE_THROW_SCOPE(lexicalGlobalObject->vm());

    if (!propertyName.isSymbol()) {
        PropertySlot slot { thisObject, PropertySlot::InternalMethodType::VMInquiry, &lexicalGlobalObject->vm() };
        bool found = JSObject::getOwnPropertySlot(thisObject, lexicalGlobalObject, propertyName, slot);
        slot.disallowVMEntry.reset();
        RETURN_IF_EXCEPTION(throwScope, false);
        if (!found) {
            if (thisObject->wrapped().isSupportedPropertyName(propertyNameToString(propertyName)))
                return typeError(lexicalGlobalObject, throwScope, shouldThrow, "Cannot set named properties on this object"_s);
        }
    }

    PropertyDescriptor newPropertyDescriptor = propertyDescriptor;
    throwScope.release();
    return JSObject::defineOwnProperty(object, lexicalGlobalObject, propertyName, newPropertyDescriptor, shouldThrow);
}

bool JSTestNamedDeleterWithIdentifier::deleteProperty(JSCell* cell, JSGlobalObject* lexicalGlobalObject, PropertyName propertyName, DeletePropertySlot& slot)
{
    auto& thisObject = *jsCast<JSTestNamedDeleterWithIdentifier*>(cell);
    auto& impl = thisObject.wrapped();
    if (!propertyName.isSymbol() && impl.isSupportedPropertyName(propertyNameToString(propertyName))) {
        if (isVisibleNamedProperty<LegacyOverrideBuiltIns::No>(*lexicalGlobalObject, thisObject, propertyName)) {
            return performLegacyPlatformObjectDeleteOperation(*lexicalGlobalObject, [&] { return impl.namedDeleter(propertyNameToString(propertyName)); });
        }
    }
    return JSObject::deleteProperty(cell, lexicalGlobalObject, propertyName, slot);
}

bool JSTestNamedDeleterWithIdentifier::deletePropertyByIndex(JSCell* cell, JSGlobalObject* lexicalGlobalObject, unsigned index)
{
    UNUSED_PARAM(lexicalGlobalObject);
    auto& thisObject = *jsCast<JSTestNamedDeleterWithIdentifier*>(cell);
    auto& impl = thisObject.wrapped();
    VM& vm = JSC::getVM(lexicalGlobalObject);
    auto propertyName = Identifier::from(vm, index);
    if (impl.isSupportedPropertyName(propertyNameToString(propertyName))) {
        if (isVisibleNamedProperty<LegacyOverrideBuiltIns::No>(*lexicalGlobalObject, thisObject, propertyName)) {
            return performLegacyPlatformObjectDeleteOperation(*lexicalGlobalObject, [&] { return impl.namedDeleter(propertyNameToString(propertyName)); });
        }
    }
    return JSObject::deletePropertyByIndex(cell, lexicalGlobalObject, index);
}

JSC_DEFINE_CUSTOM_GETTER(jsTestNamedDeleterWithIdentifierConstructor, (JSGlobalObject* lexicalGlobalObject, EncodedJSValue thisValue, PropertyName))
{
    VM& vm = JSC::getVM(lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto* prototype = jsDynamicCast<JSTestNamedDeleterWithIdentifierPrototype*>(JSValue::decode(thisValue));
    if (UNLIKELY(!prototype))
        return throwVMTypeError(lexicalGlobalObject, throwScope);
    return JSValue::encode(JSTestNamedDeleterWithIdentifier::getConstructor(JSC::getVM(lexicalGlobalObject), prototype->globalObject()));
}

static inline JSC::EncodedJSValue jsTestNamedDeleterWithIdentifierPrototypeFunction_namedDeleterBody(JSC::JSGlobalObject* lexicalGlobalObject, JSC::CallFrame* callFrame, typename IDLOperation<JSTestNamedDeleterWithIdentifier>::ClassParameter castedThis)
{
    auto& vm = JSC::getVM(lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    UNUSED_PARAM(throwScope);
    UNUSED_PARAM(callFrame);
    auto& impl = castedThis->wrapped();
    if (UNLIKELY(callFrame->argumentCount() < 1))
        return throwVMError(lexicalGlobalObject, throwScope, createNotEnoughArgumentsError(lexicalGlobalObject));
    EnsureStillAliveScope argument0 = callFrame->uncheckedArgument(0);
    auto name = convert<IDLDOMString>(*lexicalGlobalObject, argument0.value());
    RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
    RELEASE_AND_RETURN(throwScope, JSValue::encode(toJS<IDLUndefined>(*lexicalGlobalObject, throwScope, [&]() -> decltype(auto) { return impl.namedDeleter(WTFMove(name)); })));
}

JSC_DEFINE_HOST_FUNCTION(jsTestNamedDeleterWithIdentifierPrototypeFunction_namedDeleter, (JSGlobalObject* lexicalGlobalObject, CallFrame* callFrame))
{
    return IDLOperation<JSTestNamedDeleterWithIdentifier>::call<jsTestNamedDeleterWithIdentifierPrototypeFunction_namedDeleterBody>(*lexicalGlobalObject, *callFrame, "namedDeleter");
}

JSC::GCClient::IsoSubspace* JSTestNamedDeleterWithIdentifier::subspaceForImpl(JSC::VM& vm)
{
    return WebCore::subspaceForImpl<JSTestNamedDeleterWithIdentifier, UseCustomHeapCellType::No>(vm,
        [] (auto& spaces) { return spaces.m_clientSubspaceForTestNamedDeleterWithIdentifier.get(); },
        [] (auto& spaces, auto&& space) { spaces.m_clientSubspaceForTestNamedDeleterWithIdentifier = std::forward<decltype(space)>(space); },
        [] (auto& spaces) { return spaces.m_subspaceForTestNamedDeleterWithIdentifier.get(); },
        [] (auto& spaces, auto&& space) { spaces.m_subspaceForTestNamedDeleterWithIdentifier = std::forward<decltype(space)>(space); }
    );
}

void JSTestNamedDeleterWithIdentifier::analyzeHeap(JSCell* cell, HeapAnalyzer& analyzer)
{
    auto* thisObject = jsCast<JSTestNamedDeleterWithIdentifier*>(cell);
    analyzer.setWrappedObjectForCell(cell, &thisObject->wrapped());
    if (thisObject->scriptExecutionContext())
        analyzer.setLabelForCell(cell, "url "_s + thisObject->scriptExecutionContext()->url().string());
    Base::analyzeHeap(cell, analyzer);
}

bool JSTestNamedDeleterWithIdentifierOwner::isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown> handle, void*, AbstractSlotVisitor& visitor, const char** reason)
{
    UNUSED_PARAM(handle);
    UNUSED_PARAM(visitor);
    UNUSED_PARAM(reason);
    return false;
}

void JSTestNamedDeleterWithIdentifierOwner::finalize(JSC::Handle<JSC::Unknown> handle, void* context)
{
    auto* jsTestNamedDeleterWithIdentifier = static_cast<JSTestNamedDeleterWithIdentifier*>(handle.slot()->asCell());
    auto& world = *static_cast<DOMWrapperWorld*>(context);
    uncacheWrapper(world, &jsTestNamedDeleterWithIdentifier->wrapped(), jsTestNamedDeleterWithIdentifier);
}

#if ENABLE(BINDING_INTEGRITY)
#if PLATFORM(WIN)
#pragma warning(disable: 4483)
extern "C" { extern void (*const __identifier("??_7TestNamedDeleterWithIdentifier@WebCore@@6B@")[])(); }
#else
extern "C" { extern void* _ZTVN7WebCore30TestNamedDeleterWithIdentifierE[]; }
#endif
#endif

JSC::JSValue toJSNewlyCreated(JSC::JSGlobalObject*, JSDOMGlobalObject* globalObject, Ref<TestNamedDeleterWithIdentifier>&& impl)
{

    if constexpr (std::is_polymorphic_v<TestNamedDeleterWithIdentifier>) {
#if ENABLE(BINDING_INTEGRITY)
        const void* actualVTablePointer = getVTablePointer(impl.ptr());
#if PLATFORM(WIN)
        void* expectedVTablePointer = __identifier("??_7TestNamedDeleterWithIdentifier@WebCore@@6B@");
#else
        void* expectedVTablePointer = &_ZTVN7WebCore30TestNamedDeleterWithIdentifierE[2];
#endif

        // If you hit this assertion you either have a use after free bug, or
        // TestNamedDeleterWithIdentifier has subclasses. If TestNamedDeleterWithIdentifier has subclasses that get passed
        // to toJS() we currently require TestNamedDeleterWithIdentifier you to opt out of binding hardening
        // by adding the SkipVTableValidation attribute to the interface IDL definition
        RELEASE_ASSERT(actualVTablePointer == expectedVTablePointer);
#endif
    }
    return createWrapper<TestNamedDeleterWithIdentifier>(globalObject, WTFMove(impl));
}

JSC::JSValue toJS(JSC::JSGlobalObject* lexicalGlobalObject, JSDOMGlobalObject* globalObject, TestNamedDeleterWithIdentifier& impl)
{
    return wrap(lexicalGlobalObject, globalObject, impl);
}

TestNamedDeleterWithIdentifier* JSTestNamedDeleterWithIdentifier::toWrapped(JSC::VM&, JSC::JSValue value)
{
    if (auto* wrapper = jsDynamicCast<JSTestNamedDeleterWithIdentifier*>(value))
        return &wrapper->wrapped();
    return nullptr;
}

}
