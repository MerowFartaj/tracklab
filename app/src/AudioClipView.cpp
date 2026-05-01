#include "AudioClipView.h"

#include <cmath>

namespace tokens = tracklab::design;

AudioClipView::AudioClipView()
{
    formatManager.registerBasicFormats();
    setWantsKeyboardFocus (false);
}

void AudioClipView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (tokens::shadowBase.withAlpha (tokens::clipShadowAlpha));
    g.fillRoundedRectangle (bounds.translated (0.0f, 1.0f), tokens::clipCornerRadius);

    const auto clipBase = trackColour.withMultipliedSaturation (tokens::clipSurfaceSaturation)
                                     .withMultipliedBrightness (tokens::clipSurfaceBrightness);

    juce::ColourGradient clipGradient { clipBase.brighter (tokens::surfaceGradientAmount),
                                        bounds.getX(),
                                        bounds.getY(),
                                        clipBase.withMultipliedBrightness (tokens::clipGradientBottomBrightness),
                                        bounds.getX(),
                                        bounds.getBottom(),
                                        false };
    g.setGradientFill (clipGradient);
    g.fillRoundedRectangle (bounds, tokens::clipCornerRadius);

    g.setColour (tokens::highlightBase.withAlpha (tokens::clipTopHighlightAlpha));
    g.drawHorizontalLine (1, bounds.getX() + tokens::clipCornerRadius, bounds.getRight() - tokens::clipCornerRadius);

    if (getWidth() >= tokens::minimumReadableClipWidth && ! peaks.empty() && ! rms.empty())
    {
        auto waveformBounds = bounds.reduced (tokens::waveformPadding, tokens::waveformPadding);
        drawWaveformLayer (g, waveformBounds, peaks, trackColour.withAlpha (tokens::waveformPeakAlpha));
        drawWaveformLayer (g, waveformBounds, rms, trackColour.withAlpha (tokens::waveformRmsAlpha));
    }

    g.setColour (selected ? tokens::accentSecondary.withAlpha (tokens::selectedClipBorderAlpha)
                          : trackColour.darker (tokens::surfaceGradientAmount).withAlpha (tokens::panelBorderAlpha));
    g.drawRoundedRectangle (bounds, tokens::clipCornerRadius, selected ? 1.5f : 1.0f);

    if (selected)
    {
        g.setColour (tokens::accentSecondary.withAlpha (tokens::selectedGlowAlpha));
        g.drawRoundedRectangle (bounds.expanded (2.0f), tokens::clipCornerRadius + 2.0f, 1.0f);
    }

    auto labelBounds = getLocalBounds().reduced (tokens::toolbarGap, 0);
    g.setColour (tokens::textPrimary);
    g.setFont (tokens::fontMetadata());
    g.drawText (clipInfo.name, labelBounds.removeFromTop (18), juce::Justification::centredLeft, true);
}

void AudioClipView::mouseDown (const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu())
    {
        if (onSelect)
            onSelect (clipInfo.id, event.mods.isShiftDown());

        showContextMenu();
        return;
    }

    if (onSelect)
        onSelect (clipInfo.id, event.mods.isShiftDown());

    if (event.position.x <= tokens::clipTrimHandleWidth)
        dragMode = DragMode::trimStart;
    else if (event.position.x >= getWidth() - tokens::clipTrimHandleWidth)
        dragMode = DragMode::trimEnd;
    else
        dragMode = DragMode::move;
}

void AudioClipView::mouseDrag (const juce::MouseEvent& event)
{
    const auto distance = event.getDistanceFromDragStart();

    if (distance < tokens::clipDragSnapPixels)
        return;

    if (dragMode == DragMode::move)
        setTransform (juce::AffineTransform::translation (event.getDistanceFromDragStartX(),
                                                          event.getDistanceFromDragStartY()));
}

