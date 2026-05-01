#include "AudioEngine.h"

#include "app/src/DesignTokens.h"

#include <tracktion_engine/tracktion_engine.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>

namespace te = tracktion::engine;
namespace tokens = tracklab::design;

namespace
{
static constexpr int defaultTrackCount = 8;
static constexpr float minimumDb = -100.0f;
static constexpr double minimumClipLengthSeconds = 0.01;

float gainToDb (float gain)
{
    return juce::Decibels::gainToDecibels (juce::jmax (0.0f, gain), minimumDb);
}

float dbToGain (float db)
{
    return juce::Decibels::decibelsToGain (db, minimumDb);
}

juce::String defaultTrackName (int index)
{
    return juce::String::formatted ("Track %02d", index + 1);
}

juce::Colour trackColourForIndex (int index)
{
    return tokens::trackColors[static_cast<size_t> (index) % std::size (tokens::trackColors)];
}

}

struct AudioEngine::Impl
{
    struct TrackModel
    {
        int id = 0;
        juce::String name;
        juce::Colour colour;
        float volume = 1.0f;
        float pan = 0.0f;
        bool muted = false;
        bool soloed = false;
        te::AudioTrack::Ptr track;
        std::unique_ptr<te::LevelMeasurer::Client> meterClient;
    };

    struct ClipModel
    {
        int id = 0;
        int trackId = 0;
        juce::String name;
        juce::File file;
        te::WaveAudioClip::Ptr clip;
    };

    Impl()
        : engine ("Tracklab"),
          edit (te::createEmptyEdit (engine, {}))
    {
        edit->getTransport().ensureContextAllocated (true);
        attachMasterMeterClient();

        for (auto i = 0; i < defaultTrackCount; ++i)
            addTrack (defaultTrackName (i));

        updateTransportRange();
    }

    ~Impl()
    {
        detachMasterMeterClient();

        for (auto& track : tracks)
            detachTrackMeterClient (track);
    }

    bool loadFile (const juce::File& file)
    {
        stop();

        for (auto& [clipId, clip] : clips)
            if (clip.clip != nullptr)
                clip.clip->removeFromParent();

        clips.clear();
        loadedFile = juce::File();

        const auto targetTrack = tracks.empty() ? addTrack (defaultTrackName (0)) : tracks.front().id;
        const auto clipId = addClipToTrack (targetTrack, file, 0.0);

        if (clipId <= 0)
            return false;

        setPositionSeconds (0.0);
        return true;
    }

    int addTrack (const juce::String& requestedName)
    {
        auto track = edit->insertNewAudioTrack (te::TrackInsertPoint::getEndOfTracks (*edit), nullptr, true);

        if (track == nullptr)
            return 0;

        const auto trackIndex = static_cast<int> (tracks.size());
        const auto trackId = nextTrackId++;
        const auto name = requestedName.isNotEmpty() ? requestedName : defaultTrackName (trackIndex);

        track->setName (name);

        TrackModel model;
        model.id = trackId;
        model.name = name;
        model.colour = trackColourForIndex (trackIndex);
        model.track = track;
        model.meterClient = std::make_unique<te::LevelMeasurer::Client>();

        if (auto* volumePlugin = track->getVolumePlugin())
        {
            volumePlugin->setVolumeDb (gainToDb (model.volume));
            volumePlugin->setPan (model.pan);
        }

        attachTrackMeterClient (model);
        tracks.push_back (std::move (model));
        edit->getTransport().ensureContextAllocated (true);
        return trackId;
    }

    void removeTrack (int trackId)
    {
        auto trackIterator = findTrackIterator (trackId);

        if (trackIterator == tracks.end() || tracks.size() <= 1)
            return;

        for (auto clipIt = clips.begin(); clipIt != clips.end();)
        {
            if (clipIt->second.trackId == trackId)
            {
                if (clipIt->second.clip != nullptr)
                    clipIt->second.clip->removeFromParent();

                clipIt = clips.erase (clipIt);
            }
            else
            {
                ++clipIt;
            }
        }

        detachTrackMeterClient (*trackIterator);

        if (trackIterator->track != nullptr)
            edit->deleteTrack (trackIterator->track.get());

        tracks.erase (trackIterator);
        updateTransportRange();
    }

