#pragma once

#include "AudioClipView.h"
#include "DesignTokens.h"
#include "Playhead.h"
#include "TimelineRuler.h"
#include "TrackRow.h"
#include "engine/AudioEngine.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>

#include <set>

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
    void refreshFromEngine();
    void refreshPlayhead();

    void addTrack();
    void deleteSelectedClips();
    void duplicateSelectedClips();
    void selectAllClips();
    void deselectAllClips();
    void splitSelectedClipsAtPlayhead();
    int getPrimarySelectedClipId() const;
    int getPrimarySelectedTrackId() const;

    std::function<void()> onProjectChanged;
    std::function<void()> onSelectionChanged;

private:
    class TimelineContent final : public juce::Component
    {
    public:
        explicit TimelineContent (AudioEngine& engine);

        void paint (juce::Graphics& g) override;
        void paintOverChildren (juce::Graphics& g) override;
        void resized() override;
        void mouseDown (const juce::MouseEvent& event) override;
        void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

        void refreshFromEngine (const std::set<int>& selectedClipIds);
        void setTimelineGeometry (double newPixelsPerSecond, double newLengthSeconds);
        void refreshPlayhead();
        int getTrackIndexForId (int trackId) const;
        int getTrackIdForIndex (int trackIndex) const;
        int getTrackCount() const;

        std::function<void(double)> onSeek;
        std::function<void()> onDeselectAll;
        std::function<void(int)> onTrackSelected;
        std::function<void()> onAddTrack;
        std::function<void(int, bool)> onClipSelected;
        std::function<void(int, float, float, bool)> onClipMoved;
        std::function<void(int, bool, float, bool)> onClipTrimmed;
        std::function<void(int)> onClipDeleted;
        std::function<void(int)> onClipDuplicated;
        std::function<void(int, juce::String)> onClipRenamed;
        std::function<void(const juce::StringArray&, int, double)> onFilesDropped;
        std::function<void(int, juce::String)> onTrackNameChanged;
        std::function<void(int, float)> onTrackVolumeChanged;
        std::function<void(int, float)> onTrackPanChanged;
        std::function<void(int, bool)> onTrackMuteChanged;
        std::function<void(int, bool)> onTrackSoloChanged;
        std::function<void(const juce::MouseEvent&, const juce::MouseWheelDetails&)> onMouseWheel;

    private:
        void ensureRowCount (int count);
        void rebuildClipViews (const std::set<int>& selectedClipIds);
        void layoutContent();
        double xToSeconds (float x) const;

        AudioEngine& audioEngine;
        TimelineRuler ruler;
        std::vector<std::unique_ptr<TrackRow>> rows;
        std::vector<std::unique_ptr<AudioClipView>> clipViews;
        Playhead playhead;
        juce::TextButton addTrackButton { "Add Track" };
        std::vector<TrackInfo> trackInfos;
        std::vector<ClipInfo> clipInfos;

        double timelineLengthSeconds = tracklab::design::timelineEmptyLengthSeconds;
        double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;
        bool hintVisible = true;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineContent)
    };

    void updateContentSize();
    void seekTo (double seconds);
    void handleWheel (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);
    double getVisibleSecondsAt (float viewportX) const;
    double snapSeconds (double seconds, bool fine) const;
    void handleFilesDropped (const juce::StringArray& files, int trackId, double startSeconds);
    void handleClipMoved (int clipId, float deltaX, float deltaY, bool fine);
    void handleClipTrimmed (int clipId, bool trimStart, float deltaX, bool fine);
    void handleClipSelected (int clipId, bool addToSelection);
    void handleClipDeleted (int clipId);
    void handleClipDuplicated (int clipId);
    void handleClipRenamed (int clipId, juce::String name);
    void updateTimelineLengthFromEngine();
    void notifySelectionChanged();

    AudioEngine& audioEngine;
    juce::Viewport viewport;
    TimelineContent content;
    juce::OpenGLContext openGLContext;
    std::set<int> selectedClipIds;
    int selectedTrackId = 0;

    double timelineLengthSeconds = tracklab::design::timelineEmptyLengthSeconds;
    double pixelsPerSecond = tracklab::design::timelineDefaultPixelsPerSecond;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineView)
};
