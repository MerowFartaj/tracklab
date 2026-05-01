#pragma once

#include "DesignTokens.h"
#include "Playhead.h"
#include "TimelineRuler.h"
#include "TrackRow.h"
#include "engine/AudioEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>

class TimelineView final : public juce::Component
{
public:
    explicit TimelineView (AudioEngine& engine);
    ~TimelineView() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    void setAudioFile (const juce::File& file, double lengthSeconds);
    void clearAudioFile();
    void refreshPlayhead();

private:
    class TimelineContent final : public juce::Component
    {
    public:
        explicit TimelineContent (AudioEngine& engine);

        void resized() override;
        void mouseDown (const juce::MouseEvent& event) override;
        void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

        void setAudioFile (const juce::File& file, double lengthSeconds);
        void clearAudioFile();
        void setTimelineGeometry (double newPixelsPerSecond, double newLengthSeconds);
        void refreshPlayhead();

        std::function<void(double)> onSeek;
        std::function<void(const juce::MouseEvent&, const juce::MouseWheelDetails&)> onMouseWheel;

    private:
        TimelineRuler ruler;
        TrackRow trackRow;
        Playhead playhead;

        double timelineLengthSeconds = tracklab::design::timelineEmptyLengthSeconds;
        double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineContent)
    };

    void updateContentSize();
    void seekTo (double seconds);
    void handleWheel (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);
    double getVisibleSecondsAt (float viewportX) const;

    AudioEngine& audioEngine;
    juce::Viewport viewport;
    TimelineContent content;
    juce::OpenGLContext openGLContext;

    double timelineLengthSeconds = tracklab::design::timelineEmptyLengthSeconds;
    double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineView)
};
