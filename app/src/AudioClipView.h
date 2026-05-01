#pragma once

#include "DesignTokens.h"
#include "engine/AudioEngine.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

class AudioClipView final : public juce::Component
{
public:
    AudioClipView();

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;

    void setClipInfo (ClipInfo info, juce::Colour colour, bool shouldBeSelected);
    void setPixelsPerSecond (double newPixelsPerSecond);
    void clear();

    int getClipId() const noexcept { return clipInfo.id; }

    std::function<void(int, bool)> onSelect;
    std::function<void(int, float, float, bool)> onMoveFinished;
    std::function<void(int, bool, float, bool)> onTrimFinished;
    std::function<void(int)> onDeleteRequested;
    std::function<void(int)> onDuplicateRequested;
    std::function<void(int, juce::String)> onRenameRequested;

private:
    enum class DragMode
    {
        none,
        move,
        trimStart,
        trimEnd
    };

    void rebuildWaveformCache();
    void drawWaveformLayer (juce::Graphics& g,
                            juce::Rectangle<float> bounds,
                            const std::vector<float>& source,
                            juce::Colour colour);
    void showContextMenu();

    juce::AudioFormatManager formatManager;
    ClipInfo clipInfo;
    juce::Colour trackColour = tracklab::design::trackColors[0];

    std::vector<float> peaks;
    std::vector<float> rms;

    double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;
    int cachedPixelWidth = 0;
    bool selected = false;
    DragMode dragMode = DragMode::none;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioClipView)
};
