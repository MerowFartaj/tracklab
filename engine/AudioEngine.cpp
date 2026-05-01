#include "AudioEngine.h"

#include <tracktion_engine/tracktion_engine.h>

#include <iostream>

namespace te = tracktion::engine;

struct AudioEngine::Impl
{
    Impl()
        : engine ("Tracklab"),
          edit (te::createEmptyEdit (engine, {}))
    {
        edit->ensureNumberOfAudioTracks (1);
        edit->getTransport().ensureContextAllocated();
    }

    bool loadFile (const juce::File& file)
    {
        stop();

        te::AudioFile audioFile (engine, file);
        const auto info = audioFile.getInfo();

        if (! audioFile.isValid() || ! info.wasParsedOk)
        {
            std::cout << "Tracklab: unsupported or invalid audio file: "
                      << file.getFullPathName() << std::endl;
            return false;
        }

        auto* track = getOrCreateTrack();

        if (track == nullptr)
            return false;

        removeAllClips (*track);

        const auto length = tracktion::TimeDuration::fromSeconds (audioFile.getLength());
        auto clip = track->insertWaveClip (file.getFileNameWithoutExtension(),
                                           file,
                                           { { tracktion::TimePosition(), length }, {} },
                                           false);

        if (clip == nullptr)
        {
            std::cout << "Tracklab: failed to insert audio clip: "
                      << file.getFullPathName() << std::endl;
            loadedLengthSeconds = 0.0;
            return false;
        }

        loadedLengthSeconds = info.getLengthInSeconds();
        loadedFile = file;

        auto& transport = edit->getTransport();
        transport.setLoopRange (clip->getEditTimeRange());
        transport.looping = false;
        transport.setPosition (tracktion::TimePosition());
        transport.ensureContextAllocated (true);

        std::cout << "Tracklab loaded audio file: " << file.getFullPathName() << std::endl;
        std::cout << "  Sample rate: " << info.sampleRate << std::endl;
        std::cout << "  Channels: " << info.numChannels << std::endl;
        std::cout << "  Length seconds: " << loadedLengthSeconds << std::endl;

        return true;
    }

    void play()
    {
        if (! hasLoadedFile())
            return;

        auto& transport = edit->getTransport();

        if (transport.getPosition().inSeconds() >= loadedLengthSeconds)
            transport.setPosition (tracktion::TimePosition());

        transport.play (false);
    }

    void pause()
    {
        edit->getTransport().stop (false, false);
    }

    void stop()
    {
        auto& transport = edit->getTransport();
        transport.stop (false, false);
        transport.setPosition (tracktion::TimePosition());
    }

    void setPositionSeconds (double seconds)
    {
        const auto clampedSeconds = juce::jlimit (0.0, loadedLengthSeconds, seconds);
        edit->getTransport().setPosition (tracktion::TimePosition::fromSeconds (clampedSeconds));
    }

    double getPositionSeconds() const
    {
        return edit->getTransport().getPosition().inSeconds();
    }

    bool hasLoadedFile() const
    {
        return loadedLengthSeconds > 0.0;
    }

    te::AudioTrack* getOrCreateTrack()
    {
        edit->ensureNumberOfAudioTracks (1);
        auto tracks = te::getAudioTracks (*edit);
        return tracks.isEmpty() ? nullptr : tracks.getFirst();
    }

    static void removeAllClips (te::AudioTrack& track)
    {
        auto clips = track.getClips();

        for (int i = clips.size(); --i >= 0;)
            clips.getUnchecked (i)->removeFromParent();
    }

    te::Engine engine;
    std::unique_ptr<te::Edit> edit;
    juce::File loadedFile;
    double loadedLengthSeconds = 0.0;
};

AudioEngine::AudioEngine()
    : impl (std::make_unique<Impl>())
{
}

AudioEngine::~AudioEngine() = default;

bool AudioEngine::loadFile (juce::File file)
{
    return impl->loadFile (file);
}

void AudioEngine::play()
{
    impl->play();
}

void AudioEngine::pause()
{
    impl->pause();
}

void AudioEngine::stop()
{
    impl->stop();
}

void AudioEngine::setPositionSeconds (double seconds)
{
    impl->setPositionSeconds (seconds);
}

double AudioEngine::getPositionSeconds() const
{
    return impl->getPositionSeconds();
}

double AudioEngine::getLengthSeconds() const
{
    return impl->loadedLengthSeconds;
}

juce::File AudioEngine::getLoadedFile() const
{
    return impl->loadedFile;
}

bool AudioEngine::isPlaying() const
{
    return impl->edit->getTransport().isPlaying();
}
