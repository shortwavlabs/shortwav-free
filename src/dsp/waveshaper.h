#pragma once

#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>

/*
 * Chebyshev Waveshaper
 *
 * This header provides:
 *  - A numerically stable, allocation-free implementation of Chebyshev
 *    polynomials of the first kind T_n(x) using their recursive definition.
 *  - A light-weight Chebyshev-based waveshaper suitable for real-time
 *    audio processing.
 *
 * Reference:
 *   https://www.musicdsp.org/en/latest/Synthesis/187-chebyshev-waveshaper-using-their-recursive-definition.html#
 *
 * Background:
 *  - Chebyshev polynomials of the first kind T_n(x) are defined on [-1, 1]:
 *
 *        T_0(x) = 1
 *        T_1(x) = x
 *        T_{n+1}(x) = 2*x*T_n(x) - T_{n-1}(x)
 *
 *    and satisfy:
 *        T_n(cos(theta)) = cos(n * theta)
 *
 *  - When used as a waveshaper with input limited to [-1, 1], T_n produces
 *    a spectrally pure n-th harmonic (up to amplitude and phase), which
 *    makes linear combinations of T_n useful for designing targeted
 *    harmonic structures.
 *
 * Implementation notes:
 *  - We use an iterative recurrence per polynomial degree; complexity per
 *    sample is O(N) where N is the maximum order. This is efficient and
 *    numerically stable for the moderate orders typically used in audio
 *    waveshaping (e.g., N <= 16..32).
 *
 *  - Input domain:
 *      * For harmonic interpretation, Chebyshev polynomials assume x in [-1, 1].
 *      * We soft-limit/clamp input to [-1, 1] before evaluating T_n(x).
 *        By default we use a fast tanh-like saturator for values outside
 *        [-1, 1] to avoid very large outputs and preserve musical behavior.
 *
 *  - Coefficients:
 *      * The waveshaper supports up to MaxOrder (configurable via template)
 *        with user-provided weights a_n for each T_n.
 *      * Users can construct custom spectra; for example:
 *            a_1 = 1.0f, others 0     -> (approximately) clean
 *            a_2 = 1.0f               -> emphasizes 2nd harmonic
 *            a_3 = 1.0f               -> emphasizes 3rd harmonic, etc.
 *
 *  - Real-time safety:
 *      * Header-only, no dynamic allocations, no locks.
 *      * Parameter updates are simple stores; if called from multiple threads,
 *        use the same discipline as the rest of the DSP layer (e.g. host /
 *        GUI thread updates parameters between audio blocks).
 *
 *  - Denormals:
 *      * The recursive form and clamping avoid generating subnormal values
 *        for normal audio ranges. If the surrounding project uses a specific
 *        denormal strategy (e.g., global DAZ/FTZ flags or tiny DC offsets),
 *        this class remains compatible; all math is straightforward float ops.
 */

namespace ShortwavDSP
{

  //------------------------------------------------------------------------------
  // Utility: tiny helper to avoid obvious denormal / extreme-domain issues
  //------------------------------------------------------------------------------

  namespace detail
  {
    inline float softClipToUnit(float x) noexcept
    {
      // For |x| <= 1 use linear region (no change).
      // For |x| > 1 apply a fast soft saturation to keep |x| bounded.
      //
      // This keeps Chebyshev polynomials in a numerically reasonable range
      // and preserves predictable harmonic behavior for normal signals
      // while avoiding explosive growth for extreme values.
      if (x > 1.0f)
      {
        // tanh-like saturation using x / (1 + |x|)
        return 1.0f - 1.0f / (1.0f + x);
      }
      if (x < -1.0f)
      {
        const float ax = -x;
        return -1.0f + 1.0f / (1.0f + ax);
      }
      return x;
    }

    inline float clampToUnit(float x) noexcept
    {
      if (x < -1.0f)
        return -1.0f;
      if (x > 1.0f)
        return 1.0f;
      return x;
    }
  } // namespace detail

  //------------------------------------------------------------------------------
  // Chebyshev polynomial evaluator (first kind, T_n)
  //------------------------------------------------------------------------------
  //
  // Provides efficient evaluation of T_n(x) and of weighted sums
  // sum_{n=0..N} a_n * T_n(x), using the stable forward recurrence.
  //------------------------------------------------------------------------------

