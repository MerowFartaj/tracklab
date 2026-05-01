#pragma once

#include "TimelineView.h"
#include "Toolbar.h"
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
    void updateTransportState();
    void timerCallback() override;

    static bool isSupportedFile (const juce::File& file);

    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<juce::FileChooser> fileChooser;
    std::unique_ptr<TimelineView> timelineView;
    Toolbar toolbar;

    bool hasLoadedFile = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
