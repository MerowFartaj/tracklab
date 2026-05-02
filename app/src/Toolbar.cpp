#include "Toolbar.h"

#include <cmath>

namespace tokens = tracklab::design;
namespace ui = tracklab::ui;

namespace
{
juce::Rectangle<float> centredIcon (juce::Rectangle<int> bounds, int size)
{
    return bounds.withSizeKeepingCentre (size, size).toFloat();
}
}

ToolbarButton::ToolbarButton (const juce::String& text, ui::Icon iconToUse)
    : juce::TextButton (text),
      icon (iconToUse)
{
    setWantsKeyboardFocus (false);
}

void ToolbarButton::paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);
    auto base = tokens::surfaceInset;

    if (! isEnabled())
        base = tokens::surfaceInset;
    else if (getToggleState())
        base = findColour (juce::TextButton::buttonOnColourId);
    else if (isButtonDown)
        base = base.darker (tokens::surfaceGradientAmount * 2.0f);
    else if (isMouseOverButton)
        base = tokens::surfaceRaised;

    ui::drawGlassPanel (g, bounds, base, tokens::buttonCornerRadius, getToggleState());

    auto content = getLocalBounds().reduced (tokens::trackHeaderSmallGap, 0);
    const auto textColour = isEnabled() ? tokens::textPrimary : tokens::textTertiary;
    const auto iconColour = getToggleState() ? tokens::backgroundDeep : textColour;

    if (icon != ui::Icon::none && getButtonText().isEmpty())
    {
        ui::drawIcon (g,
                      icon,
                      centredIcon (getLocalBounds(), tokens::toolbarIconSize),
                      iconColour,
                      tokens::iconStrokeWidth);
        return;
    }

    if (icon != ui::Icon::none)
    {
        auto iconCell = content.removeFromLeft (tokens::toolbarIconSize + (getButtonText().isEmpty() ? 0 : tokens::trackHeaderSmallGap));
        ui::drawIcon (g, icon, centredIcon (iconCell, tokens::toolbarIconSize), iconColour, tokens::iconStrokeWidth);
    }

    if (getButtonText().isNotEmpty())
    {
        g.setColour (textColour);
        g.setFont (tokens::fontBody().withStyle (juce::Font::bold));
        g.drawFittedText (getButtonText(), content, juce::Justification::centred, 1);
    }
}

void ToolbarButton::setIcon (ui::Icon iconToUse)
{
    icon = iconToUse;
    repaint();
}

TrafficLightButton::TrafficLightButton (juce::Colour colourToUse)
    : Button (""),
      colour (colourToUse)
{
    setWantsKeyboardFocus (false);
}

void TrafficLightButton::paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = getLocalBounds().toFloat().withSizeKeepingCentre (12.0f, 12.0f);
    auto fill = colour;

    if (isButtonDown)
        fill = fill.darker (0.12f);
    else if (isMouseOverButton)
        fill = fill.brighter (0.08f);

    g.setColour (tokens::shadowBase.withAlpha (0.32f));
    g.fillEllipse (bounds.translated (0.0f, 1.0f));
    g.setColour (fill);
    g.fillEllipse (bounds);
    g.setColour (tokens::highlightBase.withAlpha (0.22f));
    g.drawEllipse (bounds.reduced (1.0f), 1.0f);
}

