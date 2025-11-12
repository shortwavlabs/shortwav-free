#pragma once

#include <cmath>

/*
 * Drift Generator
 *
 * Implementation of the "Drift Generator" described at:
 * https://www.musicdsp.org/en/latest/Synthesis/183-drift-generator.html
 *
 * Core idea from the original post:
 * - Generate very slow, smooth, pseudo-random pitch/parameter drift suitable
 *   for analog-style instability.
 * - Use a leaky / critically-damped style integrator driven by a low-rate
 *   random process to avoid discontinuities and aliasing.
 * - The output:
 *     * Is continuous (no steps).
 *     * Exhibits slow wander with controllable depth and time constants.
 *     * Is deterministic with fixed initial state.
 *
 * Design goals for this implementation:
 * - Header-only, allocation-free, no locks.
 * - Deterministic and sample-accurate for real-time DSP.
 * - Clear, minimal API:
 *      * setSampleRate()
 *      * setDepth()      : overall output scale
 *      * setRateHz()     : characteristic drift rate / time constant
 *      * reset()
 *      * next()          : per-sample drift value
 * - Numerically robust:
 *      * Avoid denormals via tiny noise injection and clamping.
 *      * Stable coefficient mapping for typical audio rates.
 *
 * Notes on mapping to the original algorithm:
 * - The classic MusicDSP drift generator uses a random walk filtered by
 *   a critically-damped 2-pole lowpass (or equivalently, cascaded one-pole
 *   smoothers) to obtain extremely low-frequency movement.
 * - This implementation follows that intent:
 *      * We generate a very low-rate white-ish excitation using a simple
 *        deterministic RNG.
 *      * We feed that into a pair of leaky integrators (2-pole low-pass)
 *        whose pole location is derived from rateHz and the sample rate.
 *      * The final drift is scaled by depth.
 *
 * Parameter semantics:
 * - rateHz:
 *      * Rough "corner" frequency of the drift (how fast it moves).
 *      * Smaller => slower, more sluggish drift.
 *      * Larger => faster wander (but still smooth).
 * - depth:
 *      * Linear scale factor for the output drift.
 *      * The raw drift is dimensionless; typical usage might treat this
 *        as semitones, cents, or as a dimensionless modulation amount
 *        elsewhere in the signal chain.
 *
 * Threading:
 * - Setters are plain stores and may be called from a control thread
 *   between blocks. For fully sample-accurate automation, wrap this
 *   in your existing parameter interpolation system.
 */

namespace ShortwavDSP
{

class DriftGenerator
{
public:
    DriftGenerator() = default;

    // Initialize or update the sample rate.
    // Must be called before next() is used in production code.
    inline void setSampleRate(float sampleRate) noexcept
    {
        if (sampleRate <= 0.0f)
            sampleRate = 44100.0f;
        sampleRate_ = sampleRate;
        updateCoeffs();
    }

    // Set overall drift depth (output gain).
    // Negative values are allowed (invert drift), but typical use is depth >= 0.
    inline void setDepth(float depth) noexcept
    {
        depth_ = depth;
    }

    // Set drift rate in Hz.
    //
    // Interpreted as an approximate lowpass corner / "how quickly can the drift
    // move". Very small values yield extremely slow drift. Values are clamped
    // to a safe minimum to avoid numerical issues.
    inline void setRateHz(float rateHz) noexcept
    {
        if (rateHz <= 0.0f)
            rateHz = kMinRateHz;
        rateHz_ = rateHz;
        updateCoeffs();
    }

    // Reset internal state to a deterministic baseline.
    // initialDrift is the starting output drift before depth scaling.
    inline void reset(float initialDrift = 0.0f) noexcept
    {
        // States are in "pre-depth" domain.
        x1_ = initialDrift;
        x2_ = initialDrift;
        // Keep rngState_ as-is to preserve reproducibility across resets
        // if the caller manages it externally.
        denormPhase_ = 0u;
    }

    // Optionally seed the internal RNG for deterministic behavior across runs.
    inline void seed(unsigned int seedValue) noexcept
    {
        if (seedValue == 0u)
            seedValue = 0x1234567u;
        rngState_ = seedValue;
    }

