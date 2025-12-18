# WavPlayer Module - Technical Documentation

## Overview

**WavPlayer** is a professional-grade WAV file player module for VCV Rack, designed for both traditional sample playback and advanced granular/slice-based synthesis. It combines robust audio file handling with flexible modulation and real-time control.

---

## Architecture

### DSP Engine
The module wraps `ShortwavDSP::WavPlayer`, a header-only C++ DSP class providing:
- RIFF/WAVE format parsing with comprehensive error handling
- Multi-format support: 8/16/24/32-bit PCM, IEEE 32-bit float
- Lock-free audio path with atomic parameters
- High-quality cubic interpolation for pitch/speed changes
- Thread-safe file I/O with mutex protection

### Module Features
- 12 parameters for comprehensive control
- 4 inputs: trigger, slice CV, speed CV, pitch CV
- 2 outputs: stereo audio (L/R)
- 33 lights: play indicator + 32 slice LEDs
- Custom waveform display widget with NanoVG rendering
- Async file loading with progress tracking
- JSON state serialization

---

## Parameters

### Transport Controls

#### PLAY_BUTTON (Range: 0-1)
- **Type**: Trigger button
- **Function**: Toggle play/pause state
- **Behavior**: 
  - Press while stopped → start playback from current position
  - Press while playing → pause (maintains position)
  - Rising edge detection via `dsp::SchmittTrigger`

#### STOP_BUTTON (Range: 0-1)
- **Type**: Trigger button
- **Function**: Stop playback and reset
- **Behavior**:
  - Stops audio immediately
  - Resets position to beginning (sample 0)
  - Clears current slice selection

### Playback Modes

#### LOOP_MODE (Range: 0-2, discrete)
- **0**: Loop Off - play once, stop at end
- **1**: Loop Forward - restart from beginning when reaching end
- **2**: Loop Ping-Pong - reverse direction at boundaries
- **DSP Mapping**:
  ```cpp
  0 → ShortwavDSP::LoopMode::Off
  1 → ShortwavDSP::LoopMode::Forward
  2 → ShortwavDSP::LoopMode::PingPong
  ```

#### REVERSE (Range: 0-1, discrete)
- **0**: Forward playback
- **1**: Reverse playback
- **Note**: Works independently of loop mode; reverses playback direction

### Audio Processing

#### SPEED_PARAM (Range: -2 to +2)
- **Units**: Octaves
- **Default**: 0 (unity speed)
- **CV Modulation**: Additive from SPEED_CV input
- **DSP Mapping**: `speed_ratio = 2^(octaves)`
  - -2 oct → 0.25x speed (4x slower)
  - -1 oct → 0.5x speed (2x slower)
  - 0 oct → 1.0x speed (normal)
  - +1 oct → 2.0x speed (2x faster)
  - +2 oct → 4.0x speed (4x faster)
- **Effect**: Changes playback rate, affecting both pitch and duration

#### PITCH_PARAM (Range: -12 to +12)
- **Units**: Semitones
- **Default**: 0 (no pitch shift)
- **CV Modulation**: Additive from PITCH_CV input
- **DSP Mapping**: `pitch_ratio = 2^(semitones/12)`
  - -12 st → 0.5x pitch (1 octave down)
  - -7 st → 0.6674x pitch (perfect fifth down)
  - 0 st → 1.0x pitch (normal)
  - +7 st → 1.4983x pitch (perfect fifth up)
  - +12 st → 2.0x pitch (1 octave up)
- **Effect**: Changes pitch without affecting duration (independent of speed)

#### VOLUME_PARAM (Range: 0-2)
- **Default**: 1.0 (unity gain)
- **DSP Mapping**: Direct multiplication of output
  - 0.0 → silence
  - 1.0 → unity (no attenuation)
  - 2.0 → +6dB boost
- **Output Scaling**: Combined with Eurorack 5V scaling factor

#### INTERP_QUALITY_PARAM (Range: 0-2, discrete)
- **0**: None - no interpolation (aliasing artifacts)
- **1**: Linear - simple linear interpolation
- **2**: Cubic (default) - high-quality 4-point cubic interpolation
- **Recommended**: Keep at 2 for best audio quality

### Visualization

