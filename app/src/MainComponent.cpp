#include "MainComponent.h"

#include "DesignTokens.h"
#include "UiDrawing.h"

namespace tokens = tracklab::design;
namespace ui = tracklab::ui;

MainComponent::MainComponent()
{
    setLookAndFeel (&lookAndFeel);

    audioEngine = std::make_unique<AudioEngine>();
    timelineView = std::make_unique<TimelineView> (*audioEngine);
    mixerPanel = std::make_unique<MixerPanel> (*audioEngine);
    workspacePanel = std::make_unique<WorkspacePanel> (*audioEngine);

    setOpaque (true);
    setWantsKeyboardFocus (true);

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

    toolbar.onLoopClicked = [this]
    {
        const auto info = audioEngine->getProjectInfo();
        audioEngine->setLoopEnabled (! info.loopEnabled);
        updateTransportState();
        workspacePanel->refreshFromEngine();
    };

    toolbar.onMetronomeClicked = [this]
    {
        const auto info = audioEngine->getProjectInfo();
        audioEngine->setMetronomeEnabled (! info.metronomeEnabled);
        updateTransportState();
        workspacePanel->refreshFromEngine();
    };

    toolbar.onRecordClicked = [this]
    {
        const auto info = audioEngine->getProjectInfo();
        audioEngine->setRecordingEnabled (! info.recordingEnabled);
        updateTransportState();
        workspacePanel->refreshFromEngine();
    };

    timelineView->onProjectChanged = [this]
    {
        mixerPanel->refreshFromEngine();
        workspacePanel->refreshFromEngine();
        updateTransportState();
    };

    timelineView->onSelectionChanged = [this]
    {
        workspacePanel->setSelection (timelineView->getPrimarySelectedClipId(),
                                      timelineView->getPrimarySelectedTrackId());
    };

    workspacePanel->onSplitSelectedClip = [this]
    {
        timelineView->splitSelectedClipsAtPlayhead();
        workspacePanel->setSelection (timelineView->getPrimarySelectedClipId(),
                                      timelineView->getPrimarySelectedTrackId());
    };

    addAndMakeVisible (toolbar);
    addAndMakeVisible (*workspacePanel);
    addAndMakeVisible (*timelineView);
    addAndMakeVisible (*mixerPanel);

    updateTransportState();
    startTimerHz (30);
}

MainComponent::~MainComponent()
{
    setLookAndFeel (nullptr);
}

void MainComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient backdrop { tokens::backgroundBase.brighter (tokens::surfaceGradientAmount),
                                    bounds.getX(),
                                    bounds.getY(),
                                    tokens::backgroundDeep,
                                    bounds.getRight(),
                                    bounds.getBottom(),
                                    false };
    backdrop.addColour (0.48, tokens::surfaceInset);
    g.setGradientFill (backdrop);
    g.fillAll();

    ui::drawSoftGlow (g, bounds.withSizeKeepingCentre (bounds.getWidth() * 0.50f, bounds.getHeight() * 0.70f)
                               .translated (bounds.getWidth() * 0.22f, -bounds.getHeight() * 0.22f),
                      tokens::accentSecondary,
                      tokens::accentWashAlpha);
    ui::drawSoftGlow (g, bounds.withSizeKeepingCentre (bounds.getWidth() * 0.38f, bounds.getHeight() * 0.42f)
                               .translated (-bounds.getWidth() * 0.34f, bounds.getHeight() * 0.28f),
                      tokens::accentPrimary,
                      tokens::accentWashAlpha);
    ui::drawSubtleStripes (g, bounds, tokens::highlightBase, tokens::decorativeStripeSpacing * 2);

    auto splitter = getSplitterBounds().toFloat();
    ui::drawGlassPanel (g, splitter, tokens::surfaceChrome, 0.0f, false);
    g.setColour (tokens::highlightBase.withAlpha (tokens::panelTopHighlightAlpha));
    g.drawHorizontalLine (juce::roundToInt (splitter.getY()), splitter.getX(), splitter.getRight());
    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawHorizontalLine (juce::roundToInt (splitter.getBottom() - 1.0f), splitter.getX(), splitter.getRight());

    auto handle = splitter.withSizeKeepingCentre (static_cast<float> (tokens::splitterHandleWidth),
                                                  static_cast<float> (tokens::splitterHandleHeight));
    g.setColour (tokens::accentCyan.withAlpha (tokens::browserSelectedAlpha));
    g.fillRoundedRectangle (handle, 1.0f);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    toolbar.setBounds (bounds.removeFromTop (tokens::toolbarHeight));
    workspacePanel->setBounds (bounds.removeFromLeft (tokens::workspacePanelWidth));

    const auto clampedMixerHeight = juce::jlimit (tokens::mixerMinHeight,
                                                  tokens::mixerMaxHeight,
                                                  mixerHeight);
    mixerHeight = clampedMixerHeight;

    mixerPanel->setBounds (bounds.removeFromBottom (mixerHeight));
    bounds.removeFromBottom (tokens::splitterHeight);
    timelineView->setBounds (bounds);
}

