# Waveshaper - Implementation Documentation

## Overview

The **Waveshaper** module is a Chebyshev polynomial-based harmonic waveshaper designed for the ShortwavFree plugin suite. It generates controlled harmonic distortion using mathematical precision, allowing you to sculpt spectra by emphasizing specific harmonics. Perfect for adding warmth, grit, or complex timbres to modular signals.

## Algorithm Specification

### Source
Based on the algorithm from [musicdsp.org](https://www.musicdsp.org/en/latest/Synthesis/187-chebyshev-waveshaper-using-their-recursive-definition.html)  
**Original Design**: Chebyshev polynomials of the first kind for harmonic generation

### Technical Architecture

The Waveshaper uses **Chebyshev polynomials T_n(x)** to generate harmonically pure overtones:

1. **Recursive Computation**: T_{n+1}(x) = 2·x·T_n(x) - T_{n-1}(x)
2. **Spectral Control**: Each T_n produces the n-th harmonic
3. **Weighted Sum**: Output = Σ(a_n · T_n(x)) where a_n are user-controlled weights
4. **Input Limiting**: Soft-clip or hard-clamp to [-1, 1] domain
5. **Gain Staging**: Input and output gain for level control

### Mathematical Background

Chebyshev polynomials satisfy:
```
T_n(cos(θ)) = cos(n·θ)
```

When applied to audio:
- **T_1(x) = x**: Fundamental (linear passthrough)
- **T_2(x) = 2x² - 1**: Pure 2nd harmonic
- **T_3(x) = 4x³ - 3x**: Pure 3rd harmonic
- **T_4(x) = 8x⁴ - 8x² + 1**: Pure 4th harmonic
- ... and so on

### Algorithm Characteristics

- **Polynomial Order**: Configurable 0-16 (0 = bypass)
- **Harmonic Generation**: Spectrally pure (single harmonic per term)
- **Input Domain**: [-1, 1] via soft-clip (tanh-like) or hard-clamp
- **CPU Complexity**: O(N) where N is polynomial order
- **Real-Time Safe**: No allocations, no locks

---

## DSP Implementation (`src/dsp/waveshaper.h`)

### Class: `ChebyshevWaveshaper<MaxOrder>`

#### Configuration Methods

```cpp
void setOrder(std::size_t order);                    // Set max polynomial order (0-MaxOrder)
void setCoefficient(std::size_t n, float coeff);     // Set weight for T_n
void setCoefficients(const float *coeffs, std::size_t count);  // Set multiple
void resetCoefficientsToLinear();                    // T_1 = 1, others = 0 (clean)
void setSoftClip(bool enable);                       // true=tanh-like, false=hard clamp
```

#### Processing Methods

```cpp
float processSample(float input);  // Process single sample
void processBuffer(const float *in, float *out, std::size_t n);  // Block processing
```

### Real-Time Safety Features

- **Zero allocations** (header-only, template-based)
- **Lock-free** design
- **Numerically stable** for moderate orders (≤16)
- **Denormal safe** (no subnormal generation in normal audio range)

### Coefficient Examples

| Configuration | T_1 | T_2 | T_3 | T_4 | Effect |
|---------------|-----|-----|-----|-----|--------|
| **Linear (clean)** | 1.0 | 0.0 | 0.0 | 0.0 | No distortion |
| **Soft saturation** | 0.8 | 0.2 | 0.0 | 0.0 | Warm 2nd harmonic |
| **Tube-like** | 0.7 | 0.2 | 0.1 | 0.0 | 2nd+3rd mix |
| **Hard clipping** | 0.5 | 0.0 | 0.5 | 0.0 | Odd harmonics |
| **Complex spectrum** | 0.5 | 0.3 | 0.2 | 0.1 | All harmonics |

---

## Plugin Module (`src/Waveshaper.hpp`, `src/Waveshaper.cpp`)

### Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| `INPUT_GAIN_PARAM` | 0-2 | 1.0 | Input level scaling |
| `OUTPUT_GAIN_PARAM` | 0-2 | 1.0 | Output level scaling |
| `ORDER_PARAM` | 0-16 | 4 | Max polynomial order (0=bypass) |
| `SOFTCLIP_PARAM` | 0/1 | 1 | Soft-clip (1) or hard-clamp (0) |
| `HARM1_PARAM` | 0-2 | 1.0 | T_1 coefficient (fundamental) |
| `HARM2_PARAM` | 0-2 | 0.0 | T_2 coefficient (2nd harmonic) |
| `HARM3_PARAM` | 0-2 | 0.0 | T_3 coefficient (3rd harmonic) |
| `HARM4_PARAM` | 0-2 | 0.0 | T_4 coefficient (4th harmonic) |

### Inputs

| Input | Function | Range |
|-------|----------|-------|
| `SIGNAL_INPUT` | Audio/CV to process | ±10V typical |

### Outputs

| Output | Function | Range |
|--------|----------|-------|
| `SIGNAL_OUTPUT` | Waveshaped audio/CV | ±10V typical |

### Processing Flow

```
Input Signal
    ↓
× Input Gain
    ↓
Soft-clip/Clamp to [-1, 1]
    ↓
Chebyshev Polynomial (order N, weighted coefficients)
    ↓
× Output Gain
    ↓
Output Signal
```

---

## Usage Examples

### Clean Boost (No Distortion)

```
Settings:
- Input Gain: 1.0
- Output Gain: 1.5
- Order: 1
- Harm1: 1.0, Harm2-4: 0.0
- Soft Clip: ON

Result: Linear amplification with gentle limiting at peaks
```

### Warm Tape Saturation

```
Settings:
- Input Gain: 1.2-1.5
- Output Gain: 0.8
- Order: 2
- Harm1: 0.8, Harm2: 0.3, Harm3-4: 0.0
- Soft Clip: ON

Result: Even-harmonic warmth, subtle compression
Use: Drums, bass, synth pads
```

### Tube-Style Overdrive

```
Settings:
- Input Gain: 1.5-2.0
- Output Gain: 0.7
- Order: 3
- Harm1: 0.7, Harm2: 0.2, Harm3: 0.15, Harm4: 0.0
- Soft Clip: ON

Result: Mixed even/odd harmonics, musical breakup
Use: Guitar-style tones, lead synths
```

### Hard Clipping (Fuzz)

```
Settings:
- Input Gain: 2.0
- Output Gain: 0.5
- Order: 4
- Harm1: 0.5, Harm2: 0.0, Harm3: 0.4, Harm4: 0.0
- Soft Clip: OFF (hard clamp)

Result: Aggressive odd-harmonic distortion
Use: Fuzz bass, aggressive leads
```

### Spectral Shaping (Complex Timbre)

```
Settings:
- Input Gain: 1.0
- Output Gain: 1.0
- Order: 4
- Harm1: 0.6, Harm2: 0.2, Harm3: 0.3, Harm4: 0.1
- Soft Clip: ON

Result: Rich harmonic content, complex spectrum
Use: Sound design, formant-like resonances
```

### CV Waveshaping (Non-Linear Modulation)

```
Input: Triangle LFO (clean)
Settings:
- Order: 3
- Harm1: 0.0, Harm2: 0.0, Harm3: 1.0, Harm4: 0.0

Result: Triangle → sine-like (3rd harmonic only)
Use: Smoothing stepped CV, creating complex LFO shapes
```

---

## Technical Specifications

### CPU Performance
- **Per-sample cost**: ~5-10 cycles (order 1) to ~30-50 cycles (order 16)
- **Memory footprint**: ~128 bytes per instance
- **Complexity**: O(N) where N = order

### Numerical Characteristics
- **Precision**: 32-bit float
- **Input domain**: Clamped/clipped to [-1, 1]
- **Output range**: Depends on coefficients (typically [-1, 1] with proper gain staging)
- **Stability**: Numerically stable for orders ≤16 at audio rates

### Harmonic Content Analysis

For input x = sin(ωt), output contains:
- **T_1**: Fundamental only
- **T_2**: 2nd harmonic (octave)
- **T_3**: 3rd harmonic (octave + fifth)
- **T_4**: 4th harmonic (two octaves)
- **Mixed**: Intermodulation products possible with multiple active terms

---

## Module Widget

### Visual Layout (3HP Panel)

```
┌─────────────┐
│  INPUT GAIN │  Knob
│ OUTPUT GAIN │  Knob
│             │
│    ORDER    │  Knob (0-16)
│  [SOFTCLIP] │  Toggle
│             │
│    HARM 1   │  Knob (T_1 weight)
│    HARM 2   │  Knob (T_2 weight)
│    HARM 3   │  Knob (T_3 weight)
│    HARM 4   │  Knob (T_4 weight)
│             │
│     IN      │  Input jack
│     OUT     │  Output jack
└─────────────┘
```

### Controls
- **INPUT GAIN**: Pre-waveshaping drive
- **OUTPUT GAIN**: Post-waveshaping level
- **ORDER**: Maximum polynomial degree computed
- **SOFTCLIP**: Smooth (ON) vs hard (OFF) input limiting
- **HARM 1-4**: Individual T_1 through T_4 weights

---

## Integration Guide

### Initialization

```cpp
#include "dsp/waveshaper.h"

static constexpr std::size_t MaxOrder = 16u;
ShortwavDSP::ChebyshevWaveshaper<MaxOrder> waveshaper;

void setup() {
    waveshaper.resetCoefficientsToLinear();  // Start clean
    waveshaper.setOrder(4);
    waveshaper.setSoftClip(true);
}
```

### Per-Sample Processing

```cpp
void process() {
    // Read input
    float input = inputs[SIGNAL_INPUT].getVoltage();
    
    // Read parameters
    float inGain = params[INPUT_GAIN_PARAM].getValue();
    float outGain = params[OUTPUT_GAIN_PARAM].getValue();
    
    // Apply input gain
    float scaled = input * inGain / 5.0f;  // Normalize to ±1 typical range
    
    // Waveshape
    float shaped = waveshaper.processSample(scaled);
    
    // Apply output gain and scale back to voltage
    float output = shaped * outGain * 5.0f;
    
    outputs[SIGNAL_OUTPUT].setVoltage(output);
}
```

### Parameter Updates (Control Rate)

```cpp
void updateParameters() {
    // Order
    int order = static_cast<int>(params[ORDER_PARAM].getValue());
    waveshaper.setOrder(clamp(order, 0, 16));
    
    // Soft clip mode
    bool softClip = params[SOFTCLIP_PARAM].getValue() > 0.5f;
    waveshaper.setSoftClip(softClip);
    
    // Coefficients (only update if changed)
    float h1 = params[HARM1_PARAM].getValue();
    float h2 = params[HARM2_PARAM].getValue();
    float h3 = params[HARM3_PARAM].getValue();
    float h4 = params[HARM4_PARAM].getValue();
    
    waveshaper.setCoefficient(1, h1);
    waveshaper.setCoefficient(2, h2);
    waveshaper.setCoefficient(3, h3);
    waveshaper.setCoefficient(4, h4);
}
```

---

## Design Rationale

### Why Chebyshev Polynomials?

1. **Spectral Purity**: T_n produces only the n-th harmonic (no intermodulation)
2. **Predictability**: Known harmonic structure for each term
3. **Efficiency**: Recursive computation is cache-friendly and fast
4. **Musical**: Clean odd harmonics (tube-like) or even harmonics (tape-like)

### Why Recursive Computation?

Alternative: Direct polynomial evaluation (a₄x⁴ + a₃x³ + ...)
- **Recursive is faster**: Fewer multiplications per sample
- **Better numerics**: Avoids very large/small intermediate values
- **Cache-friendly**: Linear memory access pattern

### Soft Clip vs Hard Clamp

**Soft Clip (tanh-like)**:
- Smooth, musical limiting
- No hard discontinuities
- Preferred for audio signals

**Hard Clamp**:
- Sharp cutoff at ±1
- More aggressive harmonics
- Useful for extreme fuzz or digital-style distortion

### Order Parameter

- **Order = 0**: Bypass (output = 0)
- **Low orders (1-4)**: Musical distortion, low CPU
- **High orders (8-16)**: Complex spectra, experimental timbres, higher CPU

---

## Common Use Cases

### 1. Warm Mix Bus Saturation
- **Order**: 2-3
- **Harm1**: 0.9, **Harm2**: 0.1-0.2
- **Input Gain**: 1.1-1.3 (gentle drive)
- **Soft Clip**: ON

### 2. Bass Enhancer
- **Order**: 2
- **Harm2**: 0.5-1.0 (emphasize octave)
- **Input Gain**: 1.5
- **Soft Clip**: ON

### 3. Vocal Exciter
- **Order**: 3-4
- **Harm2**: 0.2, **Harm3**: 0.3, **Harm4**: 0.1
- **High pass input first** (remove rumble)
- **Soft Clip**: ON

### 4. Fuzz/Distortion Pedal
- **Order**: 4
- **Harm1**: 0.5, **Harm3**: 0.5 (odd harmonics)
- **Input Gain**: 1.8-2.0 (heavy drive)
- **Soft Clip**: OFF (hard)

### 5. LFO Shaping (CV Modulation)
- **Order**: Variable
- **Experiment**: Different harmonic mixes to create non-standard LFO shapes
- **Example**: Pure T_3 turns triangle into pseudo-sine

---

## Troubleshooting

### Output Too Quiet
- **Increase Output Gain**: Especially if using fractional coefficients
- **Check Input Level**: Very low input produces minimal harmonics
- **Verify Order ≠ 0**: Order 0 is bypass

### Harsh/Aliased Sound
- **Lower Input Gain**: Excessive drive causes fold-over (non-band-limited)
- **Enable Soft Clip**: Reduces sharp edges
- **Reduce High Harmonic Coefficients**: Lower Harm3/Harm4

### No Distortion Effect
- **Increase Input Gain**: Need sufficient drive to engage non-linearity
- **Check Coefficients**: Harm1=1, others=0 is linear (no distortion)
- **Verify Order ≥ 2**: Order 1 is just gain

### DC Offset in Output
- **Expected for even harmonics**: T_2, T_4 have DC components
- **Use AC coupling**: High-pass filter output if needed
- **Balance coefficients**: Adjust weights to minimize offset

---

## Advanced Techniques

### Dynamic Waveshaping
Modulate input gain with envelope follower:
```
Audio → Envelope Follower → VCA → Waveshaper Input Gain
Result: Dynamics-dependent distortion (gentle on quiet, aggressive on loud)
```

### Parallel Processing
Split signal, waveshape one path, blend with dry:
```
Audio → Split → [Wet: Waveshaper] → Mix
               → [Dry: Passthrough] →
Result: NY-style parallel distortion (retains punch)
```

### Frequency-Selective Distortion
```
Audio → Filter (isolate band) → Waveshaper → Mix back
Result: Distort only highs or lows (multiband distortion)
```

### Stereo Width Enhancement
```
L/R → Mid-Side Encoder → Waveshape Mid only → M-S Decoder
Result: Distorted center, clean sides (or vice versa)
```

---

## Future Enhancements

Possible extensions (not currently implemented):
- **CV modulation**: Order CV, coefficient CV inputs
- **Presets**: Saved coefficient banks (Tube, Tape, Fuzz, etc.)
- **Mix control**: Dry/wet blend knob
- **DC blocker**: Optional high-pass output filter
- **Oversampling**: 2x/4x for aliasing reduction at high drive levels

---

## References

- **Algorithm source**: [musicdsp.org - Chebyshev Waveshaper](https://www.musicdsp.org/en/latest/Synthesis/187-chebyshev-waveshaper-using-their-recursive-definition.html)
- **Mathematics**: [Chebyshev Polynomials - Wikipedia](https://en.wikipedia.org/wiki/Chebyshev_polynomials)
- **Rack SDK**: [VCV Rack Plugin Development](https://vcvrack.com/manual/PluginDevelopment.html)

---

## License

Part of the ShortwavFree plugin suite  
Copyright © 2024 Shortwav Labs  
Licensed under GPL-3.0-or-later