    int getTrackCount() const
    {
        return static_cast<int> (tracks.size());
    }

    TrackInfo getTrackInfo (int trackId) const
    {
        if (const auto* track = findTrack (trackId))
            return makeTrackInfo (*track);

        return {};
    }

    std::vector<TrackInfo> getAllTrackInfo() const
    {
        std::vector<TrackInfo> result;
        result.reserve (tracks.size());

        for (const auto& track : tracks)
            result.push_back (makeTrackInfo (track));

        return result;
    }

    void setTrackName (int trackId, const juce::String& name)
    {
        if (auto* track = findTrack (trackId))
        {
            const auto cleanedName = name.trim().isNotEmpty() ? name.trim() : track->name;
            track->name = cleanedName;

            if (track->track != nullptr)
                track->track->setName (cleanedName);
        }
    }

    void setTrackVolume (int trackId, float gain)
    {
        if (auto* track = findTrack (trackId))
        {
            track->volume = juce::jlimit (0.0f, 1.5f, gain);

            if (track->track != nullptr)
                if (auto* volumePlugin = track->track->getVolumePlugin())
                    volumePlugin->setVolumeDb (gainToDb (track->volume));
        }
    }

    void setTrackPan (int trackId, float pan)
    {
        if (auto* track = findTrack (trackId))
        {
            track->pan = juce::jlimit (-1.0f, 1.0f, pan);

            if (track->track != nullptr)
                if (auto* volumePlugin = track->track->getVolumePlugin())
                    volumePlugin->setPan (track->pan);
        }
    }

    void setTrackMute (int trackId, bool muted)
    {
        if (auto* track = findTrack (trackId))
        {
            track->muted = muted;

            if (track->track != nullptr)
                track->track->setMute (muted);
        }
    }

    void setTrackSolo (int trackId, bool soloed)
    {
        if (auto* track = findTrack (trackId))
        {
            track->soloed = soloed;

            if (track->track != nullptr)
                track->track->setSolo (soloed);
        }
    }

    float getTrackLevelPeak (int trackId)
    {
        if (auto* track = findTrack (trackId))
            return readMeterClient (*track->meterClient);

        return 0.0f;
    }

    int addClipToTrack (int trackId, const juce::File& file, double startTimeSeconds)
    {
        if (! file.existsAsFile())
            return 0;

        auto* track = findTrack (trackId);

        if (track == nullptr || track->track == nullptr)
            return 0;

        te::AudioFile audioFile (engine, file);
        const auto info = audioFile.getInfo();

        if (! audioFile.isValid() || ! info.wasParsedOk)
        {
            std::cout << "Tracklab: unsupported or invalid audio file: "
                      << file.getFullPathName() << std::endl;
            return 0;
        }

        const auto lengthSeconds = info.getLengthInSeconds();
        const auto start = tracktion::TimePosition::fromSeconds (juce::jmax (0.0, startTimeSeconds));
        const auto length = tracktion::TimeDuration::fromSeconds (juce::jmax (minimumClipLengthSeconds, lengthSeconds));

        auto clip = track->track->insertWaveClip (file.getFileNameWithoutExtension(),
                                                  file,
                                                  { { start, start + length }, {} },
                                                  false);

        if (clip == nullptr)
        {
            std::cout << "Tracklab: failed to insert audio clip: "
                      << file.getFullPathName() << std::endl;
            return 0;
        }

        ClipModel model;
        model.id = nextClipId++;
        model.trackId = trackId;
        model.name = file.getFileNameWithoutExtension();
        model.file = file;
        model.clip = clip;
        clips.emplace (model.id, std::move (model));

        loadedFile = file;
        updateTransportRange();

        std::cout << "Tracklab loaded audio file: " << file.getFullPathName() << std::endl;
        std::cout << "  Sample rate: " << info.sampleRate << std::endl;
        std::cout << "  Channels: " << info.numChannels << std::endl;
        std::cout << "  Length seconds: " << lengthSeconds << std::endl;

        return nextClipId - 1;
    }

