#pragma once

#include "MixerPanel.h"
#include "TimelineView.h"
#include "Toolbar.h"
#include "WorkspacePanel.h"
#include "engine/AudioEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent final : public juce::Component,
                            private juce::Timer
{
public:
    MainComponent();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    void loadAudioFile();
    void handleFileChosen (const juce::File& file);
    void updateTransportState();
    void timerCallback() override;
    juce::Rectangle<int> getSplitterBounds() const;

    static bool isSupportedFile (const juce::File& file);

    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<juce::FileChooser> fileChooser;
    std::unique_ptr<TimelineView> timelineView;
    std::unique_ptr<MixerPanel> mixerPanel;
    std::unique_ptr<WorkspacePanel> workspacePanel;
    Toolbar toolbar;

    int mixerHeight = tracklab::design::mixerDefaultHeight;
    bool draggingSplitter = false;
    int splitterDragStartY = 0;
    int mixerHeightAtDragStart = tracklab::design::mixerDefaultHeight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
