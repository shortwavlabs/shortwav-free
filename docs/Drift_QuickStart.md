# Drift - Quick Start

## What It Does

Analog-style drift modulator for extremely slow, smooth parameter wandering. Perfect for emulating vintage synthesizer instability and adding organic "life" to static patches.

## Key Features

- ✅ **Ultra-slow modulation** (0.001-2 Hz) for imperceptible to slow drift
- ✅ **Smooth evolution** using two-pole lowpass filtering
- ✅ **Additive modulation** (Out = In + Drift)
- ✅ **Adjustable depth** (0-10 scale, ±0.5V max)
- ✅ **Deterministic seeding** (unique per instance)
- ✅ **Real-time safe** (no allocations, no locks)
- ✅ **Denormal protected** for stable operation

## Usage

1. **Connect signal** to IN input (CV or audio)
2. **Set Depth** (drift intensity, 0-10)
3. **Adjust Rate** (how fast drift evolves, 0.001-2 Hz)
4. **Use OUT** for modulated signal (IN + drift)

## Common Settings

### Classic VCO Pitch Drift
- **Depth**: 2-3 (±0.1-0.15V ≈ ±1-1.5 semitones)
- **Rate**: 0.05 Hz (~20 second period)
- **Use for**: Vintage analog synthesizer emulation

### Ultra-Slow Ambient Evolution
- **Depth**: 5-8
- **Rate**: 0.01 Hz (~100 second period)
- **Use for**: Generative patches, imperceptible parameter drift

### Filter Resonance Animation
- **Depth**: 4-6
- **Rate**: 0.1-0.2 Hz (~5-10 second period)
- **Use for**: Organic filter movement without rhythmic patterns

### Subtle Reverb Shimmer
- **Depth**: 2-4
- **Rate**: 0.025 Hz (~40 second period)
- **Use for**: Reverb time/size modulation for spatial interest

## Parameter Reference

| Control | Range | Default | Description |
|---------|-------|---------|-------------|
| **Depth** | 0-10 | 5.0 | Drift intensity (10 = ±0.5V max) |
| **Rate** | 0.001-2 Hz | 0.25 Hz | Drift speed/frequency |

**Depth to Voltage**:
- Depth = 0: No drift (bypass)
- Depth = 5: ±0.25V
- Depth = 10: ±0.5V (max)

**Rate to Period**:
- 0.001 Hz: ~1000 second period (ultra-slow)
- 0.01 Hz: ~100 second period (imperceptible)
- 0.1 Hz: ~10 second period (slow wander)
- 1.0 Hz: ~1 second period (noticeable)
- 2.0 Hz: ~0.5 second period (approaching LFO)

## Technical Specs

- **Algorithm**: Two-pole critically-damped lowpass filter
- **Source**: [musicdsp.org](https://www.musicdsp.org/en/latest/Synthesis/183-drift-generator.html)
- **CPU**: ~12-15 cycles per sample
- **Latency**: 0 samples
- **Modulation Type**: Additive (Out = In + Drift)

## Quick Patching Ideas

### Vintage VCO Drift
```
VCO V/Oct → Drift In
Drift Out → VCO V/Oct
Depth: 2-3, Rate: 0.05 Hz
Result: Classic analog pitch wander
```

### Cascaded Multi-Timescale Drift
```
Drift A (Rate=0.01, Depth=8) → Drift B In
Drift B (Rate=0.1, Depth=5) → Target
Result: Complex, non-repeating evolution
```

### LFO Rate Modulation
```
Static CV → Drift In
Drift Out → LFO Rate CV
Depth: 6, Rate: 0.05 Hz
Result: Variable-speed LFO without manual tweaking
```

### Stereo Width Animation
```
Drift → Stereo Width Control
Depth: 4, Rate: 0.1 Hz
Result: Gentle stereo field evolution
```

## Comparison with Related Modules

| Type | Drift | Random LFO | Sine LFO |
|------|-------|------------|----------|
| Speed | Ultra-slow to slow | Slow to fast | Slow to very fast |
| Typical Rate | 0.01-0.2 Hz | 1-10 Hz | 0.1-50 Hz |
| Period | 5-100+ seconds | 0.1-1 seconds | 0.02-10 seconds |
| Use Case | Analog drift | Organic modulation | Rhythmic/musical |
| Audibility | Imperceptible to subtle | Obvious | Very obvious |

## Troubleshooting

**Not hearing any change?**
- Wait longer! Drift at 0.1 Hz takes 10 seconds per cycle
- Increase Depth if too subtle
- Check Rate (0.001 Hz takes 1000 seconds!)

**Too much variation?**
- Lower Depth (try 2-4 for subtlety)
- Reduce Rate for slower evolution

**Need faster modulation?**
- Use Random LFO instead (Drift is designed for slow movement)
- Max rate is 2 Hz (near LFO territory)

**Output equals input?**
- Check Depth (at 0, drift is bypassed)
- Verify signal is connected to IN

## When to Use Drift vs. Random LFO

**Use Drift for:**
- ✅ Analog synthesizer emulation (pitch drift)
- ✅ Imperceptible evolution (generative patches)
- ✅ Very slow parameter wandering
- ✅ "Set and forget" organic variation

**Use Random LFO for:**
- ✅ Obvious modulation effects
- ✅ Faster rates (1-20 Hz)
- ✅ Variable smoothness control
- ✅ Bipolar/unipolar mode switching

## Full Documentation

See [`docs/Drift.md`](./Drift.md) for complete implementation details, DSP specifications, and integration guide.

---

Part of the ShortwavFree plugin suite  
Copyright © 2024 Shortwav Labs
