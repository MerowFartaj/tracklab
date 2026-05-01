cmake_minimum_required(VERSION 3.22)

# Tracklab is macOS-only while the project proves out its desktop toolchain.
if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    message(FATAL_ERROR "Tracklab currently supports macOS builds only.")
endif()

# Build a Universal Binary so the app runs on Apple Silicon and Intel Macs.
# This must be set before project() so CMake configures both architecture slices.
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures" FORCE)

project(Tracklab
    VERSION 0.0.1
    DESCRIPTION "AI-native DAW built with JUCE and Tracktion Engine"
    LANGUAGES C CXX)

if(NOT APPLE)
    message(FATAL_ERROR "Tracklab currently supports macOS builds only.")
endif()

# Tracktion Engine develop currently requires C++20.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Keep third-party example targets out of the Tracklab build.
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Build JUCE examples" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Build JUCE extras" FORCE)
set(TE_ADD_EXAMPLES OFF CACHE BOOL "Build Tracktion Engine examples" FORCE)

# JUCE provides the application framework, GUI modules, and CMake helpers.
add_subdirectory(external/JUCE)

# Tracktion Engine is added after JUCE so it uses the same JUCE module targets.
add_subdirectory(external/tracktion_engine)

# juce_add_gui_app creates a native macOS .app bundle for the executable.
juce_add_gui_app(tracklab_app
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "Merow Fartaj"
    PRODUCT_NAME "Tracklab"
    BUNDLE_ID "com.merowfartaj.tracklab")

target_sources(tracklab_app
    PRIVATE
        app/src/Main.cpp
        app/src/AudioClipView.cpp
        app/src/AudioClipView.h
        app/src/DesignTokens.h
        app/src/MainComponent.cpp
        app/src/MainComponent.h
        app/src/MainWindow.cpp
        app/src/MainWindow.h
        app/src/MixerPanel.cpp
        app/src/MixerPanel.h
        app/src/Playhead.cpp
        app/src/Playhead.h
        app/src/TimelineRuler.cpp
        app/src/TimelineRuler.h
        app/src/TimelineView.cpp
        app/src/TimelineView.h
        app/src/Toolbar.cpp
        app/src/Toolbar.h
        app/src/TrackRow.cpp
        app/src/TrackRow.h
        app/src/WorkspacePanel.cpp
        app/src/WorkspacePanel.h
        engine/AudioEngine.cpp
        engine/AudioEngine.h)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/app/src
    PREFIX "App"
    FILES
        app/src/Main.cpp
        app/src/AudioClipView.cpp
        app/src/AudioClipView.h
        app/src/DesignTokens.h
        app/src/MainComponent.cpp
        app/src/MainComponent.h
        app/src/MainWindow.cpp
        app/src/MainWindow.h
        app/src/MixerPanel.cpp
        app/src/MixerPanel.h
        app/src/Playhead.cpp
        app/src/Playhead.h
        app/src/TimelineRuler.cpp
        app/src/TimelineRuler.h
        app/src/TimelineView.cpp
        app/src/TimelineView.h
        app/src/Toolbar.cpp
        app/src/Toolbar.h
        app/src/TrackRow.cpp
        app/src/TrackRow.h
        app/src/WorkspacePanel.cpp
        app/src/WorkspacePanel.h)

# Let app and UI code include project headers from the repository root.
target_include_directories(tracklab_app
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})

# These flags keep optional JUCE network and browser features out of the app
# until Tracklab needs them. MP3 reading is enabled for the minimal loader.
target_compile_definitions(tracklab_app
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_MP3AUDIOFORMAT=1
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:tracklab_app,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:tracklab_app,JUCE_VERSION>")

set_target_properties(tracklab_app
    PROPERTIES
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        OSX_ARCHITECTURES "${CMAKE_OSX_ARCHITECTURES}"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.merowfartaj.tracklab")

# Link the JUCE modules that form the current desktop shell.
# Tracktion modules power the current transport, timeline, mixer, and device shell.
target_link_libraries(tracklab_app
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_opengl
        tracktion::tracktion_core
        tracktion::tracktion_graph
        tracktion::tracktion_engine)
