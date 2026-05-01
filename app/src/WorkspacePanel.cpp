#include "WorkspacePanel.h"

namespace tokens = tracklab::design;
namespace ui = tracklab::ui;

namespace
{
juce::String signatureText (const ProjectInfo& info)
{
    return juce::String::formatted ("%d/%d", info.timeSignatureNumerator, info.timeSignatureDenominator);
}

void addSignatureItems (juce::ComboBox& box)
{
    box.addItem ("2/4", 1);
    box.addItem ("3/4", 2);
    box.addItem ("4/4", 3);
    box.addItem ("6/8", 4);
    box.addItem ("7/8", 5);
}

void addKeyItems (juce::ComboBox& box)
{
    const juce::StringArray keys
    {
        "C Major", "G Major", "D Major", "A Major", "E Major", "B Major",
        "F Major", "Bb Major", "Eb Major", "A Minor", "E Minor", "D Minor"
    };

    for (auto i = 0; i < keys.size(); ++i)
        box.addItem (keys[i], i + 1);
}

void addSnapItems (juce::ComboBox& box)
{
    box.addItem ("1/1", 1);
    box.addItem ("1/2", 2);
    box.addItem ("1/4", 4);
    box.addItem ("1/8", 8);
    box.addItem ("1/16", 16);
    box.addItem ("1/32", 32);
}
}

void WorkspacePanel::BrowserItem::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);
    auto base = selected ? colour.withAlpha (tokens::browserSelectedAlpha)
                         : tokens::surfaceInset.withAlpha (tokens::browserItemAlpha);

    if (selected)
        ui::drawSoftGlow (g, bounds, colour, tokens::accentWashAlpha);

    g.setColour (base);
    g.fillRoundedRectangle (bounds, tokens::browserItemRadius);

    auto iconBounds = getLocalBounds().removeFromLeft (tokens::browserIconCell)
                                      .withSizeKeepingCentre (tokens::iconSizeMedium, tokens::iconSizeMedium)
                                      .toFloat();
    ui::drawIcon (g, icon, iconBounds, colour, tokens::iconStrokeWidth);

    g.setColour (selected ? tokens::textPrimary : tokens::textSecondary);
    g.setFont (selected ? tokens::fontBody().withStyle (juce::Font::bold) : tokens::fontBody());
    g.drawFittedText (text,
                      getLocalBounds().withTrimmedLeft (tokens::browserIconCell)
                                      .reduced (tokens::trackHeaderSmallGap, 0),
                      juce::Justification::centredLeft,
                      1);
}

void WorkspacePanel::BrowserItem::setItem (juce::String newText,
                                           ui::Icon newIcon,
                                           juce::Colour newColour,
                                           bool shouldBeSelected)
{
    text = std::move (newText);
    icon = newIcon;
    colour = newColour;
    selected = shouldBeSelected;
    repaint();
}