    void removeClip (int clipId)
    {
        auto clipIterator = clips.find (clipId);

        if (clipIterator == clips.end())
            return;

        if (clipIterator->second.clip != nullptr)
            clipIterator->second.clip->removeFromParent();

        clips.erase (clipIterator);
        updateTransportRange();
    }

    void moveClip (int clipId, int newTrackId, double newStartTimeSeconds)
    {
        auto* clip = findClip (clipId);
        auto* track = findTrack (newTrackId);

        if (clip == nullptr || clip->clip == nullptr || track == nullptr || track->track == nullptr)
            return;

        if (clip->trackId != newTrackId)
        {
            if (! clip->clip->moveTo (*track->track))
                return;

            clip->trackId = newTrackId;
        }

        auto position = clip->clip->getPosition();
        const auto newStart = tracktion::TimePosition::fromSeconds (juce::jmax (0.0, newStartTimeSeconds));
        position.time = { newStart, newStart + position.getLength() };
        clip->clip->setPosition (position);
        updateTransportRange();
    }

    void trimClip (int clipId, double newStartOffset, double newLength)
    {
        auto* clip = findClip (clipId);

        if (clip == nullptr || clip->clip == nullptr)
            return;

        auto position = clip->clip->getPosition();
        const auto oldOffset = position.offset.inSeconds();
        const auto offsetDelta = juce::jmax (0.0, newStartOffset) - oldOffset;
        const auto newStart = juce::jmax (0.0, position.time.getStart().inSeconds() + offsetDelta);
        const auto newDuration = tracktion::TimeDuration::fromSeconds (juce::jmax (minimumClipLengthSeconds, newLength));

        position.offset = tracktion::TimeDuration::fromSeconds (juce::jmax (0.0, newStartOffset));
        position.time = { tracktion::TimePosition::fromSeconds (newStart),
                          tracktion::TimePosition::fromSeconds (newStart) + newDuration };

        clip->clip->setPosition (position);
        updateTransportRange();
    }

    void renameClip (int clipId, const juce::String& name)
    {
        if (auto* clip = findClip (clipId))
        {
            const auto cleanedName = name.trim().isNotEmpty() ? name.trim() : clip->name;
            clip->name = cleanedName;

            if (clip->clip != nullptr)
                clip->clip->setName (cleanedName);
        }
    }

    std::vector<ClipInfo> getClipsForTrack (int trackId) const
    {
        std::vector<ClipInfo> result;

        for (const auto& [clipId, clip] : clips)
            if (clip.trackId == trackId)
                result.push_back (makeClipInfo (clip));

        sortClips (result);
        return result;
    }

    std::vector<ClipInfo> getAllClips() const
    {
        std::vector<ClipInfo> result;
        result.reserve (clips.size());

        for (const auto& [clipId, clip] : clips)
            result.push_back (makeClipInfo (clip));

        sortClips (result);
        return result;
    }

    ClipInfo getClipInfo (int clipId) const
    {
        if (const auto* clip = findClip (clipId))
            return makeClipInfo (*clip);

        return {};
    }

    float getMasterLevelPeak()
    {
        attachMasterMeterClient();
        return readMeterClient (masterMeterClient);
    }

    void setMasterVolume (float gain)
    {
        masterVolume = juce::jlimit (0.0f, 1.5f, gain);

        if (auto master = edit->getMasterVolumePlugin())
            master->setVolumeDb (gainToDb (masterVolume));
    }

    void play()
    {
        if (clips.empty())
            return;

        auto& transport = edit->getTransport();

        if (transport.getPosition().inSeconds() >= getLengthSeconds())
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
        const auto clampedSeconds = juce::jlimit (0.0, getLengthSeconds(), seconds);
        edit->getTransport().setPosition (tracktion::TimePosition::fromSeconds (clampedSeconds));
    }

