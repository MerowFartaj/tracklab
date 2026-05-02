#include "MainWindow.h"

#include "DesignTokens.h"

MainWindow::MainWindow()
    : DocumentWindow ("Tracklab",
                      tracklab::design::backgroundBase,
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (false);
    setTitleBarHeight (0);
    setResizable (true, true);
    setContentOwned (new MainComponent(), true);
    centreWithSize (1600, 1000);
    setVisible (true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
