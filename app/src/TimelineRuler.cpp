#include "TimelineRuler.h"

#include <cmath>

namespace tokens = tracklab::design;

TimelineRuler::TimelineRuler()
{
    setWantsKeyboardFocus (false);
}

void TimelineRuler::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient gradient { tokens::surfaceChrome.brighter (0.01f),
                                    bounds.getX(),
                                    bounds.getY(),
                                    tokens::surfaceInset,
                                    bounds.getX(),
                                    bounds.getBottom(),
                                    false };
    g.setGradientFill (gradient);
    g.fillAll();

    const auto secondsPerBeat = tokens::secondsPerMinute / tokens::defaultTempoBpm;
    const auto secondsPerBar = secondsPerBeat * tokens::beatsPerBar;
    const auto clip = g.getClipBounds();
    const auto firstBeat = juce::jmax (0, static_cast<int> (std::floor (clip.getX() / pixelsPerSecond / secondsPerBeat)));
    const auto lastBeat = static_cast<int> (std::ceil (clip.getRight() / pixelsPerSecond / secondsPerBeat));

    g.setColour (tokens::accentPrimary.withAlpha (0.42f));
    g.fillRect (bounds.removeFromTop (2.0f));

    for (auto beat = firstBeat; beat <= lastBeat; ++beat)
    {
        const auto seconds = static_cast<double> (beat) * secondsPerBeat;
        const auto x = static_cast<float> (seconds * pixelsPerSecond);
        const auto isBar = beat % tokens::beatsPerBar == 0;
        const auto barNumber = beat / tokens::beatsPerBar + 1;
        const auto isLabelledBar = (barNumber - 1) % 4 == 0;

        g.setColour (tokens::borderSubtle.withAlpha (isBar ? tokens::gridBarAlpha : tokens::subdivisionLineAlpha));
        g.drawVerticalLine (juce::roundToInt (x), isBar ? 2.0f : getHeight() * 0.58f, static_cast<float> (getHeight()));

        if (isLabelledBar)
        {
            g.setColour (tokens::textSecondary);
            g.setFont (tokens::fontMonospace());
            g.drawText (juce::String (barNumber),
                        juce::Rectangle<float> { x + 4.0f, 4.0f, static_cast<float> (secondsPerBar * pixelsPerSecond), 14.0f },
                        juce::Justification::centredLeft,
                        false);
        }
    }

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, static_cast<float> (getWidth()));
}

void TimelineRuler::mouseDown (const juce::MouseEvent& event)
{
    if (onSeek)
        onSeek (juce::jlimit (0.0, lengthSeconds, event.position.x / pixelsPerSecond));
}

void TimelineRuler::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (onMouseWheel)
        onMouseWheel (event, wheel);
}

void TimelineRuler::setTimelineLength (double seconds)
{
    lengthSeconds = juce::jmax (tokens::timelineEmptyLengthSeconds, seconds);
    repaint();
}

void TimelineRuler::setPixelsPerSecond (double newPixelsPerSecond)
{
    pixelsPerSecond = newPixelsPerSecond;
    repaint();
}

juce::String TimelineRuler::formatSeconds (double seconds)
{
    const auto totalMilliseconds = juce::jmax (0, juce::roundToInt (seconds * tokens::millisecondsPerSecond));
    const auto minutes = totalMilliseconds / tokens::millisecondsPerMinute;
    const auto secondsPart = (totalMilliseconds / tokens::millisecondsPerSecond) % tokens::secondsPerMinute;

    return juce::String::formatted ("%02d:%02d", minutes, secondsPart);
}

juce::String TimelineRuler::formatBarsBeats (double seconds)
{
    const auto secondsPerBeat = tokens::secondsPerMinute / tokens::defaultTempoBpm;
    const auto totalBeats = juce::jmax (0, static_cast<int> (std::floor (seconds / secondsPerBeat)));
    const auto bar = totalBeats / tokens::beatsPerBar + 1;
    const auto beat = totalBeats % tokens::beatsPerBar + 1;

    return juce::String::formatted ("%d.%d", bar, beat);
}
