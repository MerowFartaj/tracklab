#pragma once

#include <juce_core/juce_core.h>

#include <memory>

class AudioEngine final
{
public:
    AudioEngine();
    ~AudioEngine();

    /** Loads an audio file into a fresh single-track Tracktion Edit. */
    bool loadFile (juce::File file);

    /** Starts playback from the current transport position. */
    void play();

    /** Pauses playback without resetting the transport position. */
    void pause();

    /** Stops playback and resets the transport to the start. */
    void stop();

    /** Returns the current transport position in seconds. */
    double getPositionSeconds() const;

    /** Returns the loaded file length in seconds. */
    double getLengthSeconds() const;

    /** Returns true while the Tracktion transport is playing. */
    bool isPlaying() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEngine)
};
