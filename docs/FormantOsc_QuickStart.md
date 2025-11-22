# Formant Oscillator - Quick Start

## What It Does

AM formant synthesis oscillator that generates vowel-like timbres and vocal formants without using filters. Creates organic voice textures, FM-style bells, and complex harmonic sounds.

## Key Features

- ✅ **Filterless formant synthesis** using AM modulation
- ✅ **Vowel-like timbres** (ee, ah, oo, etc.)
- ✅ **Three main controls**: Carrier (pitch), Formant (resonance), Width (bandwidth)
- ✅ **CV modulation** for all parameters (1V/oct on carrier)
- ✅ **64 formant variations** from narrow/sharp to wide/breathy
- ✅ **Built-in DC blocker** for clean output
- ✅ **Pre-computed wavetables** for efficiency

## Usage

1. **Set Carrier Frequency** (base pitch, 65-2093 Hz)
2. **Set Formant Frequency** (resonance center, 200-4000 Hz)
3. **Adjust Width** (formant bandwidth, 0=narrow, 1=wide)
4. **Set Output Gain** (amplitude, 0-1)
5. **Optional**: Connect CV for expressive control

## Quick Vowel Presets

### "EE" (as in "beet")
- **Carrier**: 220 Hz (A3)
- **Formant**: 350 Hz
- **Width**: 0.25 (narrow, sharp)
- **Gain**: 0.6
- **Result**: Focused, bright "ee" sound

### "AH" (as in "father")
- **Carrier**: 150 Hz (D3)
- **Formant**: 750 Hz
- **Width**: 0.4 (moderate)
- **Gain**: 0.5
- **Result**: Open, natural "ah" vowel

### "OO" (as in "boot")
- **Carrier**: 100 Hz (G2)
- **Formant**: 400 Hz
- **Width**: 0.6 (broad)
- **Gain**: 0.7
- **Result**: Dark, rounded "oo" tone

### Bell / Metallic
- **Carrier**: 440 Hz (A4)
- **Formant**: 2200 Hz (inharmonic ratio)
- **Width**: 0.2 (very sharp)
- **Gain**: 0.4
- **Result**: FM-style bell timbre

## Parameter Reference

| Control | Range | Default | Description |
|---------|-------|---------|-------------|
| **Carrier Freq** | 65.4-2093 Hz | 220 Hz | Base pitch (C2-C7) |
| **Formant Freq** | 200-4000 Hz | 800 Hz | Resonance center |
| **Width** | 0-1 | 0.3 | Bandwidth (0=narrow, 1=wide) |
| **Output Gain** | 0-1 | 0.5 | Amplitude scaling |

## CV Inputs

- **Carrier CV**: 1V/octave pitch tracking (perfect for keyboard CV)
- **Formant CV**: Linear Hz modulation (~200 Hz/V)
- **Width CV**: Linear modulation (~0.1/V)

## Formant Width Guide

| Width | Character | Use Case |
|-------|-----------|----------|
| **0.0-0.2** | Very sharp, peaked | "ee" vowel, metallic tones |
| **0.3-0.4** | Moderate focus | "ah" vowel, balanced |
| **0.5-0.6** | Broad, smooth | "oo" vowel, warm pads |
| **0.7-1.0** | Very wide, breathy | Whispered vocals, texture |

## Common Patching Ideas

### Playable Vocal Synth
```
Keyboard V/Oct → Carrier CV
Expression Pedal → Formant CV
Carrier: Tracks keyboard
Formant: 500-1500 Hz (swept by expression)
Width: 0.3
Result: Expressive vowel morphing synth
```

### Choir Pad
```
3-5 Formant Osc instances:
  All follow same V/Oct
  Slightly detune carriers (±5 Hz)
  Different formants: 300, 800, 1500 Hz
  Width: 0.5-0.6 (breathy)
Mix → Reverb
Result: Rich choir texture
```

