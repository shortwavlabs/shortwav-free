# WavPlayer Quick Start

**WavPlayer** is a comprehensive WAV file player module for VCV Rack with advanced slicing, triggering, and real-time modulation capabilities.

## Key Features

- **WAV File Loading**: Supports 8/16/24/32-bit PCM and IEEE float formats
- **Slice Playback**: Split files into 2-32 equal slices with CV or manual selection
- **Trigger Input**: Edge or gate trigger modes for precise playback control
- **Real-time Modulation**: Independent speed and pitch control with CV inputs
- **Waveform Visualization**: Real-time display with zoom, playback marker, and slice boundaries
- **Loop Modes**: Off, Forward, and Ping-Pong looping

## Basic Usage

### 1. Load a WAV File
- Right-click the module → "Load WAV file..."
- Select your audio file (WAV format)
- File info displays in context menu after loading

### 2. Simple Playback
- Press **PLAY** button to start playback
- Press **STOP** to reset to beginning
- Connect **AUDIO L/R** outputs to your mixer/scope

### 3. Slice-Based Playback
- Set **SLICES** knob (1 = no slicing, 2-32 = number of equal slices)
- Use **SELECT** knob to choose which slice to play manually
- OR connect CV (0-10V) to **SLICE CV** input for voltage-controlled selection
- Send trigger to **TRIGGER** input to start playing selected slice

### 4. Modulation
- **SPEED**: -2 to +2 octaves (0.25x to 4x playback rate)
- **PITCH**: -12 to +12 semitones (independent pitch shift)
- Connect CV to **SPEED CV** and **PITCH CV** for real-time modulation

## Quick Patches

### Drum Sampler
1. Load drum loop WAV file
2. Set SLICES to 8 (8 drum hits)
3. Connect sequencer gates to TRIGGER input
4. Connect sequencer CV to SLICE CV to select different drums
5. Set TRIGGER MODE to edge (0.0)

### Granular-Style Playback
1. Load any audio file
2. Set SLICES to 16-32
3. Connect random CV to SLICE CV
4. Send regular triggers (e.g., clock divider)
5. Adjust SPEED/PITCH for texture variation

### Pitched Sampler
1. Load single-note WAV
2. Keep SLICES at 1 (full file)
3. Connect V/OCT signal to PITCH CV
4. Trigger with gates from keyboard/sequencer

## Controls Reference

### Transport
- **PLAY**: Play/pause toggle
- **STOP**: Stop and reset to beginning

### Parameters
- **LOOP MODE**: 0=Off, 1=Forward, 2=Ping-Pong
- **REVERSE**: Reverse playback direction
- **SPEED**: Playback speed in octaves
- **PITCH**: Pitch shift in semitones
- **VOLUME**: Output level (0-2, unity at 1)
- **ZOOM**: Waveform display zoom level

### Slicing
- **SLICES**: Number of equal slices (1-32)
- **SELECT**: Manual slice selection (0-31)

### Inputs
- **TRIGGER**: Trigger/gate input
- **SLICE CV**: Slice selection (0-10V mapped to slice indices)
- **SPEED CV**: Speed modulation (additive)
- **PITCH CV**: Pitch modulation (additive)

### Outputs
- **L/R**: Stereo audio outputs (5V Eurorack level)

## Tips

- **File not loading?** Check context menu for error messages. Ensure WAV file is valid PCM format.
- **Slice boundaries not visible?** Increase ZOOM parameter to see more waveform detail.
- **Triggering not working?** Verify TRIGGER MODE matches your source (edge for percussion, gate for continuous).
- **Clicks at slice boundaries?** This is expected when stopping mid-playback. Consider using LOOP MODE or shorter slices.
- **CV not selecting correct slice?** Slices are mapped linearly: 0V=slice 0, 10V=last slice.

## Next Steps

See [WavPlayer.md](WavPlayer.md) for detailed technical documentation including:
- Advanced slicing techniques
- Trigger mode details
- CV mapping specifications
- Thread-safe file loading behavior
- State serialization
