// Unit tests for ShortwavDSP ChebyshevWaveshaper and RandomLFO.
//
// These tests are designed to:
// - Use only the public APIs of the headers under test.
// - Be fast and allocation-free in the hot paths.
// - Integrate with a simple assertion-style runner compatible with common CI.
//
// The test entry point is intentionally header-only and self-contained so it can
// be compiled using the existing Makefile by adding this file to SOURCES in CI
// or a dedicated test target.

#include <cstdio>
#include <cmath>
#include <vector>
#include <limits>
#include "../dsp/waveshaper.h"
#include "../dsp/random-lfo.h"
#include "../dsp/drift.h"

namespace
{

  constexpr float kEpsilon = 1e-5f;
  constexpr float kTightEpsilon = 1e-6f;

  // Simple assertion helpers (no external framework required).
  struct TestContext
  {
    int passed = 0;
    int failed = 0;

    void assertTrue(bool cond, const char *name, const char *file, int line)
    {
      if (cond)
      {
        ++passed;
      }
      else
      {
        ++failed;
        std::printf("[FAIL] %s (%s:%d)\n", name, file, line);
      }
    }

    void assertNear(float actual, float expected, float tol,
                    const char *name, const char *file, int line)
    {
      const float diff = std::fabs(actual - expected);
      if (diff <= tol || (std::isnan(expected) && std::isnan(actual)))
      {
        ++passed;
      }
      else
      {
        ++failed;
        std::printf("[FAIL] %s: expected=%g actual=%g tol=%g (%s:%d)\n",
                    name, (double)expected, (double)actual, (double)tol, file, line);
      }
    }

    void summary() const
    {
      std::printf("[TEST SUMMARY] passed=%d failed=%d\n", passed, failed);
    }
  };

#define T_ASSERT(ctx, cond) (ctx).assertTrue((cond), #cond, __FILE__, __LINE__)
#define T_ASSERT_NEAR(ctx, actual, expected, tol) \
  (ctx).assertNear((actual), (expected), (tol), #actual " ~= " #expected, __FILE__, __LINE__)

  //------------------------------------------------------------------------------
  // ChebyshevWaveshaper tests
  //------------------------------------------------------------------------------

  void test_waveshaper_basic_linear(TestContext &ctx)
  {
    using ShortwavDSP::ChebyshevWaveshaper;

    ChebyshevWaveshaper<8> ws;
    ws.resetCoefficientsToLinear();
    ws.setOrder(1);
    ws.setOutputGain(1.0f);
    ws.setUseSoftClipForInput(false);

    // For linear config with T1(x) = x, processSample should equal input in [-1,1].
    const float inputs[] = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};
    for (float in : inputs)
    {
      float out = ws.processSample(in);
      T_ASSERT_NEAR(ctx, out, in, kTightEpsilon);
    }

    // When order==0, should act as bypass for all inputs.
    ws.setOrder(0);
    for (float in : inputs)
    {
      float out = ws.processSample(in);
      T_ASSERT_NEAR(ctx, out, in, kTightEpsilon);
    }
  }

  void test_waveshaper_input_clamp_and_softclip(TestContext &ctx)
  {
    using ShortwavDSP::ChebyshevWaveshaper;

    ChebyshevWaveshaper<4> ws;
    ws.resetCoefficientsToLinear();
    ws.setOrder(1);

    // Hard clamp mode: inputs beyond [-1,1] are clamped before shaping.
    ws.setUseSoftClipForInput(false);
    {
      float out_hi = ws.processSample(10.0f);
      float out_lo = ws.processSample(-10.0f);
      // Must be within [-1,1] and monotone w.r.t clamp endpoints.
      T_ASSERT(ctx, out_hi <= 1.0f + kEpsilon && out_hi >= 1.0f - 1e-3f);
      T_ASSERT(ctx, out_lo >= -1.0f - kEpsilon && out_lo <= -1.0f + 1e-3f);
    }

    // Soft clip mode: still bounded, continuous, monotone; must not explode.
    ws.setUseSoftClipForInput(true);
    {
      float out1 = ws.processSample(2.0f);
      float out2 = ws.processSample(10.0f);
      float out3 = ws.processSample(-2.0f);
      float out4 = ws.processSample(-10.0f);

      T_ASSERT(ctx, out1 <= 1.0f && out1 >= 0.0f);
      T_ASSERT(ctx, out2 <= 1.0f && out2 >= 0.0f);
      T_ASSERT(ctx, out3 >= -1.0f && out3 <= 0.0f);
      T_ASSERT(ctx, out4 >= -1.0f && out4 <= 0.0f);

      // Monotonicity for positive side: larger input should not yield smaller output.
      T_ASSERT(ctx, out2 >= out1 - kEpsilon);
      // Symmetry-ish: soft clip should preserve sign.
      T_ASSERT(ctx, out1 > 0.0f);
      T_ASSERT(ctx, out2 > 0.0f);
      T_ASSERT(ctx, out3 < 0.0f);
      T_ASSERT(ctx, out4 < 0.0f);
    }
  }

