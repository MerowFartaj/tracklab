#pragma once

#include "DesignTokens.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

class AudioClipView final : public juce::Component
{
public:
    AudioClipView();

    void paint (juce::Graphics& g) override;

    void setAudioFile (const juce::File& file, double lengthSeconds, juce::Colour colour);
    void setPixelsPerSecond (double newPixelsPerSecond);
    void clear();

private:
    void rebuildWaveformCache();
    void drawWaveformLayer (juce::Graphics& g,
                            juce::Rectangle<float> bounds,
                            const std::vector<float>& source,
                            juce::Colour colour);

    juce::AudioFormatManager formatManager;
    juce::File audioFile;
    juce::Colour trackColour = tracklab::design::trackColors[0];

    std::vector<float> peaks;
    std::vector<float> rms;

    double clipLengthSeconds = 0.0;
    double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;
    int cachedPixelWidth = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioClipView)
};