#### ZOOM_PARAM (Range: 0-1)
- **Function**: Waveform display zoom level
- **Behavior**:
  - 0.0 → show entire waveform
  - 1.0 → maximum zoom (show small portion around playback position)
- **UI Only**: Does not affect audio processing

### Slicing System

#### NUM_SLICES_PARAM (Range: 1-32)
- **Default**: 1 (no slicing, play full file)
- **Function**: Divide waveform into N equal slices
- **Behavior**:
  - 1 → No slicing (full file playback)
  - 2-32 → Split file into equal parts
- **Slice Calculation**:
  ```cpp
  samplesPerSlice = totalSamples / numSlices
  slice[i].start = i * samplesPerSlice
  slice[i].end = (i == numSlices-1) ? totalSamples : (i+1) * samplesPerSlice
  ```
- **UI**: Slice boundaries shown in waveform display

#### SLICE_SELECT_PARAM (Range: 0-31)
- **Function**: Manual slice selection
- **Behavior**:
  - Only active when SLICE_CV input is disconnected
  - Determines which slice plays on next trigger
  - Clamped to valid slice range (0 to numSlices-1)

#### TRIGGER_MODE_PARAM (Range: 0-1, discrete)
- **0**: Edge mode - trigger on rising edge only
- **1**: Gate mode - play while high, stop while low
- **Edge Mode**:
  - Uses `dsp::SchmittTrigger` for edge detection
  - Debounced (hysteresis prevents false triggers)
  - Ideal for percussion, one-shot samples
- **Gate Mode**:
  - Continuous playback while gate voltage > ~1V
  - Stops immediately when voltage falls below threshold
  - Ideal for sustained notes, looped playback

---

## Inputs

### TRIGGER_INPUT
- **Voltage Range**: 0-10V
- **Threshold**: ~1V (Schmitt trigger with hysteresis)
- **Function**: Start playback of current/selected slice
- **Behavior**:
  - **Edge Mode**: Triggers on rising edge (0V → 5V+)
  - **Gate Mode**: Plays while voltage high, stops when low
- **Interaction with Slices**:
  - When slicing active: seeks to selected slice start, then plays
  - When slicing off: plays from current position or seeks to beginning

### SLICE_CV_INPUT
- **Voltage Range**: 0-10V (clamped)
- **Function**: Voltage-controlled slice selection
- **Mapping**: Linear interpolation across slice indices
  ```cpp
  sliceIndex = floor(voltage / 10.0 * numSlices)
  sliceIndex = clamp(sliceIndex, 0, numSlices-1)
  ```
- **Examples** (8 slices):
  - 0.0V → slice 0
  - 1.25V → slice 1
  - 5.0V → slice 4
  - 9.9V → slice 7
  - 10.0V → slice 7 (last slice)
- **Priority**: Overrides SLICE_SELECT_PARAM when connected

### SPEED_CV_INPUT
- **Voltage Range**: Typically -5V to +5V (additive)
- **Function**: Modulate playback speed
- **Mapping**: Additive with SPEED_PARAM
  ```cpp
  finalSpeed = 2^(SPEED_PARAM + SPEED_CV)
  ```
- **Example**: If SPEED_PARAM=0 and SPEED_CV=+1V, final speed = 2^1 = 2x

### PITCH_CV_INPUT
- **Voltage Range**: Typically -5V to +5V (additive)
- **Function**: Modulate pitch shift
- **Mapping**: Additive with PITCH_PARAM (as semitones)
  ```cpp
  totalSemitones = PITCH_PARAM + (PITCH_CV * 12) // if using 1V/oct
  finalPitch = 2^(totalSemitones / 12)
  ```

---

## Outputs

### AUDIO_OUTPUT_L / AUDIO_OUTPUT_R
- **Voltage Range**: -5V to +5V (typical Eurorack audio levels)
- **Format**: Stereo audio
- **Behavior**:
  - Mono files: duplicated to both outputs
  - Stereo files: separate L/R channels
  - Scaling: DSP output (-1 to +1) × 5.0
- **When Stopped**: Outputs 0V (silence)

---

## Lights

### PLAY_LIGHT
- **Brightness**: 1.0 when playing, 0.0 when stopped/paused
- **Function**: Visual indicator of playback state

