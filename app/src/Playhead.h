#pragma once

#include "DesignTokens.h"
#include "engine/AudioEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>

class Playhead final : public juce::Component,
                       private juce::Timer
{
public:
    explicit Playhead (AudioEngine& engine);

    void paint (juce::Graphics& g) override;

    void setTimelineGeometry (double newPixelsPerSecond, double newLengthSeconds);
    void forceUpdate();

private:
    void timerCallback() override;
    int positionToX() const;
    void repaintPlayheadStrip (int oldX, int newX);

    AudioEngine& audioEngine;
    double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;
    double lengthSeconds = tracklab::design::timelineEmptyLengthSeconds;
    int previousX = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Playhead)
};
