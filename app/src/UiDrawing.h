#pragma once

#include "DesignTokens.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace tracklab::ui
{
enum class Icon
{
    none,
    audio,
    clip,
    drums,
    effects,
    eq,
    folder,
    keyboard,
    loop,
    marker,
    metronome,
    midi,
    mixer,
    pause,
    pitch,
    play,
    project,
    record,
    rewind,
    search,
    settings,
    sliders,
    speaker,
    sparkle,
    stop,
    waveform
};

void drawIcon (juce::Graphics& g,
               Icon icon,
               juce::Rectangle<float> bounds,
               juce::Colour colour,
               float strokeWidth = tracklab::design::iconStrokeWidth);

void drawGlassPanel (juce::Graphics& g,
                     juce::Rectangle<float> bounds,
                     juce::Colour base,
                     float radius,
                     bool active = false);

void drawSoftGlow (juce::Graphics& g,
                   juce::Rectangle<float> bounds,
                   juce::Colour colour,
                   float alpha = tracklab::design::glowAlpha);

void drawSubtleStripes (juce::Graphics& g,
                        juce::Rectangle<float> bounds,
                        juce::Colour colour,
                        int spacing = tracklab::design::decorativeStripeSpacing);

void drawTinySparkles (juce::Graphics& g,
                       juce::Rectangle<float> bounds,
                       juce::Colour colour);
}
