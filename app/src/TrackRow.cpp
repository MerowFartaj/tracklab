#include "TrackRow.h"

#include <cmath>

namespace tokens = tracklab::design;

TrackRow::TrackRow()
{
    addAndMakeVisible (numberLabel);
    addAndMakeVisible (nameLabel);
    addAndMakeVisible (volumeSlider);
    addAndMakeVisible (panSlider);
    addAndMakeVisible (muteButton);
    addAndMakeVisible (soloButton);
    addAndMakeVisible (recordButton);

    numberLabel.setFont (tokens::fontMonospace());
    numberLabel.setColour (juce::Label::textColourId, tokens::textTertiary);
    numberLabel.setJustificationType (juce::Justification::centredLeft);

    nameLabel.setFont (tokens::fontHeader());
    nameLabel.setColour (juce::Label::textColourId, tokens::textPrimary);
    nameLabel.setColour (juce::Label::backgroundColourId, tokens::surfaceElevated.withAlpha (0.0f));
    nameLabel.setEditable (false, true, false);
    nameLabel.onTextChange = [this]
    {
        if (onTrackNameChanged)
            onTrackNameChanged (trackInfo.id, nameLabel.getText());
    };

    volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setRange (0.0, 1.5, 0.001);
    volumeSlider.setColour (juce::Slider::trackColourId, tokens::accentPrimary);
    volumeSlider.setColour (juce::Slider::backgroundColourId, tokens::surfaceRaised);
    volumeSlider.onValueChange = [this]
    {
        if (onVolumeChanged)
            onVolumeChanged (trackInfo.id, static_cast<float> (volumeSlider.getValue()));
    };

    panSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    panSlider.setRange (-1.0, 1.0, 0.001);
    panSlider.setColour (juce::Slider::rotarySliderFillColourId, tokens::accentSecondary);
    panSlider.setColour (juce::Slider::rotarySliderOutlineColourId, tokens::surfaceRaised);
    panSlider.onValueChange = [this]
    {
        if (onPanChanged)
            onPanChanged (trackInfo.id, static_cast<float> (panSlider.getValue()));
    };

    auto configureButton = [] (juce::TextButton& button)
    {
        button.setWantsKeyboardFocus (false);
    };

    configureButton (muteButton);
    configureButton (soloButton);
    configureButton (recordButton);

    muteButton.onClick = [this]
    {
        const auto muted = ! trackInfo.muted;

        if (onMuteChanged)
            onMuteChanged (trackInfo.id, muted);
    };

    soloButton.onClick = [this]
    {
        const auto soloed = ! trackInfo.soloed;

        if (onSoloChanged)
            onSoloChanged (trackInfo.id, soloed);
    };

    recordButton.onClick = [this]
    {
        recordButton.setToggleState (! recordButton.getToggleState(), juce::dontSendNotification);
        repaint();
    };

    updateControls();
}

void TrackRow::paint (juce::Graphics& g)
{
    g.fillAll (tokens::backgroundBase);

    auto header = getLocalBounds().removeFromLeft (tokens::trackHeaderWidth);
    auto lane = getLocalBounds().withTrimmedLeft (tokens::trackHeaderWidth);

    g.setGradientFill (tokens::verticalSurfaceGradient (tokens::surfaceElevated, header.toFloat()));
    g.fillRect (header);

    g.setColour (trackInfo.colour);
    g.fillRect (header.removeFromLeft (tokens::trackColorBarWidth));

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawVerticalLine (tokens::trackHeaderWidth - 1, 0.0f, static_cast<float> (getHeight()));

    g.setColour (tokens::surfaceRaised.withAlpha (tokens::laneOverlayAlpha));
    g.fillRect (lane);

    const auto clip = g.getClipBounds();
    const auto firstSecond = juce::jmax (0, static_cast<int> (std::floor ((clip.getX() - tokens::trackHeaderWidth) / pixelsPerSecond)));
    const auto lastSecond = static_cast<int> (std::ceil ((clip.getRight() - tokens::trackHeaderWidth) / pixelsPerSecond));

    for (auto second = firstSecond; second <= lastSecond; ++second)
    {
        const auto x = tokens::trackHeaderWidth + juce::roundToInt (second * pixelsPerSecond);
        g.setColour (tokens::borderSubtle.withAlpha (tokens::gridLineAlpha));
        g.drawVerticalLine (x, 0.0f, static_cast<float> (getHeight()));
    }

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, static_cast<float> (getWidth()));
}

