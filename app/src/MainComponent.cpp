#include "MainComponent.h"

MainComponent::MainComponent()
{
    audioEngine = std::make_unique<AudioEngine>();

    setOpaque (true);

    loadButton.onClick = [this] { loadAudioFile(); };
    playButton.onClick = [this]
    {
        audioEngine->play();
        updatePlaybackState();
    };
    pauseButton.onClick = [this]
    {
        audioEngine->pause();
        updatePlaybackState();
    };
    stopButton.onClick = [this]
    {
        audioEngine->stop();
        updatePlaybackState();
    };

    for (auto* button : { &loadButton, &playButton, &pauseButton, &stopButton })
    {
        button->setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (0x2a, 0x2a, 0x2a));
        button->setColour (juce::TextButton::buttonOnColourId, juce::Colour::fromRGB (0x3a, 0x3a, 0x3a));
        button->setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        button->setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        addAndMakeVisible (*button);
    }

    positionLabel.setJustificationType (juce::Justification::centred);
    positionLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.82f));
    addAndMakeVisible (positionLabel);

    progressSlider.setRange (0.0, 1.0, 0.001);
    progressSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    progressSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    progressSlider.setEnabled (false);
    addAndMakeVisible (progressSlider);

    playButton.setEnabled (false);
    pauseButton.setEnabled (false);
    stopButton.setEnabled (false);

    updatePlaybackState();
    startTimerHz (30);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (0x1a, 0x1a, 0x1a));

    g.setColour (juce::Colours::white.withAlpha (0.92f));
    g.setFont (juce::Font { juce::FontOptions { 48.0f, juce::Font::plain } });
    g.drawText ("Tracklab", getLocalBounds().removeFromTop (240), juce::Justification::centred, false);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    auto controls = bounds.withSizeKeepingCentre (520, 220);

    loadButton.setBounds (controls.removeFromTop (44));
    controls.removeFromTop (18);

    auto transportRow = controls.removeFromTop (44);
    playButton.setBounds (transportRow.removeFromLeft (160));
    transportRow.removeFromLeft (20);
    pauseButton.setBounds (transportRow.removeFromLeft (160));
    transportRow.removeFromLeft (20);
    stopButton.setBounds (transportRow.removeFromLeft (160));

    controls.removeFromTop (24);
    positionLabel.setBounds (controls.removeFromTop (28));
    controls.removeFromTop (12);
    progressSlider.setBounds (controls.removeFromTop (32));
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
    loadButton.setButtonText ("Load different file");
    updatePlaybackState();
}

void MainComponent::updatePlaybackState()
{
    const auto length = audioEngine->getLengthSeconds();
    const auto rawPosition = audioEngine->getPositionSeconds();
    const auto hasLength = length > 0.0;
    auto playing = audioEngine->isPlaying();

    if (hasLength && playing && rawPosition >= length)
    {
        audioEngine->pause();
        playing = false;
    }

    const auto position = juce::jlimit (0.0, length, rawPosition);

    playButton.setEnabled (hasLoadedFile && ! playing);
    pauseButton.setEnabled (hasLoadedFile && playing);
    stopButton.setEnabled (hasLoadedFile);

    positionLabel.setText (formatTime (position) + " / " + formatTime (length),
                           juce::dontSendNotification);
    progressSlider.setValue (hasLength ? position / length : 0.0, juce::dontSendNotification);
}

void MainComponent::timerCallback()
{
    updatePlaybackState();
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

juce::String MainComponent::formatTime (double seconds)
{
    const auto totalSeconds = juce::jmax (0, juce::roundToInt (seconds));
    const auto minutes = totalSeconds / 60;
    const auto remainingSeconds = totalSeconds % 60;

    return juce::String::formatted ("%d:%02d", minutes, remainingSeconds);
}