  void test_waveshaper_order_and_coefficients(TestContext &ctx)
  {
    using ShortwavDSP::ChebyshevWaveshaper;

    ChebyshevWaveshaper<4> ws;
    ws.setUseSoftClipForInput(false);
    ws.setOutputGain(1.0f);

    // Configure pure T2(x) using known polynomial: T2(x) = 2x^2 - 1.
    ws.setOrder(2);
    ws.setCoefficient(0, 0.0f);
    ws.setCoefficient(1, 0.0f);
    ws.setCoefficient(2, 1.0f);

    const float xs[] = {-1.0f, -0.5f, 0.0f, 0.3f, 0.7f, 1.0f};
    for (float x : xs)
    {
      float out = ws.processSample(x);
      float expected = 2.0f * x * x - 1.0f;
      T_ASSERT_NEAR(ctx, out, expected, 5e-4f);
    }

    // Increasing order should not break continuity near 0 for reasonable coeffs.
    ws.resetCoefficientsToLinear();
    ws.setOrder(4);
    ws.setCoefficient(2, 0.3f);
    ws.setCoefficient(3, 0.1f);
    ws.setCoefficient(4, 0.05f);

    float x1 = -0.001f;
    float x2 = 0.0f;
    float x3 = 0.001f;
    float y1 = ws.processSample(x1);
    float y2 = ws.processSample(x2);
    float y3 = ws.processSample(x3);

    // Check small neighborhood is continuous and monotone-ish around 0.
    T_ASSERT(ctx, std::fabs(y1 - y2) < 1e-3f);
    T_ASSERT(ctx, std::fabs(y3 - y2) < 1e-3f);
  }

  void test_waveshaper_process_buffer(TestContext &ctx)
  {
    using ShortwavDSP::ChebyshevWaveshaper;

    ChebyshevWaveshaper<2> ws;
    ws.resetCoefficientsToLinear();
    ws.setOrder(1);
    ws.setUseSoftClipForInput(false);

    const std::vector<float> in = {-1.f, -0.5f, 0.f, 0.5f, 1.f};
    std::vector<float> out(in.size(), 0.0f);

    ws.processBuffer(in.data(), out.data(), in.size());
    for (size_t i = 0; i < in.size(); ++i)
    {
      T_ASSERT_NEAR(ctx, out[i], in[i], kTightEpsilon);
    }

    // In-place must also be correct.
    std::vector<float> inout = in;
    ws.processBuffer(inout.data(), inout.data(), inout.size());
    for (size_t i = 0; i < in.size(); ++i)
    {
      T_ASSERT_NEAR(ctx, inout[i], in[i], kTightEpsilon);
    }
  }

  void test_waveshaper_invalid_params_and_denorm_guard(TestContext &ctx)
  {
    using ShortwavDSP::ChebyshevWaveshaper;

    ChebyshevWaveshaper<2> ws;
    ws.resetCoefficientsToLinear();

    // setOrder should clamp out-of-range values safely.
    ws.setOrder(9999);
    T_ASSERT(ctx, ws.getOrder() <= 2);

    ws.setOrder(0);
    T_ASSERT(ctx, ws.getOrder() == 0);

    // setCoefficient with out-of-range index must not crash or corrupt behavior.
    ws.setOrder(1);
    ws.setCoefficient(999, 1.0f); // should be ignored

    // Stress with extremely small output to exercise denorm guard.
    ws.setOutputGain(1e-20f);
    float out = ws.processSample(1e-10f);
    // Implementation clamps only when |out| < 1e-30, so just ensure it's tiny
    // and finite to avoid denorm/performance issues.
    T_ASSERT(ctx, std::fabs(out) < 1e-15f);
    T_ASSERT(ctx, std::isfinite(out));
  }

  //------------------------------------------------------------------------------
  // RandomLFO tests
  //------------------------------------------------------------------------------