  template <std::size_t MaxOrder>
  class ChebyshevEvaluator
  {
  public:
    static_assert(MaxOrder > 0, "MaxOrder must be > 0.");

    ChebyshevEvaluator() = default;

    // Evaluate T_n(x) for a given order n (0 <= n <= MaxOrder).
    // Input x is assumed to be in [-1, 1] (call detail::clampToUnit or
    // softClipToUnit upstream if needed).
    //
    // Uses the iterative recurrence:
    //   T_0 = 1
    //   T_1 = x
    //   T_{k+1} = 2*x*T_k - T_{k-1}
    //
    // For n == 0 or 1 we return directly without looping.
    inline float evaluateSingle(std::size_t n, float x) const noexcept
    {
      if (n == 0)
        return 1.0f;
      if (n == 1)
        return x;

      float Tnm1 = 1.0f; // T_0
      float Tn = x;      // T_1

      // Loop: from k = 1 to n-1 to compute T_{k+1}
      for (std::size_t k = 1; k < n; ++k)
      {
        const float Tnp1 = 2.0f * x * Tn - Tnm1;
        Tnm1 = Tn;
        Tn = Tnp1;
      }
      return Tn;
    }

    // Evaluate a weighted Chebyshev series:
    //
    //   y = sum_{n=0..N} coeffs[n] * T_n(x)
    //
    // N is the active order (inclusive). Both coeffs[0..N] and N must be valid
    // for the calling context. This function assumes x already in [-1, 1]
    // and is intended to be called from the waveshaper process function.
    //
    // Implementation is single-pass and uses the same recurrence as above.
    inline float evaluateSeries(const float *coeffs,
                                std::size_t order,
                                float x) const noexcept
    {
      if (!coeffs || order == 0)
      {
        return 0.0f;
      }

      // Clamp order to supported range.
      if (order > MaxOrder)
        order = MaxOrder;

      // T_0 and T_1 contributions
      float sum = coeffs[0]; // coeff[0] * T_0 == coeff[0] * 1

      if (order == 1)
      {
        sum += coeffs[1] * x;
        return sum;
      }

      float Tnm1 = 1.0f; // T_0
      float Tn = x;      // T_1
      sum += coeffs[1] * Tn;

      for (std::size_t n = 1; n < order; ++n)
      {
        const float Tnp1 = 2.0f * x * Tn - Tnm1;
        Tnm1 = Tn;
        Tn = Tnp1;
        sum += coeffs[n + 1] * Tn;
      }

      return sum;
    }
  };

  //------------------------------------------------------------------------------
  // Chebyshev Waveshaper
  //------------------------------------------------------------------------------
  //
  // Public API:
  //   - Template parameter MaxOrder: maximum supported Chebyshev order.
  //   - setOrder(order):
  //        Sets the active highest polynomial order (1..MaxOrder).
  //   - setCoefficient(n, value):
  //        Sets the weight for T_n, 0 <= n <= MaxOrder.
  //   - setCoefficients(ptr, count):
  //        Bulk update of the first `count` coefficients.
  //   - setOutputGain(g):
  //        Overall linear output gain applied after the Chebyshev series.
  //   - setUseSoftClipForInput(bool):
  //        Choose between hard clamp or soft saturation for input domain control.
  //   - processSample(x):
  //        Apply Chebyshev waveshaping to a single sample.
  //   - processBuffer(in, out, numSamples):
  //        Apply waveshaping to a buffer (safe for in-place).
  //
  // Notes:
  //   - Real-time safe: no allocation, no locking.
  //   - Threading: expected usage is that parameter setters are called from a
  //     non-audio thread between blocks or using the host's standard mechanism.
  //     For lock-free cross-thread safety with sample-accurate automation,
  //     wrap access in your existing parameter / smoothing system.
  //------------------------------------------------------------------------------

  template <std::size_t MaxOrder = 16u>
  class ChebyshevWaveshaper
  {
  public:
    ChebyshevWaveshaper() = default;

