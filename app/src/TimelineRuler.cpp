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
    g.setGradientFill (tokens::verticalSurfaceGradient (tokens::surfaceElevated, bounds));
    g.fillAll();

    const auto clip = g.getClipBounds();
    const auto firstTick = juce::jmax (0, static_cast<int> (std::floor (clip.getX() / pixelsPerSecond / tokens::minorTickSeconds)));
    const auto lastTick = static_cast<int> (std::ceil (clip.getRight() / pixelsPerSecond / tokens::minorTickSeconds));

    for (auto tick = firstTick; tick <= lastTick; ++tick)
    {
        const auto seconds = tick * tokens::minorTickSeconds;
        const auto x = static_cast<float> (seconds * pixelsPerSecond);
        const auto isMajor = juce::approximatelyEqual (std::fmod (seconds, tokens::majorTickSeconds), 0.0)
                          || juce::approximatelyEqual (std::fmod (seconds, tokens::majorTickSeconds), tokens::majorTickSeconds);

        g.setColour ((isMajor ? tokens::borderSubtle : tokens::textTertiary)
                         .withAlpha (isMajor ? tokens::panelBorderAlpha : tokens::subdivisionLineAlpha));
        g.drawVerticalLine (juce::roundToInt (x), isMajor ? 0.0f : bounds.getHeight() * 0.55f, bounds.getBottom());

        if (isMajor)
        {
            g.setColour (tokens::textTertiary);
            g.setFont (tokens::fontMonospace());
            g.drawText (formatBarsBeats (seconds) + "  " + formatSeconds (seconds),
                        juce::Rectangle<float> { x + tokens::toolbarGap,
                                                 0.0f,
                                                 static_cast<float> (pixelsPerSecond) - tokens::toolbarGap,
                                                 bounds.getHeight() * 0.82f },
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
