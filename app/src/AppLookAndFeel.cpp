#include "AppLookAndFeel.h"

#include "UiDrawing.h"

#include <cmath>

namespace tokens = tracklab::design;
namespace ui = tracklab::ui;

TracklabLookAndFeel::TracklabLookAndFeel()
{
    setColour (juce::PopupMenu::backgroundColourId, tokens::surfaceElevated);
    setColour (juce::PopupMenu::textColourId, tokens::textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, tokens::accentSecondary.withAlpha (tokens::browserSelectedAlpha));
    setColour (juce::PopupMenu::highlightedTextColourId, tokens::textPrimary);
}

juce::Font TracklabLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return tokens::fontBody().withStyle (juce::Font::bold);
}

juce::Font TracklabLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return tokens::fontBody();
}

juce::Font TracklabLookAndFeel::getLabelFont (juce::Label&)
{
    return tokens::fontBody();
}

void TracklabLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                                juce::Button& button,
                                                const juce::Colour& backgroundColour,
                                                bool shouldDrawButtonAsHighlighted,
                                                bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto base = backgroundColour.isOpaque() ? backgroundColour : tokens::surfaceRaised;

    if (! button.isEnabled())
        base = tokens::surfaceRaised.withMultipliedAlpha (tokens::disabledAlpha);
    else if (button.getToggleState())
        base = button.findColour (juce::TextButton::buttonOnColourId);
    else if (shouldDrawButtonAsDown)
        base = base.darker (tokens::surfaceGradientAmount * 2.0f);
    else if (shouldDrawButtonAsHighlighted)
        base = base.brighter (tokens::surfaceGradientAmount * 2.0f);

    ui::drawGlassPanel (g, bounds, base, tokens::buttonCornerRadius, button.getToggleState());
}

void TracklabLookAndFeel::drawButtonText (juce::Graphics& g,
                                          juce::TextButton& button,
                                          bool,
                                          bool)
{
    const auto alpha = button.isEnabled() ? 1.0f : tokens::disabledAlpha;
    g.setColour (button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId
                                                            : juce::TextButton::textColourOffId).withAlpha (alpha));
    g.setFont (getTextButtonFont (button, button.getHeight()));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds().reduced (tokens::trackHeaderSmallGap, 0),
                      juce::Justification::centred, 1);
}

void TracklabLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            float sliderPos,
                                            float,
                                            float,
                                            const juce::Slider::SliderStyle style,
                                            juce::Slider& slider)
{
    const auto enabledAlpha = slider.isEnabled() ? 1.0f : tokens::disabledAlpha;
    const auto trackColour = slider.findColour (juce::Slider::trackColourId).withAlpha (enabledAlpha);
    const auto backgroundColour = slider.findColour (juce::Slider::backgroundColourId).withAlpha (enabledAlpha);

    if (style == juce::Slider::LinearVertical)
    {
        const auto rail = juce::Rectangle<float> { static_cast<float> (x + width / 2) - tokens::faderRailWidth * 0.5f,
                                                   static_cast<float> (y),
                                                   tokens::faderRailWidth,
                                                   static_cast<float> (height) };
        g.setColour (backgroundColour);
        g.fillRoundedRectangle (rail, tokens::faderRailWidth * 0.5f);

        auto filled = rail.withTop (sliderPos);
        juce::ColourGradient gradient { trackColour,
                                        filled.getCentreX(),
                                        filled.getY(),
                                        tokens::accentCyan.withAlpha (enabledAlpha),
                                        filled.getCentreX(),
                                        filled.getBottom(),
                                        false };
        g.setGradientFill (gradient);
        g.fillRoundedRectangle (filled, tokens::faderRailWidth * 0.5f);

        const auto thumb = juce::Rectangle<float> { static_cast<float> (x + 2),
                                                    sliderPos - 4.0f,
                                                    static_cast<float> (width - 4),
                                                    8.0f };
        ui::drawGlassPanel (g, thumb, tokens::surfaceChrome, tokens::buttonCornerRadius, true);
        return;
    }

    const auto rail = juce::Rectangle<float> { static_cast<float> (x),
                                               static_cast<float> (y + height / 2) - tokens::faderRailWidth * 0.5f,
                                               static_cast<float> (width),
                                               tokens::faderRailWidth };
    g.setColour (backgroundColour);
    g.fillRoundedRectangle (rail, tokens::faderRailWidth * 0.5f);

    auto filled = rail.withRight (sliderPos);
    juce::ColourGradient gradient { tokens::accentPrimary.withAlpha (enabledAlpha),
                                    filled.getX(),
                                    filled.getCentreY(),
                                    trackColour,
                                    filled.getRight(),
                                    filled.getCentreY(),
                                    false };
    g.setGradientFill (gradient);
    g.fillRoundedRectangle (filled, tokens::faderRailWidth * 0.5f);

    const auto thumbSize = static_cast<float> (juce::jmin (height - 2, tokens::iconSizeMedium));
    auto thumb = juce::Rectangle<float> { sliderPos - thumbSize * 0.5f,
                                          static_cast<float> (y + height / 2) - thumbSize * 0.5f,
                                          thumbSize,
                                          thumbSize };
    g.setColour (tokens::shadowBase.withAlpha (tokens::clipShadowAlpha));
    g.fillEllipse (thumb.translated (0.0f, 1.0f));
    g.setGradientFill (tokens::verticalSurfaceGradient (trackColour, thumb));
    g.fillEllipse (thumb);
    g.setColour (tokens::highlightBase.withAlpha (tokens::clipTopHighlightAlpha));
    g.drawEllipse (thumb.reduced (1.0f), 1.0f);
}

void TracklabLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            float sliderPosProportional,
                                            float rotaryStartAngle,
                                            float rotaryEndAngle,
                                            juce::Slider& slider)
{
    const auto enabledAlpha = slider.isEnabled() ? 1.0f : tokens::disabledAlpha;
    auto bounds = juce::Rectangle<float> { static_cast<float> (x),
                                           static_cast<float> (y),
                                           static_cast<float> (width),
                                           static_cast<float> (height) }.reduced (1.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (tokens::shadowBase.withAlpha (tokens::clipShadowAlpha));
    g.fillEllipse (bounds.translated (0.0f, 1.0f));
    g.setGradientFill (tokens::verticalSurfaceGradient (tokens::surfaceChrome.withAlpha (enabledAlpha), bounds));
    g.fillEllipse (bounds);

    juce::Path arc;
    arc.addCentredArc (centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId).withAlpha (enabledAlpha));
    g.strokePath (arc, juce::PathStrokeType { 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

    juce::Path pointer;
    pointer.startNewSubPath (centre);
    pointer.lineTo (centre.x + std::cos (angle - juce::MathConstants<float>::halfPi) * (radius - 5.0f),
                    centre.y + std::sin (angle - juce::MathConstants<float>::halfPi) * (radius - 5.0f));
    g.setColour (tokens::textPrimary.withAlpha (enabledAlpha));
    g.strokePath (pointer, juce::PathStrokeType { 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

    g.setColour (tokens::borderSubtle.withAlpha (tokens::panelBorderAlpha * enabledAlpha));
    g.drawEllipse (bounds, 1.0f);
}

void TracklabLookAndFeel::drawComboBox (juce::Graphics& g,
                                        int width,
                                        int height,
                                        bool isButtonDown,
                                        int,
                                        int,
                                        int,
                                        int,
                                        juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> { 0.5f, 0.5f, static_cast<float> (width - 1), static_cast<float> (height - 1) };
    auto base = box.findColour (juce::ComboBox::backgroundColourId);

    if (isButtonDown)
        base = base.darker (tokens::surfaceGradientAmount * 2.0f);

    ui::drawGlassPanel (g, bounds, base, tokens::buttonCornerRadius, false);

    juce::Path arrow;
    const auto cx = bounds.getRight() - tokens::browserSearchIconInset - 4.0f;
    const auto cy = bounds.getCentreY();
    arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
    arrow.lineTo (cx, cy + 2.5f);
    arrow.lineTo (cx + 4.0f, cy - 2.0f);
    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.strokePath (arrow, juce::PathStrokeType { tokens::iconStrokeWidth,
                                                juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded });
}

void TracklabLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (box.getLocalBounds().reduced (tokens::trackHeaderSmallGap, 0).withTrimmedRight (tokens::iconSizeMedium));
    label.setFont (tokens::fontBody());
    label.setJustificationType (juce::Justification::centredLeft);
}

void TracklabLookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor&)
{
    auto bounds = juce::Rectangle<float> { 0.5f, 0.5f, static_cast<float> (width - 1), static_cast<float> (height - 1) };
    ui::drawGlassPanel (g, bounds, tokens::surfaceInset, tokens::buttonCornerRadius, false);
}

void TracklabLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& editor)
{
    auto bounds = juce::Rectangle<float> { 0.5f, 0.5f, static_cast<float> (width - 1), static_cast<float> (height - 1) };
    g.setColour ((editor.hasKeyboardFocus (true) ? tokens::accentSecondary : tokens::borderSubtle)
                    .withAlpha (editor.hasKeyboardFocus (true) ? 0.72f : tokens::panelBorderAlpha));
    g.drawRoundedRectangle (bounds, tokens::buttonCornerRadius, 1.0f);
}
