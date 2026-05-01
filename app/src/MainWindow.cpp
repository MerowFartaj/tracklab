#include "MainWindow.h"

MainWindow::MainWindow()
    : DocumentWindow ("Tracklab",
                      juce::Colour::fromRGB (0x1a, 0x1a, 0x1a),
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
