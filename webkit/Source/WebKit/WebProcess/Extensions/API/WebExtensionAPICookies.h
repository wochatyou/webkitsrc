/*
 * Copyright (C) 2023-2024 Apple Inc. All rights reserved.
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

#pragma once

#if ENABLE(WK_WEB_EXTENSIONS)

#include "JSWebExtensionAPICookies.h"
#include "WebExtensionAPIEvent.h"
#include "WebExtensionAPIObject.h"

OBJC_CLASS NSDictionary;
OBJC_CLASS NSString;

namespace WebKit {

class WebExtensionAPICookies : public WebExtensionAPIObject, public JSWebExtensionWrappable {
    WEB_EXTENSION_DECLARE_JS_WRAPPER_CLASS(WebExtensionAPICookies, cookies);

public:
#if PLATFORM(COCOA)
    void get(NSDictionary *details, Ref<WebExtensionCallbackHandler>&&, NSString **outExceptionString);
    void getAll(NSDictionary *details, Ref<WebExtensionCallbackHandler>&&, NSString **outExceptionString);

    void set(NSDictionary *details, Ref<WebExtensionCallbackHandler>&&, NSString **outExceptionString);
    void remove(NSDictionary *details, Ref<WebExtensionCallbackHandler>&&, NSString **outExceptionString);

    void getAllCookieStores(Ref<WebExtensionCallbackHandler>&&);

    WebExtensionAPIEvent& onChanged();

private:
    using ParsedDetails = std::tuple<std::optional<PAL::SessionID>, String, URL>;
    static std::optional<ParsedDetails> parseCookieDetails(NSDictionary *details, NSArray *requiredKeys, NSString **outExceptionString);

    RefPtr<WebExtensionAPIEvent> m_onChanged;
#endif
};

} // namespace WebKit

#endif // ENABLE(WK_WEB_EXTENSIONS)
