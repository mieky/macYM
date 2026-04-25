# macYM — YM2149 / Atari ST sound chip synth for macOS

**A macOS native (Apple Silicon and Intel) VST3 / AU synthesizer plugin that emulates the Atari ST's YM2149 / AY-3-8910 PSG sound chip.**

If you came here looking for "ymVST for Mac", "ymVST macOS port", "Atari ST YM2149 VST for Mac", or "AY-3-8910 plugin for macOS" — this is a spiritual successor to Gareth Morris's classic Windows-only [ymVST](https://www.preromanbritain.com/ymvst/) (2003), rebuilt from scratch for modern macOS in C++ with JUCE.

![macYM screenshot](ymvst-mac.png)

## Features

- 3 square wave tone channels with per-channel fine tune, on/off, and tone/noise mixing
- Noise generator (5-bit period, 0-31)
- Hardware envelope generator with all 16 YM2149 envelope shapes
- User-drawn amplitude waveform editor (32-step, 4-bit volume) with one-shot or loop modes
- Arpeggiator with sync, speed, and length controls
- Portamento (pitch glide)
- Sound bend / Noise bend (gradual pitch shifting)
- Tremolo (LFO amplitude modulation)
- SID hard-sync mode (channel 1 syncs to channel 0 for metallic tones)
- Mono / 3-voice polyphony toggle
- 27 factory presets covering leads, basses, kicks, snares, hihats, FX, and chord patterns
- Full DAW parameter automation
- State save and restore (including the user-drawn waveform)
- MIDI CC mapping compatible with the original ymVST
- QWERTY computer keyboard input in standalone mode (Renoise / tracker layout)
- Retro Atari ST GEM desktop-style UI with custom 8x8 pixel font

## Requirements

- macOS 11 or later (Apple Silicon native, Intel x86_64 universal binary supported)
- Compatible with Renoise, Logic Pro, Ableton Live, Bitwig, REAPER, GarageBand, and any host that supports VST3 or AU plugins

## Install (pre-built binaries)

Download the latest release from the [Releases page](https://github.com/) and copy the plugins:

```bash
cp -r macYM.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -r macYM.component ~/Library/Audio/Plug-Ins/Components/
```

Then rescan plugins in your DAW.

## Build from source

### Requirements

- macOS with Xcode Command Line Tools (`xcode-select --install`)
- CMake 3.22 or later

### Build

```bash
# First-time configuration downloads JUCE (~2 min)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

This produces all three targets: VST3, AU, and a standalone app.

To rebuild quickly after making changes:

```bash
./build.sh
```

To build a specific target:

```bash
cmake --build build --target macym_VST3 --config Release
cmake --build build --target macym_AU --config Release
cmake --build build --target macym_Standalone --config Release
```

### Install (development symlinks)

```bash
ln -sf "$(pwd)/build/macym_artefacts/Release/VST3/macYM.vst3" ~/Library/Audio/Plug-Ins/VST3/
ln -sf "$(pwd)/build/macym_artefacts/Release/AU/macYM.component" ~/Library/Audio/Plug-Ins/Components/
```

### Run standalone

```bash
open build/macym_artefacts/Release/Standalone/macYM.app
```

## Standalone keyboard layout

The standalone app accepts QWERTY keyboard input using the standard tracker / Renoise layout:

```
Lower octave (C-3): Z X C V B N M    (white keys)
                    S D   G H J      (black keys)

Upper octave (C-4): Q W E R T Y U    (white keys)
                    2 3   5 6 7      (black keys)

Extended (C-5):     I O P            (white keys)
                    9 0              (black keys)
```

`Esc` triggers the panic button (all notes off).

## Why does this exist?

Gareth Morris's original ymVST is a beloved Windows-only freeware VST plugin from 2003 that recreates the Atari ST's iconic chip-tune sound. It's a SynthEdit-built 32-bit plugin and has no macOS port. macYM exists to give Mac users access to the same family of YM2149 / AY-3-8910 sounds, with the original UI aesthetic and feature set, but as a native modern plugin (Apple Silicon, VST3, AU).

## Credits

- **Gareth Morris** — original [ymVST](https://www.preromanbritain.com/ymvst/) for Windows (2003), the inspiration for this project's feature set, UI design, and MIDI CC mapping.
- **Peter Sovietov** — [ayumi](https://github.com/true-grue/ayumi), the YM2149 / AY-3-8910 emulator at the heart of this plugin (MIT license).
- **JUCE** — [framework](https://juce.com/) for cross-platform audio plugin development.

## License

macYM is released under the **GNU General Public License v3**, due to the JUCE free license terms. Source code is included; redistributions must remain GPL v3. See `LICENSE` for the full text.

The bundled ayumi library is MIT licensed.