    // Set the active order (degree) N for the polynomial series.
    // Valid range: 1..MaxOrder. Values < 1 disable shaping (bypass-like).
    inline void setOrder(std::size_t order) noexcept
    {
      if (order < 1)
        activeOrder_ = 0;
      else if (order > MaxOrder)
        activeOrder_ = MaxOrder;
      else
        activeOrder_ = order;
    }

    inline std::size_t getOrder() const noexcept
    {
      return activeOrder_;
    }

    // Set individual coefficient for T_n.
    // n == 0 is DC term, typically 0 for purely AC / harmonic use.
    inline void setCoefficient(std::size_t n, float value) noexcept
    {
      if (n > MaxOrder)
        return;
      coeffs_[n] = value;
    }

    // Bulk-set first `count` coefficients from an array.
    // Extra entries beyond MaxOrder are ignored.
    inline void setCoefficients(const float *values, std::size_t count) noexcept
    {
      if (!values || count == 0)
        return;

      const std::size_t limit = (count <= (MaxOrder + 1)) ? count : (MaxOrder + 1);
      for (std::size_t i = 0; i < limit; ++i)
      {
        coeffs_[i] = values[i];
      }
    }

    // Reset all coefficients to zero and optionally set a simple default.
    // By default, we configure T_1(x) == x (i.e. coeffs_[1] = 1) and 0 elsewhere.
    inline void resetCoefficientsToLinear() noexcept
    {
      for (std::size_t i = 0; i <= MaxOrder; ++i)
        coeffs_[i] = 0.0f;
      coeffs_[1] = 1.0f;
    }

    // Set global output gain applied after Chebyshev series.
    inline void setOutputGain(float gain) noexcept
    {
      outputGain_ = gain;
    }

    inline float getOutputGain() const noexcept
    {
      return outputGain_;
    }

    // Control input domain mapping strategy:
    //  - When enabled (default), inputs beyond [-1, 1] are softly saturated.
    //  - When disabled, inputs are hard-clamped to [-1, 1].
    inline void setUseSoftClipForInput(bool enabled) noexcept
    {
      useSoftClipInput_ = enabled;
    }

    inline bool getUseSoftClipForInput() const noexcept
    {
      return useSoftClipInput_;
    }

    // Process a single sample through the Chebyshev waveshaper.
    inline float processSample(float in) const noexcept
    {
      if (activeOrder_ == 0)
      {
        // Effectively bypass if no active order set.
        return in;
      }

      float x = useSoftClipInput_
                    ? detail::softClipToUnit(in)
                    : detail::clampToUnit(in);

      const float y = evaluator_.evaluateSeries(coeffs_.data(),
                                                activeOrder_,
                                                x);

      float out = y * outputGain_;

      // Avoid propagating potential denorms (extremely unlikely here, but cheap).
      if (std::abs(out) < 1.0e-30f)
        out = 0.0f;

      return out;
    }

    // Process a buffer of samples. Supports in-place processing (in == out).
    inline void processBuffer(const float *in,
                              float *out,
                              std::size_t numSamples) const noexcept
    {
      if (!in || !out || numSamples == 0)
        return;

      if (activeOrder_ == 0)
      {
        // Copy input to output without change if effectively bypassed.
        if (in != out)
        {
          for (std::size_t i = 0; i < numSamples; ++i)
            out[i] = in[i];
        }
        return;
      }

      for (std::size_t i = 0; i < numSamples; ++i)
      {
        out[i] = processSample(in[i]);
      }
    }

  private:
    ChebyshevEvaluator<MaxOrder> evaluator_{};

    // coeffs_[n] is the weight for T_n(x).
    // We allocate MaxOrder + 1 entries to include T_0..T_MaxOrder.
    std::array<float, MaxOrder + 1u> coeffs_{}; // zero-initialized

    // Highest active order (degree) used in evaluation.
    // 0 means "disabled"/bypass.
    std::size_t activeOrder_ = 1;

    // Global output gain applied to the Chebyshev sum.
    float outputGain_ = 1.0f;

    // Whether to use soft saturation vs. hard clamp for input domain control.
    bool useSoftClipInput_ = true;
  };

} // namespace ShortwavDSP