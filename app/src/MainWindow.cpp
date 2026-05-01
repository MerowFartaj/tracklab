#include "MainWindow.h"

#include "DesignTokens.h"

MainWindow::MainWindow()
    : DocumentWindow ("Tracklab",
                      tracklab::design::backgroundBase,
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setResizable (true, true);
    setContentOwned (new MainComponent(), true);
    centreWithSize (1280, 800);
    setVisible (true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