WorkspacePanel::WorkspacePanel (AudioEngine& engine)
    : audioEngine (engine)
{
    setOpaque (true);

    addAndMakeVisible (browserHeader);
    addAndMakeVisible (browserSearch);
    addAndMakeVisible (projectHeader);
    addAndMakeVisible (tempoSlider);
    addAndMakeVisible (signatureBox);
    addAndMakeVisible (keyBox);
    addAndMakeVisible (snapBox);
    addAndMakeVisible (loopButton);
    addAndMakeVisible (clickButton);
    addAndMakeVisible (recordButton);
    addAndMakeVisible (clipHeader);
    addAndMakeVisible (selectedClipLabel);
    addAndMakeVisible (clipGainSlider);
    addAndMakeVisible (fadeInSlider);
    addAndMakeVisible (fadeOutSlider);
    addAndMakeVisible (clipTransposeSlider);
    addAndMakeVisible (clipLoopButton);
    addAndMakeVisible (splitButton);
    addAndMakeVisible (deviceHeader);
    addAndMakeVisible (selectedTrackLabel);
    addAndMakeVisible (eqButton);
    addAndMakeVisible (pitchButton);
    addAndMakeVisible (pitchSlider);

    configureLabel (browserHeader, tokens::fontHeader(), tokens::textPrimary);
    configureLabel (projectHeader, tokens::fontHeader(), tokens::textPrimary);
    configureLabel (clipHeader, tokens::fontHeader(), tokens::textPrimary);
    configureLabel (deviceHeader, tokens::fontHeader(), tokens::textPrimary);
    configureLabel (selectedClipLabel, tokens::fontMetadata(), tokens::textSecondary);
    configureLabel (selectedTrackLabel, tokens::fontMetadata(), tokens::textSecondary);

    browserHeader.setText ("Library", juce::dontSendNotification);
    projectHeader.setText ("Project", juce::dontSendNotification);
    clipHeader.setText ("Inspector", juce::dontSendNotification);
    deviceHeader.setText ("Devices", juce::dontSendNotification);

    for (auto& item : browserItems)
        addAndMakeVisible (item);

    browserSearch.setTextToShowWhenEmpty ("Search sounds and devices", tokens::textTertiary);
    browserSearch.setFont (tokens::fontBody());
    browserSearch.setColour (juce::TextEditor::backgroundColourId, tokens::surfaceRaised);
    browserSearch.setColour (juce::TextEditor::textColourId, tokens::textPrimary);
    browserSearch.setColour (juce::TextEditor::outlineColourId, tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    browserSearch.setColour (juce::TextEditor::focusedOutlineColourId, tokens::accentSecondary.withAlpha (tokens::panelBorderAlpha));
    browserSearch.setIndents (tokens::browserIconCell, 0);

    configureSlider (tempoSlider, 20.0, 300.0, 0.01);
    tempoSlider.setTextValueSuffix (" BPM");
    configureSlider (clipGainSlider, -60.0, 24.0, 0.1);
    clipGainSlider.setTextValueSuffix (" dB");
    configureSlider (fadeInSlider, 0.0, 10.0, 0.001);
    fadeInSlider.setTextValueSuffix (" s");
    configureSlider (fadeOutSlider, 0.0, 10.0, 0.001);
    fadeOutSlider.setTextValueSuffix (" s");
    configureSlider (clipTransposeSlider, -48.0, 48.0, 0.01);
    clipTransposeSlider.setTextValueSuffix (" st");
    configureSlider (pitchSlider, -24.0, 24.0, 0.01);
    pitchSlider.setTextValueSuffix (" st");

    for (auto& slider : eqGainSliders)
    {
        addAndMakeVisible (slider);
        configureSlider (slider, -20.0, 20.0, 0.1);
        slider.setTextValueSuffix (" dB");
    }

    configureComboBox (signatureBox);
    configureComboBox (keyBox);
    configureComboBox (snapBox);
    addSignatureItems (signatureBox);
    addKeyItems (keyBox);
    addSnapItems (snapBox);

    configureButton (loopButton);
    configureButton (clickButton);
    configureButton (recordButton);
    configureButton (clipLoopButton);
    configureButton (splitButton);
    configureButton (eqButton);
    configureButton (pitchButton);

    tempoSlider.onValueChange = [this]
    {
        if (! refreshing)
            audioEngine.setTempoBpm (tempoSlider.getValue());
    };

    signatureBox.onChange = [this]
    {
        if (refreshing)
            return;

        const auto parts = juce::StringArray::fromTokens (signatureBox.getText(), "/", "");

        if (parts.size() == 2)
            audioEngine.setTimeSignature (parts[0].getIntValue(), parts[1].getIntValue());
    };

    keyBox.onChange = [this]
    {
        if (! refreshing)
            audioEngine.setProjectKey (keyBox.getText());
    };

    snapBox.onChange = [this]
    {
        if (! refreshing)
            audioEngine.setSnapDivision (snapBox.getSelectedId());
    };

    loopButton.onClick = [this]
    {
        auto info = audioEngine.getProjectInfo();
        audioEngine.setLoopEnabled (! info.loopEnabled);
        refreshFromEngine();
    };

    clickButton.onClick = [this]
    {
        auto info = audioEngine.getProjectInfo();
        audioEngine.setMetronomeEnabled (! info.metronomeEnabled);
        refreshFromEngine();
    };

    recordButton.onClick = [this]
    {
        auto info = audioEngine.getProjectInfo();
        audioEngine.setRecordingEnabled (! info.recordingEnabled);
        refreshFromEngine();
    };

    clipGainSlider.onValueChange = [this]
    {
        if (! refreshing && hasSelectedClip())
            audioEngine.setClipGainDb (selectedClipId, static_cast<float> (clipGainSlider.getValue()));
    };

    auto clipFadeChanged = [this]
    {
        if (! refreshing && hasSelectedClip())
            audioEngine.setClipFades (selectedClipId, fadeInSlider.getValue(), fadeOutSlider.getValue());
    };

    fadeInSlider.onValueChange = clipFadeChanged;
    fadeOutSlider.onValueChange = clipFadeChanged;

    clipTransposeSlider.onValueChange = [this]
    {
        if (! refreshing && hasSelectedClip())
            audioEngine.setClipTransposeSemitones (selectedClipId, static_cast<float> (clipTransposeSlider.getValue()));
    };

    clipLoopButton.onClick = [this]
    {
        if (hasSelectedClip())
        {
            const auto clip = audioEngine.getClipInfo (selectedClipId);
            audioEngine.setClipLoopEnabled (selectedClipId, ! clip.loopEnabled);
            refreshFromEngine();
        }
    };

    splitButton.onClick = [this]
    {
        if (onSplitSelectedClip)
            onSplitSelectedClip();
    };

    eqButton.onClick = [this]
    {
        if (hasSelectedTrack())
        {
            const auto state = audioEngine.getTrackDeviceState (selectedTrackId);
            audioEngine.setTrackEqEnabled (selectedTrackId, ! state.eqEnabled);
            refreshFromEngine();
        }
    };

    for (auto i = 0; i < static_cast<int> (eqGainSliders.size()); ++i)
    {
        eqGainSliders[static_cast<size_t> (i)].onValueChange = [this, i]
        {
            if (! refreshing && hasSelectedTrack())
                audioEngine.setTrackEqBandGainDb (selectedTrackId, i, static_cast<float> (eqGainSliders[static_cast<size_t> (i)].getValue()));
        };
    }

    pitchButton.onClick = [this]
    {
        if (hasSelectedTrack())
        {
            const auto state = audioEngine.getTrackDeviceState (selectedTrackId);
            audioEngine.setTrackPitchShiftEnabled (selectedTrackId, ! state.pitchShiftEnabled);
            refreshFromEngine();
        }
    };

    pitchSlider.onValueChange = [this]
    {
        if (! refreshing && hasSelectedTrack())
            audioEngine.setTrackPitchShiftSemitones (selectedTrackId, static_cast<float> (pitchSlider.getValue()));
    };

    populateBrowser();
    refreshFromEngine();
}

void WorkspacePanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient gradient { tokens::surfaceChrome,
                                    bounds.getX(),
                                    bounds.getY(),
                                    tokens::surfaceInset,
                                    bounds.getX(),
                                    bounds.getBottom(),
                                    false };
    gradient.addColour (tokens::toolbarGradientMidpoint, tokens::surfaceElevated);
    g.setGradientFill (gradient);
    g.fillAll();

    ui::drawSoftGlow (g,
                      getLocalBounds().toFloat().withSizeKeepingCentre (getWidth() * 0.90f,
                                                                        getHeight() * 0.34f)
                                               .translated (-getWidth() * 0.28f, getHeight() * 0.10f),
                      tokens::accentPrimary,
                      tokens::accentWashAlpha);

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawVerticalLine (getWidth() - 1, 0.0f, static_cast<float> (getHeight()));

    paintSectionHeader (g, browserHeader.getBounds(), "Browser");
    paintSectionHeader (g, projectHeader.getBounds(), "Project");
    paintSectionHeader (g, clipHeader.getBounds(), "Clip");
    paintSectionHeader (g, deviceHeader.getBounds(), "Devices");

    auto searchIcon = browserSearch.getBounds().removeFromLeft (tokens::browserIconCell)
                                      .withSizeKeepingCentre (tokens::iconSizeSmall, tokens::iconSizeSmall)
                                      .toFloat();
    ui::drawIcon (g, ui::Icon::search, searchIcon, tokens::textTertiary, tokens::iconStrokeWidth);
}