  void test_randomlfo_basic_determinism(TestContext &ctx)
  {
    using ShortwavDSP::RandomLFO;

    RandomLFO lfo1;
    RandomLFO lfo2;

    lfo1.setSampleRate(44100.f);
    lfo2.setSampleRate(44100.f);

    lfo1.seed(123456u);
    lfo2.seed(123456u);

    lfo1.reset(0.25f);
    lfo2.reset(0.25f);

    lfo1.setRate(2.0f);
    lfo2.setRate(2.0f);
    lfo1.setSmooth(0.5f);
    lfo2.setSmooth(0.5f);
    lfo1.setDepth(1.0f);
    lfo2.setDepth(1.0f);
    lfo1.setBipolar(true);
    lfo2.setBipolar(true);

    // Same configuration and seed => identical sequence.
    for (int i = 0; i < 2048; ++i)
    {
      float a = lfo1.processSample();
      float b = lfo2.processSample();
      T_ASSERT_NEAR(ctx, a, b, kTightEpsilon);
    }
  }

  void test_randomlfo_rate_and_phase_behavior(TestContext &ctx)
  {
    using ShortwavDSP::RandomLFO;

    RandomLFO lfo;
    lfo.setSampleRate(48000.f);
    lfo.seed(0x1u);
    lfo.reset(0.5f);
    lfo.setDepth(1.0f);
    lfo.setBipolar(false); // easier to assert in [0,1]
    lfo.setSmooth(0.5f);

    // With nonzero rate, target should change occasionally.
    lfo.setRate(10.0f);             // 10 new targets per second
    const int totalSamples = 48000; // 1 second
    float first = lfo.processSample();
    bool changed = false;
    for (int i = 1; i < totalSamples; ++i)
    {
      float v = lfo.processSample();
      if (std::fabs(v - first) > 1e-4f)
      {
        changed = true;
        break;
      }
    }
    T_ASSERT(ctx, changed);

    // With zero rate, target must not change after reset; output should remain near initial.
    lfo.reset(0.33f);
    lfo.setRate(0.0f);
    float prev = lfo.processSample();
    bool stayedSimilar = true;
    for (int i = 0; i < 2000; ++i)
    {
      float v = lfo.processSample();
      if (std::fabs(v - prev) > 0.05f)
      {
        stayedSimilar = false;
        break;
      }
      prev = v;
    }
    T_ASSERT(ctx, stayedSimilar);
  }

  void test_randomlfo_bipolar_unipolar_and_depth(TestContext &ctx)
  {
    using ShortwavDSP::RandomLFO;

    RandomLFO lfo;
    lfo.setSampleRate(44100.f);
    lfo.seed(123u);
    lfo.reset(0.5f);
    lfo.setRate(5.0f);
    lfo.setSmooth(0.8f);

    // Unipolar [0,1] with depth 1.
    lfo.setBipolar(false);
    lfo.setDepth(1.0f);
    for (int i = 0; i < 2048; ++i)
    {
      float v = lfo.processSample();
      T_ASSERT(ctx, v >= 0.0f - kEpsilon);
      T_ASSERT(ctx, v <= 1.0f + kEpsilon);
    }

    // Bipolar [-1,1] with depth 1.
    lfo.reset(0.5f);
    lfo.seed(123u);
    lfo.setBipolar(true);
    lfo.setDepth(1.0f);
    for (int i = 0; i < 2048; ++i)
    {
      float v = lfo.processSample();
      T_ASSERT(ctx, v >= -1.0f - kEpsilon);
      T_ASSERT(ctx, v <= 1.0f + kEpsilon);
    }

    // Depth scaling: with depth=0.25 bipolar, outputs must be within [-0.25,0.25].
    lfo.reset(0.5f);
    lfo.seed(123u);
    lfo.setBipolar(true);
    lfo.setDepth(0.25f);
    for (int i = 0; i < 2048; ++i)
    {
      float v = lfo.processSample();
      T_ASSERT(ctx, v >= -0.25f - kEpsilon);
      T_ASSERT(ctx, v <= 0.25f + kEpsilon);
    }
  }

