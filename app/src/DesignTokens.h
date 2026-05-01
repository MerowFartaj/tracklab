#pragma once

#include <juce_graphics/juce_graphics.h>

namespace tracklab::design
{
inline const juce::Colour backgroundBase  { 0xFF1C1C1E };
inline const juce::Colour surfaceElevated { 0xFF2A2A2C };
inline const juce::Colour surfaceRaised   { 0xFF3A3A3C };
inline const juce::Colour borderSubtle    { 0xFF3F3F42 };
inline const juce::Colour textPrimary     { 0xFFF2F2F7 };
inline const juce::Colour textSecondary   { 0xFF98989D };
inline const juce::Colour textTertiary    { 0xFF6C6C70 };

inline const juce::Colour accentPrimary   { 0xFFFF9F0A };
inline const juce::Colour accentSecondary { 0xFF5E9EFF };
inline const juce::Colour success         { 0xFF30D158 };
inline const juce::Colour warning         { 0xFFFF9F0A };
inline const juce::Colour error           { 0xFFFF453A };
inline const juce::Colour shadowBase      { 0xFF000000 };
inline const juce::Colour highlightBase   { 0xFFFFFFFF };

inline const juce::Colour trackColors[] =
{
    juce::Colour { 0xFFFF6482 },
    juce::Colour { 0xFFFF9F0A },
    juce::Colour { 0xFFFFD60A },
    juce::Colour { 0xFF30D158 },
    juce::Colour { 0xFF64D2FF },
    juce::Colour { 0xFF5E9EFF },
    juce::Colour { 0xFFBF5AF2 },
    juce::Colour { 0xFFFF375F }
};

static constexpr int toolbarHeight = 44;
static constexpr int trackHeightDefault = 72;
static constexpr int trackHeightCompact = 48;
static constexpr int rulerHeight = 28;
static constexpr int trackHeaderWidth = 180;

static constexpr float playheadWidth = 1.0f;
static constexpr float clipCornerRadius = 3.0f;
static constexpr float buttonCornerRadius = 4.0f;

static constexpr float fontHeaderSize = 13.0f;
static constexpr float fontBodySize = 11.0f;
static constexpr float fontMetadataSize = 10.0f;
static constexpr float fontMonospaceSize = 10.0f;

static constexpr float surfaceGradientAmount = 0.03f;
static constexpr float clipGradientBottomBrightness = 0.80f;
static constexpr float clipSurfaceSaturation = 0.62f;
static constexpr float clipSurfaceBrightness = 0.54f;
static constexpr float panelBorderAlpha = 0.50f;
static constexpr float panelTopHighlightAlpha = 0.05f;
static constexpr float clipTopHighlightAlpha = 0.08f;
static constexpr float clipShadowAlpha = 0.30f;
static constexpr float selectedGlowAlpha = 0.20f;
static constexpr float laneOverlayAlpha = 0.10f;
static constexpr float gridLineAlpha = 0.30f;
static constexpr float beatLineAlpha = 0.20f;
static constexpr float subdivisionLineAlpha = 0.16f;
static constexpr float waveformPeakAlpha = 0.90f;
static constexpr float waveformRmsAlpha = 0.60f;

static constexpr int toolbarPaddingX = 12;
static constexpr int toolbarButtonHeight = 28;
static constexpr int toolbarButtonWidth = 112;
static constexpr int toolbarTransportButtonWidth = 72;
static constexpr int toolbarGap = 8;
static constexpr int toolbarPositionWidth = 180;

static constexpr int trackHeaderPadding = 12;
static constexpr int trackColorBarWidth = 4;
static constexpr int clipVerticalPadding = 8;
static constexpr int waveformPadding = 4;
static constexpr int minimumReadableClipWidth = 80;

static constexpr float timelineDefaultPixelsPerSecond = 80.0f;
static constexpr float timelineMinPixelsPerSecond = 5.0f;
static constexpr float timelineMaxPixelsPerSecond = 500.0f;
static constexpr double timelineEmptyLengthSeconds = 30.0;
static constexpr double timelineTrailingSeconds = 4.0;
static constexpr double minorTickSeconds = 0.25;
static constexpr double majorTickSeconds = 1.0;
static constexpr double defaultTempoBpm = 120.0;
static constexpr int beatsPerBar = 4;

static constexpr int playheadMarkerWidth = 6;
static constexpr int playheadMarkerHeight = 8;
static constexpr int playheadRepaintPadding = 4;

static constexpr int waveformSamplesPerPixel = 2;
static constexpr int waveformReadBlockSize = 32768;
static constexpr int waveformMaxChannels = 2;

static constexpr float wheelZoomBase = 1.18f;
static constexpr float wheelZoomScale = 4.0f;
static constexpr float wheelPanScale = 480.0f;

static constexpr int millisecondsPerSecond = 1000;
static constexpr int secondsPerMinute = 60;
static constexpr int millisecondsPerMinute = secondsPerMinute * millisecondsPerSecond;

inline juce::Font fontHeader()
{
    return juce::Font { juce::FontOptions { fontHeaderSize, juce::Font::bold } };
}

inline juce::Font fontBody()
{
    return juce::Font { juce::FontOptions { fontBodySize, juce::Font::plain } };
}

inline juce::Font fontMetadata()
{
    return juce::Font { juce::FontOptions { fontMetadataSize, juce::Font::plain } };
}

inline juce::Font fontMonospace()
{
    return juce::Font { juce::FontOptions { juce::Font::getDefaultMonospacedFontName(),
                                            fontMonospaceSize,
                                            juce::Font::plain } };
}

inline juce::ColourGradient verticalSurfaceGradient (juce::Colour base,
                                                     juce::Rectangle<float> bounds)
{
    return { base.brighter (surfaceGradientAmount),
             bounds.getX(),
             bounds.getY(),
             base.darker (surfaceGradientAmount),
             bounds.getX(),
             bounds.getBottom(),
             false };
}
}