void MainComponent::mouseDown (const juce::MouseEvent& event)
{
    grabKeyboardFocus();

    if (getSplitterBounds().contains (event.getPosition()))
    {
        draggingSplitter = true;
        splitterDragStartY = event.getPosition().y;
        mixerHeightAtDragStart = mixerHeight;
    }
}

void MainComponent::mouseDrag (const juce::MouseEvent& event)
{
    if (! draggingSplitter)
        return;

    const auto deltaY = splitterDragStartY - event.getPosition().y;
    mixerHeight = juce::jlimit (tokens::mixerMinHeight,
                                tokens::mixerMaxHeight,
                                mixerHeightAtDragStart + deltaY);
    resized();
    repaint();
}

void MainComponent::mouseUp (const juce::MouseEvent&)
{
    draggingSplitter = false;
}

bool MainComponent::keyPressed (const juce::KeyPress& key)
{
    const auto modifiers = key.getModifiers();
    const auto character = key.getTextCharacter();

    if (key == juce::KeyPress::spaceKey)
    {
        if (audioEngine->isPlaying())
            audioEngine->pause();
        else
            audioEngine->play();

        updateTransportState();
        timelineView->refreshPlayhead();
        return true;
    }

    if (! modifiers.isCommandDown() && (character == 's' || character == 'S'))
    {
        audioEngine->stop();
        updateTransportState();
        timelineView->refreshPlayhead();
        return true;
    }

    if (modifiers.isCommandDown() && (character == 't' || character == 'T'))
    {
        timelineView->addTrack();
        mixerPanel->refreshFromEngine();
        return true;
    }

    if (modifiers.isCommandDown() && (character == 'd' || character == 'D'))
    {
        timelineView->duplicateSelectedClips();
        return true;
    }

    if (modifiers.isCommandDown() && (character == 'e' || character == 'E'))
    {
        timelineView->splitSelectedClipsAtPlayhead();
        return true;
    }

    if (modifiers.isCommandDown() && (character == 'l' || character == 'L'))
    {
        const auto info = audioEngine->getProjectInfo();
        audioEngine->setLoopEnabled (! info.loopEnabled);
        updateTransportState();
        workspacePanel->refreshFromEngine();
        return true;
    }

    if (modifiers.isCommandDown() && (character == 'k' || character == 'K'))
    {
        const auto info = audioEngine->getProjectInfo();
        audioEngine->setMetronomeEnabled (! info.metronomeEnabled);
        updateTransportState();
        workspacePanel->refreshFromEngine();
        return true;
    }

    if (modifiers.isCommandDown() && (character == 'a' || character == 'A'))
    {
        timelineView->selectAllClips();
        return true;
    }

    if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
    {
        timelineView->deleteSelectedClips();
        return true;
    }

    if (key == juce::KeyPress::escapeKey)
    {
        timelineView->deselectAllClips();
        return true;
    }

    return false;
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

    const auto tracks = audioEngine->getAllTrackInfo();

    if (tracks.empty())
        audioEngine->addTrack ("Track 01");

    const auto targetTrackId = audioEngine->getAllTrackInfo().front().id;

    if (audioEngine->addClipToTrack (targetTrackId, file, 0.0) <= 0)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                "Could not load file",
                                                "Tracklab could not load this audio file.");
        return;
    }

    timelineView->refreshFromEngine();
    mixerPanel->refreshFromEngine();
    workspacePanel->setSelection (timelineView->getPrimarySelectedClipId(),
                                  timelineView->getPrimarySelectedTrackId());
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
    const auto hasClips = ! audioEngine->getAllClips().empty();

    toolbar.setHasLoadedFile (hasClips);
    toolbar.setPlaying (playing);
    toolbar.setPosition (position, length);
    toolbar.setProjectInfo (audioEngine->getProjectInfo());
}

void MainComponent::timerCallback()
{
    updateTransportState();
}

juce::Rectangle<int> MainComponent::getSplitterBounds() const
{
    return { tokens::workspacePanelWidth,
             getHeight() - mixerHeight - tokens::splitterHeight,
             juce::jmax (0, getWidth() - tokens::workspacePanelWidth),
             tokens::splitterHeight };
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