    double getPositionSeconds() const
    {
        return edit->getTransport().getPosition().inSeconds();
    }

    double getLengthSeconds() const
    {
        double length = 0.0;

        for (const auto& [clipId, clip] : clips)
        {
            if (clip.clip != nullptr)
                length = juce::jmax (length, clip.clip->getPosition().time.getEnd().inSeconds());
        }

        return length;
    }

    bool hasClips() const
    {
        return ! clips.empty();
    }

    TrackModel* findTrack (int trackId)
    {
        auto iterator = findTrackIterator (trackId);
        return iterator != tracks.end() ? &(*iterator) : nullptr;
    }

    const TrackModel* findTrack (int trackId) const
    {
        auto iterator = std::find_if (tracks.begin(), tracks.end(),
                                      [trackId] (const auto& track) { return track.id == trackId; });

        return iterator != tracks.end() ? &(*iterator) : nullptr;
    }

    ClipModel* findClip (int clipId)
    {
        auto iterator = clips.find (clipId);
        return iterator != clips.end() ? &iterator->second : nullptr;
    }

    const ClipModel* findClip (int clipId) const
    {
        auto iterator = clips.find (clipId);
        return iterator != clips.end() ? &iterator->second : nullptr;
    }

    std::vector<TrackModel>::iterator findTrackIterator (int trackId)
    {
        return std::find_if (tracks.begin(), tracks.end(),
                             [trackId] (const auto& track) { return track.id == trackId; });
    }

    TrackInfo makeTrackInfo (const TrackModel& track) const
    {
        TrackInfo info;
        info.id = track.id;
        info.name = track.name;
        info.colour = track.colour;
        info.volume = track.volume;
        info.pan = track.pan;
        info.muted = track.muted;
        info.soloed = track.soloed;
        return info;
    }

    ClipInfo makeClipInfo (const ClipModel& clip) const
    {
        ClipInfo info;
        info.id = clip.id;
        info.trackId = clip.trackId;
        info.name = clip.name;
        info.file = clip.file;

        if (clip.clip != nullptr)
        {
            const auto position = clip.clip->getPosition();
            info.startTimeSeconds = position.time.getStart().inSeconds();
            info.lengthSeconds = position.time.getLength().inSeconds();
            info.sourceOffsetSeconds = position.offset.inSeconds();
        }
        else
        {
            info.lengthSeconds = 0.0;
        }

        return info;
    }

    static void sortClips (std::vector<ClipInfo>& clipInfos)
    {
        std::sort (clipInfos.begin(), clipInfos.end(),
                   [] (const auto& left, const auto& right)
                   {
                       if (left.trackId != right.trackId)
                           return left.trackId < right.trackId;

                       return left.startTimeSeconds < right.startTimeSeconds;
                   });
    }

    void updateTransportRange()
    {
        const auto length = getLengthSeconds();
        auto& transport = edit->getTransport();
        transport.setLoopRange ({ tracktion::TimePosition(),
                                  tracktion::TimePosition::fromSeconds (juce::jmax (0.0, length)) });
        transport.looping = false;
        transport.ensureContextAllocated (true);
    }

    void attachTrackMeterClient (TrackModel& track)
    {
        if (track.track == nullptr || track.meterClient == nullptr)
            return;

        if (auto* meter = track.track->getLevelMeterPlugin())
            meter->measurer.addClient (*track.meterClient);
    }

    void detachTrackMeterClient (TrackModel& track)
    {
        if (track.track == nullptr || track.meterClient == nullptr)
            return;

        if (auto* meter = track.track->getLevelMeterPlugin())
            meter->measurer.removeClient (*track.meterClient);
    }

    void attachMasterMeterClient()
    {
        if (masterMeterAttached)
            return;

        if (auto* context = edit->getCurrentPlaybackContext())
        {
            context->masterLevels.addClient (masterMeterClient);
            masterMeterAttached = true;
        }
    }