Toolbar::Toolbar()
{
    for (auto* button : { static_cast<juce::Button*> (&closeButton),
                          static_cast<juce::Button*> (&minimiseButton),
                          static_cast<juce::Button*> (&zoomButton),
                          static_cast<juce::Button*> (&loadButton),
                          static_cast<juce::Button*> (&rewindButton),
                          static_cast<juce::Button*> (&playPauseButton),
                          static_cast<juce::Button*> (&stopButton),
                          static_cast<juce::Button*> (&recordButton),
                          static_cast<juce::Button*> (&loopButton),
                          static_cast<juce::Button*> (&clickButton),
                          static_cast<juce::Button*> (&speakerButton),
                          static_cast<juce::Button*> (&settingsButton) })
    {
        addAndMakeVisible (*button);
    }

    for (auto* label : { &wordmarkLabel, &tempoLabel, &signatureLabel, &keyLabel, &snapLabel, &quantizeLabel, &positionLabel })
        addAndMakeVisible (*label);

    addAndMakeVisible (outputSlider);

    closeButton.onClick = [this]
    {
        if (onCloseClicked)
            onCloseClicked();
    };

    minimiseButton.onClick = [this]
    {
        if (onMinimiseClicked)
            onMinimiseClicked();
    };

    zoomButton.onClick = [this]
    {
        if (onZoomClicked)
            onZoomClicked();
    };

    loadButton.onClick = [this]
    {
        if (onLoadClicked)
            onLoadClicked();
    };

    rewindButton.onClick = [this]
    {
        if (onStopClicked)
            onStopClicked();
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

    playPauseButton.setColour (juce::TextButton::buttonOnColourId, tokens::success);
    recordButton.setColour (juce::TextButton::buttonOnColourId, tokens::error);
    loopButton.setColour (juce::TextButton::buttonOnColourId, tokens::accentPrimary);
    clickButton.setColour (juce::TextButton::buttonOnColourId, tokens::surfaceRaised);

    wordmarkLabel.setText ("Tracklab", juce::dontSendNotification);
    wordmarkLabel.setFont (juce::Font { juce::FontOptions { 18.0f, juce::Font::bold } });
    wordmarkLabel.setJustificationType (juce::Justification::centredLeft);
    wordmarkLabel.setColour (juce::Label::textColourId, tokens::textPrimary);

    auto configurePill = [] (juce::Label& label, bool strong = false)
    {
        label.setFont (strong ? juce::Font { juce::FontOptions { juce::Font::getDefaultMonospacedFontName(), 18.0f, juce::Font::plain } }
                              : tokens::fontMonospace());
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, strong ? tokens::textPrimary : tokens::textSecondary);
        label.setColour (juce::Label::backgroundColourId, tokens::surfaceInset.withAlpha (0.0f));
        label.setColour (juce::Label::outlineColourId, tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    };

    configurePill (positionLabel, true);
    configurePill (tempoLabel);
    configurePill (signatureLabel);
    configurePill (keyLabel);
    configurePill (snapLabel);
    configurePill (quantizeLabel);

    snapLabel.setText ("Snap\nBar", juce::dontSendNotification);
    quantizeLabel.setText ("Quantize\n1/16", juce::dontSendNotification);
    tempoLabel.setText ("120.00\nBPM", juce::dontSendNotification);
    signatureLabel.setText ("4/4\nTIME", juce::dontSendNotification);
    keyLabel.setText ("C Major\nKEY", juce::dontSendNotification);

    outputSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    outputSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    outputSlider.setRange (0.0, 1.0, 0.001);
    outputSlider.setValue (0.72, juce::dontSendNotification);
    outputSlider.setColour (juce::Slider::trackColourId, tokens::accentPrimary);
    outputSlider.setColour (juce::Slider::backgroundColourId, tokens::surfaceRaised);

    setHasLoadedFile (false);
    setPosition (26.0, tokens::demoTimelineLengthSeconds);
}

void Toolbar::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient gradient { tokens::surfaceChrome.brighter (0.02f),
                                    bounds.getX(),
                                    bounds.getY(),
                                    tokens::surfaceInset.darker (0.02f),
                                    bounds.getX(),
                                    bounds.getBottom(),
                                    false };
    gradient.addColour (0.5, tokens::backgroundDeep);
    g.setGradientFill (gradient);
    g.fillAll();

    auto drawContainer = [&g] (juce::Rectangle<int> area, float radius)
    {
        if (! area.isEmpty())
            ui::drawGlassPanel (g, area.toFloat().reduced (0.5f), tokens::surfaceInset, radius, false);
    };

    drawContainer (loadButton.getBounds(), tokens::buttonCornerRadius);
    drawContainer (rewindButton.getBounds().getUnion (loopButton.getBounds()).expanded (tokens::toolbarGap, 0),
                   tokens::controlGroupRadius);
    drawContainer (positionLabel.getBounds().getUnion (keyLabel.getBounds()).expanded (tokens::toolbarGap, 0),
                   tokens::controlGroupRadius);
    drawContainer (snapLabel.getBounds().getUnion (quantizeLabel.getBounds()).expanded (tokens::toolbarGap, 0),
                   tokens::controlGroupRadius);
    drawContainer (clickButton.getBounds().expanded (tokens::toolbarGap, 0), tokens::controlGroupRadius);
    drawContainer (settingsButton.getBounds(), tokens::buttonCornerRadius);

    if (wordmarkLabel.getWidth() > 0)
    {
        auto iconBounds = wordmarkLabel.getBounds()
                                       .withX (wordmarkLabel.getX() - tokens::toolbarWordmarkIconSize - tokens::trackHeaderSmallGap)
                                       .withWidth (tokens::toolbarWordmarkIconSize)
                                       .withSizeKeepingCentre (tokens::toolbarWordmarkIconSize,
                                                               tokens::toolbarWordmarkIconSize)
                                       .toFloat();
        ui::drawIcon (g, ui::Icon::waveform, iconBounds, tokens::accentPrimary, tokens::thickIconStrokeWidth);
    }

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, static_cast<float> (getWidth()));
}

