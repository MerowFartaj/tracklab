# App

This directory will contain the desktop application shell.

Planned scope:

- Application lifecycle
- Project window setup
- Integration between UI, engine, AI, and connectors
- macOS app packaging entry points

Current state:

- `src/Main.cpp` defines the JUCE application entry point.
- `src/MainWindow.*` creates the native desktop window.
- `src/MainComponent.*` paints the temporary placeholder screen.