    void detachMasterMeterClient()
    {
        if (! masterMeterAttached)
            return;

        if (auto* context = edit->getCurrentPlaybackContext())
            context->masterLevels.removeClient (masterMeterClient);

        masterMeterAttached = false;
    }

    static float readMeterClient (te::LevelMeasurer::Client& client)
    {
        auto peak = 0.0f;
        const auto channels = juce::jmax (1, client.getNumChannelsUsed());

        for (auto channel = 0; channel < juce::jmin (channels, te::LevelMeasurer::Client::maxNumChannels); ++channel)
            peak = juce::jmax (peak, dbToGain (client.getAndClearAudioLevel (channel).dB));

        return juce::jlimit (0.0f, 1.0f, peak);
    }

    te::Engine engine;
    std::unique_ptr<te::Edit> edit;
    juce::File loadedFile;
    std::vector<TrackModel> tracks;
    std::map<int, ClipModel> clips;
    te::LevelMeasurer::Client masterMeterClient;
    bool masterMeterAttached = false;
    float masterVolume = 1.0f;
    int nextTrackId = 1;
    int nextClipId = 1;
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

int AudioEngine::addTrack (juce::String name)
{
    return impl->addTrack (std::move (name));
}

void AudioEngine::removeTrack (int trackId)
{
    impl->removeTrack (trackId);
}

int AudioEngine::getTrackCount() const
{
    return impl->getTrackCount();
}

TrackInfo AudioEngine::getTrackInfo (int trackId) const
{
    return impl->getTrackInfo (trackId);
}

std::vector<TrackInfo> AudioEngine::getAllTrackInfo() const
{
    return impl->getAllTrackInfo();
}

void AudioEngine::setTrackName (int trackId, juce::String name)
{
    impl->setTrackName (trackId, std::move (name));
}

void AudioEngine::setTrackVolume (int trackId, float gain)
{
    impl->setTrackVolume (trackId, gain);
}

void AudioEngine::setTrackPan (int trackId, float pan)
{
    impl->setTrackPan (trackId, pan);
}

void AudioEngine::setTrackMute (int trackId, bool muted)
{
    impl->setTrackMute (trackId, muted);
}

void AudioEngine::setTrackSolo (int trackId, bool soloed)
{
    impl->setTrackSolo (trackId, soloed);
}

float AudioEngine::getTrackLevelPeak (int trackId)
{
    return impl->getTrackLevelPeak (trackId);
}

int AudioEngine::addClipToTrack (int trackId, juce::File file, double startTimeSeconds)
{
    return impl->addClipToTrack (trackId, file, startTimeSeconds);
}

void AudioEngine::removeClip (int clipId)
{
    impl->removeClip (clipId);
}

void AudioEngine::moveClip (int clipId, int newTrackId, double newStartTimeSeconds)
{
    impl->moveClip (clipId, newTrackId, newStartTimeSeconds);
}

void AudioEngine::trimClip (int clipId, double newStartOffset, double newLength)
{
    impl->trimClip (clipId, newStartOffset, newLength);
}

void AudioEngine::renameClip (int clipId, juce::String name)
{
    impl->renameClip (clipId, std::move (name));
}

std::vector<ClipInfo> AudioEngine::getClipsForTrack (int trackId) const
{
    return impl->getClipsForTrack (trackId);
}

std::vector<ClipInfo> AudioEngine::getAllClips() const
{
    return impl->getAllClips();
}

ClipInfo AudioEngine::getClipInfo (int clipId) const
{
    return impl->getClipInfo (clipId);
}

float AudioEngine::getMasterLevelPeak()
{
    return impl->getMasterLevelPeak();
}

void AudioEngine::setMasterVolume (float gain)
{
    impl->setMasterVolume (gain);
}

float AudioEngine::getMasterVolume() const
{
    return impl->masterVolume;
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
    return impl->getLengthSeconds();
}

juce::File AudioEngine::getLoadedFile() const
{
    return impl->loadedFile;
}

bool AudioEngine::isPlaying() const
{
    return impl->edit->getTransport().isPlaying();
}