### SLICE_LIGHTS[32]
- **Brightness**: 1.0 for active slice, 0.0 for inactive
- **Function**: Shows currently selected/playing slice
- **Behavior**: Only visible when NUM_SLICES > 1

---

## Context Menu

### Load WAV File...
- Opens native OS file dialog (via `osdialog`)
- Filters: .wav and .WAV extensions
- **Thread-Safe Loading**:
  - File I/O runs in separate thread
  - Atomic flags prevent race conditions
  - UI remains responsive during load
- **Error Handling**: Displays error message if load fails

### File Info
- **Displays** (when file loaded):
  - Filename
  - Sample rate (Hz)
  - Channels (1=mono, 2=stereo)
  - Duration (seconds)
  - Number of samples
  - Bit depth and format
- **Format**: Read-only informational text

### Clear File
- Unloads current file
- Resets playback state
- Frees memory

---

## Slice Playback Behavior

### Slice Boundary Detection

When slicing is active (`NUM_SLICES` > 1), the module monitors playback position:

```cpp
if (currentPlaybackPosition >= currentSlice.endSample) {
  player.stop(); // Stop at slice boundary
}
```

- **One-Shot Behavior**: Each slice plays once then stops (unless looped)
- **Retriggering**: Send new trigger to play same or different slice
- **Loop Mode Interaction**:
  - Loop Off: stops at slice end
  - Loop Forward: loops within slice boundaries
  - Ping-Pong: bounces within slice boundaries

### Slice Selection Workflow

1. **Set Slice Count**: Adjust `NUM_SLICES_PARAM` (e.g., 8 slices)
2. **Select Slice**: 
   - Manual: Turn `SLICE_SELECT_PARAM` knob
   - CV: Send 0-10V to `SLICE_CV_INPUT`
3. **Trigger Playback**: Send gate/trigger to `TRIGGER_INPUT`
4. **Module Actions**:
   - Calculates slice start sample: `slice[idx].startSample`
   - Seeks DSP engine to that position
   - Starts playback
   - Monitors for slice end boundary

---

## State Serialization

### Saved State (JSON)
```json
{
  "filePath": "/path/to/file.wav",
  "sliceOrder": [0, 1, 2, 3, 4, 5, 6, 7]
}
```

- **filePath**: Absolute path to loaded WAV file
  - Automatically reloads on patch load
  - Fails gracefully if file moved/deleted
- **sliceOrder**: Array of slice ordering indices
  - Reserved for future slice reordering feature
  - Currently maintains original order [0, 1, 2, ..., N-1]

### Non-Persisted State
- Playback position (always resets to beginning)
- Play/pause state (always starts stopped)
- Current parameter CV modulations

---

## Thread Safety

### File Loading
- **Main Thread**: UI, parameter updates, audio processing
- **Background Thread**: File I/O, WAV parsing, sample conversion
- **Synchronization**:
  ```cpp
  std::atomic<bool> fileLoading_;  // Is load in progress?
  std::atomic<bool> fileLoaded_;   // Is file ready?
  std::mutex fileMutex_;           // Protects file I/O
  ```
- **Audio Path**: Lock-free; uses atomic parameters from DSP engine

### Slice Management
- **Mutex Protection**: `std::mutex sliceMutex_`
- **Critical Sections**:
  - `updateSlices()` - modifies slice array
  - `getCurrentSlice()` - reads slice array
  - `triggerSlice()` - reads slice array
  - `process()` - updates slice LEDs

---

## Performance Characteristics

### CPU Usage
- **Idle**: Minimal (parameter polling only)
- **Playing**: Low (optimized DSP path)
  - No allocations in audio path
  - Lock-free parameter reads
  - Efficient cubic interpolation
- **File Loading**: Runs in background thread (no audio interruption)

### Memory Usage
- **Per Instance**: ~1-2KB (excluding sample data)
- **Sample Data**: Depends on file size
  - 16-bit stereo, 44.1kHz, 1 minute ≈ 10.5 MB
  - Samples stored in `std::vector<float>` (uncompressed)

### Latency
- **Audio Path**: <1ms (single sample processing)
- **Trigger Response**: <1ms (SchmittTrigger + seek operation)
- **Parameter Changes**: Immediate (atomic updates)

---

## Waveform Display Widget

