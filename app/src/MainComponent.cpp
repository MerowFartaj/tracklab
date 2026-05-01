#include "MainComponent.h"

#include "DesignTokens.h"

namespace tokens = tracklab::design;

MainComponent::MainComponent()
{
    audioEngine = std::make_unique<AudioEngine>();
    timelineView = std::make_unique<TimelineView> (*audioEngine);

    setOpaque (true);

    toolbar.onLoadClicked = [this]
    {
        loadAudioFile();
    };

    toolbar.onPlayPauseClicked = [this]
    {
        if (audioEngine->isPlaying())
            audioEngine->pause();
        else
            audioEngine->play();

        updateTransportState();
        timelineView->refreshPlayhead();
    };

    toolbar.onStopClicked = [this]
    {
        audioEngine->stop();
        updateTransportState();
        timelineView->refreshPlayhead();
    };

    addAndMakeVisible (toolbar);
    addAndMakeVisible (*timelineView);

    updateTransportState();
    startTimerHz (30);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (tokens::backgroundBase);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    toolbar.setBounds (bounds.removeFromTop (tokens::toolbarHeight));
    timelineView->setBounds (bounds);
}

void MainComponent::loadAudioFile()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Load audio file",
                                                       juce::File(),
                                                       "*.wav;*.aiff;*.aif;*.mp3;*.flac");

    const juce::Component::SafePointer<MainComponent> safeThis (this);

    fileChooser->launchAsync (juce::FileBrowserComponent::openMode
                                | juce::FileBrowserComponent::canSelectFiles,
                              [safeThis] (const juce::FileChooser& chooser)
                              {
                                  if (auto* component = safeThis.getComponent())
                                      component->handleFileChosen (chooser.getResult());
                              });
}

void MainComponent::handleFileChosen (const juce::File& file)
{
    if (file == juce::File())
        return;

    if (! isSupportedFile (file))
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                "Unsupported file",
                                                "Tracklab can load .wav, .aiff, .mp3, and .flac files.");
        return;
    }

    if (! audioEngine->loadFile (file))
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                "Could not load file",
                                                "Tracklab could not load this audio file.");
        return;
    }

    hasLoadedFile = true;
    timelineView->setAudioFile (file, audioEngine->getLengthSeconds());
    updateTransportState();
}

void MainComponent::updateTransportState()
{
    const auto length = audioEngine->getLengthSeconds();
    const auto rawPosition = audioEngine->getPositionSeconds();
    auto playing = audioEngine->isPlaying();

    if (length > 0.0 && playing && rawPosition >= length)
    {
        audioEngine->pause();
        playing = false;
    }

    const auto position = juce::jlimit (0.0, length, rawPosition);

    toolbar.setHasLoadedFile (hasLoadedFile);
    toolbar.setPlaying (playing);
    toolbar.setPosition (position, length);
}

void MainComponent::timerCallback()
{
    updateTransportState();
}

bool MainComponent::isSupportedFile (const juce::File& file)
{
    const auto extension = file.getFileExtension().toLowerCase();
    return extension == ".wav"
        || extension == ".aiff"
        || extension == ".aif"
        || extension == ".mp3"
        || extension == ".flac";
}