void TrackRow::resized()
{
    auto header = getLocalBounds().removeFromLeft (tokens::trackHeaderWidth)
                                  .withTrimmedLeft (tokens::trackColorBarWidth)
                                  .reduced (tokens::trackHeaderSmallGap, 0);

    auto top = header.removeFromTop (tokens::trackHeightDefault / 2);
    numberLabel.setBounds (top.removeFromLeft (tokens::trackNumberWidth));
    nameLabel.setBounds (top);

    auto controls = header.reduced (0, tokens::trackHeaderSmallGap);
    volumeSlider.setBounds (controls.removeFromLeft (tokens::trackHeaderVolumeWidth));
    controls.removeFromLeft (tokens::trackHeaderSmallGap);
    panSlider.setBounds (controls.removeFromLeft (tokens::trackHeaderPanSize));
    controls.removeFromLeft (tokens::trackHeaderSmallGap);
    muteButton.setBounds (controls.removeFromLeft (tokens::trackHeaderButtonSize));
    controls.removeFromLeft (tokens::trackHeaderSmallGap);
    soloButton.setBounds (controls.removeFromLeft (tokens::trackHeaderButtonSize));
    controls.removeFromLeft (tokens::trackHeaderSmallGap);
    recordButton.setBounds (controls.removeFromLeft (tokens::trackHeaderButtonSize));
}

void TrackRow::mouseDown (const juce::MouseEvent& event)
{
    if (event.position.x < tokens::trackHeaderWidth)
        return;

    if (onEmptyLaneClicked)
        onEmptyLaneClicked();

    if (onSeek)
        onSeek (xToSeconds (event.position.x));
}

void TrackRow::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (onMouseWheel)
        onMouseWheel (event, wheel);
}

bool TrackRow::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& path : files)
        if (isSupportedAudioFile (juce::File (path)))
            return true;

    return false;
}

void TrackRow::filesDropped (const juce::StringArray& files, int x, int)
{
    if (onFilesDropped)
        onFilesDropped (files, trackInfo.id, xToSeconds (static_cast<float> (x)));
}

void TrackRow::setTrackInfo (TrackInfo info, int newTrackNumber)
{
    trackInfo = std::move (info);
    trackNumber = newTrackNumber;
    updateControls();
    repaint();
}

void TrackRow::setTimelineLength (double seconds)
{
    timelineLengthSeconds = juce::jmax (tokens::timelineEmptyLengthSeconds, seconds);
    repaint();
}

void TrackRow::setPixelsPerSecond (double newPixelsPerSecond)
{
    pixelsPerSecond = newPixelsPerSecond;
    repaint();
}

bool TrackRow::isSupportedAudioFile (const juce::File& file)
{
    const auto extension = file.getFileExtension().toLowerCase();
    return extension == ".wav"
        || extension == ".aiff"
        || extension == ".aif"
        || extension == ".mp3"
        || extension == ".flac";
}

void TrackRow::updateControls()
{
    numberLabel.setText (juce::String::formatted ("%02d", trackNumber), juce::dontSendNotification);
    nameLabel.setText (trackInfo.name, juce::dontSendNotification);
    volumeSlider.setValue (trackInfo.volume, juce::dontSendNotification);
    panSlider.setValue (trackInfo.pan, juce::dontSendNotification);
    muteButton.setToggleState (trackInfo.muted, juce::dontSendNotification);
    soloButton.setToggleState (trackInfo.soloed, juce::dontSendNotification);

    muteButton.setColour (juce::TextButton::buttonColourId, trackInfo.muted ? tokens::accentPrimary : tokens::surfaceRaised);
    muteButton.setColour (juce::TextButton::textColourOffId, tokens::textPrimary);
    soloButton.setColour (juce::TextButton::buttonColourId, trackInfo.soloed ? tokens::accentSecondary : tokens::surfaceRaised);
    soloButton.setColour (juce::TextButton::textColourOffId, tokens::textPrimary);
    recordButton.setColour (juce::TextButton::buttonColourId, recordButton.getToggleState() ? tokens::error : tokens::surfaceRaised);
    recordButton.setColour (juce::TextButton::textColourOffId, tokens::textPrimary);
}

double TrackRow::xToSeconds (float x) const
{
    const auto laneX = juce::jmax (0.0f, x - tokens::trackHeaderWidth);
    return juce::jlimit (0.0, timelineLengthSeconds, laneX / pixelsPerSecond);
}
