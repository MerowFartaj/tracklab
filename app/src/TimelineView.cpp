#include "TimelineView.h"

#include "UiDrawing.h"

#include <cmath>

namespace tokens = tracklab::design;
namespace ui = tracklab::ui;

namespace
{
bool isSupportedAudioFile (const juce::File& file)
{
    const auto extension = file.getFileExtension().toLowerCase();
    return extension == ".wav"
        || extension == ".aiff"
        || extension == ".aif"
        || extension == ".mp3"
        || extension == ".flac";
}
}

TimelineView::TimelineContent::TimelineContent (AudioEngine& engine)
    : audioEngine (engine),
      playhead (engine)
{
    setOpaque (true);

    addAndMakeVisible (ruler);
    addAndMakeVisible (addTrackButton);
    addAndMakeVisible (playhead);

    ruler.onSeek = [this] (double seconds)
    {
        hintVisible = false;

        if (onSeek)
            onSeek (seconds);
    };

    ruler.onMouseWheel = [this] (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
    {
        if (onMouseWheel)
            onMouseWheel (event.getEventRelativeTo (this), wheel);
    };

    addTrackButton.setWantsKeyboardFocus (false);
    addTrackButton.setButtonText ("+ Add Track");
    addTrackButton.setColour (juce::TextButton::buttonColourId, tokens::surfaceRaised);
    addTrackButton.setColour (juce::TextButton::textColourOffId, tokens::textPrimary);
    addTrackButton.onClick = [this]
    {
        hintVisible = false;

        if (onAddTrack)
            onAddTrack();
    };
}

void TimelineView::TimelineContent::paint (juce::Graphics& g)
{
    g.fillAll (tokens::backgroundDeep);
    ui::drawSubtleStripes (g, getLocalBounds().toFloat(), tokens::highlightBase, tokens::decorativeStripeSpacing * 2);
}

void TimelineView::TimelineContent::paintOverChildren (juce::Graphics& g)
{
    if (hintVisible && clipInfos.empty())
    {
        auto visible = g.getClipBounds().toFloat();
        visible.removeFromTop (tokens::rulerHeight);

        auto hint = visible.withSizeKeepingCentre (juce::jmin (visible.getWidth() * tokens::timelineHintWidthRatio,
                                                               static_cast<float> (tokens::timelineHintMaxWidth)),
                                                   static_cast<float> (tokens::timelineHintHeight));
        ui::drawGlassPanel (g, hint, tokens::surfaceInset.withAlpha (tokens::glassAlpha), tokens::panelSectionRadius, false);
        ui::drawIcon (g,
                      ui::Icon::waveform,
                      hint.removeFromLeft (static_cast<float> (tokens::timelineHintIconWidth))
                          .withSizeKeepingCentre (tokens::iconSizeLarge, tokens::iconSizeLarge),
                      tokens::accentCyan,
                      tokens::thickIconStrokeWidth);
        ui::drawTinySparkles (g, hint.withTrimmedLeft (hint.getWidth() * tokens::timelineHintSparkleStartRatio),
                              tokens::accentPrimary);

        g.setColour (tokens::textTertiary.withAlpha (tokens::timelineHintAlpha));
        g.setFont (tokens::fontHeader());
        g.drawText ("Drop audio files to start",
                    hint,
                    juce::Justification::centred,
                    false);
    }
}

void TimelineView::TimelineContent::resized()
{
    layoutContent();
}

void TimelineView::TimelineContent::mouseDown (const juce::MouseEvent& event)
{
    hintVisible = false;

    if (onDeselectAll)
        onDeselectAll();

    const auto trackIndex = static_cast<int> (std::floor ((event.position.y - tokens::rulerHeight) / tokens::trackHeightDefault));

    if (onTrackSelected && juce::isPositiveAndBelow (trackIndex, getTrackCount()))
        onTrackSelected (getTrackIdForIndex (trackIndex));

    if (onSeek)
        onSeek (xToSeconds (event.position.x));
}

void TimelineView::TimelineContent::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (onMouseWheel)
        onMouseWheel (event, wheel);
}

void TimelineView::TimelineContent::refreshFromEngine (const std::set<int>& selectedClipIds)
{
    trackInfos = audioEngine.getAllTrackInfo();
    clipInfos = audioEngine.getAllClips();
    ensureRowCount (static_cast<int> (trackInfos.size()));

    for (auto i = 0; i < static_cast<int> (trackInfos.size()); ++i)
        rows[static_cast<size_t> (i)]->setTrackInfo (trackInfos[static_cast<size_t> (i)], i + 1);

    rebuildClipViews (selectedClipIds);
    layoutContent();
    repaint();
}

