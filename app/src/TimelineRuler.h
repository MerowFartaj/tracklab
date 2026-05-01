#pragma once

#include "DesignTokens.h"

#include <juce_gui_basics/juce_gui_basics.h>

class TimelineRuler final : public juce::Component
{
public:
    TimelineRuler();

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    void setTimelineLength (double seconds);
    void setPixelsPerSecond (double newPixelsPerSecond);

    std::function<void(double)> onSeek;
    std::function<void(const juce::MouseEvent&, const juce::MouseWheelDetails&)> onMouseWheel;

private:
    static juce::String formatSeconds (double seconds);
    static juce::String formatBarsBeats (double seconds);

    double lengthSeconds = tracklab::design::timelineEmptyLengthSeconds;
    double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineRuler)
};
