#pragma once

#include "DesignTokens.h"

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

    std::function<void()> onLoadClicked;
    std::function<void()> onPlayPauseClicked;
    std::function<void()> onStopClicked;

private:
    static juce::String formatTime (double seconds);

    ToolbarButton loadButton { "Load" };
    ToolbarButton playPauseButton { "Play" };
    ToolbarButton stopButton { "Stop" };
    juce::Label positionLabel;

    bool hasLoadedFile = false;
    bool isPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Toolbar)
};