  void test_randomlfo_smooth_parameter_effect(TestContext &ctx)
  {
    using ShortwavDSP::RandomLFO;

    RandomLFO lowSmooth;
    RandomLFO highSmooth;
    lowSmooth.setSampleRate(44100.f);
    highSmooth.setSampleRate(44100.f);

    // Use fixed seeds and reset so the comparison is stable.
    lowSmooth.seed(42u);
    highSmooth.seed(42u);
    lowSmooth.reset(0.5f);
    highSmooth.reset(0.5f);

    lowSmooth.setRate(5.0f);
    highSmooth.setRate(5.0f);

    // Low smooth => relatively quicker response.
    lowSmooth.setSmooth(0.0f);
    // High smooth => relatively slower response.
    highSmooth.setSmooth(1.0f);

    lowSmooth.setDepth(1.0f);
    highSmooth.setDepth(1.0f);

    lowSmooth.setBipolar(false);
    highSmooth.setBipolar(false);

    // Measure average absolute step size as a loose proxy for movement.
    float lowPrev = lowSmooth.processSample();
    float highPrev = highSmooth.processSample();
    float lowAccum = 0.f;
    float highAccum = 0.f;
    const int N = 8000;

    for (int i = 1; i < N; ++i)
    {
      float lv = lowSmooth.processSample();
      float hv = highSmooth.processSample();
      lowAccum += std::fabs(lv - lowPrev);
      highAccum += std::fabs(hv - highPrev);
      lowPrev = lv;
      highPrev = hv;
    }

    const float lowAvgStep = lowAccum / (N - 1);
    const float highAvgStep = highAccum / (N - 1);

    // Only assert a qualitative relationship that should hold for this design:
    // lower smoothness should not yield LESS movement than higher smoothness.
    T_ASSERT(ctx, lowAvgStep >= highAvgStep * 0.8f);
    T_ASSERT(ctx, lowAvgStep <= 0.05f + 0.5f); // keep extremely loose, just guard against explosions
  }

  void test_randomlfo_boundary_conditions(TestContext &ctx)
  {
    using ShortwavDSP::RandomLFO;

    RandomLFO lfo;
    lfo.setSampleRate(192000.f); // high rate
    lfo.seed(99u);
    lfo.reset(0.0f);

    // Very low but non-zero rate.
    lfo.setRate(0.01f);
    lfo.setSmooth(0.9f);
    lfo.setDepth(1.0f);
    lfo.setBipolar(false);

    // Over a moderate buffer, output should remain in range and not blow up.
    for (int i = 0; i < 192000; ++i)
    {
      float v = lfo.processSample();
      T_ASSERT(ctx, v >= 0.0f - kEpsilon);
      T_ASSERT(ctx, v <= 1.0f + kEpsilon);
    }

    // Reset/reseed sanity:
    // For a non-zero seed, ensure output is within expected numeric bounds
    // and calls do not depend on any undefined behavior.
    lfo.setSampleRate(44100.f);
    lfo.setRate(5.0f);
    lfo.reset(0.5f);
    lfo.seed(1u);
    float v1 = lfo.processSample();
    T_ASSERT(ctx, std::isfinite(v1));
  }

  //------------------------------------------------------------------------------
  // DriftGenerator tests
  //------------------------------------------------------------------------------

  void test_drift_generator_basic_finiteness(TestContext &ctx)
  {
    using ShortwavDSP::DriftGenerator;

    DriftGenerator drift;
    drift.setSampleRate(44100.0f);
    drift.setRateHz(0.2f);
    drift.setDepth(1.0f);
    drift.seed(123456u);
    drift.reset(0.0f);

    // Large number of samples: must remain finite and bounded reasonably.
    float minV = std::numeric_limits<float>::max();
    float maxV = -std::numeric_limits<float>::max();

    for (int i = 0; i < 44100 * 10; ++i) // 10 seconds
    {
      float v = drift.next();
      T_ASSERT(ctx, std::isfinite(v));

      if (v < minV)
        minV = v;
      if (v > maxV)
        maxV = v;
    }

    // Extremely loose bounds: drift should not explode to huge values.
    T_ASSERT(ctx, minV > -5.0f);
    T_ASSERT(ctx, maxV < 5.0f);
  }

  void test_drift_generator_continuity(TestContext &ctx)
  {
    using ShortwavDSP::DriftGenerator;

    DriftGenerator drift;
    drift.setSampleRate(48000.0f);
    drift.setRateHz(0.5f);
    drift.setDepth(1.0f);
    drift.seed(42u);
    drift.reset(0.0f);

    float prev = drift.next();
    bool hasReasonableContinuity = true;

    for (int i = 0; i < 48000 * 5; ++i)
    {
      float v = drift.next();
      float step = std::fabs(v - prev);

      // Steps should be tiny for slow drift; allow a loose but finite bound.
      if (step > 0.1f)
      {
        hasReasonableContinuity = false;
        break;
      }
      prev = v;
    }

    T_ASSERT(ctx, hasReasonableContinuity);
  }

