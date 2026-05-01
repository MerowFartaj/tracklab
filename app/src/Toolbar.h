#pragma once

#include "DesignTokens.h"
#include "engine/AudioEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>

class ToolbarButton final : public juce::TextButton
{
public:
    explicit ToolbarButton (const juce::String& text);

    void paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
};

class Toolbar final : public juce::Component
{
public:
    Toolbar();

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setHasLoadedFile (bool hasFile);
    void setPlaying (bool shouldShowPlaying);
    void setPosition (double positionSeconds, double lengthSeconds);
    void setProjectInfo (const ProjectInfo& info);

    std::function<void()> onLoadClicked;
    std::function<void()> onPlayPauseClicked;
    std::function<void()> onStopClicked;
    std::function<void()> onLoopClicked;
    std::function<void()> onMetronomeClicked;
    std::function<void()> onRecordClicked;

private:
    static juce::String formatTime (double seconds);

    ToolbarButton loadButton { "Load" };
    ToolbarButton playPauseButton { "Play" };
    ToolbarButton stopButton { "Stop" };
    ToolbarButton recordButton { "Rec" };
    ToolbarButton loopButton { "Loop" };
    ToolbarButton clickButton { "Click" };
    juce::Label wordmarkLabel;
    juce::Label tempoLabel;
    juce::Label signatureLabel;
    juce::Label positionLabel;

    bool hasLoadedFile = false;
    bool isPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Toolbar)
};
