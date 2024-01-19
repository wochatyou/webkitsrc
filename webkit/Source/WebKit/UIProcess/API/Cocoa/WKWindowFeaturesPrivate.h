/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#import <WebKit/WKWindowFeatures.h>

NS_ASSUME_NONNULL_BEGIN

@interface WKWindowFeatures (WKPrivate)

@property (nonatomic, readonly) BOOL _wantsPopup WK_API_AVAILABLE(macos(14.2), ios(17.2));
@property (nonatomic, readonly) BOOL _hasAdditionalFeatures WK_API_AVAILABLE(macos(14.2), ios(17.2));
@property (nullable, nonatomic, readonly) NSNumber *_popup WK_API_AVAILABLE(macos(14.2), ios(17.2));
@property (nullable, nonatomic, readonly) NSNumber *_locationBarVisibility WK_API_AVAILABLE(macos(10.13), ios(11.0));
@property (nullable, nonatomic, readonly) NSNumber *_scrollbarsVisibility WK_API_AVAILABLE(macos(10.13), ios(11.0));
@property (nullable, nonatomic, readonly) NSNumber *_fullscreenDisplay WK_API_AVAILABLE(macos(10.13), ios(11.0));
@property (nullable, nonatomic, readonly) NSNumber *_dialogDisplay WK_API_AVAILABLE(macos(10.13), ios(11.0));

@end

NS_ASSUME_NONNULL_END
