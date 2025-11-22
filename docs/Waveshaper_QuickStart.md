# Waveshaper - Quick Start

## What It Does

Chebyshev polynomial-based harmonic waveshaper for adding controlled distortion and harmonic content to audio and CV signals. Mathematically precise harmonic generation.

## Key Features

- ✅ **Chebyshev polynomials** for spectrally pure harmonics
- ✅ **Adjustable order** (0-16) controls complexity
- ✅ **Four harmonic controls** (T_1 through T_4)
- ✅ **Soft-clip or hard-clamp** input limiting
- ✅ **Input/output gain** staging
- ✅ **Real-time safe** (no allocations, lock-free)
- ✅ **Low CPU** (O(N) where N = order)

## Usage

1. **Connect audio/CV** to SIGNAL_INPUT
2. **Set Input Gain** (drive/saturation amount)
3. **Choose Order** (how many harmonics to compute)
4. **Adjust Harmonic weights** (Harm1-4 knobs)
5. **Set Output Gain** (final level)
6. **Toggle Soft Clip** (smooth vs hard limiting)

## Quick Presets

### Clean Boost (No Distortion)
- **Order**: 1
- **Harm1**: 1.0, **Harm2-4**: 0.0
- **Soft Clip**: ON
- **Result**: Linear amplification with gentle peak limiting

### Warm Tape Saturation
- **Order**: 2
- **Input Gain**: 1.3
- **Harm1**: 0.8, **Harm2**: 0.3
- **Soft Clip**: ON
- **Result**: Even-harmonic warmth (2nd harmonic adds octave richness)

### Tube Overdrive
- **Order**: 3
- **Input Gain**: 1.6
- **Harm1**: 0.7, **Harm2**: 0.2, **Harm3**: 0.15
- **Soft Clip**: ON
- **Result**: Mixed harmonics for tube-style breakup

### Hard Fuzz
- **Order**: 4
- **Input Gain**: 2.0
- **Harm1**: 0.5, **Harm2**: 0.0, **Harm3**: 0.4
- **Soft Clip**: OFF (hard clamp)
- **Result**: Aggressive odd-harmonic distortion

### Complex Spectrum
- **Order**: 4
- **Harm1**: 0.6, **Harm2**: 0.2, **Harm3**: 0.3, **Harm4**: 0.1
- **Result**: Rich harmonic content for sound design

## Parameter Reference

| Control | Range | Default | Description |
|---------|-------|---------|-------------|
| **Input Gain** | 0-2 | 1.0 | Pre-waveshaping drive |
| **Output Gain** | 0-2 | 1.0 | Post-waveshaping level |
| **Order** | 0-16 | 4 | Max polynomial degree (0=bypass) |
| **Soft Clip** | Toggle | ON | Smooth (ON) vs hard (OFF) limiting |
| **Harm1** | 0-2 | 1.0 | T_1 weight (fundamental) |
| **Harm2** | 0-2 | 0.0 | T_2 weight (2nd harmonic) |
| **Harm3** | 0-2 | 0.0 | T_3 weight (3rd harmonic) |
| **Harm4** | 0-2 | 0.0 | T_4 weight (4th harmonic) |

## Harmonic Guide

| Polynomial | Harmonic | Musical Interval | Character |
|------------|----------|------------------|-----------|
| **T_1** | Fundamental | Unison | Linear/clean |
| **T_2** | 2nd | Octave | Warm, even |
| **T_3** | 3rd | Octave + fifth | Odd, hollow |
| **T_4** | 4th | Two octaves | Even, bright |

## Common Use Cases

### Mix Bus Warmth
```
Stereo mix → Waveshaper
Order: 2, Harm1: 0.9, Harm2: 0.15
Input: 1.1-1.2 (gentle drive)
Result: Subtle analog-style glue
```

### Bass Enhancer
```
Bass → Waveshaper → Output
Order: 2, Harm2: 0.8 (octave emphasis)
Input: 1.5
Result: Perceived bass boost without EQ
```

### Aggressive Lead
```
Lead synth → Waveshaper
Order: 4, Harm1: 0.5, Harm3: 0.5 (odd harmonics)
Input: 1.8, Soft Clip: OFF
Result: Fuzzy, aggressive distortion
```

### LFO Waveshaping (CV)
```
Triangle LFO → Waveshaper
Order: 3, Harm3: 1.0 (only 3rd harmonic)
Result: Triangle → smooth sine-like shape
```

## Technical Specs

- **Algorithm**: Chebyshev polynomials (recursive computation)
- **Source**: [musicdsp.org](https://www.musicdsp.org/en/latest/Synthesis/187-chebyshev-waveshaper-using-their-recursive-definition.html)
- **CPU**: ~5-50 cycles/sample (depends on order)
- **Complexity**: O(N) where N = order
- **Latency**: 0 samples

## Troubleshooting

**No distortion effect?**
- Increase Input Gain (need drive to engage non-linearity)
- Check coefficients (Harm1=1, others=0 is linear)
- Verify Order ≥ 2

**Harsh/aliased sound?**
- Lower Input Gain (excessive drive causes fold-over)
- Enable Soft Clip
- Reduce high harmonic coefficients

**Output too quiet?**
- Increase Output Gain
- Check input level
- Verify Order ≠ 0 (0 is bypass)

**DC offset?**
- Expected with even harmonics (T_2, T_4)
- Use high-pass filter if needed
- Balance coefficient weights

## Tips & Tricks

**Even Harmonics (warm)**:
- Emphasize Harm2, Harm4
- Similar to tape/tube saturation
- Softer, rounder sound

**Odd Harmonics (aggressive)**:
- Emphasize Harm1, Harm3
- Similar to hard clipping/fuzz
- Harsher, edgier sound

**Parallel Processing**:
- Split signal, waveshape one path
- Mix dry + wet for NY-style compression

**Dynamics-Dependent**:
- Use envelope follower → Input Gain CV (if implemented)
- Gentle on quiet, aggressive on loud

## Comparison with Other Distortion

| Type | Waveshaper | Overdrive | Saturation |
|------|------------|-----------|------------|
| Control | Precise harmonic selection | Fixed algorithm | Analog modeling |
| CPU | Medium (order-dependent) | Low | Variable |
| Harmonics | Pure, predictable | Mixed | Analog-like |
| Use Case | Sound design, precision | Musical, guitar-like | Mixing, warmth |

## Full Documentation

See [`docs/Waveshaper.md`](./Waveshaper.md) for complete implementation details, mathematical background, and integration guide.

---

Part of the ShortwavFree plugin suite  
Copyright © 2024 Shortwav Labs