void AudioClipView::mouseUp (const juce::MouseEvent& event)
{
    if (dragMode == DragMode::move && onMoveFinished)
        onMoveFinished (clipInfo.id,
                        static_cast<float> (event.getDistanceFromDragStartX()),
                        static_cast<float> (event.getDistanceFromDragStartY()),
                        event.mods.isShiftDown());
    else if ((dragMode == DragMode::trimStart || dragMode == DragMode::trimEnd) && onTrimFinished)
        onTrimFinished (clipInfo.id,
                        dragMode == DragMode::trimStart,
                        static_cast<float> (event.getDistanceFromDragStartX()),
                        event.mods.isShiftDown());

    setTransform ({});
    dragMode = DragMode::none;
}

void AudioClipView::setClipInfo (ClipInfo info, juce::Colour colour, bool shouldBeSelected)
{
    const auto shouldRebuild = clipInfo.file != info.file
                            || ! juce::approximatelyEqual (clipInfo.lengthSeconds, info.lengthSeconds)
                            || ! juce::approximatelyEqual (clipInfo.sourceOffsetSeconds, info.sourceOffsetSeconds);

    clipInfo = std::move (info);
    trackColour = colour;
    selected = shouldBeSelected;

    if (shouldRebuild)
        rebuildWaveformCache();

    repaint();
}

void AudioClipView::setPixelsPerSecond (double newPixelsPerSecond)
{
    if (juce::approximatelyEqual (pixelsPerSecond, newPixelsPerSecond))
        return;

    pixelsPerSecond = newPixelsPerSecond;
    rebuildWaveformCache();
    repaint();
}

void AudioClipView::clear()
{
    clipInfo = {};
    cachedPixelWidth = 0;
    peaks.clear();
    rms.clear();
    repaint();
}

void AudioClipView::rebuildWaveformCache()
{
    cachedPixelWidth = juce::roundToInt (clipInfo.lengthSeconds * pixelsPerSecond);
    peaks.clear();
    rms.clear();

    if (! clipInfo.file.existsAsFile() || clipInfo.lengthSeconds <= 0.0 || cachedPixelWidth <= 0)
        return;

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (clipInfo.file));

    if (reader == nullptr || reader->lengthInSamples <= 0 || reader->numChannels == 0)
        return;

    const auto cacheSize = juce::jmax (1, cachedPixelWidth * tokens::waveformSamplesPerPixel);
    peaks.assign (static_cast<size_t> (cacheSize), 0.0f);
    rms.assign (static_cast<size_t> (cacheSize), 0.0f);

    std::vector<double> squareSums (static_cast<size_t> (cacheSize), 0.0);
    std::vector<int> sampleCounts (static_cast<size_t> (cacheSize), 0);

    const auto channelsToRead = juce::jlimit (1, tokens::waveformMaxChannels, static_cast<int> (reader->numChannels));
    juce::AudioBuffer<float> buffer (channelsToRead, tokens::waveformReadBlockSize);
    const auto startSample = static_cast<juce::int64> (clipInfo.sourceOffsetSeconds * reader->sampleRate);
    const auto samplesAvailable = juce::jmax<juce::int64> (0, reader->lengthInSamples - startSample);
    const auto samplesToScan = juce::jmin<juce::int64> (samplesAvailable,
                                                        static_cast<juce::int64> (clipInfo.lengthSeconds * reader->sampleRate));
    const auto samplesPerCacheEntry = static_cast<double> (juce::jmax<juce::int64> (1, samplesToScan)) / cacheSize;

    for (juce::int64 samplePosition = 0; samplePosition < samplesToScan;)
    {
        const auto samplesToRead = static_cast<int> (juce::jmin<juce::int64> (tokens::waveformReadBlockSize,
                                                                              samplesToScan - samplePosition));
        buffer.clear();
        reader->read (&buffer, 0, samplesToRead, startSample + samplePosition, true, channelsToRead > 1);

        for (auto sample = 0; sample < samplesToRead; ++sample)
        {
            const auto cacheIndex = juce::jlimit (0,
                                                  cacheSize - 1,
                                                  static_cast<int> ((samplePosition + sample) / samplesPerCacheEntry));

            auto peak = 0.0f;
            auto squareSum = 0.0;

            for (auto channel = 0; channel < channelsToRead; ++channel)
            {
                const auto value = buffer.getSample (channel, sample);
                peak = juce::jmax (peak, std::abs (value));
                squareSum += static_cast<double> (value * value);
            }

            peaks[static_cast<size_t> (cacheIndex)] = juce::jmax (peaks[static_cast<size_t> (cacheIndex)], peak);
            squareSums[static_cast<size_t> (cacheIndex)] += squareSum / channelsToRead;
            ++sampleCounts[static_cast<size_t> (cacheIndex)];
        }

        samplePosition += samplesToRead;
    }

    for (auto i = 0; i < cacheSize; ++i)
    {
        if (sampleCounts[static_cast<size_t> (i)] > 0)
            rms[static_cast<size_t> (i)] = static_cast<float> (std::sqrt (squareSums[static_cast<size_t> (i)]
                                                                         / sampleCounts[static_cast<size_t> (i)]));
    }
}

