# Tracklab

An AI-native DAW that connects to your whole music ecosystem.

![Status](https://img.shields.io/badge/status-pre--alpha%20%7C%20in%20development-orange)
![License](https://img.shields.io/badge/license-GPLv3-blue)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20more%20coming-lightgrey)

Tracklab is a modern digital audio workstation built on JUCE and Tracktion Engine. It combines a professional DAW foundation with an AI co-producer and an open connector framework for services such as Splice, Spotify, Google Drive, Suno, and local sample libraries.

## Why Tracklab?

Most DAWs treat the project as the whole world. Your samples, references, stems, generated ideas, cloud folders, playlists, and collaborator assets live somewhere else.

Tracklab is designed around the reality of modern music production. The DAW should understand the project in front of you and the wider music ecosystem around it.

The built-in AI co-producer works with structured project state and connector metadata so it can help find sounds, explain matches, organize assets, and suggest next steps without turning the core DAW into a black box.

## Features

### Engine

Tracklab will use Tracktion Engine for the core production surface:

- Audio playback and editing
- MIDI sequencing
- VST3 and AU plugin hosting
- Mixer routing
- Automation
- Project save and load

### What's New

Tracklab adds a modern layer around the engine:

- Connector framework for external services and local resources
- AI co-producer that can use tools across connectors
- Deterministic music theory layer for key and BPM compatibility
- Custom JUCE-based interface designed for modern production workflows

## Connectors

Planned launch connectors:

- Splice
- Local Library
- Spotify
- Google Drive
- Suno

The connector framework is planned to be open and extensible. Connectors expose typed capabilities to the AI layer, such as searching metadata, importing assets, watching folders, or collecting references.

See [docs/CONNECTORS.md](docs/CONNECTORS.md) for the current connector design.

## Tech Stack

- C++
- JUCE
- Tracktion Engine
- Anthropic API for AI

## Status

Tracklab is in pre-alpha and active planning. The repository currently contains documentation and scaffolding only. No audio engine code, UI code, or third-party submodules have been added yet.

See [docs/ROADMAP.md](docs/ROADMAP.md) for planned milestones.

## Building Locally

Build instructions are coming soon. The first code pass will add the JUCE, Tracktion Engine, and CMake setup.

See [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) for the planned local toolchain.

## Contributing

Tracklab will open to outside contributions after the v1 architecture and core app structure are stable. Until then, issues and design discussion may be used to shape the project direction.

## License

Tracklab is licensed under the GNU General Public License v3.0. This is required by the planned Tracktion Engine integration.

See [LICENSE](LICENSE) for details.