### Rendering
- **Technology**: NanoVG (OpenGL-accelerated)
- **Update Rate**: 60 Hz (synced to UI refresh)
- **Draw Operations**:
  1. Draw background (dark)
  2. Draw waveform (sampled, anti-aliased)
  3. Draw slice boundaries (vertical lines)
  4. Draw playback position (red vertical line)
  5. Draw zoom focus region

### Zoom Behavior
- **Centered on Playback**: Zoom focuses on current playback position
- **Dynamic Range**: Automatically scales amplitude for visibility
- **Slice Markers**: Scale with zoom level

---

## Known Limitations

1. **Mono-to-Stereo**: Mono files duplicate to both channels (no independent L/R processing)
2. **Fixed Slice Count**: Slices are always equal-length (no manual boundary setting)
3. **Slice Reordering**: UI for reordering slices not yet implemented (reserved in JSON)
4. **No Time-Stretching**: Speed change affects pitch (use PITCH param to compensate)
5. **File Format**: WAV/RIFF only (no MP3, FLAC, OGG support)

---

## Use Cases

### 1. Drum Sampler
- Load drum loop, slice into 8-16 hits
- Sequence via TRIGGER + SLICE_CV
- Use SPEED_CV for swing/humanization

### 2. Granular Synthesis
- Load textural audio
- Set 16-32 slices
- Randomize SLICE_CV + rapid triggers
- Modulate SPEED/PITCH for dense textures

### 3. Melodic Sampler
- Load single-note samples
- No slicing (NUM_SLICES=1)
- Use PITCH_CV for melodic control (1V/oct)
- Gate TRIGGER for ADSR envelope

### 4. Loop Mangler
- Load loop, set 4-8 slices
- Use sequencer to jump between slices
- Apply reverse/ping-pong for variation
- Modulate SPEED for rhythmic effects

---

## Troubleshooting

### File Won't Load
- **Check Format**: Must be valid WAV/RIFF file
- **Check Encoding**: PCM or IEEE float only (no compressed formats)
- **Check Size**: Very large files (>100MB) may take time to load
- **Check Path**: File must exist and be readable

### No Audio Output
- **File Loaded?** Check context menu for file info
- **Playing?** PLAY_LIGHT should be lit
- **Volume?** Check VOLUME_PARAM (default=1.0)
- **Cables?** Verify output connections

### Slicing Not Working
- **NUM_SLICES > 1?** Must be set to 2 or higher
- **File Loaded?** Slices calculate on load
- **Trigger Connected?** Need trigger to start slice playback
- **SLICE_CV Connected?** If yes, it overrides manual SELECT

### Clicks/Pops
- **At Slice Boundaries**: Expected when stopping mid-sample
- **During Speed Changes**: Use smaller CV changes or add slew limiter
- **At Loop Points**: Ensure loop-friendly audio (zero-crossings aligned)

---

## Algorithm Details

### Cubic Interpolation
```cpp
// 4-point Lagrange interpolation for high-quality resampling
y = c0*x^3 + c1*x^2 + c2*x + c3
// Coefficients calculated from surrounding samples y[-1], y[0], y[1], y[2]
```
- **Advantages**: Smooth, minimal aliasing, good transient response
- **Cost**: 4 multiplies + 3 adds per sample

### Speed/Pitch Independence
- **Speed**: Changes playback pointer increment rate
  - `pointerIncrement = baseRate * speedRatio`
- **Pitch**: Additional interpolation scaling
  - `interpolationPhase *= pitchRatio`
- **Combined**: Allows independent control of both parameters

---

## Future Enhancements (Potential)

- Manual slice boundary editing
- Slice reordering via drag-drop in display
- Multiple loop modes per slice
- Crossfade at slice boundaries
- ADSR envelope per slice
- Sample start/end markers
- Reverse-only slices
- Sub-sampling (skip every N slices)

---

## See Also

- [WavPlayer_QuickStart.md](WavPlayer_QuickStart.md) - Quick reference guide
- [src/dsp/wav-player.h](../src/dsp/wav-player.h) - DSP engine source code
- [src/WavPlayer.hpp](../src/WavPlayer.hpp) - Module header
- [src/WavPlayer.cpp](../src/WavPlayer.cpp) - Module implementation
