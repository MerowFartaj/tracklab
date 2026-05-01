#include "Toolbar.h"

namespace tokens = tracklab::design;

ToolbarButton::ToolbarButton (const juce::String& text)
    : juce::TextButton (text)
{
    setWantsKeyboardFocus (false);
}

void ToolbarButton::paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);
    auto base = tokens::surfaceRaised;

    if (! isEnabled())
        base = tokens::surfaceElevated;
    else if (isButtonDown)
        base = base.darker (tokens::surfaceGradientAmount);
    else if (isMouseOverButton)
        base = base.brighter (tokens::surfaceGradientAmount);

    g.setGradientFill (tokens::verticalSurfaceGradient (base, bounds));
    g.fillRoundedRectangle (bounds, tokens::buttonCornerRadius);

    g.setColour (tokens::highlightBase.withAlpha (tokens::panelTopHighlightAlpha));
    g.drawHorizontalLine (0, bounds.getX() + tokens::buttonCornerRadius, bounds.getRight() - tokens::buttonCornerRadius);

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawRoundedRectangle (bounds, tokens::buttonCornerRadius, 1.0f);

    g.setColour (isEnabled() ? tokens::textPrimary : tokens::textTertiary);
    g.setFont (tokens::fontBody());
    g.drawFittedText (getButtonText(), getLocalBounds(), juce::Justification::centred, 1);
}

Toolbar::Toolbar()
{
    addAndMakeVisible (loadButton);
    addAndMakeVisible (playPauseButton);
    addAndMakeVisible (stopButton);
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

    positionLabel.setFont (tokens::fontMonospace());
    positionLabel.setJustificationType (juce::Justification::centredLeft);
    positionLabel.setColour (juce::Label::textColourId, tokens::textSecondary);

    setHasLoadedFile (false);
    setPosition (0.0, 0.0);
}

void Toolbar::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (tokens::verticalSurfaceGradient (tokens::surfaceElevated, bounds));
    g.fillAll();

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, static_cast<float> (getWidth()));
}

void Toolbar::resized()
{
    auto bounds = getLocalBounds().reduced (tokens::toolbarPaddingX, 0);
    auto row = bounds.withHeight (tokens::toolbarButtonHeight)
                     .withCentre ({ bounds.getCentreX(), getHeight() / 2 });

    loadButton.setBounds (row.removeFromLeft (tokens::toolbarButtonWidth));
    row.removeFromLeft (tokens::toolbarGap);
    playPauseButton.setBounds (row.removeFromLeft (tokens::toolbarTransportButtonWidth));
    row.removeFromLeft (tokens::toolbarGap);
    stopButton.setBounds (row.removeFromLeft (tokens::toolbarTransportButtonWidth));
    row.removeFromLeft (tokens::toolbarGap);
    positionLabel.setBounds (row.removeFromLeft (tokens::toolbarPositionWidth));
}

void Toolbar::setHasLoadedFile (bool hasFile)
{
    hasLoadedFile = hasFile;
    playPauseButton.setEnabled (hasLoadedFile);
    stopButton.setEnabled (hasLoadedFile);
    loadButton.setButtonText (hasLoadedFile ? "Load different file" : "Load audio file");
}

void Toolbar::setPlaying (bool shouldShowPlaying)
{
    isPlaying = shouldShowPlaying;
    playPauseButton.setButtonText (isPlaying ? "Pause" : "Play");
}

void Toolbar::setPosition (double positionSeconds, double lengthSeconds)
{
    positionLabel.setText (formatTime (positionSeconds) + " / " + formatTime (lengthSeconds),
                           juce::dontSendNotification);
}

juce::String Toolbar::formatTime (double seconds)
{
    const auto totalMilliseconds = juce::jmax (0, juce::roundToInt (seconds * tokens::millisecondsPerSecond));
    const auto minutes = totalMilliseconds / tokens::millisecondsPerMinute;
    const auto secondsPart = (totalMilliseconds / tokens::millisecondsPerSecond) % tokens::secondsPerMinute;
    const auto milliseconds = totalMilliseconds % tokens::millisecondsPerSecond;

    return juce::String::formatted ("%02d:%02d.%03d", minutes, secondsPart, milliseconds);
}
