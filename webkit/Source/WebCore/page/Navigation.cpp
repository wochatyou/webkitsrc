/*
 * Copyright (C) 2023 Igalia S.L. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "Navigation.h"

#include "JSNavigationHistoryEntry.h"
#include <wtf/IsoMallocInlines.h>

namespace WebCore {

WTF_MAKE_ISO_ALLOCATED_IMPL(Navigation);

Navigation::Navigation(ScriptExecutionContext* context, LocalDOMWindow& window)
    : ContextDestructionObserver(context)
    , LocalDOMWindowProperty(&window)
{
}

Navigation::~Navigation() = default;

ScriptExecutionContext* Navigation::scriptExecutionContext() const
{
    return ContextDestructionObserver::scriptExecutionContext();
}

EventTargetInterface Navigation::eventTargetInterface() const
{
    return NavigationEventTargetInterfaceType;
}

Navigation::Result Navigation::navigate(const String& /* url */, NavigateOptions&&, Ref<DeferredPromise>&& committed, Ref<DeferredPromise>&& finished)
{
    // FIXME: keep track of promises to resolve later.
    auto entry = NavigationHistoryEntry::create(scriptExecutionContext());
    auto globalObject = committed->globalObject();
    Navigation::Result result = { DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(committed->promise())), DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(finished->promise())) };
    committed->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    finished->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    return result;
}

Navigation::Result Navigation::reload(ReloadOptions&&, Ref<DeferredPromise>&& committed, Ref<DeferredPromise>&& finished)
{
    // FIXME: keep track of promises to resolve later.
    auto entry = NavigationHistoryEntry::create(scriptExecutionContext());
    auto globalObject = committed->globalObject();
    Navigation::Result result = { DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(committed->promise())), DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(finished->promise())) };
    committed->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    finished->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    return result;
}

Navigation::Result Navigation::traverseTo(const String& /* key */, Options&&, Ref<DeferredPromise>&& committed, Ref<DeferredPromise>&& finished)
{
    // FIXME: keep track of promises to resolve later.
    auto entry = NavigationHistoryEntry::create(scriptExecutionContext());
    auto globalObject = committed->globalObject();
    Navigation::Result result = { DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(committed->promise())), DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(finished->promise())) };
    committed->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    finished->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    return result;
}

Navigation::Result Navigation::back(Options&&, Ref<DeferredPromise>&& committed, Ref<DeferredPromise>&& finished)
{
    // FIXME: keep track of promises to resolve later.
    auto entry = NavigationHistoryEntry::create(scriptExecutionContext());
    auto globalObject = committed->globalObject();
    Navigation::Result result = { DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(committed->promise())), DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(finished->promise())) };
    committed->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    finished->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    return result;
}

Navigation::Result Navigation::forward(Options&&, Ref<DeferredPromise>&& committed, Ref<DeferredPromise>&& finished)
{
    // FIXME: keep track of promises to resolve later.
    auto entry = NavigationHistoryEntry::create(scriptExecutionContext());
    auto globalObject = committed->globalObject();
    Navigation::Result result = { DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(committed->promise())), DOMPromise::create(*globalObject, *JSC::jsCast<JSC::JSPromise*>(finished->promise())) };
    committed->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    finished->resolve<IDLInterface<NavigationHistoryEntry>>(entry.get());
    return result;
}

void Navigation::updateCurrentEntry(UpdateCurrentEntryOptions&&)
{
}


} // namespace WebCore
