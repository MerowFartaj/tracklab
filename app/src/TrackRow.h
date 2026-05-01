#pragma once

#include "DesignTokens.h"
#include "engine/AudioEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>

class TrackRow final : public juce::Component,
                       public juce::FileDragAndDropTarget
{
public:
    TrackRow();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    void setTrackInfo (TrackInfo info, int trackNumber);
    void setTimelineLength (double seconds);
    void setPixelsPerSecond (double newPixelsPerSecond);

    std::function<void(double)> onSeek;
    std::function<void()> onEmptyLaneClicked;
    std::function<void(int, juce::String)> onTrackNameChanged;
    std::function<void(int, float)> onVolumeChanged;
    std::function<void(int, float)> onPanChanged;
    std::function<void(int, bool)> onMuteChanged;
    std::function<void(int, bool)> onSoloChanged;
    std::function<void(const juce::StringArray&, int, double)> onFilesDropped;
    std::function<void(const juce::MouseEvent&, const juce::MouseWheelDetails&)> onMouseWheel;

private:
    static bool isSupportedAudioFile (const juce::File& file);
    void updateControls();
    double xToSeconds (float x) const;

    TrackInfo trackInfo;
    int trackNumber = 1;

    juce::Label numberLabel;
    juce::Label nameLabel;
    juce::Slider volumeSlider;
    juce::Slider panSlider;
    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };
    juce::TextButton recordButton { "R" };

    double timelineLengthSeconds = tracklab::design::timelineEmptyLengthSeconds;
    double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackRow)
};