void AudioClipView::drawWaveformLayer (juce::Graphics& g,
                                       juce::Rectangle<float> bounds,
                                       const std::vector<float>& source,
                                       juce::Colour colour)
{
    if (source.empty() || bounds.getWidth() <= 0.0f)
        return;

    const auto width = juce::jmax (1, juce::roundToInt (bounds.getWidth()));
    const auto centreY = bounds.getCentreY();
    const auto amplitude = bounds.getHeight() * 0.5f;

    juce::Path path;
    std::vector<float> topPoints (static_cast<size_t> (width));
    std::vector<float> bottomPoints (static_cast<size_t> (width));

    for (auto x = 0; x < width; ++x)
    {
        const auto start = static_cast<int> (static_cast<double> (x) * source.size() / width);
        const auto end = juce::jmax (start + 1,
                                     static_cast<int> (static_cast<double> (x + 1) * source.size() / width));

        auto level = 0.0f;

        for (auto i = start; i < juce::jmin (end, static_cast<int> (source.size())); ++i)
            level = juce::jmax (level, source[static_cast<size_t> (i)]);

        level = juce::jlimit (0.0f, 1.0f, level);
        topPoints[static_cast<size_t> (x)] = centreY - level * amplitude;
        bottomPoints[static_cast<size_t> (x)] = centreY + level * amplitude;
    }

    path.startNewSubPath (bounds.getX(), topPoints.front());

    for (auto x = 1; x < width; ++x)
        path.lineTo (bounds.getX() + static_cast<float> (x), topPoints[static_cast<size_t> (x)]);

    for (auto x = width; --x >= 0;)
        path.lineTo (bounds.getX() + static_cast<float> (x), bottomPoints[static_cast<size_t> (x)]);

    path.closeSubPath();

    g.setColour (colour);
    g.fillPath (path);
}

void AudioClipView::showContextMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, "Delete");
    menu.addItem (2, "Duplicate");
    menu.addItem (3, "Rename");

    const juce::Component::SafePointer<AudioClipView> safeThis (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                        [safeThis] (int result)
                        {
                            auto* clipView = safeThis.getComponent();

                            if (clipView == nullptr)
                                return;

                            if (result == 1 && clipView->onDeleteRequested)
                                clipView->onDeleteRequested (clipView->clipInfo.id);
                            else if (result == 2 && clipView->onDuplicateRequested)
                                clipView->onDuplicateRequested (clipView->clipInfo.id);
                            else if (result == 3 && clipView->onRenameRequested)
                                clipView->onRenameRequested (clipView->clipInfo.id, clipView->clipInfo.name + " Copy");
                        });
}