### Throat Singing Drone
```
Formant Osc 1: Carrier=110Hz, Formant=550Hz
Formant Osc 2: Carrier=110Hz, Formant=1100Hz
Formant Osc 3: Carrier=110Hz, Formant=2200Hz
Width: 0.2-0.3 (sharp overtones)
Result: Tuvan throat singing emulation
```

### Auto-Wah Effect
```
Envelope Follower (from drums) → Formant CV
Carrier: 200 Hz (static)
Formant: 600 Hz base (swept by envelope)
Width: 0.4
Result: Vowel filter-like effect
```

### FM Bell Tones
```
Carrier: 440 Hz
Formant: 1380 Hz (3.14 × carrier, inharmonic)
Width: 0.15 (very sharp)
Envelope: Fast decay on Gain CV
Result: Metallic bell percussion
```

## Vowel Formant Quick Reference

Approximate formant frequencies (male voice):

| Vowel | Formant Freq | Width | Description |
|-------|--------------|-------|-------------|
| **EE** | 270-350 Hz | 0.2-0.3 | Beet, feet |
| **IH** | 400 Hz | 0.3 | Bit, sit |
| **EH** | 530 Hz | 0.3-0.4 | Bet, set |
| **AH** | 640-750 Hz | 0.4-0.5 | But, father |
| **AW** | 490 Hz | 0.5 | Bought, thought |
| **OO** | 300-400 Hz | 0.5-0.6 | Boot, food |

*For richer vowels, use multiple Formant Osc instances tuned to F1, F2, F3.*

## Technical Specs

- **Algorithm**: Double carrier AM modulation
- **Source**: [musicdsp.org](https://www.musicdsp.org/en/latest/Synthesis/224-am-formantic-synthesis.html)
- **CPU**: ~40-60 cycles/sample
- **Memory**: ~68KB wavetable + ~96B state
- **Latency**: 0 samples
- **DC Blocker**: Built-in (20 Hz corner)

## Troubleshooting

**Sounds like a sine wave?**
- Increase Formant vs Carrier ratio (try Formant = 2-3× Carrier)
- Reduce Width (narrower = more formant character)
- Check formant frequency (should be >200 Hz)

**Too harsh/piercing?**
- Increase Width (smoother, less sharp)
- Lower Formant Freq
- Reduce Output Gain

**Output too quiet?**
- Increase Output Gain (default 0.5 may be low)
- Check carrier frequency (audible range)

**No pitch tracking with keyboard?**
- Verify Carrier CV connected to V/Oct output
- Check voltage range (should be 0-10V)

## Tips & Tricks

**Vowel Morphing**:
- Use LFO or envelope → Formant CV
- Slow LFO (0.1-0.5 Hz) for evolving vowels
- Fast envelope for "wah" effect

**Multi-Formant Vowels**:
- Stack 2-3 instances with same Carrier
- Tune each to different Formant freq (F1, F2, F3)
- Mix for realistic vowel synthesis

**Inharmonic Tones**:
- Set Formant to non-integer ratios of Carrier
- Example: Carrier=100Hz, Formant=314Hz (π ratio)
- Result: Bell/gong-like timbres

**Stereo Width**:
- Two instances, slightly different Formants (L: 800Hz, R: 820Hz)
- Result: Wide, chorused vocal texture

## Comparison with Filter Formants

| Feature | Formant Osc | Filter-Based |
|---------|-------------|--------------|
| Method | AM synthesis | Resonant filter |
| Stability | Always stable | Can self-oscillate |
| CPU | Medium | Variable |
| Harmonic Control | Pre-computed | Real-time |
| Vowel Quality | Fixed shapes | Continuously variable |
| Use Case | Synth voices | Effects, vocals |

## Full Documentation

See [`docs/FormantOsc.md`](./FormantOsc.md) for complete implementation details, vowel formant tables, and advanced techniques.

---

Part of the ShortwavFree plugin suite  
Copyright © 2024 Shortwav Labs
