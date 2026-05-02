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

class TrafficLightButton final : public juce::Button
{
public:
    explicit TrafficLightButton (juce::Colour colourToUse);

    void paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

private:
    juce::Colour colour;
};

class Toolbar final : public juce::Component
{
public:
    Toolbar();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;

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
    std::function<void()> onCloseClicked;
    std::function<void()> onMinimiseClicked;
    std::function<void()> onZoomClicked;

private:
    static juce::String formatTime (double seconds);
    static juce::String formatBarsBeatsTicks (double seconds);

    TrafficLightButton closeButton { tracklab::design::trafficClose };
    TrafficLightButton minimiseButton { tracklab::design::trafficMinimise };
    TrafficLightButton zoomButton { tracklab::design::trafficZoom };
    ToolbarButton loadButton { "Load", tracklab::ui::Icon::folder };
    ToolbarButton rewindButton { "", tracklab::ui::Icon::rewind };
    ToolbarButton playPauseButton { "", tracklab::ui::Icon::play };
    ToolbarButton stopButton { "Stop", tracklab::ui::Icon::stop };
    ToolbarButton recordButton { "Rec", tracklab::ui::Icon::record };
    ToolbarButton loopButton { "", tracklab::ui::Icon::loop };
    ToolbarButton clickButton { "Metronome", tracklab::ui::Icon::metronome };
    ToolbarButton settingsButton { "", tracklab::ui::Icon::settings };
    ToolbarButton speakerButton { "", tracklab::ui::Icon::speaker };
    juce::Label wordmarkLabel;
    juce::Label tempoLabel;
    juce::Label signatureLabel;
    juce::Label keyLabel;
    juce::Label snapLabel;
    juce::Label quantizeLabel;
    juce::Label positionLabel;
    juce::Slider outputSlider;
    juce::ComponentDragger windowDragger;

    bool hasLoadedFile = false;
    bool isPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Toolbar)
};
