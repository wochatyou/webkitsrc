/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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
#include "StyleContentAlignmentData.h"

#include <wtf/text/TextStream.h>

namespace WebCore {

TextStream& operator<<(TextStream& ts, const StyleContentAlignmentData& o)
{
    return ts << o.position() << " " << o.distribution() << " " << o.overflow();
}

bool StyleContentAlignmentData::isStartward(TextDirection inlineDirection, bool isFlexReverse) const
{
    switch (static_cast<ContentPosition>(m_position)) {
    case ContentPosition::Normal:
        return static_cast<ContentDistribution>(m_distribution) == ContentDistribution::Default
            || static_cast<ContentDistribution>(m_distribution) == ContentDistribution::Stretch
            || static_cast<ContentDistribution>(m_distribution) == ContentDistribution::SpaceBetween;
    case ContentPosition::Start:
    case ContentPosition::Baseline:
        return true;
    case ContentPosition::End:
    case ContentPosition::LastBaseline:
    case ContentPosition::Center:
        return false;
    case ContentPosition::FlexStart:
        return !isFlexReverse;
    case ContentPosition::FlexEnd:
        return isFlexReverse;
    case ContentPosition::Left:
        return inlineDirection == TextDirection::LTR;
    case ContentPosition::Right:
        return inlineDirection == TextDirection::RTL;
    default:
        ASSERT("Invalid ContentPosition");
        return true;
    }
}
bool StyleContentAlignmentData::isEndward(TextDirection inlineDirection, bool isFlexReverse) const
{
    switch (static_cast<ContentPosition>(m_position)) {
    case ContentPosition::Normal:
    case ContentPosition::Start:
    case ContentPosition::Baseline:
        return false;
    case ContentPosition::End:
    case ContentPosition::LastBaseline:
    case ContentPosition::Center:
        return true;
    case ContentPosition::FlexStart:
        return isFlexReverse;
    case ContentPosition::FlexEnd:
        return !isFlexReverse;
    case ContentPosition::Left:
        return inlineDirection == TextDirection::RTL;
    case ContentPosition::Right:
        return inlineDirection == TextDirection::LTR;
    default:
        ASSERT("Invalid ContentPosition");
        return false;
    }
}

bool StyleContentAlignmentData::isCentered() const
{
    return static_cast<ContentPosition>(m_position) == ContentPosition::Center
        || static_cast<ContentDistribution>(m_distribution) == ContentDistribution::SpaceAround
        || static_cast<ContentDistribution>(m_distribution) == ContentDistribution::SpaceEvenly;
}

}
