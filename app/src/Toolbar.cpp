#include "Toolbar.h"

namespace tokens = tracklab::design;
namespace ui = tracklab::ui;

ToolbarButton::ToolbarButton (const juce::String& text, ui::Icon iconToUse)
    : juce::TextButton (text),
      icon (iconToUse)
{
    setWantsKeyboardFocus (false);
}

void ToolbarButton::paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);
    auto base = tokens::surfaceRaised;

    if (! isEnabled())
        base = tokens::surfaceElevated;
    else if (getToggleState())
        base = findColour (juce::TextButton::buttonOnColourId);
    else if (isButtonDown)
        base = base.darker (tokens::surfaceGradientAmount);
    else if (isMouseOverButton)
        base = base.brighter (tokens::surfaceGradientAmount);

    ui::drawGlassPanel (g, bounds, base, tokens::buttonCornerRadius, getToggleState());

    auto content = getLocalBounds().reduced (tokens::trackHeaderSmallGap, 0);
    const auto iconColour = isEnabled() ? tokens::textPrimary : tokens::textTertiary;

    if (icon != ui::Icon::none)
    {
        auto iconBounds = content.removeFromLeft (tokens::toolbarIconSize + tokens::trackHeaderSmallGap)
                                 .withSizeKeepingCentre (tokens::toolbarIconSize, tokens::toolbarIconSize)
                                 .toFloat();
        ui::drawIcon (g, icon, iconBounds, iconColour, tokens::iconStrokeWidth);
    }

    g.setColour (isEnabled() ? tokens::textPrimary : tokens::textTertiary);
    g.setFont (tokens::fontBody());
    g.drawFittedText (getButtonText(), content, juce::Justification::centred, 1);
}

void ToolbarButton::setIcon (ui::Icon iconToUse)
{
    icon = iconToUse;
    repaint();
}

Toolbar::Toolbar()
{
    addAndMakeVisible (wordmarkLabel);
    addAndMakeVisible (loadButton);
    addAndMakeVisible (playPauseButton);
    addAndMakeVisible (stopButton);
    addAndMakeVisible (recordButton);
    addAndMakeVisible (loopButton);
    addAndMakeVisible (clickButton);
    addAndMakeVisible (tempoLabel);
    addAndMakeVisible (signatureLabel);
    addAndMakeVisible (positionLabel);

    loadButton.onClick = [this]
    {
        if (onLoadClicked)
            onLoadClicked();
    };

    playPauseButton.onClick = [this]
    {
        if (onPlayPauseClicked)
            onPlayPauseClicked();
    };

    stopButton.onClick = [this]
    {
        if (onStopClicked)
            onStopClicked();
    };

    recordButton.onClick = [this]
    {
        if (onRecordClicked)
            onRecordClicked();
    };

    loopButton.onClick = [this]
    {
        if (onLoopClicked)
            onLoopClicked();
    };

    clickButton.onClick = [this]
    {
        if (onMetronomeClicked)
            onMetronomeClicked();
    };

    recordButton.setColour (juce::TextButton::buttonOnColourId, tokens::error);
    loopButton.setColour (juce::TextButton::buttonOnColourId, tokens::accentPrimary);
    clickButton.setColour (juce::TextButton::buttonOnColourId, tokens::accentSecondary);

    wordmarkLabel.setText ("Tracklab", juce::dontSendNotification);
    wordmarkLabel.setFont (tokens::fontHeader());
    wordmarkLabel.setJustificationType (juce::Justification::centredLeft);
    wordmarkLabel.setColour (juce::Label::textColourId, tokens::textPrimary);

    auto configurePill = [] (juce::Label& label)
    {
        label.setFont (tokens::fontMonospace());
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, tokens::textSecondary);
        label.setColour (juce::Label::backgroundColourId, tokens::surfaceRaised.withAlpha (0.0f));
        label.setColour (juce::Label::outlineColourId, tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    };

    configurePill (tempoLabel);
    configurePill (signatureLabel);
    configurePill (positionLabel);

    tempoLabel.setText ("120.00 BPM", juce::dontSendNotification);
    signatureLabel.setText ("4/4", juce::dontSendNotification);

    setHasLoadedFile (false);
    setPosition (0.0, 0.0);
}

void Toolbar::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient gradient { tokens::surfaceChrome.brighter (tokens::surfaceGradientAmount),
                                    bounds.getX(),
                                    bounds.getY(),
                                    tokens::surfaceElevated.darker (tokens::surfaceGradientAmount),
                                    bounds.getX(),
                                    bounds.getBottom(),
                                    false };
    gradient.addColour (tokens::toolbarGradientMidpoint, tokens::surfaceElevated);
    g.setGradientFill (gradient);
    g.fillAll();

    ui::drawSoftGlow (g, wordmarkLabel.getBounds().toFloat().expanded (tokens::glowRadius),
                      tokens::accentPrimary,
                      tokens::accentWashAlpha);

    if (playPauseButton.getWidth() > 0 && stopButton.getWidth() > 0)
    {
        auto transportBounds = playPauseButton.getBounds().getUnion (stopButton.getBounds())
                                                    .getUnion (recordButton.getBounds())
                                                    .getUnion (loopButton.getBounds())
                                                    .getUnion (clickButton.getBounds())
                                                    .expanded (tokens::trackHeaderSmallGap, 0)
                                                    .toFloat();
        ui::drawGlassPanel (g, transportBounds, tokens::surfaceInset, tokens::controlGroupRadius, false);
    }

    if (wordmarkLabel.getWidth() > 0)
    {
        auto iconBounds = wordmarkLabel.getBounds()
                                       .withX (wordmarkLabel.getX() - tokens::toolbarWordmarkIconSize - tokens::trackHeaderSmallGap)
                                       .withWidth (tokens::toolbarWordmarkIconSize)
                                       .withSizeKeepingCentre (tokens::toolbarWordmarkIconSize,
                                                               tokens::toolbarWordmarkIconSize)
                                       .toFloat();
        ui::drawIcon (g, ui::Icon::waveform, iconBounds, tokens::accentPrimary, tokens::thickIconStrokeWidth);
        ui::drawTinySparkles (g, iconBounds.expanded (tokens::trackHeaderSmallGap), tokens::accentCyan);
    }

    for (const auto* label : { &tempoLabel, &signatureLabel, &positionLabel })
        if (label->getWidth() > 0)
            ui::drawGlassPanel (g, label->getBounds().toFloat().reduced (0.5f),
                                tokens::surfaceInset,
                                tokens::buttonCornerRadius,
                                false);

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, static_cast<float> (getWidth()));
}

