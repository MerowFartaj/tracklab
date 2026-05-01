#include "MixerPanel.h"

namespace tokens = tracklab::design;

MixerPanel::ChannelStrip::ChannelStrip()
{
    setOpaque (true);

    addAndMakeVisible (nameLabel);
    addAndMakeVisible (panSlider);
    addAndMakeVisible (muteButton);
    addAndMakeVisible (soloButton);
    addAndMakeVisible (volumeSlider);
    addAndMakeVisible (volumeLabel);

    nameLabel.setFont (tokens::fontMetadata());
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setColour (juce::Label::textColourId, tokens::textSecondary);

    panSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    panSlider.setRange (-1.0, 1.0, 0.001);
    panSlider.setColour (juce::Slider::rotarySliderFillColourId, tokens::accentSecondary);
    panSlider.setColour (juce::Slider::rotarySliderOutlineColourId, tokens::surfaceRaised);
    panSlider.onValueChange = [this]
    {
        if (! isMaster && onPanChanged)
            onPanChanged (trackInfo.id, static_cast<float> (panSlider.getValue()));
    };

    muteButton.setWantsKeyboardFocus (false);
    soloButton.setWantsKeyboardFocus (false);

    muteButton.onClick = [this]
    {
        if (! isMaster && onMuteChanged)
            onMuteChanged (trackInfo.id, ! trackInfo.muted);
    };

    soloButton.onClick = [this]
    {
        if (! isMaster && onSoloChanged)
            onSoloChanged (trackInfo.id, ! trackInfo.soloed);
    };

    volumeSlider.setSliderStyle (juce::Slider::LinearVertical);
    volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setRange (0.0, 1.5, 0.001);
    volumeSlider.setColour (juce::Slider::trackColourId, tokens::accentPrimary);
    volumeSlider.setColour (juce::Slider::backgroundColourId, tokens::surfaceRaised);
    volumeSlider.onValueChange = [this]
    {
        const auto gain = static_cast<float> (volumeSlider.getValue());
        volumeLabel.setText (gainToText (gain), juce::dontSendNotification);

        if (isMaster)
        {
            if (onMasterVolumeChanged)
                onMasterVolumeChanged (gain);
        }
        else if (onVolumeChanged)
        {
            onVolumeChanged (trackInfo.id, gain);
        }
    };

    volumeLabel.setFont (tokens::fontMonospace());
    volumeLabel.setJustificationType (juce::Justification::centred);
    volumeLabel.setColour (juce::Label::textColourId, tokens::textTertiary);
}

void MixerPanel::ChannelStrip::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);
    g.setGradientFill (tokens::verticalSurfaceGradient (tokens::surfaceElevated, bounds));
    g.fillRect (bounds);

    g.setColour ((isMaster ? tokens::accentPrimary : trackInfo.colour).withAlpha (isMaster ? tokens::panelBorderAlpha : 1.0f));
    g.fillRect (0, 0, getWidth(), tokens::mixerTrackColorBarHeight);

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawRect (getLocalBounds(), 1);

    auto meter = getMeterBounds().toFloat();
    g.setColour (tokens::surfaceRaised.withAlpha (tokens::meterInactiveAlpha));
    g.fillRoundedRectangle (meter, tokens::clipCornerRadius);

    if (meterLevel > 0.0f)
    {
        const auto filledHeight = meter.getHeight() * juce::jlimit (0.0f, 1.0f, meterLevel);
        auto filled = meter.removeFromBottom (filledHeight);
        juce::ColourGradient gradient { tokens::success,
                                        filled.getX(),
                                        filled.getBottom(),
                                        tokens::error,
                                        filled.getX(),
                                        filled.getY(),
                                        false };
        gradient.addColour (tokens::meterYellowPoint, tokens::warning);
        gradient.addColour (tokens::meterRedPoint, tokens::error);
        g.setGradientFill (gradient);
        g.fillRoundedRectangle (filled, tokens::clipCornerRadius);
    }
}

void MixerPanel::ChannelStrip::resized()
{
    auto bounds = getLocalBounds().reduced (tokens::mixerStripPadding);
    bounds.removeFromTop (tokens::mixerTrackColorBarHeight);

    nameLabel.setBounds (bounds.removeFromTop (tokens::mixerReadoutHeight));
    bounds.removeFromTop (tokens::trackHeaderSmallGap);

    panSlider.setBounds (bounds.removeFromTop (tokens::mixerPanSize).withSizeKeepingCentre (tokens::mixerPanSize,
                                                                                            tokens::mixerPanSize));
    bounds.removeFromTop (tokens::trackHeaderSmallGap);

    auto buttonRow = bounds.removeFromTop (tokens::mixerButtonSize);
    muteButton.setBounds (buttonRow.removeFromLeft (tokens::mixerButtonSize));
    buttonRow.removeFromLeft (tokens::trackHeaderSmallGap);
    soloButton.setBounds (buttonRow.removeFromLeft (tokens::mixerButtonSize));

    bounds.removeFromTop (tokens::trackHeaderSmallGap);
    auto lower = bounds.removeFromTop (tokens::mixerFaderHeight);
    volumeSlider.setBounds (lower.withWidth (tokens::mixerButtonSize).withX (lower.getCentreX() - tokens::mixerButtonSize / 2));
    volumeLabel.setBounds (bounds.removeFromBottom (tokens::mixerReadoutHeight));
}

