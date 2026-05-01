#pragma once

#include "DesignTokens.h"
#include "UiDrawing.h"
#include "engine/AudioEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>

class ToolbarButton final : public juce::TextButton
{
public:
    explicit ToolbarButton (const juce::String& text,
                            tracklab::ui::Icon iconToUse = tracklab::ui::Icon::none);

    void paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    void setIcon (tracklab::ui::Icon iconToUse);

private:
    tracklab::ui::Icon icon = tracklab::ui::Icon::none;
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

    ToolbarButton loadButton { "Load", tracklab::ui::Icon::folder };
    ToolbarButton playPauseButton { "Play", tracklab::ui::Icon::play };
    ToolbarButton stopButton { "Stop", tracklab::ui::Icon::stop };
    ToolbarButton recordButton { "Rec", tracklab::ui::Icon::record };
    ToolbarButton loopButton { "Loop", tracklab::ui::Icon::loop };
    ToolbarButton clickButton { "Click", tracklab::ui::Icon::metronome };
    juce::Label wordmarkLabel;
    juce::Label tempoLabel;
    juce::Label signatureLabel;
    juce::Label positionLabel;

    bool hasLoadedFile = false;
    bool isPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Toolbar)
};
