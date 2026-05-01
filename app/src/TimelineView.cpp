#include "TimelineView.h"

#include <cmath>

namespace tokens = tracklab::design;

TimelineView::TimelineContent::TimelineContent (AudioEngine& engine)
    : playhead (engine)
{
    addAndMakeVisible (ruler);
    addAndMakeVisible (trackRow);
    addAndMakeVisible (playhead);

    ruler.onSeek = [this] (double seconds)
    {
        if (onSeek)
            onSeek (seconds);
    };

    trackRow.onSeek = [this] (double seconds)
    {
        if (onSeek)
            onSeek (seconds);
    };

    ruler.onMouseWheel = [this] (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
    {
        if (onMouseWheel)
            onMouseWheel (event.getEventRelativeTo (this), wheel);
    };

    trackRow.onMouseWheel = [this] (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
    {
        if (onMouseWheel)
            onMouseWheel (event.getEventRelativeTo (this), wheel);
    };
}

void TimelineView::TimelineContent::resized()
{
    const auto width = getWidth();
    const auto timelineWidth = juce::jmax (1, width - tokens::trackHeaderWidth);

    ruler.setBounds (tokens::trackHeaderWidth, 0, timelineWidth, tokens::rulerHeight);
    trackRow.setBounds (0, tokens::rulerHeight, width, tokens::trackHeightDefault);
    playhead.setBounds (0, 0, width, tokens::rulerHeight + tokens::trackHeightDefault);
}

void TimelineView::TimelineContent::mouseDown (const juce::MouseEvent& event)
{
    if (onSeek)
    {
        const auto laneX = juce::jmax (0.0f, event.position.x - tokens::trackHeaderWidth);
        onSeek (juce::jlimit (0.0, timelineLengthSeconds, laneX / pixelsPerSecond));
    }
}

void TimelineView::TimelineContent::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (onMouseWheel)
        onMouseWheel (event, wheel);
}

void TimelineView::TimelineContent::setAudioFile (const juce::File& file, double lengthSeconds)
{
    trackRow.setAudioFile (file, lengthSeconds);
}

void TimelineView::TimelineContent::clearAudioFile()
{
    trackRow.clearAudioFile();
}

void TimelineView::TimelineContent::setTimelineGeometry (double newPixelsPerSecond, double newLengthSeconds)
{
    pixelsPerSecond = newPixelsPerSecond;
    timelineLengthSeconds = juce::jmax (tokens::timelineEmptyLengthSeconds, newLengthSeconds);

    ruler.setPixelsPerSecond (pixelsPerSecond);
    ruler.setTimelineLength (timelineLengthSeconds);
    trackRow.setPixelsPerSecond (pixelsPerSecond);
    trackRow.setTimelineLength (timelineLengthSeconds);
    playhead.setTimelineGeometry (pixelsPerSecond, timelineLengthSeconds);
    resized();
    repaint();
}

void TimelineView::TimelineContent::refreshPlayhead()
{
    playhead.forceUpdate();
}

TimelineView::TimelineView (AudioEngine& engine)
    : audioEngine (engine),
      content (engine)
{
    setOpaque (true);
    addAndMakeVisible (viewport);

    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (false, false);
    viewport.setWantsKeyboardFocus (false);

    content.onSeek = [this] (double seconds)
    {
        seekTo (seconds);
    };

    content.onMouseWheel = [this] (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
    {
        handleWheel (event.getEventRelativeTo (this), wheel);
    };

    openGLContext.setComponentPaintingEnabled (true);
    openGLContext.setContinuousRepainting (false);
    openGLContext.attachTo (*this);

    addMouseListener (this, true);
    updateContentSize();
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
            viewport.setViewPosition (juce::jmax (0, juce::roundToInt (newViewX)), 0);
        }

        return;
    }

    const auto horizontalDelta = wheel.deltaX * tokens::wheelPanScale;
    const auto verticalAsHorizontalDelta = wheel.deltaY * tokens::wheelPanScale;
    const auto newViewX = viewport.getViewPositionX()
                        - juce::roundToInt (horizontalDelta + verticalAsHorizontalDelta);

    viewport.setViewPosition (juce::jlimit (0,
                                            juce::jmax (0, content.getWidth() - viewport.getWidth()),
                                            newViewX),
                              0);
}

void TimelineView::setAudioFile (const juce::File& file, double lengthSeconds)
{
    timelineLengthSeconds = juce::jmax (tokens::timelineEmptyLengthSeconds, lengthSeconds + tokens::timelineTrailingSeconds);
    content.setAudioFile (file, lengthSeconds);
    updateContentSize();
}

void TimelineView::clearAudioFile()
{
    timelineLengthSeconds = tokens::timelineEmptyLengthSeconds;
    content.clearAudioFile();
    updateContentSize();
}

void TimelineView::refreshPlayhead()
{
    content.refreshPlayhead();
}

void TimelineView::updateContentSize()
{
    const auto timelinePixelWidth = juce::roundToInt (timelineLengthSeconds * pixelsPerSecond);
    const auto contentWidth = juce::jmax (getWidth(), tokens::trackHeaderWidth + timelinePixelWidth);
    const auto contentHeight = juce::jmax (getHeight(), tokens::rulerHeight + tokens::trackHeightDefault);

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

double TimelineView::getVisibleSecondsAt (float viewportX) const
{
    const auto contentX = viewport.getViewPositionX() + viewportX;
    return juce::jlimit (0.0,
                         timelineLengthSeconds,
                         (contentX - tokens::trackHeaderWidth) / pixelsPerSecond);
}
