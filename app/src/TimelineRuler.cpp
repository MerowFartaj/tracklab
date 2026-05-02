#include "TimelineRuler.h"

#include <cmath>

namespace tokens = tracklab::design;

namespace
{
struct RulerScale
{
    int labelEveryBars = 4;
    bool showBeatTicks = true;
    bool showBeatLabels = false;
    bool showSixteenthTicks = false;
    bool showSixteenthLabels = false;
};

double getSecondsPerBeat()
{
    return tokens::secondsPerMinute / tokens::defaultTempoBpm;
}

double getSecondsPerSixteenth()
{
    return getSecondsPerBeat() / tokens::sixteenthsPerBeat;
}

RulerScale getRulerScale (double pixelsPerSecond)
{
    const auto pixelsPerBeat = pixelsPerSecond * getSecondsPerBeat();
    const auto pixelsPerBar = pixelsPerBeat * tokens::beatsPerBar;
    const auto pixelsPerSixteenth = pixelsPerBeat / tokens::sixteenthsPerBeat;

    RulerScale scale;

    if (pixelsPerBar < 18.0)
        scale.labelEveryBars = 16;
    else if (pixelsPerBar < 28.0)
        scale.labelEveryBars = 8;
    else if (pixelsPerBar < 60.0)
        scale.labelEveryBars = 4;
    else if (pixelsPerBar < 110.0)
        scale.labelEveryBars = 2;
    else
        scale.labelEveryBars = 1;

    scale.showBeatTicks = pixelsPerBeat >= tokens::rulerBeatTickMinPixels;
    scale.showBeatLabels = pixelsPerBeat >= tokens::rulerBeatLabelMinPixels;
    scale.showSixteenthTicks = pixelsPerSixteenth >= tokens::rulerSixteenthTickMinPixels;
    scale.showSixteenthLabels = pixelsPerSixteenth >= tokens::rulerSixteenthLabelMinPixels;

    return scale;
}

juce::String getBarLabel (int barIndex)
{
    return juce::String (barIndex + 1);
}

juce::String getBeatLabel (int barIndex, int beatInBar)
{
    return juce::String (barIndex + 1) + "." + juce::String (beatInBar + 1);
}

juce::String getSixteenthLabel (int barIndex, int beatInBar, int sixteenthInBeat)
{
    return getBeatLabel (barIndex, beatInBar) + "." + juce::String (sixteenthInBeat + 1);
}

bool drawLabelIfRoom (juce::Graphics& g,
                      juce::String label,
                      float x,
                      float width,
                      float& lastLabelRight)
{
    const auto labelX = x + tokens::rulerLabelInset;

    if (labelX < lastLabelRight + tokens::rulerLabelMinGap)
        return false;

    g.drawText (label,
                juce::Rectangle<float> { labelX, 5.0f, width, 14.0f },
                juce::Justification::centredLeft,
                false);
    lastLabelRight = labelX + width;
    return true;
}
}

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

    const auto secondsPerSixteenth = getSecondsPerSixteenth();
    const auto scale = getRulerScale (pixelsPerSecond);
    const auto clip = g.getClipBounds();
    const auto firstSixteenth = juce::jmax (0, static_cast<int> (std::floor (clip.getX() / pixelsPerSecond / secondsPerSixteenth)));
    const auto lastSixteenth = static_cast<int> (std::ceil (clip.getRight() / pixelsPerSecond / secondsPerSixteenth));

    g.setColour (tokens::accentPrimary.withAlpha (tokens::rulerAmberRailAlpha));
    g.fillRect (bounds.removeFromTop (2.0f));
    g.setFont (tokens::fontMonospace());

    auto lastLabelRight = -tokens::rulerLabelMinGap;

    for (auto sixteenth = firstSixteenth; sixteenth <= lastSixteenth; ++sixteenth)
    {
        const auto beat = sixteenth / tokens::sixteenthsPerBeat;
        const auto sixteenthInBeat = sixteenth % tokens::sixteenthsPerBeat;
        const auto beatInBar = beat % tokens::beatsPerBar;
        const auto barIndex = beat / tokens::beatsPerBar;
        const auto isBeat = sixteenthInBeat == 0;
        const auto isBar = isBeat && beatInBar == 0;

        if (! isBar && isBeat && ! scale.showBeatTicks)
            continue;

        if (! isBar && ! isBeat && ! scale.showSixteenthTicks)
            continue;

        const auto seconds = static_cast<double> (sixteenth) * secondsPerSixteenth;
        const auto x = static_cast<float> (seconds * pixelsPerSecond);
        const auto tickTop = isBar ? tokens::rulerBarTickTop
                         : isBeat ? getHeight() * tokens::rulerBeatTickTopRatio
                                  : getHeight() * tokens::rulerSixteenthTickTopRatio;
        const auto tickAlpha = isBar ? tokens::gridBarAlpha
                           : isBeat ? tokens::beatLineAlpha
                                    : tokens::subdivisionLineAlpha * 0.72f;

        g.setColour (tokens::borderSubtle.withAlpha (tickAlpha));
        g.drawVerticalLine (juce::roundToInt (x), tickTop, static_cast<float> (getHeight()));

        if (isBar && barIndex % scale.labelEveryBars == 0)
        {
            g.setColour (tokens::textSecondary);
            drawLabelIfRoom (g, getBarLabel (barIndex), x, tokens::rulerBarLabelWidth, lastLabelRight);
        }
        else if (isBeat && scale.showBeatLabels)
        {
            g.setColour (tokens::textTertiary);
            drawLabelIfRoom (g, getBeatLabel (barIndex, beatInBar), x, tokens::rulerBeatLabelWidth, lastLabelRight);
        }
        else if (scale.showSixteenthLabels)
        {
            g.setColour (tokens::textTertiary.withAlpha (0.82f));
            drawLabelIfRoom (g,
                             getSixteenthLabel (barIndex, beatInBar, sixteenthInBeat),
                             x,
                             tokens::rulerSixteenthLabelWidth,
                             lastLabelRight);
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