void WorkspacePanel::resized()
{
    auto bounds = getLocalBounds().reduced (tokens::workspacePadding);

    browserHeader.setBounds (bounds.removeFromTop (tokens::workspaceSectionHeaderHeight)
                                   .withTrimmedLeft (tokens::browserIconCell));
    browserSearch.setBounds (bounds.removeFromTop (tokens::workspaceControlHeight));
    bounds.removeFromTop (tokens::workspaceSmallGap);

    for (auto& item : browserItems)
        item.setBounds (bounds.removeFromTop (tokens::workspaceBrowserItemHeight));

    bounds.removeFromTop (tokens::workspaceGap);
    projectHeader.setBounds (bounds.removeFromTop (tokens::workspaceSectionHeaderHeight)
                                    .withTrimmedLeft (tokens::browserIconCell));
    tempoSlider.setBounds (bounds.removeFromTop (tokens::workspaceRowHeight));

    auto projectRow = bounds.removeFromTop (tokens::workspaceRowHeight);
    signatureBox.setBounds (projectRow.removeFromLeft (tokens::workspaceButtonWidth));
    projectRow.removeFromLeft (tokens::workspaceSmallGap);
    keyBox.setBounds (projectRow.removeFromLeft (tokens::workspaceLabelWidth + tokens::workspaceButtonWidth));

    auto projectRowTwo = bounds.removeFromTop (tokens::workspaceRowHeight);
    snapBox.setBounds (projectRowTwo.removeFromLeft (tokens::workspaceButtonWidth));
    projectRowTwo.removeFromLeft (tokens::workspaceSmallGap);
    loopButton.setBounds (projectRowTwo.removeFromLeft (tokens::workspaceButtonWidth));
    projectRowTwo.removeFromLeft (tokens::workspaceSmallGap);
    clickButton.setBounds (projectRowTwo.removeFromLeft (tokens::workspaceButtonWidth));
    projectRowTwo.removeFromLeft (tokens::workspaceSmallGap);
    recordButton.setBounds (projectRowTwo.removeFromLeft (tokens::workspaceButtonWidth));

    bounds.removeFromTop (tokens::workspaceGap);
    clipHeader.setBounds (bounds.removeFromTop (tokens::workspaceSectionHeaderHeight)
                                 .withTrimmedLeft (tokens::browserIconCell));
    selectedClipLabel.setBounds (bounds.removeFromTop (tokens::workspaceRowHeight));
    clipGainSlider.setBounds (bounds.removeFromTop (tokens::workspaceRowHeight));
    fadeInSlider.setBounds (bounds.removeFromTop (tokens::workspaceRowHeight));
    fadeOutSlider.setBounds (bounds.removeFromTop (tokens::workspaceRowHeight));
    clipTransposeSlider.setBounds (bounds.removeFromTop (tokens::workspaceRowHeight));

    auto clipButtons = bounds.removeFromTop (tokens::workspaceRowHeight);
    clipLoopButton.setBounds (clipButtons.removeFromLeft (tokens::workspaceButtonWidth));
    clipButtons.removeFromLeft (tokens::workspaceSmallGap);
    splitButton.setBounds (clipButtons.removeFromLeft (tokens::workspaceButtonWidth));

    bounds.removeFromTop (tokens::workspaceGap);
    deviceHeader.setBounds (bounds.removeFromTop (tokens::workspaceSectionHeaderHeight)
                                   .withTrimmedLeft (tokens::browserIconCell));
    selectedTrackLabel.setBounds (bounds.removeFromTop (tokens::workspaceRowHeight));
    eqButton.setBounds (bounds.removeFromTop (tokens::workspaceControlHeight).removeFromLeft (tokens::workspaceButtonWidth));

    for (auto& slider : eqGainSliders)
        slider.setBounds (bounds.removeFromTop (tokens::workspaceRowHeight));

    auto pitchRow = bounds.removeFromTop (tokens::workspaceRowHeight);
    pitchButton.setBounds (pitchRow.removeFromLeft (tokens::workspaceButtonWidth));
    pitchRow.removeFromLeft (tokens::workspaceSmallGap);
    pitchSlider.setBounds (pitchRow);
}

