#pragma once

#include "AudioClipView.h"
#include "DesignTokens.h"

#include <juce_gui_basics/juce_gui_basics.h>

class TrackRow final : public juce::Component
{
public:
    TrackRow();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    void setAudioFile (const juce::File& file, double lengthSeconds);
    void clearAudioFile();
    void setTimelineLength (double seconds);
    void setPixelsPerSecond (double newPixelsPerSecond);

    std::function<void(double)> onSeek;
    std::function<void(const juce::MouseEvent&, const juce::MouseWheelDetails&)> onMouseWheel;

private:
    void updateClipBounds();

    AudioClipView clipView;
    juce::File audioFile;
    juce::Colour trackColour = tracklab::design::trackColors[0];

    double clipLengthSeconds = 0.0;
    double timelineLengthSeconds = tracklab::design::timelineEmptyLengthSeconds;
    double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackRow)
};