void TimelineView::TimelineContent::setTimelineGeometry (double newPixelsPerSecond, double newLengthSeconds)
{
    pixelsPerSecond = newPixelsPerSecond;
    timelineLengthSeconds = juce::jmax (tokens::timelineEmptyLengthSeconds, newLengthSeconds);

    ruler.setPixelsPerSecond (pixelsPerSecond);
    ruler.setTimelineLength (timelineLengthSeconds);

    for (auto& row : rows)
    {
        row->setPixelsPerSecond (pixelsPerSecond);
        row->setTimelineLength (timelineLengthSeconds);
    }

    for (auto& clipView : clipViews)
        clipView->setPixelsPerSecond (pixelsPerSecond);

    playhead.setTimelineGeometry (pixelsPerSecond, timelineLengthSeconds);
    layoutContent();
    repaint();
}

void TimelineView::TimelineContent::refreshPlayhead()
{
    playhead.forceUpdate();
}

int TimelineView::TimelineContent::getTrackIndexForId (int trackId) const
{
    for (auto i = 0; i < static_cast<int> (trackInfos.size()); ++i)
        if (trackInfos[static_cast<size_t> (i)].id == trackId)
            return i;

    return -1;
}

int TimelineView::TimelineContent::getTrackIdForIndex (int trackIndex) const
{
    if (! juce::isPositiveAndBelow (trackIndex, static_cast<int> (trackInfos.size())))
        return 0;

    return trackInfos[static_cast<size_t> (trackIndex)].id;
}

int TimelineView::TimelineContent::getTrackCount() const
{
    return static_cast<int> (trackInfos.size());
}