void WorkspacePanel::refreshFromEngine()
{
    const juce::ScopedValueSetter<bool> setter (refreshing, true);

    refreshProjectControls();
    refreshClipControls();
    refreshDeviceControls();
    refreshButtonColours();
}

void WorkspacePanel::setSelection (int clipId, int trackId)
{
    selectedClipId = clipId;
    selectedTrackId = trackId;

    if (! hasSelectedTrack())
    {
        const auto tracks = audioEngine.getAllTrackInfo();

        if (! tracks.empty())
            selectedTrackId = tracks.front().id;
    }

    refreshFromEngine();
}

void WorkspacePanel::configureLabel (juce::Label& label, juce::Font font, juce::Colour colour)
{
    label.setFont (font);
    label.setColour (juce::Label::textColourId, colour);
    label.setJustificationType (juce::Justification::centredLeft);
}

void WorkspacePanel::configureButton (juce::TextButton& button)
{
    button.setWantsKeyboardFocus (false);
    button.setColour (juce::TextButton::textColourOffId, tokens::textPrimary);
    button.setColour (juce::TextButton::textColourOnId, tokens::textPrimary);
}

void WorkspacePanel::configureSlider (juce::Slider& slider, double minimum, double maximum, double interval)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, tokens::workspaceLabelWidth, tokens::workspaceControlHeight);
    slider.setRange (minimum, maximum, interval);
    slider.setColour (juce::Slider::trackColourId, tokens::accentPrimary);
    slider.setColour (juce::Slider::backgroundColourId, tokens::surfaceRaised);
    slider.setColour (juce::Slider::textBoxTextColourId, tokens::textSecondary);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, tokens::surfaceRaised);
    slider.setColour (juce::Slider::textBoxOutlineColourId, tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
}

