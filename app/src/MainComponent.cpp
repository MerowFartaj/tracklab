#include "MainComponent.h"

MainComponent::MainComponent()
{
    setOpaque (true);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (0x1a, 0x1a, 0x1a));

    g.setColour (juce::Colours::white.withAlpha (0.92f));
    g.setFont (juce::Font { juce::FontOptions { 48.0f, juce::Font::plain } });
    g.drawText ("Tracklab", getLocalBounds(), juce::Justification::centred, false);
}