void TimelineView::TimelineContent::ensureRowCount (int count)
{
    while (static_cast<int> (rows.size()) < count)
    {
        auto row = std::make_unique<TrackRow>();

        row->onSeek = [this] (double seconds)
        {
            hintVisible = false;

            if (onSeek)
                onSeek (seconds);
        };

        row->onEmptyLaneClicked = [this] (int trackId)
        {
            hintVisible = false;

            if (onDeselectAll)
                onDeselectAll();

            if (onTrackSelected)
                onTrackSelected (trackId);
        };

        row->onTrackNameChanged = [this] (int trackId, juce::String name)
        {
            if (onTrackNameChanged)
                onTrackNameChanged (trackId, std::move (name));
        };

        row->onVolumeChanged = [this] (int trackId, float gain)
        {
            if (onTrackVolumeChanged)
                onTrackVolumeChanged (trackId, gain);
        };

        row->onPanChanged = [this] (int trackId, float pan)
        {
            if (onTrackPanChanged)
                onTrackPanChanged (trackId, pan);
        };

        row->onMuteChanged = [this] (int trackId, bool muted)
        {
            if (onTrackMuteChanged)
                onTrackMuteChanged (trackId, muted);
        };

        row->onSoloChanged = [this] (int trackId, bool soloed)
        {
            if (onTrackSoloChanged)
                onTrackSoloChanged (trackId, soloed);
        };

        row->onFilesDropped = [this] (const juce::StringArray& files, int trackId, double seconds)
        {
            hintVisible = false;

            if (onFilesDropped)
                onFilesDropped (files, trackId, seconds);
        };

        row->onMouseWheel = [this] (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
        {
            if (onMouseWheel)
                onMouseWheel (event.getEventRelativeTo (this), wheel);
        };

        addAndMakeVisible (*row);
        rows.push_back (std::move (row));
    }

    while (static_cast<int> (rows.size()) > count)
    {
        removeChildComponent (rows.back().get());
        rows.pop_back();
    }
}

void TimelineView::TimelineContent::rebuildClipViews (const std::set<int>& selectedClipIds)
{
    for (auto& clipView : clipViews)
        removeChildComponent (clipView.get());

    clipViews.clear();

    for (const auto& clip : clipInfos)
    {
        const auto trackIndex = getTrackIndexForId (clip.trackId);

        if (trackIndex < 0)
            continue;

        auto clipView = std::make_unique<AudioClipView>();
        clipView->setPixelsPerSecond (pixelsPerSecond);
        clipView->setClipInfo (clip,
                               trackInfos[static_cast<size_t> (trackIndex)].colour,
                               selectedClipIds.contains (clip.id));

        clipView->onSelect = [this] (int clipId, bool addToSelection)
        {
            hintVisible = false;

            if (onClipSelected)
                onClipSelected (clipId, addToSelection);
        };

        clipView->onMoveFinished = [this] (int clipId, float deltaX, float deltaY, bool fine)
        {
            if (onClipMoved)
                onClipMoved (clipId, deltaX, deltaY, fine);
        };

        clipView->onTrimFinished = [this] (int clipId, bool trimStart, float deltaX, bool fine)
        {
            if (onClipTrimmed)
                onClipTrimmed (clipId, trimStart, deltaX, fine);
        };

        clipView->onDeleteRequested = [this] (int clipId)
        {
            if (onClipDeleted)
                onClipDeleted (clipId);
        };

        clipView->onDuplicateRequested = [this] (int clipId)
        {
            if (onClipDuplicated)
                onClipDuplicated (clipId);
        };

        clipView->onRenameRequested = [this] (int clipId, juce::String name)
        {
            if (onClipRenamed)
                onClipRenamed (clipId, std::move (name));
        };

        addAndMakeVisible (*clipView);
        clipViews.push_back (std::move (clipView));
    }

    playhead.toFront (false);
}

void TimelineView::TimelineContent::layoutContent()
{
    const auto width = getWidth();
    const auto timelineWidth = juce::jmax (1, width - tokens::trackHeaderWidth);

    ruler.setBounds (tokens::trackHeaderWidth, 0, timelineWidth, tokens::rulerHeight);

    for (auto i = 0; i < static_cast<int> (rows.size()); ++i)
    {
        rows[static_cast<size_t> (i)]->setBounds (0,
                                                  tokens::rulerHeight + i * tokens::trackHeightDefault,
                                                  width,
                                                  tokens::trackHeightDefault);
    }

    for (auto& clipView : clipViews)
    {
        const auto clip = audioEngine.getClipInfo (clipView->getClipId());
        const auto trackIndex = getTrackIndexForId (clip.trackId);

        if (trackIndex < 0)
            continue;

        clipView->setBounds (tokens::trackHeaderWidth + juce::roundToInt (clip.startTimeSeconds * pixelsPerSecond),
                             tokens::rulerHeight + trackIndex * tokens::trackHeightDefault + tokens::clipVerticalPadding,
                             juce::jmax (1, juce::roundToInt (clip.lengthSeconds * pixelsPerSecond)),
                             tokens::trackHeightDefault - tokens::clipVerticalPadding * 2);
    }

    addTrackButton.setBounds (tokens::trackColorBarWidth,
                              tokens::rulerHeight + static_cast<int> (rows.size()) * tokens::trackHeightDefault,
                              tokens::trackHeaderWidth - tokens::trackColorBarWidth - tokens::toolbarGap,
                              tokens::addTrackHeight);

    playhead.setBounds (0,
                        0,
                        width,
                        tokens::rulerHeight + static_cast<int> (rows.size()) * tokens::trackHeightDefault);
    playhead.toFront (false);
}

double TimelineView::TimelineContent::xToSeconds (float x) const
{
    const auto laneX = juce::jmax (0.0f, x - tokens::trackHeaderWidth);
    return juce::jlimit (0.0, timelineLengthSeconds, laneX / pixelsPerSecond);
}

TimelineView::TimelineView (AudioEngine& engine)
    : audioEngine (engine),
      content (engine)
{
    setOpaque (true);
    addAndMakeVisible (viewport);

    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, true);
    viewport.setWantsKeyboardFocus (false);

    content.onSeek = [this] (double seconds)
    {
        seekTo (seconds);
    };

    content.onDeselectAll = [this]
    {
        deselectAllClips();
    };

    content.onTrackSelected = [this] (int trackId)
    {
        selectedTrackId = trackId;
        notifySelectionChanged();
    };

    content.onAddTrack = [this]
    {
        addTrack();
    };

    content.onClipSelected = [this] (int clipId, bool addToSelection)
    {
        handleClipSelected (clipId, addToSelection);
    };

    content.onClipMoved = [this] (int clipId, float deltaX, float deltaY, bool fine)
    {
        handleClipMoved (clipId, deltaX, deltaY, fine);
    };

    content.onClipTrimmed = [this] (int clipId, bool trimStart, float deltaX, bool fine)
    {
        handleClipTrimmed (clipId, trimStart, deltaX, fine);
    };

    content.onClipDeleted = [this] (int clipId)
    {
        handleClipDeleted (clipId);
    };

    content.onClipDuplicated = [this] (int clipId)
    {
        handleClipDuplicated (clipId);
    };

    content.onClipRenamed = [this] (int clipId, juce::String name)
    {
        handleClipRenamed (clipId, std::move (name));
    };

    content.onFilesDropped = [this] (const juce::StringArray& files, int trackId, double seconds)
    {
        handleFilesDropped (files, trackId, seconds);
    };

    content.onTrackNameChanged = [this] (int trackId, juce::String name)
    {
        audioEngine.setTrackName (trackId, std::move (name));
        refreshFromEngine();
    };

    content.onTrackVolumeChanged = [this] (int trackId, float gain)
    {
        audioEngine.setTrackVolume (trackId, gain);
    };

    content.onTrackPanChanged = [this] (int trackId, float pan)
    {
        audioEngine.setTrackPan (trackId, pan);
    };

    content.onTrackMuteChanged = [this] (int trackId, bool muted)
    {
        audioEngine.setTrackMute (trackId, muted);
        refreshFromEngine();
    };

    content.onTrackSoloChanged = [this] (int trackId, bool soloed)
    {
        audioEngine.setTrackSolo (trackId, soloed);
        refreshFromEngine();
    };

    content.onMouseWheel = [this] (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
    {
        handleWheel (event.getEventRelativeTo (this), wheel);
    };

    openGLContext.setComponentPaintingEnabled (true);
    openGLContext.setContinuousRepainting (false);
    openGLContext.attachTo (*this);

    addMouseListener (this, true);
    refreshFromEngine();
}

TimelineView::~TimelineView()
{
    removeMouseListener (this);
    openGLContext.detach();
}

void TimelineView::paint (juce::Graphics& g)
{
    g.fillAll (tokens::backgroundBase);
}

void TimelineView::resized()
{
    viewport.setBounds (getLocalBounds());
    updateContentSize();
}

void TimelineView::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    handleWheel (event, wheel);
}

