#pragma once

#include "MainComponent.h"

class MainWindow final : public juce::DocumentWindow
{
public:
    MainWindow();

    void closeButtonPressed() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};
