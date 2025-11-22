# Random LFO - Quick Start

## What It Does

Smooth random modulation source for evolving, organic parameter movement. Generates continuously varying control voltages without hard discontinuities.

## Key Features

- ✅ **Smooth random generation** using second-order tracking system
- ✅ **Adjustable rate** (0.01-20 Hz) for slow drift to fast modulation
- ✅ **Variable smoothness** (0-1) controls transition sharpness
- ✅ **Bipolar/Unipolar modes** (±5V or 0-10V output)
- ✅ **CV modulation** for all parameters (Rate, Depth, Smooth)
- ✅ **Deterministic seeding** (different per instance, same per patch)
- ✅ **Real-time safe** (no allocations, no locks)
- ✅ **Zero latency** output

## Usage

1. **Connect LFO output** to any CV input
2. **Set Rate** (how often values change)
3. **Adjust Depth** (output intensity)
4. **Tune Smooth** (0=sharp changes, 1=very smooth)
5. **Choose mode** (Bipolar/Unipolar switch)

## Common Settings

### Slow Organic Drift
- **Rate**: 0.1-0.5 Hz
- **Smooth**: 0.8-0.95
- **Use for**: Filter sweeps, subtle pitch variation, long-form ambient

### Moderate Modulation
- **Rate**: 1-4 Hz
- **Smooth**: 0.5-0.7
- **Use for**: Tremolo alternatives, pan modulation, effect parameter variation

### Fast Sample & Hold-ish
- **Rate**: 8-16 Hz
- **Smooth**: 0.0-0.3
- **Use for**: Rhythmic sequences, stepped modulation with slight smoothing

### Ultra-Slow Evolution
- **Rate**: 0.01-0.05 Hz (one change every 20-100 seconds!)
- **Smooth**: 0.9+
- **Use for**: Generative patches, imperceptible evolution, analog-style drift

## Parameter Reference

| Control | Range | Default | Description |
|---------|-------|---------|-------------|
| **Rate** | 0.01-20 Hz | 1 Hz | Target generation frequency |
| **Depth** | 0-1 | 1.0 | Output amplitude scaling |
| **Smooth** | 0-1 | 0.75 | Transition smoothness/correlation |
| **Bipolar** | Toggle | ON | ±5V (on) or 0-10V (off) |

## CV Inputs

- **Rate CV**: 1V/octave multiplicative modulation
- **Depth CV**: Additive modulation (0-10V adds to base depth)
- **Smooth CV**: Additive modulation (0-10V adds to base smooth)

## Technical Specs

- **Algorithm**: Second-order critically-damped tracking system
- **Source**: [musicdsp.org](https://www.musicdsp.org/en/latest/Synthesis/269-smooth-random-lfo-generator.html)
- **CPU**: ~15-20 cycles per sample
- **Latency**: 0 samples
- **Determinism**: Seeded per-instance (same patch = same behavior)

## Quick Patching Ideas

### Evolving Filter Patch
```
Random LFO → Filter Cutoff
Rate: 0.5 Hz, Smooth: 0.8, Bipolar: Yes
Result: Gentle, organic filter sweep
```

### VCO Analog Drift Emulation
```
Random LFO → VCO V/Oct (via attenuator ~0.1x)
Rate: 0.05 Hz, Smooth: 0.95, Bipolar: Yes
Result: Subtle vintage-style pitch wander
```

### Cross-Modulated Chaos
```
Random LFO 1 → Rate CV of Random LFO 2
Random LFO 2 → Effect parameter
LFO1: Slow (0.2 Hz), Smooth (0.9)
LFO2: Fast (5 Hz), Moderate smooth (0.4)
Result: Variable-speed complex modulation
```

### Stereo Pan Animation
```
Random LFO → Stereo Panner CV
Rate: 2 Hz, Smooth: 0.6, Bipolar: Yes
Result: Unpredictable stereo movement
```

## Comparison with Other LFOs

| Type | Random LFO | Sine LFO | Sample & Hold |
|------|------------|----------|---------------|
| Repeats | Never | Always | Values vary, timing repeats |
| Smoothness | Adjustable | Always smooth | Hard steps |
| Musicality | Organic | Rhythmic | Percussive |
| CPU | Medium | Low | Low |

## Troubleshooting

**Output not changing?**
- Check Rate (very low values take time to show movement)
- Increase Depth if too subtle
- Lower Smooth if it's changing too slowly

**Too steppy/clicky?**
- Increase Smooth parameter (try 0.5+)
- Lower Rate if high rates cause audible steps

**Want identical modulation across instances?**
- Not directly supported (each instance is uniquely seeded)
- Alternative: Use mult to split one Random LFO to multiple destinations

## Full Documentation

See [`docs/RandomLfo.md`](./RandomLfo.md) for complete implementation details, DSP specifications, and integration guide.

---

Part of the ShortwavFree plugin suite  
Copyright © 2024 Shortwav Labs
