#include "Playhead.h"

namespace tokens = tracklab::design;

Playhead::Playhead (AudioEngine& engine)
    : audioEngine (engine)
{
    setInterceptsMouseClicks (false, false);
    previousX = positionToX();
    startTimerHz (60);
}

void Playhead::paint (juce::Graphics& g)
{
    const auto x = positionToX();

    juce::Path marker;
    marker.startNewSubPath (static_cast<float> (x - tokens::playheadMarkerWidth / 2), 0.0f);
    marker.lineTo (static_cast<float> (x + tokens::playheadMarkerWidth / 2), 0.0f);
    marker.lineTo (static_cast<float> (x), static_cast<float> (tokens::playheadMarkerHeight));
    marker.closeSubPath();

    g.setColour (tokens::accentPrimary);
    g.fillPath (marker);
    g.fillRect (juce::Rectangle<float> { static_cast<float> (x) - tokens::playheadWidth * 0.5f,
                                         0.0f,
                                         tokens::playheadWidth,
                                         static_cast<float> (getHeight()) });
}

void Playhead::setTimelineGeometry (double newPixelsPerSecond, double newLengthSeconds)
{
    const auto oldX = previousX;
    pixelsPerSecond = newPixelsPerSecond;
    lengthSeconds = juce::jmax (tokens::timelineEmptyLengthSeconds, newLengthSeconds);
    previousX = positionToX();
    repaintPlayheadStrip (oldX, previousX);
}

void Playhead::forceUpdate()
{
    const auto newX = positionToX();
    repaintPlayheadStrip (previousX, newX);
    previousX = newX;
}

void Playhead::timerCallback()
{
    const auto newX = positionToX();

    if (newX == previousX)
        return;

    repaintPlayheadStrip (previousX, newX);
    previousX = newX;
}

int Playhead::positionToX() const
{
    const auto clampedPosition = juce::jlimit (0.0, lengthSeconds, audioEngine.getPositionSeconds());
    return tokens::trackHeaderWidth + juce::roundToInt (clampedPosition * pixelsPerSecond);
}

void Playhead::repaintPlayheadStrip (int oldX, int newX)
{
    const auto left = juce::jmin (oldX, newX) - tokens::playheadMarkerWidth - tokens::playheadRepaintPadding;
    const auto right = juce::jmax (oldX, newX) + tokens::playheadMarkerWidth + tokens::playheadRepaintPadding;
    repaint (juce::Rectangle<int> { left, 0, right - left, getHeight() });
}
