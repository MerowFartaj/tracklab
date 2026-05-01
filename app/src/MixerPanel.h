#pragma once

#include "DesignTokens.h"
#include "engine/AudioEngine.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

class MixerPanel final : public juce::Component,
                         private juce::Timer
{
public:
    explicit MixerPanel (AudioEngine& engine);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void refreshFromEngine();

private:
    class StripContainer final : public juce::Component
    {
    public:
        void paint (juce::Graphics& g) override
        {
            g.fillAll (tracklab::design::surfaceInset);
        }
    };

    class ChannelStrip final : public juce::Component
    {
    public:
        ChannelStrip();

        void paint (juce::Graphics& g) override;
        void resized() override;

        void setTrackInfo (TrackInfo info);
        void setMaster (float volume);
        void setMeterLevel (float level);

        int getTrackId() const noexcept { return trackInfo.id; }
        bool isMasterStrip() const noexcept { return isMaster; }

        std::function<void(int, float)> onVolumeChanged;
        std::function<void(int, float)> onPanChanged;
        std::function<void(int, bool)> onMuteChanged;
        std::function<void(int, bool)> onSoloChanged;
        std::function<void(float)> onMasterVolumeChanged;

    private:
        juce::String gainToText (float gain) const;
        juce::Rectangle<int> getMeterBounds() const;

        TrackInfo trackInfo;
        bool isMaster = false;
        float meterLevel = 0.0f;

        juce::Label nameLabel;
        juce::Slider panSlider;
        juce::TextButton muteButton { "M" };
        juce::TextButton soloButton { "S" };
        juce::Slider volumeSlider;
        juce::Label volumeLabel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStrip)
    };

    void timerCallback() override;
    void ensureStripCount (int count);

    AudioEngine& audioEngine;
    juce::Viewport viewport;
    StripContainer stripContainer;
    std::vector<std::unique_ptr<ChannelStrip>> strips;
    std::unique_ptr<ChannelStrip> masterStrip;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerPanel)
};
