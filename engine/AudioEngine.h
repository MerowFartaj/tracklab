#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

#include <memory>
#include <vector>

struct TrackInfo
{
    int id = 0;
    juce::String name;
    juce::Colour colour;
    float volume = 1.0f;
    float pan = 0.0f;
    bool muted = false;
    bool soloed = false;
};

struct ClipInfo
{
    int id = 0;
    int trackId = 0;
    juce::String name;
    juce::File file;
    double startTimeSeconds = 0.0;
    double lengthSeconds = 0.0;
    double sourceOffsetSeconds = 0.0;
};

class AudioEngine final
{
public:
    AudioEngine();
    ~AudioEngine();

    /** Loads an audio file onto the first track and replaces the current clips. */
    bool loadFile (juce::File file);

    /** Adds a new audio track and returns its stable Tracklab track ID. */
    int addTrack (juce::String name);

    /** Removes a track and any clips it owns. */
    void removeTrack (int trackId);

    /** Returns the number of audio tracks in the edit. */
    int getTrackCount() const;

    /** Returns display and mixer state for a track. */
    TrackInfo getTrackInfo (int trackId) const;

    /** Returns every track in timeline order. */
    std::vector<TrackInfo> getAllTrackInfo() const;

    /** Renames a track while keeping the Tracktion edit in sync. */
    void setTrackName (int trackId, juce::String name);

    /** Sets a track gain, where 1.0 is unity. */
    void setTrackVolume (int trackId, float gain);

    /** Sets track pan from hard left (-1.0) to hard right (1.0). */
    void setTrackPan (int trackId, float pan);

    /** Mutes or unmutes a track using Tracktion's native track mute. */
    void setTrackMute (int trackId, bool muted);

    /** Solos or unsolos a track using Tracktion's native track solo. */
    void setTrackSolo (int trackId, bool soloed);

    /** Returns the latest peak level for a track meter as linear gain. */
    float getTrackLevelPeak (int trackId);

    /** Adds a wave audio clip to a track at the requested timeline position. */
    int addClipToTrack (int trackId, juce::File file, double startTimeSeconds);

    /** Removes a clip from both the Tracktion edit and Tracklab's clip map. */
    void removeClip (int clipId);

    /** Moves a clip to a new track and timeline start. */
    void moveClip (int clipId, int newTrackId, double newStartTimeSeconds);

    /** Trims a clip by setting its source offset and visible length. */
    void trimClip (int clipId, double newStartOffset, double newLength);

    /** Renames a clip without changing its source file. */
    void renameClip (int clipId, juce::String name);

    /** Returns every clip currently assigned to a track. */
    std::vector<ClipInfo> getClipsForTrack (int trackId) const;

    /** Returns every clip in the edit. */
    std::vector<ClipInfo> getAllClips() const;

    /** Returns a clip by ID, or an empty ClipInfo if it does not exist. */
    ClipInfo getClipInfo (int clipId) const;

    /** Returns the latest master output peak as linear gain. */
    float getMasterLevelPeak();

    /** Sets master output gain, where 1.0 is unity. */
    void setMasterVolume (float gain);

    /** Returns the current master output gain. */
    float getMasterVolume() const;

    /** Starts playback from the current transport position. */
    void play();

    /** Pauses playback without resetting the transport position. */
    void pause();

    /** Stops playback and resets the transport to the start. */
    void stop();

    /** Moves the transport to a position in seconds, clamped to the loaded file. */
    void setPositionSeconds (double seconds);

    /** Returns the current transport position in seconds. */
    double getPositionSeconds() const;

    /** Returns the loaded file length in seconds. */
    double getLengthSeconds() const;

    /** Returns the most recently loaded audio file, or an empty file if none is loaded. */
    juce::File getLoadedFile() const;

    /** Returns true while the Tracktion transport is playing. */
    bool isPlaying() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEngine)
};
