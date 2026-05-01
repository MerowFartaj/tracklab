# Tracklab Base DAW Feature Matrix

This document maps the base DAW features Tracklab should grow toward. It is based on the public Logic Pro and Ableton Live manuals, then translated into Tracklab terms so implementation can stay focused and honest.

Research date: May 1, 2026

## Sources

- Logic Pro User Guide: https://support.apple.com/guide/logicpro/welcome/mac
- Logic Pro scrubbing: https://support.apple.com/guide/logicpro/scrub-a-project-lgcpf7c0c6b5/mac
- Logic Pro Channel EQ: https://support.apple.com/guide/logicpro/channel-eq-overview-lgcef1edce5b/mac
- Logic Pro Transposer: https://support.apple.com/guide/logicpro/transposer-controls-lgceee5a5e6f/mac
- Ableton Live 12 Manual: https://www.ableton.com/en/live-manual/12/
- Ableton Live audio effects: https://www.ableton.com/en/live-manual/12/live-audio-effect-reference/
- Ableton Live audio clips, tempo, and warping: https://www.ableton.com/en/live-manual/12/audio-clips-tempo-and-warping/

## Scope Rule

Tracklab treats these as base DAW features, not third-party plugin hosting:

- Project transport, tempo, time signature, key, loop, click, and snap
- Tracks, clips, clip editing, waveform views, lanes, and mixer controls
- Recording readiness, monitoring, count-in, punch ranges, and take workflows
- Built-in channel tools like EQ, pitch shift, transpose, utility gain, and metering
- MIDI editing, piano roll, quantize, velocity, scale, and groove tools
- Browsers, project media, markers, arrangement sections, export, and project save

Third-party VST3, AU, CLAP, and external plugin hosting stay out of this pass.

## Implemented In The Current Foundation

- Multi-track audio timeline with clip import, move, trim, duplicate, delete, and waveform rendering
- Mixer strips with volume, pan, mute, solo, track meters, master meter, and master gain
- Project transport with play, pause, stop, position, and timeline click-to-seek
- Project tempo, time signature, key, loop range, metronome state, and snap mode as engine-owned state
- Clip inspector state for gain, fade in, fade out, transpose, source offset, and visible length
- Clip split at the playhead for selected audio clips
- Built-in track device state for Tracktion Equaliser and Pitch Shifter
- Browser, inspector, and device-rack shell in the app UI

## Feature Map

| Area | Logic and Ableton baseline | Tracklab status |
| --- | --- | --- |
| Transport | Play, stop, record, cycle or arrangement loop, count-in, metronome, tempo, time signature, key | Foundation state and UI. Recording and count-in need dedicated audio input work |
| Navigation | Zoom, scroll, locator, click-to-seek, scrub, markers | Zoom, scroll, seek work. Scrub is mapped to fast playhead audition state for now |
| Arrangement | Audio and MIDI tracks, regions or clips, split, trim, duplicate, fades, crossfades, loop regions | Audio clip basics work. Crossfades, region looping UI, and MIDI clips are future |
| Mixer | Channel strips, master strip, meters, volume, pan, mute, solo, sends, returns, groups | Core strips work. Sends, returns, groups, and routing UI are future |
| Built-in EQ | Logic Channel EQ and Ableton EQ Eight style channel tone shaping | Tracktion 4-band Equaliser can be inserted and controlled from the rack |
| Built-in pitch tools | Logic Transposer, Flex Pitch, Ableton clip pitch and Shifter | Tracktion Pitch Shifter can be inserted. Clip transpose is stored on audio clips |
| Warping and Flex | Tempo following, transient or warp markers, Flex Time and pitch | Not implemented. Needs a dedicated time-stretch and warp-marker session |
| Recording | Audio and MIDI record arm, monitoring, punch, takes, comping | Record arm UI exists. Real recording, punch, take lanes, and comping are future |
| MIDI | Piano roll, note editing, velocity, quantize, scale, transpose, step sequencing | Planned. This pass only creates project and device foundation |
| Session clips | Ableton Session View and Logic Live Loops style launching | Planned after arrangement and MIDI are stable |
| Automation | Track, clip, mixer, tempo, and device automation | Planned after device and parameter model hardens |
| Browser | Samples, project media, presets, devices, search, tags, preview | Shell UI exists. File indexing, preview, tags, and connectors are future |
| Export and project files | Save, load, collect media, bounce, export tracks or mix | Planned. Tracktion has rendering and DAWproject support to evaluate |

## Next Dedicated Sessions

1. Project save and load, including media reference repair.
2. Real recording: audio input, monitoring, count-in, punch range, and take files.
3. MIDI track type with piano roll, quantize, velocity, scale, and transpose.
4. Audio clip fades, crossfades, looping, reverse, normalize, and bounce in place.
5. Warp and tempo features: auto tempo, stretch modes, clip tempo, and warp markers.
6. Automation lanes for volume, pan, tempo, and built-in device parameters.
7. Browser indexing and preview with local samples and connector-ready metadata.
