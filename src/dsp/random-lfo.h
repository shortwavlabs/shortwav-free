#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <limits>

/*
 * Smooth Random LFO Generator
 *
 * Based on the algorithm described at:
 * https://www.musicdsp.org/en/latest/Synthesis/269-smooth-random-lfo-generator.html
 *
 * Core idea:
 * - Generate random target values at a given rate.
 * - Drive a critically / slightly under-damped second-order system (biquad-like)
 *   towards each new random target.
 * - This acts as a smooth, band-limited random modulation source without hard
 *   discontinuities between values.
 *
 * This implementation is designed for real-time audio use:
 * - No allocations in the audio path.
 * - No locks.
 * - All math is per-sample and efficient.
 * - Numerically stable for typical audio sample rates (e.g. 24kHz - 192kHz).
 *
 * Public interface is intentionally small:
 * - Call setSampleRate() once at initialization or when the host rate changes.
 * - Configure:
 *     - rateHz: how often (on average) new random targets are chosen.
 *     - depth: output scale (0..1).
 *     - smooth: correlation / damping factor (0..1) controlling how "sharp"
 *               transitions can be; higher values = smoother / more correlated.
 *     - bipolar: output in [-1, 1] when true, [0, 1] when false.
 * - Call reset() to reset phase and output.
 * - Call processSample() each sample to get the next LFO value.
 */

namespace ShortwavDSP
{

class RandomLFO
{
public:
    RandomLFO() = default;

    // Initialize with a given sample rate.
    // Must be called before processSample().
    void setSampleRate(float sampleRate)
    {
        sampleRate_ = (sampleRate > 1.f) ? sampleRate : 44100.f;
        updateStepRate();
    }

    // Reset state to a known value.
    // phase is reset, current and target values are zeroed.
    void reset(float initial = 0.f)
    {
        x_ = clamp01(initial);
        v_ = 0.f;
        target_ = x_;
        phase_ = 0.f;
        updateFilterCoeffs();
    }

    // Set how often a new random target is generated (in Hz).
    // Example: 1.0 => roughly one new random value per second.
    void setRate(float rateHz)
    {
        rateHz_ = std::max(rateHz, 0.f);
        updateStepRate();
    }

    // Set modulation depth/intensity (0..1).
    // Acts as a simple output gain.
    void setDepth(float depth)
    {
        depth_ = std::max(0.f, depth);
    }

    // Set smoothness / correlation in [0, 1].
    // - 0   : fastest response (less correlation, more movement).
    // - 1.0 : very smooth / slow response towards new targets.
    //
    // Internally mapped to a damping factor for the second-order system.
    void setSmooth(float smooth)
    {
        smooth_ = clamp01(smooth);
        updateFilterCoeffs();
    }

    // Set whether the LFO output is bipolar [-1, 1] or unipolar [0, 1].
    void setBipolar(bool bipolar)
    {
        bipolar_ = bipolar;
    }

    // Optionally seed the internal RNG for deterministic behavior.
    void seed(uint32_t seedValue)
    {
        if (seedValue == 0) {
            seedValue = 0x1234567u;
        }
        rngState_ = seedValue;
    }

    // Generate next LFO sample.
    // This is real-time safe and intended for per-sample use in an audio callback.
    float processSample()
    {
        if (sampleRate_ <= 0.f) {
            // Failsafe: return zero if not configured.
            return 0.f;
        }

        // Time to jump to a new random target?
        // Use a simple deterministic LCG RNG.
        phase_ += stepPerSample_;
        if (phase_ >= 1.f) {
            phase_ -= 1.f;
            target_ = nextRandom01();
        }

        // Second-order critically-damped-like system that smoothly tracks target_.
        // Discrete-time formulation:
        //
        //   a = spring stiffness (derived from smoothness and rate)
        //   b = damping coefficient
        //
        //   v += (a * (target - x) - b * v);
        //   x += v;
        //
        // Coefficients (a_, b_) are precomputed in updateFilterCoeffs().
        const float error = target_ - x_;
        v_ += (a_ * error - b_ * v_);
        x_ += v_;

        // Lightly constrain x_ to avoid numerical creep
        if (x_ < -0.1f) x_ = -0.1f;
        if (x_ > 1.1f)  x_ = 1.1f;

        float out = x_;

        // Map to desired polarity.
        if (bipolar_) {
            // x in [0,1] -> [-1,1]
            out = out * 2.f - 1.f;
        }
        else {
            // Hard clamp to unipolar [0,1].
            out = clamp01(out);
        }

        // Apply depth as final scale.
        out *= depth_;

        return out;
    }

private:
    float sampleRate_ = 44100.f;
    float rateHz_     = 1.0f;  // average rate of new random targets
    float depth_      = 1.0f;
    float smooth_     = 0.75f; // [0,1] smoothing / correlation control
    bool  bipolar_    = true;