    // Generate the next drift sample.
    //
    // Real-time safe: no allocations, no branches with locks, constant-time.
    inline float next() noexcept
    {
        // Fail-safe: if sampleRate_ wasn't set, use defaults.
        if (sampleRate_ <= 0.0f)
            setSampleRate(44100.0f);

        // Generate a low-rate excitation.
        //
        // Instead of changing the target at a fixed interval, we inject
        // a very small white-like excitation at each sample, scaled such
        // that the resulting process matches the "slow wander" character
        // of the reference design when filtered by the 2-pole lowpass.
        const float excitation = nextRandomBipolar() * excitationScale_;

        // Two cascaded one-pole filters (equivalent to a 2-pole lowpass)
        // driven by the excitation. This mirrors the "drift" behaviour
        // from the MusicDSP post: a smooth, correlated random walk.
        x1_ = a1_ * x1_ + excitation;
        x2_ = a1_ * x2_ + x1_;

        // Basic clamp to avoid runaway due to any pathological parameter combos.
        if (x2_ > kStateClamp)
            x2_ = kStateClamp;
        else if (x2_ < -kStateClamp)
            x2_ = -kStateClamp;

        float out = x2_ * depth_;

        // Denormal guard:
        // Inject very small toggling DC when amplitude is extremely low to keep
        // subnormals from accumulating on some platforms.
        if (std::fabs(out) < kDenormThreshold)
        {
            // Simple deterministic +/- epsilon pattern; no branching on RNG.
            denormPhase_ ^= 1u;
            const float d = (denormPhase_ ? kDenormNoise : -kDenormNoise);
            x1_ += d;
            x2_ += d;
            out = 0.0f; // keep observable drift at exactly 0 in this extreme case
        }

        // Ensure final value is finite.
        if (!std::isfinite(out))
            out = 0.0f;

        return out;
    }

private:
    //-------------------------------------------------------------------------
    // Internal configuration
    //-------------------------------------------------------------------------

    // Minimal sensible rate to keep coefficients well-behaved.
    static constexpr float kMinRateHz       = 0.0001f;   // 0.1 mHz: ~10,000 s
    static constexpr float kMaxRateHz       = 10.0f;     // prevent overly fast/aliasy drift
    static constexpr float kDefaultRateHz   = 0.1f;      // 0.1 Hz default
    static constexpr float kDefaultDepth    = 1.0f;
    static constexpr float kStateClamp      = 10.0f;     // generous bound for internal states
    static constexpr float kDenormThreshold = 1.0e-30f;
    static constexpr float kDenormNoise     = 1.0e-20f;

    float sampleRate_ = 44100.0f;
    float depth_      = kDefaultDepth;
    float rateHz_     = kDefaultRateHz;

    // One-pole coefficient (same for both stages).
    // We model the drift as a 2-pole lowpass with repeated pole at 'pole'.
    float a1_ = 0.0f;

    // Scale for the white excitation to get usable drift magnitude
    // before 'depth' is applied. Derived in updateCoeffs().
    float excitationScale_ = 0.0f;

    // 2-stage state for the lowpass cascade.
    float x1_ = 0.0f;
    float x2_ = 0.0f;

    // Simple RNG state for excitation (LCG).
    unsigned int rngState_ = 0x1234567u;

    // Small state for denorm guard toggling.
    unsigned int denormPhase_ = 0u;

    //-------------------------------------------------------------------------
    // Helpers
    //-------------------------------------------------------------------------

    inline void updateCoeffs() noexcept
    {
        // Clamp to safe, meaningful range.
        float r = rateHz_;
        if (r < kMinRateHz) r = kMinRateHz;
        if (r > kMaxRateHz) r = kMaxRateHz;

        // Map rateHz to a one-pole coefficient.
        //
        // We choose:
        //   pole = exp(-2*pi*rateHz / sampleRate)
        //
        // so that rateHz approximates the -3 dB frequency of the underlying
        // first-order lowpass. Cascading two identical one-poles then yields
        // a gentle, 2-pole low-frequency rolloff which fits the drift behavior.
        const float dt   = 1.0f / sampleRate_;
        const float omega = 2.0f * 3.14159265359f * r;
        const float pole  = std::exp(-omega * dt);

        a1_ = pole;

        // Choose excitation scaling such that the RMS drift before depth scaling
        // stays in a musically reasonable range across typical parameters.
        //
        // For a two-pole lowpass with white input, steady-state variance is:
        //   sigma^2 ~ sigma_e^2 / (1 - pole)^2   (approx for small 1-pole)
        //
        // We invert this approximately so that sigma of x2_ is O(1) for nominal
        // parameters, then let the public 'depth' control handle actual range.
        const float oneMinusPole = 1.0f - pole;
        const float targetSigma  = 0.5f; // arbitrary but musical pre-depth wander
        if (oneMinusPole > 0.0f)
        {
            const float sigma_e = targetSigma * oneMinusPole * oneMinusPole;
            // White source in [-1,1] has variance 1/3; include that in scale.
            excitationScale_ = sigma_e * std::sqrt(3.0f);
        }
        else
        {
            excitationScale_ = 0.0f;
        }
    }

    // Deterministic, cheap RNG in [-1, 1].
    inline float nextRandomBipolar() noexcept
    {
        // LCG: same pattern as used in other DSP utils.
        rngState_ = rngState_ * 1664525u + 1013904223u;
        // Use upper bits for uniform [0,1).
        const float v = static_cast<float>((rngState_ >> 8) & 0x00FFFFFFu)
                      / static_cast<float>(0x01000000u);
        return v * 2.0f - 1.0f;
    }
};

} // namespace ShortwavDSP