void Toolbar::resized()
{
    auto bounds = getLocalBounds().reduced (tokens::toolbarPaddingX, 0);
    auto row = bounds.withHeight (tokens::toolbarButtonHeight)
                     .withCentre ({ bounds.getCentreX(), getHeight() / 2 });

    auto wordmark = row.removeFromLeft (tokens::toolbarWordmarkWidth);
    wordmark.removeFromLeft (tokens::toolbarWordmarkIconSize + tokens::trackHeaderSmallGap);
    wordmarkLabel.setBounds (wordmark);
    row.removeFromLeft (tokens::toolbarGap);
    loadButton.setBounds (row.removeFromLeft (tokens::toolbarButtonWidth));
    row.removeFromLeft (tokens::toolbarGap);

    auto transportGroup = row.removeFromLeft (tokens::toolbarTransportGroupWidth).reduced (tokens::trackHeaderSmallGap, 0);
    playPauseButton.setBounds (transportGroup.removeFromLeft (tokens::toolbarTransportButtonWidth));
    transportGroup.removeFromLeft (tokens::trackHeaderSmallGap);
    stopButton.setBounds (transportGroup.removeFromLeft (tokens::toolbarTransportButtonWidth));
    transportGroup.removeFromLeft (tokens::trackHeaderSmallGap);
    recordButton.setBounds (transportGroup.removeFromLeft (tokens::toolbarSmallButtonWidth));
    transportGroup.removeFromLeft (tokens::trackHeaderSmallGap);
    loopButton.setBounds (transportGroup.removeFromLeft (tokens::toolbarSmallButtonWidth));
    transportGroup.removeFromLeft (tokens::trackHeaderSmallGap);
    clickButton.setBounds (transportGroup.removeFromLeft (tokens::toolbarSmallButtonWidth));

    row.removeFromLeft (tokens::toolbarGap);
    tempoLabel.setBounds (row.removeFromLeft (tokens::toolbarTempoWidth)
                              .withSizeKeepingCentre (tokens::toolbarTempoWidth, tokens::toolbarPillHeight));
    row.removeFromLeft (tokens::toolbarGap);
    signatureLabel.setBounds (row.removeFromLeft (tokens::toolbarSignatureWidth)
                                  .withSizeKeepingCentre (tokens::toolbarSignatureWidth, tokens::toolbarPillHeight));
    row.removeFromLeft (tokens::toolbarGap);
    positionLabel.setBounds (row.removeFromLeft (tokens::toolbarPositionWidth));
}

void Toolbar::setHasLoadedFile (bool hasFile)
{
    hasLoadedFile = hasFile;
    playPauseButton.setEnabled (hasLoadedFile);
    stopButton.setEnabled (hasLoadedFile);
    loadButton.setButtonText ("Load");
}

void Toolbar::setPlaying (bool shouldShowPlaying)
{
    isPlaying = shouldShowPlaying;
    playPauseButton.setButtonText (isPlaying ? "Pause" : "Play");
    playPauseButton.setIcon (isPlaying ? ui::Icon::pause : ui::Icon::play);
}

void Toolbar::setPosition (double positionSeconds, double lengthSeconds)
{
    positionLabel.setText (formatTime (positionSeconds) + " / " + formatTime (lengthSeconds),
                           juce::dontSendNotification);
}

void Toolbar::setProjectInfo (const ProjectInfo& info)
{
    tempoLabel.setText (juce::String (info.tempoBpm, 2) + " BPM", juce::dontSendNotification);
    signatureLabel.setText (juce::String::formatted ("%d/%d",
                                                     info.timeSignatureNumerator,
                                                     info.timeSignatureDenominator),
                            juce::dontSendNotification);
    loopButton.setToggleState (info.loopEnabled, juce::dontSendNotification);
    clickButton.setToggleState (info.metronomeEnabled, juce::dontSendNotification);
    recordButton.setToggleState (info.recordingEnabled, juce::dontSendNotification);
}

juce::String Toolbar::formatTime (double seconds)
{
    const auto totalMilliseconds = juce::jmax (0, juce::roundToInt (seconds * tokens::millisecondsPerSecond));
    const auto minutes = totalMilliseconds / tokens::millisecondsPerMinute;
    const auto secondsPart = (totalMilliseconds / tokens::millisecondsPerSecond) % tokens::secondsPerMinute;
    const auto milliseconds = totalMilliseconds % tokens::millisecondsPerSecond;

    return juce::String::formatted ("%02d:%02d.%03d", minutes, secondsPart, milliseconds);
}