  void test_drift_generator_determinism(TestContext &ctx)
  {
    using ShortwavDSP::DriftGenerator;

    DriftGenerator a;
    DriftGenerator b;

    a.setSampleRate(44100.0f);
    b.setSampleRate(44100.0f);

    a.setRateHz(0.3f);
    b.setRateHz(0.3f);

    a.setDepth(0.75f);
    b.setDepth(0.75f);

    a.seed(987654u);
    b.seed(987654u);

    a.reset(0.1f);
    b.reset(0.1f);

    for (int i = 0; i < 44100 * 4; ++i)
    {
      float va = a.next();
      float vb = b.next();
      T_ASSERT_NEAR(ctx, va, vb, 1e-6f);
    }
  }

  void test_drift_generator_parameter_effects(TestContext &ctx)
  {
    using ShortwavDSP::DriftGenerator;

    DriftGenerator slow;
    DriftGenerator fast;

    slow.setSampleRate(44100.0f);
    fast.setSampleRate(44100.0f);

    slow.seed(1u);
    fast.seed(1u);

    slow.reset(0.0f);
    fast.reset(0.0f);

    // Same depth, different rates: fast should exhibit more movement.
    slow.setDepth(1.0f);
    fast.setDepth(1.0f);

    slow.setRateHz(0.05f); // very slow
    fast.setRateHz(1.0f);  // faster

    float prevSlow = slow.next();
    float prevFast = fast.next();
    float accSlow = 0.0f;
    float accFast = 0.0f;
    const int N = 44100 * 6;

    for (int i = 1; i < N; ++i)
    {
      float vs = slow.next();
      float vf = fast.next();
      accSlow += std::fabs(vs - prevSlow);
      accFast += std::fabs(vf - prevFast);
      prevSlow = vs;
      prevFast = vf;
    }

    const float avgSlow = accSlow / (N - 1);
    const float avgFast = accFast / (N - 1);

    // Qualitative: faster rate should yield more average movement.
    T_ASSERT(ctx, avgFast > avgSlow * 1.2f);
  }

  void test_drift_generator_boundary_conditions(TestContext &ctx)
  {
    using ShortwavDSP::DriftGenerator;

    DriftGenerator drift;

    drift.setSampleRate(192000.0f); // high sample rate
    drift.setDepth(2.0f);
    drift.setRateHz(0.0001f); // extremely slow, near minimum
    drift.seed(321u);
    drift.reset(0.0f);

    for (int i = 0; i < 192000; ++i)
    {
      float v = drift.next();
      // Must remain finite and not blow up even with extreme settings.
      T_ASSERT(ctx, std::isfinite(v));
      T_ASSERT(ctx, v > -10.0f);
      T_ASSERT(ctx, v < 10.0f);
    }
  }

  // Entry point to run all tests when this TU is built as a standalone test binary.
  // In normal plugin builds this file is not included.
} // namespace

// Standalone test runner entry point.
// When including this translation unit directly in a test binary, define
// SHORTWAV_DSP_RUN_TESTS to provide a main() that executes all tests.
//
// This avoids conflicting with any host/plugin-defined main() when the file is
// merely compiled but not intended as the executable entry.
#ifdef SHORTWAV_DSP_RUN_TESTS
int main()
{
  ::TestContext ctx;

  // ChebyshevWaveshaper
  ::test_waveshaper_basic_linear(ctx);
  ::test_waveshaper_input_clamp_and_softclip(ctx);
  ::test_waveshaper_order_and_coefficients(ctx);
  ::test_waveshaper_process_buffer(ctx);
  ::test_waveshaper_invalid_params_and_denorm_guard(ctx);

  // RandomLFO
  ::test_randomlfo_basic_determinism(ctx);
  ::test_randomlfo_rate_and_phase_behavior(ctx);
  ::test_randomlfo_bipolar_unipolar_and_depth(ctx);
  ::test_randomlfo_smooth_parameter_effect(ctx);
  ::test_randomlfo_boundary_conditions(ctx);

  // DriftGenerator
  ::test_drift_generator_basic_finiteness(ctx);
  ::test_drift_generator_continuity(ctx);
  ::test_drift_generator_determinism(ctx);
  ::test_drift_generator_parameter_effects(ctx);
  ::test_drift_generator_boundary_conditions(ctx);

  ctx.summary();
  return (ctx.failed == 0) ? 0 : 1;
}
#endif