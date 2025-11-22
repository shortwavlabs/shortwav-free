# Formant Oscillator - Implementation Documentation

## Overview

The **Formant Oscillator** module is an AM (Amplitude Modulation) formant synthesis oscillator designed for the ShortwavFree plugin suite. It generates vowel-like timbres and vocal formants without using filters, based on double-carrier amplitude modulation techniques. Perfect for creating organic, voice-like textures, FM-style bells, and complex harmonic timbres.

## Algorithm Specification

### Source
Based on the algorithm from [musicdsp.org](https://www.musicdsp.org/en/latest/Synthesis/224-am-formantic-synthesis.html)  
**Original Design**: Thierry Rochebois via Paul Sernine  
**Technique**: Double carrier amplitude modulation for filterless formant synthesis

### Technical Architecture

The Formant Oscillator uses **pre-calculated formant waveforms** modulated at audio rate:

1. **Formant Table Generation**: Pre-computed waveforms with varying bandwidth (width parameter)
   - Sum of harmonics with Hann windowing
   - Gaussian envelope for frequency rolloff
   - 64 different formant shapes (from narrow/peaked to broad/wide)

2. **Double Carrier Modulation**: Pitch-shifts formant waveforms while preserving harmonic structure
   - Cosine-phased carriers to avoid phase interference
   - Carrier frequency = base pitch
   - Formant frequency = spectral resonance center

3. **DC Blocking**: High-pass filter removes DC offset from AM process

4. **Output Scaling**: Gain control for amplitude management

### Algorithm Characteristics

- **Synthesis Method**: Additive with AM modulation (non-filter-based)
- **Formant Generation**: Pre-calculated wavetables (256 samples × 64 width variations)
- **Carrier Oscillators**: Phase-accurate cosine oscillators for AM
- **DC Blocker**: First-order high-pass (~20 Hz corner)
- **Output Range**: Typically ±1.0 (before gain scaling)

---

## DSP Implementation (`src/dsp/formant-osc.h`)

### Class: `FormantOscillator`

#### Configuration Methods

```cpp
void setSampleRate(float sampleRate);       // Set processing sample rate (required)
void setCarrierFreq(float freqHz);          // Base pitch/fundamental (Hz)
void setFormantFreq(float freqHz);          // Formant center frequency (Hz)
void setFormantWidth(float width);          // Formant bandwidth (0..1, 0=narrow, 1=wide)
void setOutputGain(float gain);             // Overall amplitude scaling (0..1+)
void reset();                                // Reset phase accumulators and DC blocker
```

#### Processing Methods

```cpp
float processSample();  // Generate next audio sample
void processBuffer(float *output, std::size_t numSamples);  // Block processing
```

#### Query Methods

```cpp
float getSampleRate() const;
float getCarrierFreq() const;
float getFormantFreq() const;
float getFormantWidth() const;
float getOutputGain() const;
```

### Real-Time Safety Features

- **Zero allocations** in audio path (table pre-computed at construction)
- **Lock-free** design (no mutexes)
- **Numerically stable** phase accumulators
- **DC-safe** with built-in blocker
- **Denormal protected** via small constant additions

### Parameter Ranges

| Parameter | Range | Typical | Description |
|-----------|-------|---------|-------------|
| **Carrier Freq** | 20-4000 Hz | 100-500 Hz | Base pitch/fundamental |
| **Formant Freq** | 200-4000 Hz | 500-2000 Hz | Resonance center |
| **Formant Width** | 0-1 | 0.2-0.5 | Bandwidth (0=narrow/peaked, 1=wide) |
| **Output Gain** | 0-1+ | 0.3-0.7 | Amplitude scaling |

### Formant Table Structure

```cpp
static constexpr int kTableSize = 256 + 1;   // Wavetable size (257 for wrap)
static constexpr int kMaxWidthIndex = 64;    // 64 different formant bandwidths

float formantTable_[kMaxWidthIndex][kTableSize];  // Pre-computed at construction
```

### Frequency Response Characteristics

| Width | Q (Quality) | Harmonic Spread | Character |
|-------|-------------|-----------------|-----------|
| **0.0-0.2** | High (~10-20) | Narrow, focused | Vocal "ee", sharp resonance |
| **0.3-0.5** | Medium (~5-10) | Moderate | Vocal "ah", balanced |
| **0.6-0.8** | Low (~2-5) | Broad | Vocal "oo", mellow |
| **0.9-1.0** | Very low (~1-2) | Wide, diffuse | Breathy, noisy |

---

## Plugin Module (`src/FormantOsc.hpp`, `src/FormantOsc.cpp`)

### Parameters

| Parameter | Range | Default | Unit | Description |
|-----------|-------|---------|------|-------------|
| `CARRIER_FREQ_PARAM` | 65.4-2093 Hz | 220 Hz | Hz | Base pitch (C2-C7) |
| `FORMANT_FREQ_PARAM` | 200-4000 Hz | 800 Hz | Hz | Formant resonance center |
| `FORMANT_WIDTH_PARAM` | 0-1 | 0.3 | - | Bandwidth/Q control |
| `OUTPUT_GAIN_PARAM` | 0-1 | 0.5 | - | Output amplitude |

### CV Inputs

| Input | Function | Range | Behavior |
|-------|----------|-------|----------|
| `CARRIER_FREQ_CV_INPUT` | Pitch modulation | 0-10V | 1V/oct |
| `FORMANT_FREQ_CV_INPUT` | Formant modulation | 0-10V | Linear Hz modulation |
| `FORMANT_WIDTH_CV_INPUT` | Bandwidth modulation | 0-10V | Linear width modulation |

**CV Modulation Details**:
- **Carrier Freq CV**: Applied as 1V/octave: `finalFreq = baseFreq * 2^(cv/1.0)`
- **Formant Freq CV**: Additive linear modulation in Hz
- **Formant Width CV**: Additive modulation, clamped to [0, 1]

### Outputs

| Output | Range | Description |
|--------|-------|-------------|
| `AUDIO_OUTPUT` | ±5V typical | Formant-synthesized audio |

**Voltage Scaling**:
- Internal DSP output: ±1.0
- Module output: Scaled by `OUTPUT_GAIN_PARAM` to ±5V range

---

## Usage Examples

### Classic Vowel "EE" Sound

```
Settings:
- Carrier Freq: 220 Hz (A3)
- Formant Freq: 350 Hz (first formant of "ee")
- Formant Width: 0.25 (narrow, peaked)
- Output Gain: 0.6

Result: Sharp, focused "ee" vowel tone
Use cases: Synthetic vocals, lead tones
```

### Warm "AH" Vowel

```
Settings:
- Carrier Freq: 150 Hz (close to D3)
- Formant Freq: 750 Hz ("ah" formant)
- Formant Width: 0.4 (moderate)
- Output Gain: 0.5

Result: Open, natural "ah" sound
Use cases: Choir pads, warm leads
```

### Deep "OO" Vowel

```
Settings:
- Carrier Freq: 100 Hz (close to G2)
- Formant Freq: 400 Hz (low formant)
- Formant Width: 0.6 (broad)
- Output Gain: 0.7

Result: Dark, rounded "oo" tone
Use cases: Bass tones, sub-vocal textures
```

### FM Bell-Like Timbre

```
Settings:
- Carrier Freq: 440 Hz (A4)
- Formant Freq: 2200 Hz (high formant, inharmonic)
- Formant Width: 0.2 (sharp)
- Output Gain: 0.4

Result: Metallic, bell-like tone with formant peaks
Use cases: Percussion, FM-style synthesis, sound design
```

### Animated Vocal Formant Sweep

```
Envelope → Formant Freq CV Input
Carrier: 200 Hz (static)
Formant: 500 Hz base (swept by envelope)
Width: 0.3

Result: Vowel morphing (ee → ah → oo)
Use cases: Expressive synth voices, filter-like sweeps
```

### Complex Throat Singing Texture

```
Multiple Formant Osc instances in parallel:
  Osc 1: Carrier=100Hz, Formant=500Hz, Width=0.4
  Osc 2: Carrier=100Hz, Formant=1200Hz, Width=0.3
  Osc 3: Carrier=100Hz, Formant=2400Hz, Width=0.5
Mix outputs

Result: Rich, multi-formant vocal texture
Use cases: Choir sounds, Tuvan throat singing emulation
```

---

## Technical Specifications

### CPU Performance
- **Per-sample cost**: ~40-60 CPU cycles
- **Memory footprint**: ~68KB (formant table) + ~96 bytes (state)
- **Table size**: 256 × 64 × 4 bytes = 65,536 bytes
- **SIMD**: Scalar implementation

### Numerical Characteristics
- **Phase accumulators**: 32-bit float (adequate for audio-rate oscillators)
- **DC blocker pole**: ~0.998 @ 44.1kHz (20 Hz corner)
- **Wavetable interpolation**: Linear (between samples and width indices)
- **Output range**: Typically ±0.8 to ±1.2 before gain scaling

### Formant Table Generation (Initialization)

For each width index `w` (0..63):
1. Compute Q factor from width: `Q = f(w)` (narrow → high Q, wide → low Q)
2. Generate harmonics with Gaussian envelope: `A_n = exp(-n²/Q²)`
3. Apply Hann windowing for smooth spectrum
4. Normalize to ±1.0 range
5. Store in `formantTable_[w][sample]`

---

## Module Widget

### Visual Layout (3HP Panel)

```
┌─────────────┐
│ CARRIER HZ  │  Large knob (base pitch)
│             │
│ FORMANT HZ  │  Large knob (formant center)
│             │
│   WIDTH     │  Large knob (bandwidth)
│             │
│    GAIN     │  Large knob (output level)
│             │
│ CV: C F W   │  Three CV inputs (Carrier, Formant, Width)
│             │
│     OUT     │  Audio output
└─────────────┘
```

### Controls
- **CARRIER HZ**: Base pitch/fundamental frequency
- **FORMANT HZ**: Formant resonance center frequency
- **WIDTH**: Formant bandwidth/Q (0=narrow, 1=wide)
- **GAIN**: Output amplitude scaling

---

## Integration Guide

### Initialization

```cpp
#include "dsp/formant-osc.h"

ShortwavDSP::FormantOscillator osc;

void setup(float sampleRate) {
    osc.setSampleRate(sampleRate);
    osc.setCarrierFreq(220.0f);    // A3
    osc.setFormantFreq(800.0f);    // Mid-range formant
    osc.setFormantWidth(0.3f);     // Moderate bandwidth
    osc.setOutputGain(0.5f);       // Half amplitude
    osc.reset();                    // Clear phase/DC blocker
}
```

### Per-Sample Processing

```cpp
void process() {
    // Get next audio sample
    float sample = osc.processSample();
    
    // Scale to ±5V Rack voltage range
    float outputVolts = sample * params[OUTPUT_GAIN_PARAM].getValue() * 5.0f;
    
    outputs[AUDIO_OUTPUT].setVoltage(outputVolts);
}
```

### Parameter Updates with CV Modulation

```cpp
void updateParameters() {
    // Carrier frequency with V/oct CV
    float carrierBase = params[CARRIER_FREQ_PARAM].getValue();
    if (inputs[CARRIER_FREQ_CV_INPUT].isConnected()) {
        float cv = inputs[CARRIER_FREQ_CV_INPUT].getVoltage();
        carrierBase *= std::pow(2.0f, cv);  // 1V/oct
    }
    osc.setCarrierFreq(clamp(carrierBase, 20.0f, 4000.0f));
    
    // Formant frequency with linear CV
    float formantBase = params[FORMANT_FREQ_PARAM].getValue();
    if (inputs[FORMANT_FREQ_CV_INPUT].isConnected()) {
        float cv = inputs[FORMANT_FREQ_CV_INPUT].getVoltage();
        formantBase += cv * 200.0f;  // 200 Hz per volt
    }
    osc.setFormantFreq(clamp(formantBase, 200.0f, 4000.0f));
    
    // Formant width with linear CV
    float width = params[FORMANT_WIDTH_PARAM].getValue();
    if (inputs[FORMANT_WIDTH_CV_INPUT].isConnected()) {
        float cv = inputs[FORMANT_WIDTH_CV_INPUT].getVoltage();
        width += cv * 0.1f;  // 0.1 per volt
    }
    osc.setFormantWidth(clamp(width, 0.0f, 1.0f));
    
    // Output gain (no CV)
    osc.setOutputGain(params[OUTPUT_GAIN_PARAM].getValue());
}
```

---

## Design Rationale

### Why AM Formant Synthesis?

**Advantages over filter-based formants**:
1. **No filter instability**: No resonance runaway or instability at high Q
2. **Precise harmonic control**: Pre-computed formant shapes with known spectra
3. **Efficient**: Single table lookup + AM (no filter recursion)
4. **Scalable**: Easy to add multiple formants in parallel

**Trade-offs**:
- Fixed formant shapes (not continuously variable like parametric filter)
- Requires wavetable storage (65KB)
- Limited to pre-computed bandwidth variations

### Why Double Carrier Modulation?

Single carrier AM produces sideband pairs (carrier ± formant). Double carrier using:
```
output = formant(t) * cos(carrier_t) + formant(t+phase) * cos(carrier_t+phase)
```
...cancels unwanted sidebands and phase artifacts, producing cleaner formant structure.

### DC Blocker Necessity

AM synthesis generates DC offset from:
- Asymmetric formant waveforms
- Non-zero mean of modulation signal

First-order high-pass filter removes DC without affecting low frequencies:
```
y[n] = x[n] - x[n-1] + 0.998 * y[n-1]
```

### Formant Width Implementation

The 64 width variations allow smooth morphing from:
- **Narrow** (width=0): High Q, sharp resonance (vowel "ee")
- **Wide** (width=1): Low Q, broad spectrum (breathy vowels)

Linear interpolation between adjacent width indices provides smooth parameter response.

---

## Vowel Formant Reference

Approximate formant frequencies for common vowels (male voice):

| Vowel | F1 (Hz) | F2 (Hz) | F3 (Hz) | Carrier | Width |
|-------|---------|---------|---------|---------|-------|
| **EE** (beet) | 270 | 2300 | 3000 | 120-150 | 0.2-0.3 |
| **IH** (bit) | 400 | 2000 | 2550 | 120-150 | 0.3 |
| **EH** (bet) | 530 | 1850 | 2500 | 120-150 | 0.3-0.4 |
| **AE** (bat) | 660 | 1700 | 2400 | 120-150 | 0.4 |
| **AH** (but) | 640 | 1200 | 2400 | 100-130 | 0.4-0.5 |
| **AW** (bought) | 490 | 910 | 2350 | 100-130 | 0.5 |
| **OO** (boot) | 300 | 870 | 2250 | 100-120 | 0.5-0.6 |
| **UH** (book) | 440 | 1020 | 2250 | 100-130 | 0.4-0.5 |

**Usage**: For single-formant approximation, use F1 or F2. For richer timbres, use multiple Formant Osc instances tuned to F1, F2, F3.

---

## Common Use Cases

### 1. Synthetic Vocal Lead
- **Carrier**: 200-400 Hz (melodic range)
- **Formant**: 500-1500 Hz (vowel range)
- **Width**: 0.3-0.4 (balanced)
- **Modulation**: LFO → Formant Freq for vowel morphing

### 2. Choir Pad
- **Multiple instances** (3-5) with slightly detuned carriers
- **Formant**: Various vowel formants (300, 800, 1500 Hz)
- **Width**: 0.5-0.7 (smooth, breathy)
- **Mix**: Equal levels, add reverb

### 3. Throat Singing / Drone
- **Carrier**: Very low (60-120 Hz)
- **Formant**: Multiple instances (500, 1200, 2400 Hz)
- **Width**: 0.2-0.4 (sharp formants)
- **Result**: Overtone singing emulation

### 4. Bell / FM-Like Tones
- **Carrier**: 440+ Hz
- **Formant**: Inharmonic ratios (carrier × 3.14, × 5.67)
- **Width**: 0.1-0.3 (sharp, metallic)
- **Envelope**: Fast decay

### 5. Expressive Synth Voice
- **Carrier**: Keyboard CV (V/oct)
- **Formant**: Expression pedal → Formant Freq CV
- **Width**: LFO → Width CV (slow vibrato-like variation)
- **Result**: Playable, expressive vocal synth

---

## Troubleshooting

### Output Is Weak/Quiet
- **Increase Output Gain**: Default 0.5 may be too low
- **Check formant frequency**: Very low/high formants produce less energy
- **Verify carrier frequency**: Needs to be in audible range

### Harsh/Piercing Sound
- **Increase Formant Width**: Narrow formants are very sharp
- **Lower formant frequency**: High formants are bright
- **Reduce output gain**: Prevent clipping

### No Formant Character (Sounds Like Sine)
- **Check formant vs carrier ratio**: Should be at least 1.5:1
- **Adjust width**: Very wide (>0.8) loses formant definition
- **Verify formant frequency**: Should be above carrier

### DC Offset in Output
- **Not likely**: Module has built-in DC blocker
- **If present**: Check downstream modules, not FormantOsc issue

### Aliasing/Digital Artifacts
- **Lower formant frequency**: Very high formants (>3kHz) can alias
- **Reduce carrier frequency**: High carriers with high formants worse
- **Expected behavior**: No oversampling; keep formants <Nyquist/2

---

## Advanced Techniques

### Multi-Formant Vowel Synthesis
Stack 3-4 Formant Osc instances:
```
All use same Carrier Freq (V/oct tracking)
Formant 1: F1 frequency (low)
Formant 2: F2 frequency (mid)
Formant 3: F3 frequency (high)
Mix with varying levels per formant
```

### Dynamic Vowel Morphing
```
Envelope Follower (from input) → Formant Freq CV
Carrier: Fixed or V/oct tracked
Result: Vowel changes based on input dynamics (auto-wah style)
```

### Formant Filter Effect
```
Audio Input → VCA (carrier freq = audio rate)
Formant Osc output replaces carrier in AM formula
Result: Vocoder-like formant imposition on input
(Requires custom patching or external ring modulation)
```

### Throat Singing Patch
```
Formant Osc 1: Carrier=110Hz, Formant=550Hz, Width=0.3
Formant Osc 2: Carrier=110Hz, Formant=1100Hz, Width=0.25
Formant Osc 3: Carrier=110Hz, Formant=2200Hz, Width=0.2
Sum outputs → Subtle chorus → Reverb
```

---

## Future Enhancements

Possible extensions (not currently implemented):
- **Formant presets**: Saved vowel configurations (A, E, I, O, U)
- **Formant sequencer**: Step through vowel positions
- **Ring mod input**: External carrier for vocoder-like effects
- **Oversampling**: 2x/4x for alias reduction at high frequencies
- **Noise source**: Add breathiness/aspiration
- **Second formant output**: Built-in dual formant per instance

---

## References

- **Algorithm source**: [musicdsp.org - AM Formant Synthesis](https://www.musicdsp.org/en/latest/Synthesis/224-am-formantic-synthesis.html)
- **Formant theory**: [Vowel Formants - Wikipedia](https://en.wikipedia.org/wiki/Formant)
- **AM synthesis**: [Amplitude Modulation Synthesis](https://en.wikipedia.org/wiki/Amplitude_modulation#Modulation_index)
- **Rack SDK**: [VCV Rack Plugin Development](https://vcvrack.com/manual/PluginDevelopment.html)

---

## License

Part of the ShortwavFree plugin suite  
Copyright © 2024 Shortwav Labs  
Licensed under GPL-3.0-or-later
