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

#pragma once

#include "JSDOMWrapper.h"
#include <wtf/NeverDestroyed.h>

namespace WebCore {

class ShadowRealmGlobalScope;

class JSShadowRealmGlobalScope : public JSDOMWrapper<ShadowRealmGlobalScope> {
public:
    using Base = JSDOMWrapper<ShadowRealmGlobalScope>;
    static JSShadowRealmGlobalScope* create(JSC::VM& vm, JSC::Structure* structure, Ref<ShadowRealmGlobalScope>&& impl, JSC::JSGlobalProxy* proxy)
    {
        JSShadowRealmGlobalScope* ptr = new (NotNull, JSC::allocateCell<JSShadowRealmGlobalScope>(vm)) JSShadowRealmGlobalScope(vm, structure, WTFMove(impl));
        ptr->finishCreation(vm, proxy);
        return ptr;
    }

    static ShadowRealmGlobalScope* toWrapped(JSC::VM&, JSC::JSValue);
    static void destroy(JSC::JSCell*);

    DECLARE_INFO;

    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::GlobalObjectType, StructureFlags), info(), JSC::NonArray);
    }

    static JSC::JSValue getConstructor(JSC::VM&, const JSC::JSGlobalObject*);
    template<typename, JSC::SubspaceAccess mode> static JSC::GCClient::IsoSubspace* subspaceFor(JSC::VM& vm)
    {
        if constexpr (mode == JSC::SubspaceAccess::Concurrently)
            return nullptr;
        return subspaceForImpl(vm);
    }
    static JSC::GCClient::IsoSubspace* subspaceForImpl(JSC::VM& vm);
    static void analyzeHeap(JSCell*, JSC::HeapAnalyzer&);
public:
    static constexpr unsigned StructureFlags = (Base::StructureFlags & ~JSC::IsImmutablePrototypeExoticObject) | JSC::HasStaticPropertyTable;
protected:
    JSShadowRealmGlobalScope(JSC::VM&, JSC::Structure*, Ref<ShadowRealmGlobalScope>&&);
    void finishCreation(JSC::VM&, JSC::JSGlobalProxy*);
};

class JSShadowRealmGlobalScopeOwner final : public JSC::WeakHandleOwner {
public:
    bool isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown>, void* context, JSC::AbstractSlotVisitor&, const char**) final;
    void finalize(JSC::Handle<JSC::Unknown>, void* context) final;
};

inline JSC::WeakHandleOwner* wrapperOwner(DOMWrapperWorld&, ShadowRealmGlobalScope*)
{
    static NeverDestroyed<JSShadowRealmGlobalScopeOwner> owner;
    return &owner.get();
}

inline void* wrapperKey(ShadowRealmGlobalScope* wrappableObject)
{
    return wrappableObject;
}


template<> struct JSDOMWrapperConverterTraits<ShadowRealmGlobalScope> {
    using WrapperClass = JSShadowRealmGlobalScope;
    using ToWrappedReturnType = ShadowRealmGlobalScope*;
};

} // namespace WebCore