void Toolbar::resized()
{
    auto bounds = getLocalBounds().reduced (10, 0);
    auto row = bounds.withHeight (tokens::toolbarButtonHeight)
                     .withCentre ({ bounds.getCentreX(), getHeight() / 2 });

    closeButton.setBounds (row.removeFromLeft (18));
    minimiseButton.setBounds (row.removeFromLeft (18));
    zoomButton.setBounds (row.removeFromLeft (18));
    row.removeFromLeft (8);

    auto wordmark = row.removeFromLeft (tokens::toolbarWordmarkWidth);
    wordmark.removeFromLeft (tokens::toolbarWordmarkIconSize + tokens::trackHeaderSmallGap);
    wordmarkLabel.setBounds (wordmark);
    row.removeFromLeft (tokens::toolbarGap);

    loadButton.setBounds (row.removeFromLeft (tokens::toolbarButtonWidth));
    row.removeFromLeft (tokens::toolbarGap);
    rewindButton.setBounds (row.removeFromLeft (tokens::toolbarTransportButtonWidth));
    playPauseButton.setBounds (row.removeFromLeft (tokens::toolbarTransportButtonWidth));
    stopButton.setBounds (row.removeFromLeft (tokens::toolbarTransportButtonWidth));
    recordButton.setBounds (row.removeFromLeft (tokens::toolbarTransportButtonWidth));
    loopButton.setBounds (row.removeFromLeft (tokens::toolbarTransportButtonWidth));
    row.removeFromLeft (tokens::toolbarGap);

    positionLabel.setBounds (row.removeFromLeft (tokens::toolbarPositionWidth));
    tempoLabel.setBounds (row.removeFromLeft (tokens::toolbarTempoWidth));
    signatureLabel.setBounds (row.removeFromLeft (tokens::toolbarSignatureWidth));
    keyLabel.setBounds (row.removeFromLeft (76));
    row.removeFromLeft (tokens::toolbarGap);

    snapLabel.setBounds (row.removeFromLeft (72));
    quantizeLabel.setBounds (row.removeFromLeft (88));
    row.removeFromLeft (tokens::toolbarGap);

    clickButton.setBounds (row.removeFromLeft (118));
    row.removeFromLeft (tokens::toolbarGap);
    speakerButton.setBounds (row.removeFromLeft (tokens::toolbarTransportButtonWidth));
    outputSlider.setBounds (row.removeFromLeft (92));
    row.removeFromLeft (tokens::toolbarGap);
    settingsButton.setBounds (row.removeFromRight (tokens::toolbarTransportButtonWidth));
}

void Toolbar::mouseDown (const juce::MouseEvent& event)
{
    if (auto* topLevel = getTopLevelComponent())
        windowDragger.startDraggingComponent (topLevel, event);
}

void Toolbar::mouseDrag (const juce::MouseEvent& event)
{
    if (auto* topLevel = getTopLevelComponent())
        windowDragger.dragComponent (topLevel, event, nullptr);
}

void Toolbar::setHasLoadedFile (bool hasFile)
{
    hasLoadedFile = hasFile;
    playPauseButton.setEnabled (hasLoadedFile);
    stopButton.setEnabled (hasLoadedFile);
    rewindButton.setEnabled (hasLoadedFile);
    loadButton.setButtonText ("Load");
}

void Toolbar::setPlaying (bool shouldShowPlaying)
{
    isPlaying = shouldShowPlaying;
    playPauseButton.setToggleState (isPlaying, juce::dontSendNotification);
    playPauseButton.setIcon (isPlaying ? ui::Icon::pause : ui::Icon::play);
}

void Toolbar::setPosition (double positionSeconds, double)
{
    positionLabel.setText (formatBarsBeatsTicks (positionSeconds), juce::dontSendNotification);
}

void Toolbar::setProjectInfo (const ProjectInfo& info)
{
    tempoLabel.setText (juce::String (info.tempoBpm, 2) + "\nBPM", juce::dontSendNotification);
    signatureLabel.setText (juce::String::formatted ("%d/%d\nTIME",
                                                     info.timeSignatureNumerator,
                                                     info.timeSignatureDenominator),
                            juce::dontSendNotification);
    keyLabel.setText (info.keyName + "\nKEY", juce::dontSendNotification);
    snapLabel.setText ("Snap\nBar", juce::dontSendNotification);
    quantizeLabel.setText ("Quantize\n1/16", juce::dontSendNotification);
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

juce::String Toolbar::formatBarsBeatsTicks (double seconds)
{
    const auto secondsPerBeat = tokens::secondsPerMinute / tokens::defaultTempoBpm;
    const auto totalTicks = juce::jmax (0, juce::roundToInt (seconds / secondsPerBeat * 960.0));
    const auto ticksPerBar = 960 * tokens::beatsPerBar;
    const auto bar = totalTicks / ticksPerBar + 1;
    const auto beat = (totalTicks % ticksPerBar) / 960 + 1;
    const auto tick = totalTicks % 960;

    return juce::String::formatted ("%02d:%02d:%02d:%02d", bar, beat, tick / 40, tick % 40);
}
