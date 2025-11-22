# Random LFO - Implementation Documentation

## Overview

The **Random LFO** module is a smooth random modulation source designed for evolving, organic parameter movement in the ShortwavFree plugin suite. It generates continuously varying random control voltages without hard discontinuities, making it ideal for adding subtle variation or dramatic modulation to patches.

## Algorithm Specification

### Source
Based on the algorithm from [musicdsp.org](https://www.musicdsp.org/en/latest/Synthesis/269-smooth-random-lfo-generator.html)  
**Original Concept**: Smooth random LFO generation via second-order system tracking

### Technical Architecture

The Random LFO uses a **critically-damped second-order system** that smoothly tracks randomly generated target values:

1. **Random Target Generation**: New random values are generated at a configurable rate (rateHz)
2. **Second-Order Tracking**: A biquad-like filter smoothly transitions from current value to new target
3. **Smooth Interpolation**: Damping factor controls transition smoothness and correlation between values
4. **Band-Limited Output**: No aliasing or hard discontinuities in the output signal

### Algorithm Characteristics

- **System Type**: Second-order critically/under-damped tracking system
- **Target Generation**: Fast LCG (Linear Congruential Generator) for random values
- **Update Rate**: Configurable (0.01 Hz to 20 Hz typical)
- **Output Range**: Bipolar [-5V, +5V] or Unipolar [0V, +10V]
- **Smoothness**: Adjustable correlation factor (0 = sharp, 1 = very smooth)

---

## DSP Implementation (`src/dsp/random-lfo.h`)

### Class: `RandomLFO`

#### Configuration Methods

```cpp
void setSampleRate(float sampleRate);  // Set processing sample rate (required)
void setRate(float rateHz);            // How often new targets spawn (0.01-20 Hz)
void setDepth(float depth);            // Output gain/intensity (0..1+)
void setSmooth(float smooth);          // Correlation/damping (0..1, 0=sharp, 1=smooth)
void setBipolar(bool bipolar);         // true=[-1,1], false=[0,1]
void seed(uint32_t seedValue);         // Seed RNG for deterministic behavior
```

#### Processing Methods

```cpp
float processSample();  // Generate next LFO sample (call once per sample)
void reset(float initial = 0.f);  // Reset state to initial value
```

### Real-Time Safety Features

- **Zero allocations** in audio path
- **Lock-free** design (no mutexes)
- **Deterministic** seeding for consistent behavior across instances
- **Numerically stable** for sample rates 24kHz-192kHz
- **Fast RNG** using LCG (minimal CPU overhead)

### Parameter Ranges

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Rate** | 0.01-20 Hz | 1 Hz | Frequency of new random targets |
| **Depth** | 0-1 (or higher) | 1.0 | Output amplitude scaling |
| **Smooth** | 0-1 | 0.75 | Damping/correlation (0=fast, 1=slow) |
| **Bipolar** | false/true | true | Output range mode |

---

## Plugin Module (`src/RandomLfo.hpp`, `src/RandomLfo.cpp`)

### Parameters

| Parameter | Range | Default | Unit | Description |
|-----------|-------|---------|------|-------------|
| `RATE_PARAM` | 0.01-20 Hz | 1 Hz | Hz | Target generation rate |
| `DEPTH_PARAM` | 0-1 | 1.0 | - | Output amplitude |
| `SMOOTH_PARAM` | 0-1 | 0.75 | - | Smoothness/correlation |
| `BIPOLAR_PARAM` | 0/1 | 1 | - | Unipolar/Bipolar toggle |

### CV Inputs

| Input | Function | Range | Behavior |
|-------|----------|-------|----------|
| `RATE_CV_INPUT` | Rate modulation | 0-10V | Multiplicative (V/oct) |
| `DEPTH_CV_INPUT` | Depth modulation | 0-10V | Additive |
| `SMOOTH_CV_INPUT` | Smooth modulation | 0-10V | Additive |

**CV Modulation Details**:
- **Rate CV**: Applied multiplicatively as 1V/octave: `finalRate = baseRate * 2^(cv/1.0)`
- **Depth CV**: Added to base depth, clamped to [0, 2]
- **Smooth CV**: Added to base smooth, clamped to [0, 1]

### Outputs

| Output | Range | Description |
|--------|-------|-------------|
| `LFO_OUTPUT` | ±5V or 0-10V | Random modulation signal |

**Voltage Conventions**:
- **Bipolar mode**: Output scaled to [-5V, +5V] range
- **Unipolar mode**: Output scaled to [0V, +10V] range

---

## Usage Examples

### Basic Random Modulation

```
Random LFO Setup:
- Rate: 2 Hz (two new random values per second)
- Depth: 1.0 (full intensity)
- Smooth: 0.5 (moderate smoothness)
- Bipolar: ON

Result: Smoothly wandering CV at ±5V, updates twice per second
Use cases: Filter cutoff sweep, VCO pitch drift, effect parameter variation
```

### Slow Texture Evolution

```
Random LFO Setup:
- Rate: 0.1 Hz (one change every 10 seconds)
- Depth: 0.8 (80% intensity)
- Smooth: 0.9 (very smooth)
- Bipolar: OFF

Result: Extremely slow, smooth unipolar drift (0-8V)
Use cases: Long-form ambient textures, gradual timbre evolution
```

### Sample & Hold Simulation

```
Random LFO Setup:
- Rate: 8 Hz (fast updates)
- Depth: 1.0
- Smooth: 0.0 (minimal smoothing)
- Bipolar: ON

Result: Quasi-sample-and-hold with slight smoothing to prevent clicks
Use cases: Rhythmic modulation, stepped sequences with slight portamento
```

### Multi-LFO Cross-Modulation

```
Random LFO 1 → RATE_CV of Random LFO 2
Random LFO 2 → Filter cutoff

LFO1 Settings: Rate=0.5Hz, Smooth=0.8 (slow, smooth)
LFO2 Settings: Rate=4Hz base (modulated by LFO1), Smooth=0.3 (faster)

Result: Complex, non-repeating modulation with variable speed
Use cases: Generative patches, evolving soundscapes
```

---

## Technical Specifications

### CPU Performance
- **Per-sample cost**: ~15-20 CPU cycles
- **Memory footprint**: ~64 bytes per instance
- **SIMD**: Scalar implementation (single-channel)

### Numerical Characteristics
- **Phase accumulator**: 32-bit float (adequate for audio-rate random generation)
- **Filter state**: 32-bit float (x, v)
- **RNG**: 32-bit LCG with cycle length 2^32

### Frequency Response
The smoothing filter acts as a lowpass on the random excitation:
- **Corner frequency**: Approximately `rateHz * (1 - smooth)`
- **Rolloff**: Second-order (~12dB/octave)
- **DC offset**: None (symmetric bipolar, or 0.5 offset for unipolar)

---

## Module Widget

### Visual Layout (3HP Panel)

```
┌─────────────┐
│   RATE      │  Large knob (top)
│             │
│   DEPTH     │  Large knob (middle-upper)
│             │
│   SMOOTH    │  Large knob (middle-lower)
│             │
│  [BIPOLAR]  │  Toggle switch (0/1)
│             │
│  CV: R D S  │  Three CV inputs (Rate, Depth, Smooth)
│             │
│     OUT     │  Single CV output
└─────────────┘
```

### Controls
- **RATE knob**: Sets target generation frequency
- **DEPTH knob**: Controls output amplitude
- **SMOOTH knob**: Adjusts transition smoothness
- **BIPOLAR switch**: Up=bipolar, Down=unipolar

---

## Integration Guide

### Initialization

```cpp
#include "dsp/random-lfo.h"

ShortwavDSP::RandomLFO lfo;

void setup(float sampleRate) {
    lfo.setSampleRate(sampleRate);
    lfo.seed(12345);  // Deterministic seed
    lfo.setRate(1.0f);
    lfo.setDepth(1.0f);
    lfo.setSmooth(0.75f);
    lfo.setBipolar(true);
}
```

### Per-Sample Processing

```cpp
void process() {
    float modulation = lfo.processSample();
    // modulation is in range [-1, 1] or [0, 1] depending on bipolar setting
    
    // Scale to voltage range for Rack output
    float outputVolts = bipolar ? modulation * 5.0f : modulation * 10.0f;
}
```

### Parameter Updates (Control Rate)

```cpp
void updateParameters() {
    // Read from UI or CV inputs
    float rate = params[RATE_PARAM].getValue();
    
    // Apply CV modulation (if connected)
    if (inputs[RATE_CV_INPUT].isConnected()) {
        float cv = inputs[RATE_CV_INPUT].getVoltage();
        rate *= std::pow(2.0f, cv / 1.0f);  // 1V/oct
    }
    
    lfo.setRate(clamp(rate, 0.01f, 20.0f));
}
```

---

## Design Rationale

### Why Second-Order Tracking?

First-order smoothing (simple exponential) produces "corners" when targets change. A second-order critically-damped system provides:
1. **Smooth acceleration**: No discontinuities in velocity
2. **Natural feel**: Movement similar to physical systems
3. **Adjustable overshoot**: Under-damping can add interest

### Why LCG Instead of Mersenne Twister?

- **Speed**: LCG is ~10x faster than cryptographic RNG
- **Determinism**: Simple seeding with module pointer
- **Quality**: Adequate for audio-rate modulation (spectral properties are masked by smoothing)
- **Size**: Minimal state (one uint32_t)

### Seeding Strategy

```cpp
lfo.seed(reinterpret_cast<uint64_t>(this) & 0xFFFFFFFFu);
```

Each module instance uses its pointer address as seed, ensuring:
- **Different sequences** per instance (avoid identical modulation)
- **Deterministic** across sessions (same patch = same behavior)
- **No coordination** required between modules

---

## Common Use Cases

### 1. Filter Modulation
- **Rate**: 0.5-2 Hz
- **Smooth**: 0.7-0.9 (slow, organic)
- **Bipolar**: Yes (sweep both sides of center)

### 2. VCO Pitch Drift (Analog Emulation)
- **Rate**: 0.05-0.2 Hz (very slow)
- **Depth**: 0.1-0.3 (subtle)
- **Smooth**: 0.85-0.95 (very smooth)
- **Scale output**: ±0.2V (approx ±2 semitones)

### 3. Pan Modulation
- **Rate**: 1-4 Hz
- **Smooth**: 0.5-0.7 (moderate)
- **Bipolar**: Yes
- **Map to pan control**: -1 = left, +1 = right

### 4. Reverb/Delay Time Modulation
- **Rate**: 0.1-0.5 Hz
- **Smooth**: 0.8+ (avoid audible pitch shifts)
- **Unipolar**: Yes (positive delay offset)
- **Depth**: 0.3-0.6 (moderate variation)

---

## Comparison with Other LFO Types

| Feature | Random LFO | Sine LFO | Sample & Hold |
|---------|------------|----------|---------------|
| **Repeatability** | Never repeats | Perfectly periodic | Periodic (at rate) |
| **Smoothness** | Adjustable | Always smooth | Hard steps |
| **Predictability** | Unpredictable | Fully predictable | Unpredictable values, predictable timing |
| **Harmonics** | Band-limited noise | Single fundamental | Aliased (all harmonics) |
| **CPU Cost** | Medium | Low | Low |
| **Use Case** | Organic variation | Rhythmic/musical | Stepped sequences |

---

## Troubleshooting

### Output Seems Static
- **Check Rate parameter**: Very low rates (<0.05 Hz) change extremely slowly
- **Check Smooth parameter**: At smooth=1.0, transitions take many seconds
- **Check Depth**: At depth=0, output is zero

### Output Has Steps/Clicks
- **Increase Smooth parameter**: Values <0.3 may have audible discontinuities at high rates
- **Check sample rate**: Ensure `setSampleRate()` was called with correct value

### Instances Sound Identical
- **Verify seeding**: Module should call `lfo.seed()` with unique value (typically module pointer)
- **Check initialization**: Each instance needs separate `RandomLFO` object

### Rate CV Not Working
- **Check voltage range**: CV should be 0-10V; negative values map to low frequencies
- **Verify connection**: Use `inputs[RATE_CV_INPUT].isConnected()` before reading
- **Check scaling**: 1V/oct means 1V doubles the rate

---

## Future Enhancements

Possible extensions (not currently implemented):
- **Multiple outputs**: Quadrature (90° phase) or multiple uncorrelated channels
- **Quantization**: Snap to scales/intervals
- **Triggered mode**: Reset to new random value on gate/trigger
- **Slew limiting**: Additional control over maximum rate of change
- **Tempo sync**: BPM-based rate control

---

## References

- **Algorithm source**: [musicdsp.org - Smooth Random LFO](https://www.musicdsp.org/en/latest/Synthesis/269-smooth-random-lfo-generator.html)
- **Rack SDK**: [VCV Rack Plugin Development](https://vcvrack.com/manual/PluginDevelopment.html)
- **Related modules**: Drift (analog-style slow drift), Waveshaper (harmonic processing)

---

## License

Part of the ShortwavFree plugin suite  
Copyright © 2024 Shortwav Labs  
Licensed under GPL-3.0-or-later
