#pragma once

#include "engine/AudioEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent final : public juce::Component,
                            private juce::Timer
{
public:
    MainComponent();

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void loadAudioFile();
    void handleFileChosen (const juce::File& file);
    void updatePlaybackState();
    void timerCallback() override;

    static bool isSupportedFile (const juce::File& file);
    static juce::String formatTime (double seconds);

    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TextButton loadButton { "Load audio file" };
    juce::TextButton playButton { "Play" };
    juce::TextButton pauseButton { "Pause" };
    juce::TextButton stopButton { "Stop" };
    juce::Label positionLabel;
    juce::Slider progressSlider;
    bool hasLoadedFile = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