    // Internal random generator state.
    // Very small, deterministic LCG suitable for audio-rate use.
    uint32_t rngState_ = 0x1234567u;

    // Random target system.
    float phase_         = 0.f; // [0,1) for new target scheduling
    float stepPerSample_ = 1.f / 44100.f;
    float target_        = 0.f;

    // Second-order system state:
    float x_ = 0.f; // current output (unipolar domain)
    float v_ = 0.f; // current velocity

    // Precomputed filter coefficients.
    float a_ = 0.f; // stiffness
    float b_ = 0.f; // damping

    static float clamp01(float x)
    {
        if (x < 0.f) return 0.f;
        if (x > 1.f) return 1.f;
        return x;
    }

    // Very small and fast linear congruential generator:
    // Returns next float in [0,1].
    float nextRandom01()
    {
        // Constants from Numerical Recipes.
        rngState_ = rngState_ * 1664525u + 1013904223u;
        const float scaled = static_cast<float>(rngState_ & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
        return scaled;
    }

    void updateStepRate()
    {
        if (sampleRate_ <= 0.f) {
            stepPerSample_ = 0.f;
            return;
        }
        if (rateHz_ <= 0.f) {
            // If rate is zero or negative, effectively never change target.
            stepPerSample_ = 0.f;
        }
        else {
            // How fast we move phase_ from 0 -> 1 for each random step.
            const float stepsPerSecond = rateHz_;
            stepPerSample_ = stepsPerSecond / sampleRate_;
        }

        updateFilterCoeffs();
    }

    // Map smoothness to second-order system coefficients.
    //
    // We loosely base this on the idea of a damped spring:
    //   a ~= (2*pi*f_c)^2
    //   b ~= 2*zeta*(2*pi*f_c)
    //
    // where f_c is tied to the LFO step rate and smooth is mapped to damping / cutoff.
    void updateFilterCoeffs()
    {
        if (sampleRate_ <= 0.f) {
            a_ = 0.f;
            b_ = 0.f;
            return;
        }

        // Base frequency related to how quickly we want to track new random values.
        // Ensure a minimal non-zero frequency for stability.
        const float minHz = 0.05f;
        const float effectiveRate = std::max(rateHz_, minHz);

        // Map smoothness to a normalized factor that influences bandwidth.
        const float smoothClamped = clamp01(smooth_);

        // Higher smooth => lower cutoff => smaller a.
        // Lower smooth => higher cutoff => larger a.
        const float baseOmega = 2.f * 3.14159265359f * effectiveRate;
        const float maxScale = 1.0f;
        const float minScale = 0.05f;
        const float scale = minScale + (maxScale - minScale) * (1.f - smoothClamped);
        const float omega = baseOmega * scale / sampleRate_;

        // Stiffness and damping in discrete time (small omega).
        a_ = omega * omega;

        // Damping factor:
        // smooth near 1 => more damping; smooth near 0 => lighter damping.
        const float minDamp = 0.2f;
        const float maxDamp = 1.2f;
        const float damp = minDamp + (maxDamp - minDamp) * smoothClamped;

        b_ = 2.f * damp * omega;
    }
};

} // namespace ShortwavDSP