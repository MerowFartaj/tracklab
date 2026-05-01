# Development

This guide covers the current local build path for the Tracklab desktop app.
The app is intentionally minimal right now: it opens an empty JUCE window and
links Tracktion Engine so the toolchain is proven end to end.

## Prerequisites

- macOS 13 or newer
- Xcode 15 or newer
- CMake 3.22 or newer
- Git

Tracktion Engine `develop` currently requires C++20, so Tracklab builds with
C++20 even though the rest of the app shell is simple.

## Clone

Clone the repository with submodules:

```sh
git clone --recursive https://github.com/MerowFartaj/tracklab.git
cd tracklab
```

If you already cloned without `--recursive`, initialize the submodules:

```sh
git submodule update --init --recursive
```

## Build

Run the helper script from the repository root:

```sh
./scripts/build.sh
```

On a full Xcode installation, the script configures an Xcode build directory
and builds the Debug app bundle:

```sh
cmake -B build -G Xcode
cmake --build build --config Debug
```

If full Xcode is not installed, the helper script falls back to Unix Makefiles
so a local smoke build can still produce the app bundle. Install and select
Xcode to generate the Xcode project.

## Run

After a successful Debug build, launch:

```sh
open build/tracklab_app_artefacts/Debug/Tracklab.app
```

You should see a native macOS window titled `Tracklab` with a dark placeholder
screen. DAW features, audio playback, plugin hosting, and project loading are
planned for later milestones.

## Troubleshooting

### CMake is not installed

Install CMake with Homebrew or from Kitware:

```sh
brew install cmake
```

Then run `./scripts/build.sh` again.

### Xcode generator is unavailable

Install Xcode 15 or newer from Apple, then select it:

```sh
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
```

Accept the license if prompted:

```sh
sudo xcodebuild -license
```

### Submodules are missing

If CMake cannot find JUCE or Tracktion Engine, initialize submodules:

```sh
git submodule update --init --recursive
```

Tracktion Engine also contains its own nested JUCE submodule. Tracklab uses the
top-level `external/JUCE` checkout instead, so the nested Tracktion copy does
not need to be initialized for this build.

### The build directory has stale settings

If you switch generators or change Xcode versions, remove the build directory
and configure again:

```sh
rm -rf build
./scripts/build.sh
```

### Architecture build issues

Tracklab currently builds a Universal Binary for `arm64` and `x86_64`.
Make sure your selected Xcode installation includes the macOS SDK required for
both architecture slices.