void MixerPanel::ChannelStrip::setTrackInfo (TrackInfo info)
{
    isMaster = false;
    trackInfo = std::move (info);
    nameLabel.setText (trackInfo.name, juce::dontSendNotification);
    panSlider.setEnabled (true);
    soloButton.setVisible (true);
    volumeSlider.setValue (trackInfo.volume, juce::dontSendNotification);
    panSlider.setValue (trackInfo.pan, juce::dontSendNotification);
    muteButton.setToggleState (trackInfo.muted, juce::dontSendNotification);
    soloButton.setToggleState (trackInfo.soloed, juce::dontSendNotification);
    muteButton.setColour (juce::TextButton::buttonColourId, trackInfo.muted ? tokens::accentPrimary : tokens::surfaceRaised);
    soloButton.setColour (juce::TextButton::buttonColourId, trackInfo.soloed ? tokens::accentSecondary : tokens::surfaceRaised);
    muteButton.setColour (juce::TextButton::textColourOffId, tokens::textPrimary);
    soloButton.setColour (juce::TextButton::textColourOffId, tokens::textPrimary);
    volumeLabel.setText (gainToText (trackInfo.volume), juce::dontSendNotification);
    repaint();
}

void MixerPanel::ChannelStrip::setMaster (float volume)
{
    isMaster = true;
    trackInfo = {};
    trackInfo.name = "MASTER";
    trackInfo.volume = volume;
    nameLabel.setText (trackInfo.name, juce::dontSendNotification);
    panSlider.setEnabled (false);
    soloButton.setVisible (false);
    volumeSlider.setValue (volume, juce::dontSendNotification);
    volumeLabel.setText (gainToText (volume), juce::dontSendNotification);
    muteButton.setColour (juce::TextButton::buttonColourId, tokens::surfaceRaised);
    muteButton.setColour (juce::TextButton::textColourOffId, tokens::textPrimary);
    repaint();
}

void MixerPanel::ChannelStrip::setMeterLevel (float level)
{
    meterLevel = juce::jlimit (0.0f, 1.0f, level);
    repaint (getMeterBounds());
}

juce::String MixerPanel::ChannelStrip::gainToText (float gain) const
{
    if (gain <= 0.0001f)
        return "-inf";

    return juce::String (juce::Decibels::gainToDecibels (gain), 1) + " dB";
}

juce::Rectangle<int> MixerPanel::ChannelStrip::getMeterBounds() const
{
    return { getWidth() - tokens::mixerStripPadding - tokens::mixerMeterWidth,
             tokens::mixerStripPadding + tokens::mixerTrackColorBarHeight + tokens::mixerReadoutHeight,
             tokens::mixerMeterWidth,
             tokens::mixerFaderHeight };
}

MixerPanel::MixerPanel (AudioEngine& engine)
    : audioEngine (engine)
{
    setOpaque (true);
    addAndMakeVisible (viewport);
    stripContainer.setOpaque (true);
    viewport.setViewedComponent (&stripContainer, false);
    viewport.setScrollBarsShown (false, true);

    masterStrip = std::make_unique<ChannelStrip>();
    masterStrip->onMasterVolumeChanged = [this] (float gain)
    {
        audioEngine.setMasterVolume (gain);
    };
    addAndMakeVisible (*masterStrip);

    refreshFromEngine();
    startTimerHz (tokens::mixerTimerHz);
}

void MixerPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (tokens::verticalSurfaceGradient (tokens::surfaceElevated, bounds));
    g.fillAll();

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha));
    g.drawHorizontalLine (0, 0.0f, static_cast<float> (getWidth()));
}

void MixerPanel::resized()
{
    auto bounds = getLocalBounds();
    auto masterBounds = bounds.removeFromRight (tokens::mixerMasterStripWidth);
    bounds.removeFromRight (tokens::toolbarGap);
    viewport.setBounds (bounds);
    masterStrip->setBounds (masterBounds);

    const auto stripWidth = tokens::mixerStripWidth;
    stripContainer.setSize (juce::jmax (bounds.getWidth(), static_cast<int> (strips.size()) * stripWidth),
                            bounds.getHeight());

    for (auto i = 0; i < static_cast<int> (strips.size()); ++i)
        strips[static_cast<size_t> (i)]->setBounds (i * stripWidth, 0, stripWidth, bounds.getHeight());
}

void MixerPanel::refreshFromEngine()
{
    const auto tracks = audioEngine.getAllTrackInfo();
    ensureStripCount (static_cast<int> (tracks.size()));

    for (auto i = 0; i < static_cast<int> (tracks.size()); ++i)
        strips[static_cast<size_t> (i)]->setTrackInfo (tracks[static_cast<size_t> (i)]);

    masterStrip->setMaster (audioEngine.getMasterVolume());
    resized();
}

void MixerPanel::timerCallback()
{
    for (auto& strip : strips)
        strip->setMeterLevel (audioEngine.getTrackLevelPeak (strip->getTrackId()));

    masterStrip->setMeterLevel (audioEngine.getMasterLevelPeak());
}

void MixerPanel::ensureStripCount (int count)
{
    while (static_cast<int> (strips.size()) < count)
    {
        auto strip = std::make_unique<ChannelStrip>();

        strip->onVolumeChanged = [this] (int trackId, float gain)
        {
            audioEngine.setTrackVolume (trackId, gain);
        };

        strip->onPanChanged = [this] (int trackId, float pan)
        {
            audioEngine.setTrackPan (trackId, pan);
        };

        strip->onMuteChanged = [this] (int trackId, bool muted)
        {
            audioEngine.setTrackMute (trackId, muted);
            refreshFromEngine();
        };

        strip->onSoloChanged = [this] (int trackId, bool soloed)
        {
            audioEngine.setTrackSolo (trackId, soloed);
            refreshFromEngine();
        };

        stripContainer.addAndMakeVisible (*strip);
        strips.push_back (std::move (strip));
    }

    while (static_cast<int> (strips.size()) > count)
    {
        stripContainer.removeChildComponent (strips.back().get());
        strips.pop_back();
    }
}