void WorkspacePanel::configureComboBox (juce::ComboBox& comboBox)
{
    comboBox.setColour (juce::ComboBox::backgroundColourId, tokens::surfaceRaised);
    comboBox.setColour (juce::ComboBox::textColourId, tokens::textPrimary);
    comboBox.setColour (juce::ComboBox::outlineColourId, tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    comboBox.setColour (juce::ComboBox::arrowColourId, tokens::textSecondary);
}

void WorkspacePanel::paintSectionHeader (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title)
{
    if (bounds.isEmpty())
        return;

    auto headerIcon = ui::Icon::sliders;

    if (title == "Browser")
        headerIcon = ui::Icon::folder;
    else if (title == "Project")
        headerIcon = ui::Icon::project;
    else if (title == "Clip")
        headerIcon = ui::Icon::clip;

    auto iconBounds = bounds.withX (tokens::workspacePadding)
                            .withWidth (tokens::browserIconCell)
                            .withSizeKeepingCentre (tokens::iconSizeMedium, tokens::iconSizeMedium)
                            .toFloat();
    ui::drawIcon (g, headerIcon, iconBounds, tokens::accentPrimary, tokens::iconStrokeWidth);

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawHorizontalLine (bounds.getBottom() - 1, static_cast<float> (bounds.getX()), static_cast<float> (bounds.getRight()));
}

void WorkspacePanel::refreshProjectControls()
{
    const auto info = audioEngine.getProjectInfo();
    tempoSlider.setValue (info.tempoBpm, juce::dontSendNotification);
    signatureBox.setText (signatureText (info), juce::dontSendNotification);
    keyBox.setText (info.keyName, juce::dontSendNotification);
    snapBox.setSelectedId (info.snapDivision, juce::dontSendNotification);
    loopButton.setToggleState (info.loopEnabled, juce::dontSendNotification);
    clickButton.setToggleState (info.metronomeEnabled, juce::dontSendNotification);
    recordButton.setToggleState (info.recordingEnabled, juce::dontSendNotification);
}

void WorkspacePanel::refreshClipControls()
{
    const auto clip = audioEngine.getClipInfo (selectedClipId);
    const auto enabled = clip.id > 0;

    selectedClipLabel.setText (enabled ? clip.name : "No clip selected", juce::dontSendNotification);
    clipGainSlider.setEnabled (enabled);
    fadeInSlider.setEnabled (enabled);
    fadeOutSlider.setEnabled (enabled);
    clipTransposeSlider.setEnabled (enabled);
    clipLoopButton.setEnabled (enabled);
    splitButton.setEnabled (enabled);

    if (enabled)
    {
        clipGainSlider.setValue (clip.gainDb, juce::dontSendNotification);
        fadeInSlider.setValue (clip.fadeInSeconds, juce::dontSendNotification);
        fadeOutSlider.setValue (clip.fadeOutSeconds, juce::dontSendNotification);
        clipTransposeSlider.setValue (clip.transposeSemitones, juce::dontSendNotification);
        clipLoopButton.setToggleState (clip.loopEnabled, juce::dontSendNotification);
        selectedTrackId = clip.trackId;
    }
    else
    {
        clipGainSlider.setValue (0.0, juce::dontSendNotification);
        fadeInSlider.setValue (0.0, juce::dontSendNotification);
        fadeOutSlider.setValue (0.0, juce::dontSendNotification);
        clipTransposeSlider.setValue (0.0, juce::dontSendNotification);
        clipLoopButton.setToggleState (false, juce::dontSendNotification);
    }
}

void WorkspacePanel::refreshDeviceControls()
{
    auto track = audioEngine.getTrackInfo (selectedTrackId);
    const auto enabled = track.id > 0;
    const auto state = audioEngine.getTrackDeviceState (selectedTrackId);

    selectedTrackLabel.setText (enabled ? track.name : "No track selected", juce::dontSendNotification);
    eqButton.setEnabled (enabled);
    pitchButton.setEnabled (enabled);
    pitchSlider.setEnabled (enabled);
    eqButton.setToggleState (enabled && state.eqEnabled, juce::dontSendNotification);
    pitchButton.setToggleState (enabled && state.pitchShiftEnabled, juce::dontSendNotification);
    pitchSlider.setValue (enabled ? state.pitchShiftSemitones : 0.0f, juce::dontSendNotification);

    for (auto i = 0; i < static_cast<int> (eqGainSliders.size()); ++i)
    {
        auto& slider = eqGainSliders[static_cast<size_t> (i)];
        slider.setEnabled (enabled);
        slider.setValue (enabled ? state.eqGainDb[static_cast<size_t> (i)] : 0.0f, juce::dontSendNotification);
    }
}

void WorkspacePanel::refreshButtonColours()
{
    auto colourButton = [] (juce::TextButton& button, juce::Colour activeColour)
    {
        button.setColour (juce::TextButton::buttonColourId, button.getToggleState() ? activeColour : tokens::surfaceRaised);
        button.setColour (juce::TextButton::buttonOnColourId, activeColour);
    };

    colourButton (loopButton, tokens::accentPrimary);
    colourButton (clickButton, tokens::accentSecondary);
    colourButton (recordButton, tokens::error);
    colourButton (clipLoopButton, tokens::accentPrimary);
    colourButton (eqButton, tokens::accentSecondary);
    colourButton (pitchButton, tokens::accentPrimary);
}

void WorkspacePanel::populateBrowser()
{
    struct BrowserSeed
    {
        juce::String text;
        ui::Icon icon;
        juce::Colour colour;
    };

    const std::array<BrowserSeed, 8> items
    {
        BrowserSeed { "Audio Files", ui::Icon::audio, tokens::trackColors[4] },
        BrowserSeed { "MIDI Clips", ui::Icon::midi, tokens::trackColors[6] },
        BrowserSeed { "Drums", ui::Icon::drums, tokens::trackColors[1] },
        BrowserSeed { "Instruments", ui::Icon::keyboard, tokens::trackColors[3] },
        BrowserSeed { "Audio Effects", ui::Icon::effects, tokens::trackColors[5] },
        BrowserSeed { "MIDI Effects", ui::Icon::sparkle, tokens::trackColors[2] },
        BrowserSeed { "Project Media", ui::Icon::project, tokens::textSecondary },
        BrowserSeed { "Markers", ui::Icon::marker, tokens::accentPrimary }
    };

    for (auto i = 0; i < static_cast<int> (browserItems.size()); ++i)
    {
        const auto& item = items[static_cast<size_t> (i)];
        browserItems[static_cast<size_t> (i)].setItem (item.text, item.icon, item.colour, i == 0);
    }
}

bool WorkspacePanel::hasSelectedClip() const
{
    return audioEngine.getClipInfo (selectedClipId).id > 0;
}

bool WorkspacePanel::hasSelectedTrack() const
{
    return audioEngine.getTrackInfo (selectedTrackId).id > 0;
}
