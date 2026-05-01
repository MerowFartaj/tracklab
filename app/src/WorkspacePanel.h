#pragma once

#include "DesignTokens.h"
#include "UiDrawing.h"
#include "engine/AudioEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

class WorkspacePanel final : public juce::Component
{
public:
    explicit WorkspacePanel (AudioEngine& engine);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void refreshFromEngine();
    void setSelection (int clipId, int trackId);

    std::function<void()> onSplitSelectedClip;

private:
    class BrowserItem final : public juce::Component
    {
    public:
        void paint (juce::Graphics& g) override;
        void setItem (juce::String newText, tracklab::ui::Icon newIcon, juce::Colour newColour, bool shouldBeSelected);

    private:
        juce::String text;
        tracklab::ui::Icon icon = tracklab::ui::Icon::none;
        juce::Colour colour = tracklab::design::textSecondary;
        bool selected = false;
    };

    void configureLabel (juce::Label& label, juce::Font font, juce::Colour colour);
    void configureButton (juce::TextButton& button);
    void configureSlider (juce::Slider& slider, double minimum, double maximum, double interval);
    void configureComboBox (juce::ComboBox& comboBox);
    void paintSectionHeader (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title);

    void refreshProjectControls();
    void refreshClipControls();
    void refreshDeviceControls();
    void refreshButtonColours();
    void populateBrowser();
    bool hasSelectedClip() const;
    bool hasSelectedTrack() const;

    AudioEngine& audioEngine;
    bool refreshing = false;
    int selectedClipId = 0;
    int selectedTrackId = 0;

    juce::Label browserHeader;
    juce::TextEditor browserSearch;
    std::array<BrowserItem, 8> browserItems;

    juce::Label projectHeader;
    juce::Slider tempoSlider;
    juce::ComboBox signatureBox;
    juce::ComboBox keyBox;
    juce::ComboBox snapBox;
    juce::TextButton loopButton { "Loop" };
    juce::TextButton clickButton { "Click" };
    juce::TextButton recordButton { "Rec" };

    juce::Label clipHeader;
    juce::Label selectedClipLabel;
    juce::Slider clipGainSlider;
    juce::Slider fadeInSlider;
    juce::Slider fadeOutSlider;
    juce::Slider clipTransposeSlider;
    juce::TextButton clipLoopButton { "Loop" };
    juce::TextButton splitButton { "Split" };

    juce::Label deviceHeader;
    juce::Label selectedTrackLabel;
    juce::TextButton eqButton { "EQ" };
    std::array<juce::Slider, 4> eqGainSliders;
    juce::TextButton pitchButton { "Pitch" };
    juce::Slider pitchSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WorkspacePanel)
};
