#include "TrackRow.h"

#include <cmath>

namespace tokens = tracklab::design;

TrackRow::TrackRow()
{
    addAndMakeVisible (clipView);
    clipView.setVisible (false);
}

void TrackRow::paint (juce::Graphics& g)
{
    g.fillAll (tokens::backgroundBase);

    auto header = getLocalBounds().removeFromLeft (tokens::trackHeaderWidth);
    auto lane = getLocalBounds().withTrimmedLeft (tokens::trackHeaderWidth);

    g.setGradientFill (tokens::verticalSurfaceGradient (tokens::surfaceElevated, header.toFloat()));
    g.fillRect (header);

    g.setColour (trackColour);
    g.fillRect (header.removeFromLeft (tokens::trackColorBarWidth));

    auto headerText = header.reduced (tokens::trackHeaderPadding, 0);
    g.setColour (tokens::textPrimary);
    g.setFont (tokens::fontHeader());
    g.drawText ("Track 1", headerText.removeFromTop (tokens::trackHeightDefault / 2), juce::Justification::bottomLeft, false);

    g.setColour (tokens::textTertiary);
    g.setFont (tokens::fontMetadata());
    g.drawText ("Audio", headerText, juce::Justification::topLeft, false);

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawVerticalLine (tokens::trackHeaderWidth - 1, 0.0f, static_cast<float> (getHeight()));

    g.setColour (tokens::surfaceRaised.withAlpha (tokens::laneOverlayAlpha));
    g.fillRect (lane);

    const auto clip = g.getClipBounds();
    const auto firstSecond = juce::jmax (0, static_cast<int> (std::floor ((clip.getX() - tokens::trackHeaderWidth) / pixelsPerSecond)));
    const auto lastSecond = static_cast<int> (std::ceil ((clip.getRight() - tokens::trackHeaderWidth) / pixelsPerSecond));

    for (auto second = firstSecond; second <= lastSecond; ++second)
    {
        const auto x = tokens::trackHeaderWidth + juce::roundToInt (second * pixelsPerSecond);
        g.setColour (tokens::borderSubtle.withAlpha (tokens::gridLineAlpha));
        g.drawVerticalLine (x, 0.0f, static_cast<float> (getHeight()));
    }

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, static_cast<float> (getWidth()));
}

void TrackRow::resized()
{
    updateClipBounds();
}

void TrackRow::mouseDown (const juce::MouseEvent& event)
{
    if (onSeek)
    {
        const auto laneX = juce::jmax (0.0f, event.position.x - tokens::trackHeaderWidth);
        onSeek (juce::jlimit (0.0, timelineLengthSeconds, laneX / pixelsPerSecond));
    }
}

void TrackRow::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (onMouseWheel)
        onMouseWheel (event, wheel);
}

void TrackRow::setAudioFile (const juce::File& file, double lengthSeconds)
{
    audioFile = file;
    clipLengthSeconds = lengthSeconds;
    clipView.setAudioFile (audioFile, clipLengthSeconds, trackColour);
    clipView.setVisible (audioFile.existsAsFile() && clipLengthSeconds > 0.0);
    updateClipBounds();
}

void TrackRow::clearAudioFile()
{
    audioFile = juce::File();
    clipLengthSeconds = 0.0;
    clipView.clear();
    clipView.setVisible (false);
}

void TrackRow::setTimelineLength (double seconds)
{
    timelineLengthSeconds = juce::jmax (tokens::timelineEmptyLengthSeconds, seconds);
    repaint();
}

void TrackRow::setPixelsPerSecond (double newPixelsPerSecond)
{
    pixelsPerSecond = newPixelsPerSecond;
    clipView.setPixelsPerSecond (pixelsPerSecond);
    updateClipBounds();
    repaint();
}

void TrackRow::updateClipBounds()
{
    if (! clipView.isVisible())
        return;

    const auto clipWidth = juce::jmax (1, juce::roundToInt (clipLengthSeconds * pixelsPerSecond));
    clipView.setBounds (tokens::trackHeaderWidth,
                        tokens::clipVerticalPadding,
                        clipWidth,
                        juce::jmax (1, getHeight() - tokens::clipVerticalPadding * 2));
}