void TimelineView::setAudioFile (const juce::File&, double)
{
    refreshFromEngine();
}

void TimelineView::clearAudioFile()
{
    selectedClipIds.clear();
    refreshFromEngine();
}

void TimelineView::refreshFromEngine()
{
    updateTimelineLengthFromEngine();

    const auto tracks = audioEngine.getAllTrackInfo();

    if (! tracks.empty() && audioEngine.getTrackInfo (selectedTrackId).id <= 0)
        selectedTrackId = tracks.front().id;

    content.refreshFromEngine (selectedClipIds);
    updateContentSize();

    if (onProjectChanged)
        onProjectChanged();
}

void TimelineView::refreshPlayhead()
{
    content.refreshPlayhead();
}

void TimelineView::addTrack()
{
    const auto nextNumber = audioEngine.getTrackCount() + 1;
    selectedTrackId = audioEngine.addTrack (juce::String::formatted ("Track %02d", nextNumber));
    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::deleteSelectedClips()
{
    for (const auto clipId : selectedClipIds)
        audioEngine.removeClip (clipId);

    selectedClipIds.clear();
    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::duplicateSelectedClips()
{
    const auto targets = selectedClipIds.empty() ? std::set<int> {} : selectedClipIds;
    std::set<int> duplicatedClipIds;

    for (const auto clipId : targets)
    {
        const auto clip = audioEngine.getClipInfo (clipId);

        if (clip.id > 0)
        {
            const auto newClipId = audioEngine.addClipToTrack (clip.trackId, clip.file, clip.startTimeSeconds + tokens::timelineBarSnapSeconds);

            if (newClipId > 0)
            {
                audioEngine.trimClip (newClipId, clip.sourceOffsetSeconds, clip.lengthSeconds);
                audioEngine.setClipGainDb (newClipId, clip.gainDb);
                audioEngine.setClipFades (newClipId, clip.fadeInSeconds, clip.fadeOutSeconds);
                audioEngine.setClipTransposeSemitones (newClipId, clip.transposeSemitones);
                audioEngine.setClipLoopEnabled (newClipId, clip.loopEnabled);
                duplicatedClipIds.insert (newClipId);
            }
        }
    }

    if (! duplicatedClipIds.empty())
        selectedClipIds = duplicatedClipIds;

    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::selectAllClips()
{
    selectedClipIds.clear();

    for (const auto& clip : audioEngine.getAllClips())
        selectedClipIds.insert (clip.id);

    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::deselectAllClips()
{
    if (selectedClipIds.empty())
        return;

    selectedClipIds.clear();
    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::splitSelectedClipsAtPlayhead()
{
    if (selectedClipIds.empty())
        return;

    const auto playheadSeconds = audioEngine.getPositionSeconds();
    const auto targets = selectedClipIds;
    selectedClipIds.clear();

    for (const auto clipId : targets)
    {
        for (const auto newClipId : audioEngine.splitClipAt (clipId, playheadSeconds))
            selectedClipIds.insert (newClipId);
    }

    refreshFromEngine();
    notifySelectionChanged();
}

int TimelineView::getPrimarySelectedClipId() const
{
    if (selectedClipIds.empty())
        return 0;

    return *selectedClipIds.rbegin();
}

int TimelineView::getPrimarySelectedTrackId() const
{
    if (audioEngine.getTrackInfo (selectedTrackId).id > 0)
        return selectedTrackId;

    const auto tracks = audioEngine.getAllTrackInfo();
    return tracks.empty() ? 0 : tracks.front().id;
}

void TimelineView::updateContentSize()
{
    updateTimelineLengthFromEngine();

    const auto timelinePixelWidth = juce::roundToInt (timelineLengthSeconds * pixelsPerSecond);
    const auto contentWidth = juce::jmax (getWidth(), tokens::trackHeaderWidth + timelinePixelWidth);
    const auto trackAreaHeight = tokens::rulerHeight
                               + content.getTrackCount() * tokens::trackHeightDefault
                               + tokens::addTrackHeight;
    const auto contentHeight = juce::jmax (getHeight(), trackAreaHeight);

    content.setSize (contentWidth, contentHeight);
    content.setTimelineGeometry (pixelsPerSecond, timelineLengthSeconds);
}

void TimelineView::seekTo (double seconds)
{
    const auto wasPlaying = audioEngine.isPlaying();
    audioEngine.setPositionSeconds (seconds);

    if (wasPlaying)
        audioEngine.play();

    refreshPlayhead();
}

void TimelineView::handleWheel (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (event.mods.isCommandDown())
    {
        const auto anchorSeconds = getVisibleSecondsAt (event.position.x);
        const auto zoomPower = wheel.deltaY * tokens::wheelZoomScale;
        const auto zoomMultiplier = std::pow (tokens::wheelZoomBase, zoomPower);
        const auto newPixelsPerSecond = juce::jlimit (static_cast<double> (tokens::timelineMinPixelsPerSecond),
                                                      static_cast<double> (tokens::timelineMaxPixelsPerSecond),
                                                      pixelsPerSecond * zoomMultiplier);

        if (! juce::approximatelyEqual (newPixelsPerSecond, pixelsPerSecond))
        {
            pixelsPerSecond = newPixelsPerSecond;
            updateContentSize();

            const auto newViewX = tokens::trackHeaderWidth + anchorSeconds * pixelsPerSecond - event.position.x;
            viewport.setViewPosition (juce::jmax (0, juce::roundToInt (newViewX)),
                                      viewport.getViewPositionY());
        }

        return;
    }

    const auto horizontalDelta = wheel.deltaX * tokens::wheelPanScale;
    const auto verticalDelta = wheel.deltaY * tokens::wheelPanScale;
    const auto newViewX = viewport.getViewPositionX()
                        - juce::roundToInt (horizontalDelta + verticalDelta);
    const auto newViewY = viewport.getViewPositionY()
                        - juce::roundToInt (wheel.deltaY * tokens::wheelPanScale);

    viewport.setViewPosition (juce::jlimit (0,
                                            juce::jmax (0, content.getWidth() - viewport.getWidth()),
                                            newViewX),
                              juce::jlimit (0,
                                            juce::jmax (0, content.getHeight() - viewport.getHeight()),
                                            newViewY));
}

double TimelineView::getVisibleSecondsAt (float viewportX) const
{
    const auto contentX = viewport.getViewPositionX() + viewportX;
    return juce::jlimit (0.0,
                         timelineLengthSeconds,
                         (contentX - tokens::trackHeaderWidth) / pixelsPerSecond);
}

double TimelineView::snapSeconds (double seconds, bool fine) const
{
    const auto grid = fine ? tokens::timelineFineSnapSeconds : tokens::timelineBarSnapSeconds;
    return juce::jmax (0.0, std::round (seconds / grid) * grid);
}

void TimelineView::handleFilesDropped (const juce::StringArray& files, int trackId, double startSeconds)
{
    const auto baseTrackIndex = content.getTrackIndexForId (trackId);

    if (baseTrackIndex < 0)
        return;

    auto addedFileCount = 0;

    for (const auto& path : files)
    {
        const juce::File file (path);

        if (! isSupportedAudioFile (file))
            continue;

        auto targetIndex = baseTrackIndex + addedFileCount;

        while (targetIndex >= audioEngine.getTrackCount())
            audioEngine.addTrack (juce::String::formatted ("Track %02d", audioEngine.getTrackCount() + 1));

        const auto tracks = audioEngine.getAllTrackInfo();

        if (juce::isPositiveAndBelow (targetIndex, static_cast<int> (tracks.size())))
        {
            audioEngine.addClipToTrack (tracks[static_cast<size_t> (targetIndex)].id, file, startSeconds);
            selectedTrackId = tracks[static_cast<size_t> (targetIndex)].id;
        }

        ++addedFileCount;
    }

    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::handleClipMoved (int clipId, float deltaX, float deltaY, bool fine)
{
    if (! selectedClipIds.contains (clipId))
        handleClipSelected (clipId, false);

    const auto rowDelta = juce::roundToInt (deltaY / tokens::trackHeightDefault);
    const auto timeDelta = deltaX / pixelsPerSecond;

    for (const auto selectedClipId : selectedClipIds)
    {
        const auto clip = audioEngine.getClipInfo (selectedClipId);
        const auto currentTrackIndex = content.getTrackIndexForId (clip.trackId);

        if (clip.id <= 0 || currentTrackIndex < 0)
            continue;

        const auto targetIndex = juce::jlimit (0, content.getTrackCount() - 1, currentTrackIndex + rowDelta);
        const auto targetTrackId = content.getTrackIdForIndex (targetIndex);
        audioEngine.moveClip (clip.id, targetTrackId, snapSeconds (clip.startTimeSeconds + timeDelta, fine));
        selectedTrackId = targetTrackId;
    }

    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::handleClipTrimmed (int clipId, bool trimStart, float deltaX, bool fine)
{
    const auto clip = audioEngine.getClipInfo (clipId);

    if (clip.id <= 0)
        return;

    const auto rawDelta = deltaX / pixelsPerSecond;
    const auto grid = fine ? tokens::timelineFineSnapSeconds : tokens::timelineBarSnapSeconds;
    const auto deltaSeconds = std::round (rawDelta / grid) * grid;

    if (trimStart)
    {
        const auto maxDelta = clip.lengthSeconds - tokens::timelineFineSnapSeconds;
        const auto clampedDelta = juce::jlimit (-clip.sourceOffsetSeconds, maxDelta, deltaSeconds);
        audioEngine.trimClip (clip.id,
                              clip.sourceOffsetSeconds + clampedDelta,
                              clip.lengthSeconds - clampedDelta);
    }
    else
    {
        audioEngine.trimClip (clip.id,
                              clip.sourceOffsetSeconds,
                              juce::jmax (tokens::timelineFineSnapSeconds, clip.lengthSeconds + deltaSeconds));
    }

    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::handleClipSelected (int clipId, bool addToSelection)
{
    if (! addToSelection)
        selectedClipIds.clear();

    if (selectedClipIds.contains (clipId) && addToSelection)
        selectedClipIds.erase (clipId);
    else
        selectedClipIds.insert (clipId);

    const auto clip = audioEngine.getClipInfo (clipId);

    if (clip.trackId > 0)
        selectedTrackId = clip.trackId;

    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::handleClipDeleted (int clipId)
{
    if (! selectedClipIds.contains (clipId))
        selectedClipIds = { clipId };

    deleteSelectedClips();
}

void TimelineView::handleClipDuplicated (int clipId)
{
    if (! selectedClipIds.contains (clipId))
        selectedClipIds = { clipId };

    duplicateSelectedClips();
}

void TimelineView::handleClipRenamed (int clipId, juce::String name)
{
    audioEngine.renameClip (clipId, std::move (name));
    refreshFromEngine();
    notifySelectionChanged();
}

void TimelineView::updateTimelineLengthFromEngine()
{
    timelineLengthSeconds = juce::jmax (tokens::timelineEmptyLengthSeconds,
                                        audioEngine.getLengthSeconds() + tokens::timelineTrailingSeconds);
}

void TimelineView::notifySelectionChanged()
{
    if (onSelectionChanged)
        onSelectionChanged();
}
