# Drift - Implementation Documentation

## Overview

The **Drift** module is an analog-style drift modulator designed for the ShortwavFree plugin suite. It generates extremely slow, smooth pseudo-random modulation suitable for emulating analog instability, pitch drift, and other long-form parameter wandering. Unlike traditional LFOs, Drift produces imperceptibly slow evolution ideal for adding organic "life" to static patches.

## Algorithm Specification

### Source
Based on the algorithm from [musicdsp.org](https://www.musicdsp.org/en/latest/Synthesis/183-drift-generator.html)  
**Original Design**: Drift Generator for analog emulation

### Technical Architecture

The Drift module uses a **critically-damped two-pole lowpass filter** driven by very low-rate random excitation:

1. **Random Walk**: Deterministic RNG generates white-ish excitation
2. **Leaky Integrators**: Two cascaded one-pole smoothers (2-pole lowpass)
3. **Extremely Low Corner**: Filter pole placement creates very slow movement
4. **Additive Modulation**: Output is added to input signal: `Out = In + Drift`

### Algorithm Characteristics

- **Filter Type**: Two-pole critically-damped lowpass (cascaded 1-pole)
- **Excitation**: Deterministic LCG-based white noise
- **Rate Range**: 0.001 Hz to 2 Hz (1000 seconds to 0.5 seconds per cycle)
- **Output**: Continuous, smooth drift with no discontinuities
- **Denormal Protection**: Tiny noise injection and clamping prevents denormals

---

## DSP Implementation (`src/dsp/drift.h`)

### Class: `DriftGenerator`

#### Configuration Methods

```cpp
void setSampleRate(float sampleRate);  // Set processing sample rate (required)
void setDepth(float depth);            // Overall output scale (any value, typically 0-1)
void setRateHz(float rateHz);          // Drift rate/corner frequency (0.001-2 Hz)
void seed(uint32_t seedValue);         // Seed internal RNG for determinism
```

#### Processing Methods

```cpp
float next();                  // Generate next drift sample
void reset(float initial = 0.f);  // Reset internal state
```

### Real-Time Safety Features

- **Zero allocations** in audio path
- **Lock-free** design (no mutexes)
- **Deterministic** seeding for consistent behavior
- **Denormal protected** with noise injection and clamping
- **Numerically stable** for typical audio sample rates

### Parameter Ranges

| Parameter | Range | Typical | Description |
|-----------|-------|---------|-------------|
| **RateHz** | 0.001-2 Hz | 0.1-0.25 Hz | Drift corner frequency |
| **Depth** | Any (0-1 typical) | 0.5 | Output scaling |
| **Sample Rate** | 24kHz-192kHz | 44.1-48kHz | Processing rate |

### Frequency Response

The drift generator acts as an extremely low-frequency noise source:

| Rate (Hz) | Approximate Period | Use Case |
|-----------|-------------------|----------|
| **0.001** | ~1000 seconds | Ultra-long evolution |
| **0.01** | ~100 seconds | Imperceptible drift |
| **0.1** | ~10 seconds | Slow analog wander |
| **0.5** | ~2 seconds | Noticeable variation |
| **2.0** | ~0.5 seconds | Fast drift (near LFO territory) |

---

## Plugin Module (`src/Drift.hpp`, `src/Drift.cpp`)

### Parameters

| Parameter | Range | Default | Unit | Description |
|-----------|-------|---------|------|-------------|
| `DEPTH_PARAM` | 0-10 | 5.0 | - | Drift intensity (0-10 scale) |
| `RATE_PARAM` | 0.001-2 Hz | 0.25 Hz | Hz | Drift rate/speed |

### Inputs

| Input | Function | Description |
|-------|----------|-------------|
| `IN_INPUT` | Audio/CV input | Signal to modulate with drift |

### Outputs

| Output | Function | Description |
|--------|----------|-------------|
| `OUT_OUTPUT` | Modulated signal | `Input + Drift` |

### Internal Constants

```cpp
static constexpr float kMaxDriftVolts = 0.5f;
```

Maximum drift amplitude at `DEPTH_PARAM = 10` is ±0.5V, providing clearly audible variation without overwhelming the signal.

**Depth Scaling**:
- `DEPTH_PARAM = 0`: No drift (bypass-like)
- `DEPTH_PARAM = 5`: ±0.25V drift
- `DEPTH_PARAM = 10`: ±0.5V drift

---

## Usage Examples

### VCO Pitch Drift (Classic Analog Emulation)

```
VCO V/Oct → Drift In
Drift Out → VCO V/Oct

Drift Settings:
- Depth: 2-3 (±0.1-0.15V ≈ ±1-1.5 semitones)
- Rate: 0.05 Hz (very slow, ~20 second period)

Result: Subtle vintage-style pitch wander
```

### Filter Resonance Animation

```
Static CV → Drift In
Drift Out → Filter Resonance

Drift Settings:
- Depth: 5 (±0.25V)
- Rate: 0.1 Hz (slow evolution)

Result: Organic resonance movement without rhythmic patterns
```

### Reverb Time Modulation

```
Reverb Time CV → Drift In
Drift Out → Reverb Time

Drift Settings:
- Depth: 3-4 (subtle variation)
- Rate: 0.025 Hz (ultra-slow, ~40 second period)

Result: Imperceptible reverb space evolution for ambient textures
```

### Cascaded Drift for Complex Evolution

```
Drift 1 → Drift 2 In
Drift 2 Out → Target parameter

Drift 1: Depth=8, Rate=0.01 Hz (ultra-slow, ±0.4V)
Drift 2: Depth=5, Rate=0.1 Hz (slow, ±0.25V)

Result: Multi-timescale drift with complex, non-repeating evolution
```

### Stereo Width Variation

```
Stereo Width Control → Drift In
Drift Out → Stereo Width

Drift Settings:
- Depth: 3 (subtle)
- Rate: 0.05 Hz

Result: Gentle stereo field movement for spatial interest
```

---

## Technical Specifications

### CPU Performance
- **Per-sample cost**: ~12-15 CPU cycles
- **Memory footprint**: ~48 bytes per instance
- **SIMD**: Scalar implementation

### Numerical Characteristics
- **State variables**: Two float state variables (z1, z2) for two-pole filter
- **RNG**: 32-bit LCG with 2^32 cycle length
- **Denormal prevention**: Noise injection at ~1e-15 level + clamping

### Filter Design
The two-pole lowpass filter is designed as:
```
z1[n] = z1[n-1] * a + noise[n] * (1 - a)
z2[n] = z2[n-1] * a + z1[n] * (1 - a)
output[n] = z2[n] * depth
```

Where `a` is the pole coefficient derived from `rateHz` and sample rate:
```
a = exp(-2π * rateHz / sampleRate)
```

---

## Module Widget

### Visual Layout (3HP Panel)

```
┌─────────────┐
│   DEPTH     │  Large knob (top)
│             │
│   RATE      │  Large knob (middle)
│             │
│             │
│     IN      │  Input jack (CV/audio)
│             │
│     OUT     │  Output jack (IN + drift)
└─────────────┘
```

### Controls
- **DEPTH knob**: Controls drift intensity (0-10 scale)
- **RATE knob**: Sets drift speed in Hz (0.001-2 Hz)

---

## Integration Guide

### Initialization

```cpp
#include "dsp/drift.h"

ShortwavDSP::DriftGenerator drift;

void setup(float sampleRate) {
    drift.setSampleRate(sampleRate);
    drift.seed(reinterpret_cast<uint32_t>(this));  // Unique per instance
    drift.setRateHz(0.1f);    // Slow drift
    drift.setDepth(1.0f);     // Unit depth
    drift.reset(0.0f);        // Start from zero
}
```

### Per-Sample Processing

```cpp
void process() {
    // Read input signal
    float input = inputs[IN_INPUT].getVoltage();
    
    // Get current drift value
    float driftValue = drift.next();
    
    // Scale drift to voltage range
    float driftVolts = driftValue * kMaxDriftVolts * depthParam;
    
    // Additive modulation
    float output = input + driftVolts;
    
    outputs[OUT_OUTPUT].setVoltage(output);
}
```

### Parameter Updates (Control Rate)

```cpp
void updateParameters() {
    // Read UI parameters
    float depth = params[DEPTH_PARAM].getValue();
    float rate = params[RATE_PARAM].getValue();
    
    // Update drift generator
    drift.setDepth(depth / 10.0f);  // Normalize 0-10 to 0-1
    drift.setRateHz(clamp(rate, 0.001f, 2.0f));
}
```

---

## Design Rationale

### Why Additive vs Multiplicative Modulation?

**Additive** (`Out = In + Drift`) was chosen because:
1. **Predictable range**: Drift amount is independent of input level
2. **DC-safe**: Works correctly with DC or near-DC signals
3. **Musically useful**: Can offset static control voltages
4. **Simple**: No division-by-zero concerns

### Why Two-Pole Instead of Higher Order?

- **Adequate smoothness**: Two poles provide ~12dB/octave rolloff (sufficient for drift rates)
- **Efficiency**: Minimal CPU overhead per sample
- **Stability**: Easier to design stable coefficients across wide sample rate range
- **Simplicity**: Less state to manage, easier to understand

### Seeding Strategy

```cpp
drift.seed(reinterpret_cast<uint64_t>(this) & 0xFFFFFFFFu);
```

Each module instance uses its memory address as seed:
- **Unique sequences** per instance
- **Deterministic** across session reloads
- **No coordination** required between instances

### Depth Scaling Choice

The `kMaxDriftVolts = 0.5f` constant provides:
- **Audible but not overwhelming** pitch drift (±6 semitones at max)
- **Useful CV modulation range** for typical 0-10V controls
- **Headroom** when cascading multiple drift instances

---

## Comparison with Related Modules

| Feature | Drift | Random LFO | Sine LFO |
|---------|-------|------------|----------|
| **Rate Range** | 0.001-2 Hz | 0.01-20 Hz | 0.01-100+ Hz |
| **Primary Use** | Analog emulation | Organic modulation | Rhythmic/musical |
| **Typical Period** | 10-1000 seconds | 0.05-10 seconds | 0.01-1 seconds |
| **Smoothness** | Always very smooth | Adjustable | Always smooth |
| **Repeatability** | Never | Never | Perfect cycles |
| **CPU Cost** | Low | Medium | Low |
| **Audibility** | Imperceptible to subtle | Obvious | Very obvious |

---

## Common Use Cases

### 1. VCO Pitch Drift (Vintage Synth Emulation)
- **Depth**: 2-4 (0.1-0.2V ≈ 1-2 semitones)
- **Rate**: 0.03-0.1 Hz
- **Result**: Classic analog wander

### 2. LFO Rate Modulation
- **Depth**: 5-7
- **Rate**: 0.05 Hz
- **Result**: Non-repeating LFO speed variation

### 3. Delay Time Shimmer
- **Depth**: 3-5
- **Rate**: 0.02-0.05 Hz
- **Result**: Subtle tape-style wow/flutter

### 4. Panning Drift
- **Depth**: 4-6
- **Rate**: 0.1-0.2 Hz
- **Result**: Gentle stereo field movement

### 5. Generative Patch Evolution
- **Multiple Drift instances** at different rates
- **Modulate various parameters**: Filter, oscillator tuning, effects
- **Result**: Ever-evolving soundscape without human intervention

---

## Troubleshooting

### Output Seems Static
- **Check Rate**: Very low rates (<0.01 Hz) take 100+ seconds to show movement
- **Check Depth**: At depth=0, output equals input (no drift)
- **Monitor over time**: Drift is intentionally slow; watch for 30+ seconds

### Too Much Variation
- **Reduce Depth**: Lower from default (5) to 1-3 for subtlety
- **Lower Rate**: Slower rates produce less dramatic changes

### Audible Clicks or Discontinuities
- **Verify sample rate**: Ensure `setSampleRate()` called with correct value
- **Check input signal**: Drift is smooth; clicks likely from input source

### Instances Sound Too Similar
- **Not a bug**: Drift instances are seeded uniquely per instance
- **Confirm separate instances**: Each Drift module has its own `DriftGenerator`
- **Wait longer**: Very slow rates may appear similar over short timescales

### Want Faster Modulation
- **Use Random LFO instead**: Drift is specifically designed for slow movement
- **Increase Rate to max (2 Hz)**: Approaches slow LFO territory

---

## Advanced Techniques

### Multi-Timescale Drift
Cascade multiple Drift modules with different rates:
```
Drift A (Rate=0.01 Hz, Depth=8) → Drift B In
Drift B (Rate=0.1 Hz, Depth=5) → Target

Result: Slow baseline wander + faster variation
```

### Cross-Modulated Drift
Use one drift to modulate another's rate:
```
Drift A Out → VCA/Attenuator → Drift B Rate CV (if implemented)

Result: Variable-speed drift (requires custom CV modulation)
```

### Stereo Drift Decorrelation
Two independent Drift instances for L/R channels:
```
Drift L (Seed=12345) → Left channel parameter
Drift R (Seed=67890) → Right channel parameter

Result: Wide, decorrelated stereo field evolution
```

---

## Future Enhancements

Possible extensions (not currently implemented):
- **CV modulation inputs**: Depth CV, Rate CV
- **Multiple outputs**: Raw drift output (without input passthrough)
- **Quantization**: Snap drift to scales/intervals
- **Symmetry control**: Bias toward positive or negative drift
- **Reset/Trigger input**: Jump to new random state on gate

---

## References

- **Algorithm source**: [musicdsp.org - Drift Generator](https://www.musicdsp.org/en/latest/Synthesis/183-drift-generator.html)
- **Rack SDK**: [VCV Rack Plugin Development](https://vcvrack.com/manual/PluginDevelopment.html)
- **Related modules**: Random LFO (faster random modulation), Waveshaper (harmonic processing)

---

## License

Part of the ShortwavFree plugin suite  
Copyright © 2024 Shortwav Labs  
Licensed under GPL-3.0-or-later
