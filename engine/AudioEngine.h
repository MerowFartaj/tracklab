#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

#include <array>
#include <memory>
#include <vector>

struct ProjectInfo
{
    double tempoBpm = 120.0;
    int timeSignatureNumerator = 4;
    int timeSignatureDenominator = 4;
    juce::String keyName = "C Major";
    bool loopEnabled = false;
    double loopStartSeconds = 0.0;
    double loopEndSeconds = 8.0;
    bool metronomeEnabled = false;
    bool recordingEnabled = false;
    int snapDivision = 4;
};

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
    float gainDb = 0.0f;
    double fadeInSeconds = 0.0;
    double fadeOutSeconds = 0.0;
    float transposeSemitones = 0.0f;
    bool loopEnabled = false;
};

struct TrackDeviceState
{
    bool eqEnabled = false;
    std::array<float, 4> eqGainDb { 0.0f, 0.0f, 0.0f, 0.0f };
    std::array<float, 4> eqFrequencyHz { 80.0f, 800.0f, 3000.0f, 12000.0f };
    std::array<float, 4> eqQ { 0.5f, 0.7f, 0.7f, 0.5f };
    bool pitchShiftEnabled = false;
    float pitchShiftSemitones = 0.0f;
};

class AudioEngine final
{
public:
    AudioEngine();
    ~AudioEngine();

    /** Returns project-level transport and musical settings. */
    ProjectInfo getProjectInfo() const;

    /** Sets the project tempo in beats per minute. */
    void setTempoBpm (double bpm);

    /** Sets the global time signature. */
    void setTimeSignature (int numerator, int denominator);

    /** Stores the project key label used by the UI and future MIDI tools. */
    void setProjectKey (juce::String keyName);

    /** Enables or disables the arrangement loop. */
    void setLoopEnabled (bool enabled);

    /** Sets the arrangement loop range in seconds. */
    void setLoopRange (double startSeconds, double endSeconds);

    /** Enables or disables Tracktion's click track. */
    void setMetronomeEnabled (bool enabled);

    /** Stores whether the transport is armed for future recording work. */
    void setRecordingEnabled (bool enabled);

    /** Sets the musical snap division used by editing tools. */
    void setSnapDivision (int division);

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

    /** Splits a clip at a timeline position and returns the new clip IDs. */
    std::vector<int> splitClipAt (int clipId, double timelineSeconds);

    /** Sets non-destructive clip gain in decibels. */
    void setClipGainDb (int clipId, float gainDb);

    /** Sets clip fade-in and fade-out lengths in seconds. */
    void setClipFades (int clipId, double fadeInSeconds, double fadeOutSeconds);

    /** Sets audio clip pitch transposition in semitones. */
    void setClipTransposeSemitones (int clipId, float semitones);

    /** Enables or disables clip looping across its visible range. */
    void setClipLoopEnabled (int clipId, bool enabled);

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

    /** Returns built-in device state for a track. */
    TrackDeviceState getTrackDeviceState (int trackId) const;

    /** Enables or bypasses the built-in 4-band equaliser on a track. */
    void setTrackEqEnabled (int trackId, bool enabled);

    /** Sets a built-in equaliser band gain in decibels. */
    void setTrackEqBandGainDb (int trackId, int bandIndex, float gainDb);

    /** Enables or bypasses the built-in pitch shifter on a track. */
    void setTrackPitchShiftEnabled (int trackId, bool enabled);

    /** Sets the built-in pitch shifter transposition in semitones. */
    void setTrackPitchShiftSemitones (int trackId, float semitones);

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
