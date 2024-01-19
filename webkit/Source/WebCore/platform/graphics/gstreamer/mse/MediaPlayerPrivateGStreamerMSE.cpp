/*
 * Copyright (C) 2007, 2009 Apple Inc.  All rights reserved.
 * Copyright (C) 2007 Collabora Ltd.  All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2009 Gustavo Noronha Silva <gns@gnome.org>
 * Copyright (C) 2009, 2010, 2011, 2012, 2013, 2016, 2017, 2018, 2019, 2020, 2021 Igalia S.L
 * Copyright (C) 2015 Sebastian Dröge <sebastian@centricular.com>
 * Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021 Metrological Group B.V.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * aint with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "MediaPlayerPrivateGStreamerMSE.h"

#if ENABLE(VIDEO) && USE(GSTREAMER) && ENABLE(MEDIA_SOURCE)

#include "AppendPipeline.h"
#include "AudioTrackPrivateGStreamer.h"
#include "GStreamerCommon.h"
#include "GStreamerRegistryScannerMSE.h"
#include "InbandTextTrackPrivateGStreamer.h"
#include "MIMETypeRegistry.h"
#include "MediaDescription.h"
#include "MediaPlayer.h"
#include "MediaSourceTrackGStreamer.h"
#include "SourceBufferPrivateGStreamer.h"
#include "TimeRanges.h"
#include "VideoTrackPrivateGStreamer.h"
#include "WebKitMediaSourceGStreamer.h"

#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <gst/video/video.h>
#include <wtf/Condition.h>
#include <wtf/HashSet.h>
#include <wtf/NativePromise.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/RunLoop.h>
#include <wtf/StringPrintStream.h>
#include <wtf/URL.h>
#include <wtf/text/AtomString.h>
#include <wtf/text/AtomStringHash.h>
#include <wtf/text/StringToIntegerConversion.h>

static const char* dumpReadyState(WebCore::MediaPlayer::ReadyState readyState)
{
    switch (readyState) {
    case WebCore::MediaPlayer::ReadyState::HaveNothing: return "HaveNothing";
    case WebCore::MediaPlayer::ReadyState::HaveMetadata: return "HaveMetadata";
    case WebCore::MediaPlayer::ReadyState::HaveCurrentData: return "HaveCurrentData";
    case WebCore::MediaPlayer::ReadyState::HaveFutureData: return "HaveFutureData";
    case WebCore::MediaPlayer::ReadyState::HaveEnoughData: return "HaveEnoughData";
    default: return "(unknown)";
    }
}

GST_DEBUG_CATEGORY(webkit_mse_debug);
#define GST_CAT_DEFAULT webkit_mse_debug

namespace WebCore {

class MediaPlayerFactoryGStreamerMSE final : public MediaPlayerFactory {
private:
    MediaPlayerEnums::MediaEngineIdentifier identifier() const final { return MediaPlayerEnums::MediaEngineIdentifier::GStreamerMSE; };

    Ref<MediaPlayerPrivateInterface> createMediaEnginePlayer(MediaPlayer* player) const final
    {
        return adoptRef(*new MediaPlayerPrivateGStreamerMSE(player));
    }

    void getSupportedTypes(HashSet<String>& types) const final
    {
        return MediaPlayerPrivateGStreamerMSE::getSupportedTypes(types);
    }

    MediaPlayer::SupportsType supportsTypeAndCodecs(const MediaEngineSupportParameters& parameters) const final
    {
        return MediaPlayerPrivateGStreamerMSE::supportsType(parameters);
    }

    bool supportsKeySystem(const String& keySystem, const String& mimeType) const final
    {
        return MediaPlayerPrivateGStreamerMSE::supportsKeySystem(keySystem, mimeType);
    }
};

void MediaPlayerPrivateGStreamerMSE::registerMediaEngine(MediaEngineRegistrar registrar)
{
    GST_DEBUG_CATEGORY_INIT(webkit_mse_debug, "webkitmse", 0, "WebKit MSE media player");
    registrar(makeUnique<MediaPlayerFactoryGStreamerMSE>());
}

MediaPlayerPrivateGStreamerMSE::MediaPlayerPrivateGStreamerMSE(MediaPlayer* player)
    : MediaPlayerPrivateGStreamer(player)
{
    GST_TRACE("creating the player (%p)", this);
}

MediaPlayerPrivateGStreamerMSE::~MediaPlayerPrivateGStreamerMSE()
{
    GST_TRACE("destroying the player (%p)", this);

    m_source.clear();
}

void MediaPlayerPrivateGStreamerMSE::load(const String&)
{
    // This media engine only supports MediaSource URLs.
    m_networkState = MediaPlayer::NetworkState::FormatError;
    if (auto player = m_player.get())
        player->networkStateChanged();
}

void MediaPlayerPrivateGStreamerMSE::load(const URL& url, const ContentType&, MediaSourcePrivateClient& mediaSource)
{
    auto mseBlobURI = makeString("mediasource", url.string().isEmpty() ? "blob://"_s : url.string());
    GST_DEBUG("Loading %s", mseBlobURI.ascii().data());
    m_mediaSourcePrivate = MediaSourcePrivateGStreamer::open(mediaSource, *this);

    MediaPlayerPrivateGStreamer::load(mseBlobURI);
}

void MediaPlayerPrivateGStreamerMSE::play()
{
    GST_DEBUG_OBJECT(pipeline(), "Play requested");
    m_isPaused = false;
    if (!m_playbackRate && m_playbackRatePausedState == PlaybackRatePausedState::ManuallyPaused)
        m_playbackRatePausedState = PlaybackRatePausedState::RatePaused;
    updateStates();
}

void MediaPlayerPrivateGStreamerMSE::pause()
{
    GST_DEBUG_OBJECT(pipeline(), "Pause requested");
    m_isPaused = true;
    m_playbackRatePausedState = PlaybackRatePausedState::ManuallyPaused;
    updateStates();
}

MediaTime MediaPlayerPrivateGStreamerMSE::durationMediaTime() const
{
    if (UNLIKELY(!m_pipeline || m_didErrorOccur))
        return MediaTime();

    return m_mediaTimeDuration;
}

void MediaPlayerPrivateGStreamerMSE::seekToTarget(const SeekTarget& target)
{
    GST_DEBUG_OBJECT(pipeline(), "Requested seek to %s", target.time.toString().utf8().data());
    doSeek(target, m_playbackRate);
}

bool MediaPlayerPrivateGStreamerMSE::doSeek(const SeekTarget& target, float rate)
{
    // This method should only be called outside of MediaPlayerPrivateGStreamerMSE by MediaPlayerPrivateGStreamer::setRate().

    // Note: An important difference between seek with WebKitMediaSrc and regular playback is that seeking before
    // pre-roll (e.g. to start playback at a non-zero position) is supported in WebKitMediaSrc but not in regular
    // playback. This is relevant in MSE because pre-roll may never occur if the JS code never appends a range starting
    // at zero, creating a chicken and egg problem.

    // GStreamer doesn't support zero as a valid playback rate. Instead, that is implemented in WebKit by pausing
    // the pipeline.
    if (rate <= 0)
        rate = 1.0;

    m_seekTarget = target;
    m_isSeeking = true;
    m_isWaitingForPreroll = true;
    m_isEndReached = false;

    // Important: In order to ensure correct propagation whether pre-roll has happened or not, we send the seek directly
    // to the source element, rather than letting playbin do the routing.
    gst_element_seek(m_source.get(), rate, GST_FORMAT_TIME, m_seekFlags,
        GST_SEEK_TYPE_SET, toGstClockTime(target.time), GST_SEEK_TYPE_NONE, 0);
    invalidateCachedPosition();

    // Notify MediaSource and have new frames enqueued (when they're available).
    // Seek should only continue once the seekToTarget completionhandler has run.
    // This will also add support for fastSeek once done (see webkit.org/b/260607)
    if (!m_mediaSourcePrivate)
        return false;
    m_mediaSourcePrivate->waitForTarget(target)->whenSettled(RunLoop::current(), [this, weakThis = WeakPtr { *this }](auto&& result) {
        if (!weakThis || !result)
            return;

        if (m_mediaSourcePrivate)
            m_mediaSourcePrivate->seekToTime(*result);

        auto player = m_player.get();
        if (player && !player->isVideoPlayer() && m_audioSink) {
            gboolean audioSinkPerformsAsyncStateChanges;
            g_object_get(m_audioSink.get(), "async", &audioSinkPerformsAsyncStateChanges, nullptr);
            if (!audioSinkPerformsAsyncStateChanges) {
                // If audio-only pipeline's sink is not performing async state changes
                // we must simulate preroll right away as otherwise nothing will trigger it.
                didPreroll();
            }
        }
    });
    return true;
}

void MediaPlayerPrivateGStreamerMSE::setNetworkState(MediaPlayer::NetworkState networkState)
{
    if (networkState == m_mediaSourceNetworkState)
        return;

    m_mediaSourceNetworkState = networkState;
    m_networkState = networkState;
    updateStates();
    if (auto player = m_player.get())
        player->networkStateChanged();
}

void MediaPlayerPrivateGStreamerMSE::setReadyState(MediaPlayer::ReadyState mediaSourceReadyState)
{
    // Something important to bear in mind is that the readyState we get here comes from MediaSource.
    // From MediaSource perspective, as long as the sample for currentTime exists in the sample map, we are >= HaveCurrentData.
    // This is NOT true from the player perspective though, because there needs to pass some time since we have the first frame
    // (>=HaveCurrentData for MediaSource) and we have decoded it and sent it to the sink/compositor (>=HaveCurrentData in HTMLMediaElement).
    // The way we implement this is by keeping track of the MediaSource readyState internally in m_mediaSourceReadyState but not
    // spreading states >= HaveCurrentData to the player until prerolled.
    if (mediaSourceReadyState == m_mediaSourceReadyState)
        return;

    GST_DEBUG("MediaSource called setReadyState(%p): %s -> %s Current player state: %s Waiting for preroll: %s", this,
        dumpReadyState(m_mediaSourceReadyState), dumpReadyState(mediaSourceReadyState), dumpReadyState(m_readyState), boolForPrinting(m_isWaitingForPreroll));
    m_mediaSourceReadyState = mediaSourceReadyState;

    if (mediaSourceReadyState < MediaPlayer::ReadyState::HaveCurrentData || !m_isWaitingForPreroll)
        propagateReadyStateToPlayer();
}

void MediaPlayerPrivateGStreamerMSE::propagateReadyStateToPlayer()
{
    ASSERT(m_mediaSourceReadyState < MediaPlayer::ReadyState::HaveCurrentData || !m_isWaitingForPreroll);
    if (m_readyState == m_mediaSourceReadyState)
        return;
    GST_DEBUG("Propagating MediaSource readyState %s to player ready state (currently %s)", dumpReadyState(m_mediaSourceReadyState), dumpReadyState(m_readyState));

    m_readyState = m_mediaSourceReadyState;
    updateStates(); // Set the pipeline to PLAYING or PAUSED if necessary.
    auto player = m_player.get();
    if (player)
        player->readyStateChanged();

    // The readyState change may be a result of monitorSourceBuffers() finding that currentTime == duration, which
    // should cause the video to be marked as ended. Let's have the player check that.
    if (player && (!m_isWaitingForPreroll || currentMediaTime() == durationMediaTime()))
        player->timeChanged();
}

void MediaPlayerPrivateGStreamerMSE::didPreroll()
{
    ASSERT(GST_STATE(m_pipeline.get()) >= GST_STATE_PAUSED);
    // There are three circumstances in which a preroll can occur:
    // a) At the begining of playback. This is the point where we propagate >=HaveCurrentData to the player.
    // b) At the end of a seek. We emit the "seeked" event as well.
    // c) At the end of a flush (forced quality change). These should not produce either of these outcomes.
    // We identify (a) and (b) by setting m_isWaitingForPreroll = true at the initialization of the player and
    // at the beginning of a seek.
    GST_DEBUG("Pipeline prerolled. currentMediaTime = %s", currentMediaTime().toString().utf8().data());
    if (!m_isWaitingForPreroll) {
        GST_DEBUG("Preroll was consequence of a flush, nothing to do at this level.");
        return;
    }
    m_isWaitingForPreroll = false;

    // The order of these sections is important. In the case of a seek, the "seeked" event must be emitted
    // before the "playing" event (which is emitted on HAVE_ENOUGH_DATA). Therefore, we take care of them in
    // that order.

    if (m_isSeeking) {
        m_isSeeking = false;
        GST_DEBUG("Seek complete because of preroll. currentMediaTime = %s", currentMediaTime().toString().utf8().data());
        // By calling timeChanged(), m_isSeeking will be checked an a "seeked" event will be emitted.
        timeChanged(currentMediaTime());
    }

    propagateReadyStateToPlayer();
}

const PlatformTimeRanges& MediaPlayerPrivateGStreamerMSE::buffered() const
{
    if (m_mediaSourcePrivate)
        return m_mediaSourcePrivate->buffered();
    return PlatformTimeRanges::emptyRanges();
}

void MediaPlayerPrivateGStreamerMSE::sourceSetup(GstElement* sourceElement)
{
    ASSERT(WEBKIT_IS_MEDIA_SRC(sourceElement));
    GST_DEBUG_OBJECT(pipeline(), "Source %p setup (old was: %p)", sourceElement, m_source.get());
    webKitMediaSrcSetPlayer(WEBKIT_MEDIA_SRC(sourceElement), WeakPtr { *this });
    m_source = sourceElement;

    if (m_mediaSourcePrivate && m_mediaSourcePrivate->hasAllTracks())
        webKitMediaSrcEmitStreams(WEBKIT_MEDIA_SRC(m_source.get()), m_tracks);
}

void MediaPlayerPrivateGStreamerMSE::updateStates()
{
    bool shouldBePlaying = (!m_isPaused && readyState() >= MediaPlayer::ReadyState::HaveFutureData && m_playbackRatePausedState != PlaybackRatePausedState::RatePaused)
        || m_playbackRatePausedState == PlaybackRatePausedState::ShouldMoveToPlaying;
    GST_DEBUG_OBJECT(pipeline(), "shouldBePlaying = %s, m_isPipelinePlaying = %s", boolForPrinting(shouldBePlaying), boolForPrinting(m_isPipelinePlaying));
    if (shouldBePlaying && !m_isPipelinePlaying) {
        if (!changePipelineState(GST_STATE_PLAYING))
            GST_ERROR_OBJECT(pipeline(), "Setting the pipeline to PLAYING failed");
        m_isPipelinePlaying = true;
        m_playbackRatePausedState = PlaybackRatePausedState::Playing;
    } else if (!shouldBePlaying && m_isPipelinePlaying) {
        if (!changePipelineState(GST_STATE_PAUSED))
            GST_ERROR_OBJECT(pipeline(), "Setting the pipeline to PAUSED failed");
        m_isPipelinePlaying = false;
    }
}

bool MediaPlayerPrivateGStreamerMSE::isTimeBuffered(const MediaTime &time) const
{

    bool result = m_mediaSourcePrivate && m_mediaSourcePrivate->buffered().contain(time);
    GST_DEBUG("Time %s buffered? %s", toString(time).utf8().data(), boolForPrinting(result));
    return result;
}

void MediaPlayerPrivateGStreamerMSE::durationChanged()
{
    ASSERT(isMainThread());

    MediaTime previousDuration = m_mediaTimeDuration;
    m_mediaTimeDuration = m_mediaSourcePrivate ? m_mediaSourcePrivate->duration() : MediaTime::invalidTime();

    GST_TRACE("previous=%s, new=%s", toString(previousDuration).utf8().data(), toString(m_mediaTimeDuration).utf8().data());

    // Avoid emiting durationchanged in the case where the previous duration was 0 because that case is already handled
    // by the HTMLMediaElement.
    if (m_mediaTimeDuration != previousDuration && m_mediaTimeDuration.isValid() && previousDuration.isValid()) {
        if (auto player = m_player.get())
            player->durationChanged();
    }
}

void MediaPlayerPrivateGStreamerMSE::setInitialVideoSize(const FloatSize& videoSize)
{
    ASSERT(isMainThread());

    // We set the size of the video only for the first initialization segment.
    // This is intentional: Normally the video size depends on the frames arriving
    // at the sink in the playback pipeline, not in the append pipeline; but we still
    // want to report an initial size for HAVE_METADATA (first initialization segment).

    if (!m_videoSize.isEmpty())
        return;

    GST_DEBUG("Setting initial video size: %gx%g", videoSize.width(), videoSize.height());
    m_videoSize = videoSize;
}

void MediaPlayerPrivateGStreamerMSE::startSource(const Vector<RefPtr<MediaSourceTrackGStreamer>>& tracks)
{
    m_tracks = tracks;
    webKitMediaSrcEmitStreams(WEBKIT_MEDIA_SRC(m_source.get()), tracks);
}

void MediaPlayerPrivateGStreamerMSE::getSupportedTypes(HashSet<String>& types)
{
    GStreamerRegistryScannerMSE::getSupportedDecodingTypes(types);
}

MediaPlayer::SupportsType MediaPlayerPrivateGStreamerMSE::supportsType(const MediaEngineSupportParameters& parameters)
{
    MediaPlayer::SupportsType result = MediaPlayer::SupportsType::IsNotSupported;
    if (!parameters.isMediaSource)
        return result;

    if (!ensureGStreamerInitialized())
        return result;

    auto containerType = parameters.type.containerType();

    // YouTube TV provides empty types for some videos and we want to be selected as best media engine for them.
    if (containerType.isEmpty()) {
        result = MediaPlayer::SupportsType::MayBeSupported;
        GST_DEBUG("mime-type \"%s\" supported: %s", parameters.type.raw().utf8().data(), convertEnumerationToString(result).utf8().data());
        return result;
    }

    registerWebKitGStreamerElements();

    GST_DEBUG("Checking mime-type \"%s\"", parameters.type.raw().utf8().data());
    auto& gstRegistryScanner = GStreamerRegistryScannerMSE::singleton();
    result = gstRegistryScanner.isContentTypeSupported(GStreamerRegistryScanner::Configuration::Decoding, parameters.type, parameters.contentTypesRequiringHardwareSupport);

    GST_DEBUG("Supported: %s", convertEnumerationToString(result).utf8().data());
    return result;
}

MediaTime MediaPlayerPrivateGStreamerMSE::maxMediaTimeSeekable() const
{
    if (UNLIKELY(m_didErrorOccur))
        return MediaTime::zeroTime();

    GST_DEBUG("maxMediaTimeSeekable");
    MediaTime result = durationMediaTime();
    // Infinite duration means live stream.
    if (result.isPositiveInfinite()) {
        MediaTime maxBufferedTime = buffered().maximumBufferedTime();
        // Return the highest end time reported by the buffered attribute.
        result = maxBufferedTime.isValid() ? maxBufferedTime : MediaTime::zeroTime();
    }

    return result;
}

bool MediaPlayerPrivateGStreamerMSE::currentMediaTimeMayProgress() const
{
    if (!m_mediaSourcePrivate)
        return false;
    return m_mediaSourcePrivate->hasFutureTime(currentMediaTime());
}

void MediaPlayerPrivateGStreamerMSE::notifyActiveSourceBuffersChanged()
{
    if (auto player = m_player.get())
        player->activeSourceBuffersChanged();
}

#undef GST_CAT_DEFAULT

} // namespace WebCore.

#endif // USE(GSTREAMER)
