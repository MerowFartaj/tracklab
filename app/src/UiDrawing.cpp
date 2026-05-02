#include "UiDrawing.h"

#include <array>
#include <cmath>

namespace tokens = tracklab::design;

namespace
{
juce::Rectangle<float> opticalBounds (juce::Rectangle<float> bounds)
{
    return bounds.reduced (bounds.getWidth() * 0.14f, bounds.getHeight() * 0.14f);
}

void strokePath (juce::Graphics& g, const juce::Path& path, juce::Colour colour, float width)
{
    g.setColour (colour);
    g.strokePath (path, juce::PathStrokeType { width, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
}

void drawBars (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour, float width)
{
    const auto barWidth = bounds.getWidth() * 0.16f;
    const auto gap = bounds.getWidth() * 0.10f;
    const auto bottom = bounds.getBottom();

    for (auto i = 0; i < 4; ++i)
    {
        const auto height = bounds.getHeight() * (0.35f + 0.13f * static_cast<float> ((i + 2) % 4));
        const auto x = bounds.getX() + static_cast<float> (i) * (barWidth + gap);
        juce::Path bar;
        bar.addRoundedRectangle (juce::Rectangle<float> { x, bottom - height, barWidth, height }, width);
        g.setColour (colour.withAlpha (0.45f + 0.12f * static_cast<float> (i)));
        g.fillPath (bar);
    }
}
}

namespace tracklab::ui
{
void drawIcon (juce::Graphics& g, Icon icon, juce::Rectangle<float> bounds, juce::Colour colour, float strokeWidth)
{
    if (icon == Icon::none || bounds.isEmpty())
        return;

    auto r = opticalBounds (bounds);
    juce::Path path;

    switch (icon)
    {
        case Icon::play:
            path.startNewSubPath (r.getX(), r.getY());
            path.lineTo (r.getRight(), r.getCentreY());
            path.lineTo (r.getX(), r.getBottom());
            path.closeSubPath();
            g.setColour (colour);
            g.fillPath (path);
            return;

        case Icon::pause:
            g.setColour (colour);
            g.fillRoundedRectangle ({ r.getX(), r.getY(), r.getWidth() * 0.28f, r.getHeight() }, 1.2f);
            g.fillRoundedRectangle ({ r.getRight() - r.getWidth() * 0.28f, r.getY(), r.getWidth() * 0.28f, r.getHeight() }, 1.2f);
            return;

        case Icon::stop:
            g.setColour (colour);
            g.fillRoundedRectangle (r.reduced (r.getWidth() * 0.08f), 1.4f);
            return;

        case Icon::record:
            g.setColour (colour);
            g.fillEllipse (r.reduced (r.getWidth() * 0.06f));
            return;

        case Icon::rewind:
            path.startNewSubPath (r.getRight(), r.getY());
            path.lineTo (r.getCentreX(), r.getCentreY());
            path.lineTo (r.getRight(), r.getBottom());
            path.closeSubPath();
            g.setColour (colour);
            g.fillPath (path);
            path.clear();
            path.startNewSubPath (r.getCentreX(), r.getY());
            path.lineTo (r.getX(), r.getCentreY());
            path.lineTo (r.getCentreX(), r.getBottom());
            path.closeSubPath();
            g.fillPath (path);
            return;

        case Icon::folder:
            path.startNewSubPath (r.getX(), r.getY() + r.getHeight() * 0.25f);
            path.lineTo (r.getX() + r.getWidth() * 0.34f, r.getY() + r.getHeight() * 0.25f);
            path.lineTo (r.getX() + r.getWidth() * 0.44f, r.getY() + r.getHeight() * 0.38f);
            path.lineTo (r.getRight(), r.getY() + r.getHeight() * 0.38f);
            path.lineTo (r.getRight(), r.getBottom());
            path.lineTo (r.getX(), r.getBottom());
            path.closeSubPath();
            strokePath (g, path, colour, strokeWidth);
            return;

        case Icon::search:
            g.setColour (colour);
            g.drawEllipse (r.removeFromLeft (r.getWidth() * 0.72f).removeFromTop (r.getHeight() * 0.72f), strokeWidth);
            path.startNewSubPath (bounds.getCentreX() + bounds.getWidth() * 0.10f, bounds.getCentreY() + bounds.getHeight() * 0.10f);
            path.lineTo (bounds.getRight() - bounds.getWidth() * 0.18f, bounds.getBottom() - bounds.getHeight() * 0.18f);
            strokePath (g, path, colour, strokeWidth);
            return;

        case Icon::speaker:
            path.startNewSubPath (r.getX(), r.getY() + r.getHeight() * 0.38f);
            path.lineTo (r.getX() + r.getWidth() * 0.28f, r.getY() + r.getHeight() * 0.38f);
            path.lineTo (r.getX() + r.getWidth() * 0.52f, r.getY() + r.getHeight() * 0.18f);
            path.lineTo (r.getX() + r.getWidth() * 0.52f, r.getBottom() - r.getHeight() * 0.18f);
            path.lineTo (r.getX() + r.getWidth() * 0.28f, r.getBottom() - r.getHeight() * 0.38f);
            path.lineTo (r.getX(), r.getBottom() - r.getHeight() * 0.38f);
            path.closeSubPath();
            strokePath (g, path, colour, strokeWidth);
            path.clear();
            path.startNewSubPath (r.getX() + r.getWidth() * 0.66f, r.getY() + r.getHeight() * 0.34f);
            path.quadraticTo (r.getRight(), r.getCentreY(), r.getX() + r.getWidth() * 0.66f, r.getBottom() - r.getHeight() * 0.34f);
            strokePath (g, path, colour, strokeWidth);
            return;

        case Icon::loop:
            path.startNewSubPath (r.getX(), r.getCentreY());
            path.cubicTo (r.getX(), r.getY(), r.getCentreX(), r.getY(), r.getCentreX(), r.getCentreY());
            path.cubicTo (r.getCentreX(), r.getBottom(), r.getRight(), r.getBottom(), r.getRight(), r.getCentreY());
            strokePath (g, path, colour, strokeWidth);
            path.clear();
            path.startNewSubPath (r.getRight() - r.getWidth() * 0.20f, r.getCentreY() - r.getHeight() * 0.18f);
            path.lineTo (r.getRight(), r.getCentreY());
            path.lineTo (r.getRight() - r.getWidth() * 0.20f, r.getCentreY() + r.getHeight() * 0.18f);
            strokePath (g, path, colour, strokeWidth);
            return;

        case Icon::metronome:
            path.startNewSubPath (r.getCentreX(), r.getY());
            path.lineTo (r.getRight(), r.getBottom());
            path.lineTo (r.getX(), r.getBottom());
            path.closeSubPath();
            strokePath (g, path, colour, strokeWidth);
            path.clear();
            path.startNewSubPath (r.getCentreX(), r.getBottom() - r.getHeight() * 0.12f);
            path.lineTo (r.getRight() - r.getWidth() * 0.20f, r.getY() + r.getHeight() * 0.25f);
            strokePath (g, path, colour, strokeWidth);
            g.setColour (colour);
            g.fillEllipse ({ r.getCentreX() - 1.5f, r.getY() + r.getHeight() * 0.42f, 3.0f, 3.0f });
            return;

        case Icon::waveform:
        case Icon::audio:
            path.startNewSubPath (r.getX(), r.getCentreY());
            for (auto i = 0; i <= 8; ++i)
            {
                const auto x = r.getX() + r.getWidth() * static_cast<float> (i) / 8.0f;
                const auto amp = std::sin (static_cast<float> (i) * juce::MathConstants<float>::pi * 0.72f);
                const auto y = r.getCentreY() - amp * r.getHeight() * 0.34f;
                path.lineTo (x, y);
            }
            strokePath (g, path, colour, strokeWidth);
            return;

        case Icon::drums:
            g.setColour (colour);
            g.drawEllipse (r.withTrimmedTop (r.getHeight() * 0.30f), strokeWidth);
            path.startNewSubPath (r.getX() + r.getWidth() * 0.15f, r.getY());
            path.lineTo (r.getCentreX(), r.getY() + r.getHeight() * 0.34f);
            path.startNewSubPath (r.getRight() - r.getWidth() * 0.15f, r.getY());
            path.lineTo (r.getCentreX(), r.getY() + r.getHeight() * 0.34f);
            strokePath (g, path, colour, strokeWidth);
            return;

        case Icon::keyboard:
        case Icon::midi:
            g.setColour (colour);
            g.drawRoundedRectangle (r, 2.0f, strokeWidth);
            for (auto i = 1; i < 5; ++i)
                g.drawVerticalLine (juce::roundToInt (r.getX() + r.getWidth() * static_cast<float> (i) / 5.0f),
                                    r.getY() + r.getHeight() * 0.45f,
                                    r.getBottom());
            g.fillRect (juce::Rectangle<float> { r.getX() + r.getWidth() * 0.16f, r.getY(), r.getWidth() * 0.10f, r.getHeight() * 0.48f });
            g.fillRect (juce::Rectangle<float> { r.getX() + r.getWidth() * 0.46f, r.getY(), r.getWidth() * 0.10f, r.getHeight() * 0.48f });
            g.fillRect (juce::Rectangle<float> { r.getX() + r.getWidth() * 0.66f, r.getY(), r.getWidth() * 0.10f, r.getHeight() * 0.48f });
            return;

        case Icon::effects:
        case Icon::sparkle:
            drawTinySparkles (g, bounds, colour);
            return;

        case Icon::sliders:
        case Icon::eq:
        case Icon::mixer:
            drawBars (g, r, colour, strokeWidth);
            return;

        case Icon::pitch:
            path.startNewSubPath (r.getX(), r.getBottom());
            path.lineTo (r.getX(), r.getY());
            path.lineTo (r.getRight() - r.getWidth() * 0.18f, r.getY());
            path.quadraticTo (r.getRight(), r.getY() + r.getHeight() * 0.28f,
                              r.getRight() - r.getWidth() * 0.18f, r.getY() + r.getHeight() * 0.52f);
            path.lineTo (r.getX(), r.getY() + r.getHeight() * 0.52f);
            strokePath (g, path, colour, strokeWidth);
            return;

        case Icon::clip:
            g.setColour (colour.withAlpha (0.28f));
            g.fillRoundedRectangle (r, tokens::clipCornerRadius);
            g.setColour (colour);
            g.drawRoundedRectangle (r, tokens::clipCornerRadius, strokeWidth);
            drawIcon (g, Icon::waveform, r.reduced (r.getWidth() * 0.12f), colour, strokeWidth);
            return;

        case Icon::marker:
            path.startNewSubPath (r.getCentreX(), r.getY());
            path.lineTo (r.getRight(), r.getY() + r.getHeight() * 0.38f);
            path.lineTo (r.getCentreX(), r.getBottom());
            path.lineTo (r.getX(), r.getY() + r.getHeight() * 0.38f);
            path.closeSubPath();
            strokePath (g, path, colour, strokeWidth);
            return;

        case Icon::project:
            g.setColour (colour.withAlpha (0.22f));
            g.fillRoundedRectangle (r, 2.0f);
            g.setColour (colour);
            g.drawRoundedRectangle (r, 2.0f, strokeWidth);
            g.drawHorizontalLine (juce::roundToInt (r.getY() + r.getHeight() * 0.35f), r.getX(), r.getRight());
            return;

        case Icon::settings:
            g.setColour (colour);
            g.drawEllipse (r.reduced (r.getWidth() * 0.22f), strokeWidth);
            for (auto i = 0; i < 8; ++i)
            {
                const auto angle = juce::MathConstants<float>::twoPi * static_cast<float> (i) / 8.0f;
                const auto inner = juce::Point<float> { r.getCentreX() + std::cos (angle) * r.getWidth() * 0.34f,
                                                        r.getCentreY() + std::sin (angle) * r.getHeight() * 0.34f };
                const auto outer = juce::Point<float> { r.getCentreX() + std::cos (angle) * r.getWidth() * 0.48f,
                                                        r.getCentreY() + std::sin (angle) * r.getHeight() * 0.48f };
                g.drawLine ({ inner, outer }, strokeWidth);
            }
            g.fillEllipse (r.withSizeKeepingCentre (r.getWidth() * 0.18f, r.getHeight() * 0.18f));
            return;

        default:
            return;
    }
}

void drawGlassPanel (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour base, float radius, bool active)
{
    const auto top = active ? base.brighter (tokens::surfaceGradientAmount * 2.0f) : base.brighter (tokens::surfaceGradientAmount);
    const auto bottom = active ? base.darker (tokens::surfaceGradientAmount) : base.darker (tokens::surfaceGradientAmount * 2.0f);
    juce::ColourGradient gradient { top, bounds.getX(), bounds.getY(), bottom, bounds.getX(), bounds.getBottom(), false };

    g.setGradientFill (gradient);
    g.fillRoundedRectangle (bounds, radius);

    g.setColour (tokens::highlightBase.withAlpha (active ? tokens::clipTopHighlightAlpha : tokens::panelTopHighlightAlpha));
    g.drawHorizontalLine (juce::roundToInt (bounds.getY()), bounds.getX() + radius, bounds.getRight() - radius);

    g.setColour ((active ? tokens::accentSecondary : tokens::borderSubtle).withAlpha (active ? 0.72f : tokens::panelBorderAlpha));
    g.drawRoundedRectangle (bounds, radius, 1.0f);
}

void drawSoftGlow (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour, float alpha)
{
    auto glow = bounds.expanded (tokens::glowRadius);
    juce::ColourGradient gradient { colour.withAlpha (alpha),
                                    glow.getCentreX(),
                                    glow.getCentreY(),
                                    colour.withAlpha (0.0f),
                                    glow.getRight(),
                                    glow.getBottom(),
                                    true };
    g.setGradientFill (gradient);
    g.fillEllipse (glow);
}

void drawSubtleStripes (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour, int spacing)
{
    g.setColour (colour.withAlpha (tokens::laneStripeAlpha));

    for (auto x = juce::roundToInt (bounds.getX()); x < bounds.getRight(); x += spacing)
        g.fillRect (juce::Rectangle<int> { x,
                                           juce::roundToInt (bounds.getY()),
                                           tokens::decorativeStripeWidth,
                                           juce::roundToInt (bounds.getHeight()) });
}

void drawTinySparkles (juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour)
{
    const std::array<juce::Point<float>, 3> centres
    {
        juce::Point<float> { bounds.getX() + bounds.getWidth() * 0.32f, bounds.getY() + bounds.getHeight() * 0.30f },
        juce::Point<float> { bounds.getX() + bounds.getWidth() * 0.66f, bounds.getY() + bounds.getHeight() * 0.48f },
        juce::Point<float> { bounds.getX() + bounds.getWidth() * 0.42f, bounds.getY() + bounds.getHeight() * 0.72f }
    };

    const std::array<float, 3> sizes { 3.0f, 2.2f, 1.8f };

    for (auto i = 0; i < static_cast<int> (centres.size()); ++i)
    {
        const auto c = centres[static_cast<size_t> (i)];
        const auto s = sizes[static_cast<size_t> (i)];
        juce::Path star;
        star.startNewSubPath (c.x, c.y - s);
        star.lineTo (c.x + s, c.y);
        star.lineTo (c.x, c.y + s);
        star.lineTo (c.x - s, c.y);
        star.closeSubPath();
        g.setColour (colour.withAlpha (0.72f - 0.12f * static_cast<float> (i)));
        g.fillPath (star);
    }
}
}
