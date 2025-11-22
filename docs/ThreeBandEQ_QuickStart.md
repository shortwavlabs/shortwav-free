# 3-Band Equalizer - Quick Start

## What It Does

Professional 3-band equalizer with adjustable crossover frequencies and ±12dB gain range per band.

## Key Features

- ✅ **High-quality 24dB/octave filters** (4-pole cascaded design)
- ✅ **Adjustable crossovers**: Low (80-250Hz), High (1-4kHz)
- ✅ **±12dB gain range** per band
- ✅ **Stereo processing** with independent L/R channels
- ✅ **CV modulation** for all parameters
- ✅ **6 built-in presets** (Bass Boost, Vocal Enhance, Bright, Warm, Smiley, Flat)
- ✅ **Real-time safe** (no allocations, no locks)
- ✅ **Denormal protected**

## Usage

1. **Connect audio** to L/R inputs
2. **Adjust crossovers** with top two knobs
3. **Set gains** with bottom three knobs (Low, Mid, High)
4. **Optional**: Add CV sources to modulate parameters
5. **Optional**: Access presets via right-click context menu

## Presets

- **Flat**: 0dB all bands (transparent)
- **Bass Boost**: +6dB low, -3dB high
- **Vocal Enhance**: -3dB low, +6dB mid, +3dB high
- **Bright**: -3dB low, +6dB high
- **Warm**: +4dB low, +2dB mid, -2dB high
- **Smiley**: +6dB low, -6dB mid, +6dB high

## Technical Specs

- **Algorithm**: Based on [musicdsp.org](https://www.musicdsp.org/en/latest/Filters/236-3-band-equaliser.html)
- **Filter Type**: 24dB/oct cascaded IIR
- **CPU**: ~0.03% per instance
- **Latency**: ~3 samples (0.06ms @ 48kHz)
- **Tested**: 1.59M+ automated test assertions

## Full Documentation

See [`docs/ThreeBandEQ.md`](./ThreeBandEQ.md) for complete implementation details, API reference, and integration guide.

## Build & Test

```bash
./build.sh       # Build plugin
./run_tests.sh   # Run unit tests
./install.sh     # Install to Rack
```
