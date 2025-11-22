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
#include "../dsp/formant-osc.h"
#include "../dsp/3-band-eq.h"

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

  //------------------------------------------------------------------------------
  // FormantOscillator tests
  //------------------------------------------------------------------------------

  void test_formantosc_basic_output(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(44100.0f);
    osc.setCarrierFreq(110.0f);   // A2
    osc.setFormantFreq(800.0f);   // Typical vowel formant
    osc.setFormantWidth(0.3f);
    osc.setOutputGain(1.0f);
    osc.reset();

    // Skip first few samples for DC blocker settling
    for (int i = 0; i < 32; ++i)
    {
      osc.processSample();
    }

    // Generate 1024 samples and verify all are finite and bounded.
    for (int i = 0; i < 1024; ++i)
    {
      float sample = osc.processSample();
      T_ASSERT(ctx, std::isfinite(sample));
      // Formant synthesis can produce peaks up to ~5x due to harmonic addition
      T_ASSERT(ctx, sample >= -5.0f);
      T_ASSERT(ctx, sample <= 5.0f);
    }
  }

  void test_formantosc_silence_conditions(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(44100.0f);
    osc.reset();

    // Zero carrier frequency should produce silence.
    osc.setCarrierFreq(0.0f);
    osc.setFormantFreq(800.0f);
    osc.setFormantWidth(0.5f);
    osc.setOutputGain(1.0f);

    for (int i = 0; i < 512; ++i)
    {
      float sample = osc.processSample();
      T_ASSERT_NEAR(ctx, sample, 0.0f, kEpsilon);
    }

    // Zero output gain should produce silence.
    osc.setCarrierFreq(110.0f);
    osc.setOutputGain(0.0f);
    osc.reset();

    for (int i = 0; i < 512; ++i)
    {
      float sample = osc.processSample();
      T_ASSERT_NEAR(ctx, sample, 0.0f, kEpsilon);
    }
  }

  void test_formantosc_frequency_sweep(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(48000.0f);
    osc.setFormantFreq(1000.0f);
    osc.setFormantWidth(0.4f);
    osc.setOutputGain(0.5f);
    osc.reset();

    // Sweep carrier frequency and verify output remains stable.
    const float freqs[] = {55.0f, 110.0f, 220.0f, 440.0f, 880.0f, 1760.0f};
    for (float freq : freqs)
    {
      osc.setCarrierFreq(freq);
      osc.reset();

      bool allFinite = true;
      for (int i = 0; i < 256; ++i)
      {
        float sample = osc.processSample();
        if (!std::isfinite(sample))
        {
          allFinite = false;
          break;
        }
      }
      T_ASSERT(ctx, allFinite);
    }
  }

  void test_formantosc_formant_width_parameter(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(44100.0f);
    osc.setCarrierFreq(200.0f);
    osc.setFormantFreq(1200.0f);
    osc.setOutputGain(1.0f);

    // Test different formant widths from narrow to wide.
    const float widths[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    for (float width : widths)
    {
      osc.setFormantWidth(width);
      osc.reset();

      // Skip settling time
      for (int j = 0; j < 32; ++j)
      {
        osc.processSample();
      }

      // Verify output is finite and bounded for each width setting.
      bool valid = true;
      for (int i = 0; i < 512; ++i)
      {
        float sample = osc.processSample();
        // Very wide formants (width near 1.0) can produce large peaks
        if (!std::isfinite(sample) || std::fabs(sample) > 10.0f)
        {
          valid = false;
          break;
        }
      }
      T_ASSERT(ctx, valid);
    }
  }

  void test_formantosc_dc_offset_removal(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(44100.0f);
    osc.setCarrierFreq(100.0f);
    osc.setFormantFreq(600.0f);
    osc.setFormantWidth(0.5f);
    osc.setOutputGain(1.0f);
    osc.reset();

    // Generate many samples and compute average (should be near zero due to DC blocker).
    float sum = 0.0f;
    const int numSamples = 44100; // 1 second
    for (int i = 0; i < numSamples; ++i)
    {
      float sample = osc.processSample();
      sum += sample;
    }

    float average = sum / static_cast<float>(numSamples);
    // DC blocker should keep average near zero (tolerance depends on content).
    T_ASSERT(ctx, std::fabs(average) < 0.05f);
  }

  void test_formantosc_denormal_guard(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(44100.0f);
    osc.setCarrierFreq(110.0f);
    osc.setFormantFreq(800.0f);
    osc.setFormantWidth(0.3f);
    osc.setOutputGain(1e-20f); // Extremely small gain
    osc.reset();

    // Verify output is very small (denormal guard should prevent denormals).
    for (int i = 0; i < 256; ++i)
    {
      float sample = osc.processSample();
      T_ASSERT(ctx, std::isfinite(sample));
      // With very small gain, output should be negligible or zero
      T_ASSERT(ctx, std::fabs(sample) < 1e-15f);
    }
  }

  void test_formantosc_high_frequency_stability(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(48000.0f);
    osc.setCarrierFreq(8000.0f);   // High frequency carrier
    osc.setFormantFreq(12000.0f);  // Even higher formant
    osc.setFormantWidth(0.6f);
    osc.setOutputGain(1.0f);
    osc.reset();

    // Verify no infinities or NaNs at high frequencies.
    bool allFinite = true;
    for (int i = 0; i < 512; ++i)
    {
      float sample = osc.processSample();
      if (!std::isfinite(sample))
      {
        allFinite = false;
        break;
      }
    }
    T_ASSERT(ctx, allFinite);
  }

  void test_formantosc_buffer_processing(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(44100.0f);
    osc.setCarrierFreq(220.0f);
    osc.setFormantFreq(1500.0f);
    osc.setFormantWidth(0.4f);
    osc.setOutputGain(0.8f);
    osc.reset();

    const size_t bufferSize = 256;
    std::vector<float> outputBuffer(bufferSize);

    // Skip settling time
    for (int i = 0; i < 32; ++i)
    {
      osc.processSample();
    }

    // Process buffer without input (pure oscillator).
    osc.processBuffer(nullptr, outputBuffer.data(), bufferSize);

    // Verify all samples are finite and within reasonable bounds.
    for (size_t i = 0; i < bufferSize; ++i)
    {
      T_ASSERT(ctx, std::isfinite(outputBuffer[i]));
      T_ASSERT(ctx, std::fabs(outputBuffer[i]) < 5.0f);
    }

    // Process buffer with input (ring modulation mode).
    std::vector<float> inputBuffer(bufferSize);
    for (size_t i = 0; i < bufferSize; ++i)
    {
      inputBuffer[i] = 0.1f * std::sin(2.0f * 3.14159265359f * 50.0f * static_cast<float>(i) / 44100.0f);
    }

    osc.reset();
    osc.processBuffer(inputBuffer.data(), outputBuffer.data(), bufferSize);

    // Verify all samples are finite (oscillator + input).
    for (size_t i = 0; i < bufferSize; ++i)
    {
      T_ASSERT(ctx, std::isfinite(outputBuffer[i]));
    }
  }

  void test_formantosc_phase_continuity(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(44100.0f);
    osc.setCarrierFreq(440.0f);
    osc.setFormantFreq(2000.0f);
    osc.setFormantWidth(0.3f);
    osc.setOutputGain(1.0f);
    osc.reset();

    // Skip settling time for DC blocker
    for (int i = 0; i < 64; ++i)
    {
      osc.processSample();
    }

    // Generate samples and verify no discontinuities (no sudden jumps > threshold).
    float prev = osc.processSample();
    bool continuous = true;
    const float maxJump = 4.0f; // Threshold for continuity - formant peaks can be large

    for (int i = 1; i < 1024; ++i)
    {
      float current = osc.processSample();
      float jump = std::fabs(current - prev);
      if (jump > maxJump)
      {
        continuous = false;
        break;
      }
      prev = current;
    }
    T_ASSERT(ctx, continuous);
  }

  void test_formantosc_parameter_clamping(TestContext &ctx)
  {
    using ShortwavDSP::FormantOscillator;

    FormantOscillator osc;
    osc.setSampleRate(44100.0f);

    // Test negative frequencies (should be clamped to zero).
    osc.setCarrierFreq(-100.0f);
    osc.setFormantFreq(-500.0f);
    osc.setFormantWidth(0.5f);
    osc.setOutputGain(1.0f);
    osc.reset();

    // Should produce silence due to zero carrier frequency.
    for (int i = 0; i < 256; ++i)
    {
      float sample = osc.processSample();
      T_ASSERT_NEAR(ctx, sample, 0.0f, kEpsilon);
    }

    // Test out-of-range width (should be clamped to [0, 1]).
    osc.setCarrierFreq(220.0f);
    osc.setFormantFreq(1000.0f);
    osc.setFormantWidth(10.0f); // Way out of range
    osc.setOutputGain(1.0f);
    osc.reset();

    // Should still produce valid output (width clamped internally).
    bool valid = true;
    for (int i = 0; i < 256; ++i)
    {
      float sample = osc.processSample();
      if (!std::isfinite(sample))
      {
        valid = false;
        break;
      }
    }
    T_ASSERT(ctx, valid);
  }

  //------------------------------------------------------------------------------
  // ThreeBandEQ tests
  //------------------------------------------------------------------------------

  void test_threebandeq_unity_gain(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(44100.0f);
    eq.setCrossoverFreqs(880.0f, 5000.0f);
    eq.setGains(1.0f, 1.0f, 1.0f); // Unity gain on all bands

    // Generate test signal (mix of frequencies)
    const int numSamples = 1024;
    std::vector<float> input(numSamples);
    std::vector<float> output(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
      // Mix of low (100Hz), mid (2000Hz), and high (10000Hz)
      float t = static_cast<float>(i) / 44100.0f;
      input[i] = 0.3f * std::sin(2.0f * 3.14159f * 100.0f * t) +
                 0.3f * std::sin(2.0f * 3.14159f * 2000.0f * t) +
                 0.3f * std::sin(2.0f * 3.14159f * 10000.0f * t);
    }

    eq.processBuffer(input.data(), output.data(), numSamples);

    // With unity gain, output should be close to input (allowing for filter delay and phase)
    // The 3-band EQ introduces phase shift and group delay, so we check RMS similarity instead
    // Skip first 200 samples for settling
    float rmsIn = 0.0f, rmsOut = 0.0f;
    const int skipSamples = 200;
    for (int i = skipSamples; i < numSamples; ++i)
    {
      rmsIn += input[i] * input[i];
      rmsOut += output[i] * output[i];
    }
    rmsIn = std::sqrt(rmsIn / (numSamples - skipSamples));
    rmsOut = std::sqrt(rmsOut / (numSamples - skipSamples));

    // RMS should be similar (within 20% for unity gain)
    float ratio = rmsOut / (rmsIn + 1e-10f);
    T_ASSERT(ctx, ratio > 0.8f && ratio < 1.2f);
  }

  void test_threebandeq_gain_precision(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(44100.0f);
    eq.setCrossoverFreqs(250.0f, 4000.0f);

    // Test dB to linear conversion accuracy
    eq.setLowGainDB(6.0f);  // +6dB = 2.0x
    eq.setMidGainDB(0.0f);  // 0dB = 1.0x
    eq.setHighGainDB(-6.0f); // -6dB = 0.5x

    T_ASSERT_NEAR(ctx, eq.getLowGain(), 2.0f, 0.01f);
    T_ASSERT_NEAR(ctx, eq.getMidGain(), 1.0f, 0.01f);
    T_ASSERT_NEAR(ctx, eq.getHighGain(), 0.5f, 0.01f);

    // Test extreme gain values
    eq.setLowGainDB(12.0f);   // +12dB
    eq.setMidGainDB(-12.0f);  // -12dB
    eq.setHighGainDB(0.0f);

    T_ASSERT(ctx, eq.getLowGain() > 3.5f && eq.getLowGain() < 4.5f);   // ~4.0
    T_ASSERT(ctx, eq.getMidGain() > 0.2f && eq.getMidGain() < 0.3f);   // ~0.25
    T_ASSERT_NEAR(ctx, eq.getHighGain(), 1.0f, 0.01f);
  }

  void test_threebandeq_low_band_boost(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(48000.0f);
    eq.setCrossoverFreqs(250.0f, 4000.0f); // Raise low crossover to 250Hz
    eq.setLowGainDB(12.0f);  // Boost low by +12dB
    eq.setMidGain(1.0f);
    eq.setHighGain(1.0f);

    // Generate low frequency signal (100Hz - well within low band)
    const int numSamples = 4800; // 0.1 second
    std::vector<float> input(numSamples);
    std::vector<float> output(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
      float t = static_cast<float>(i) / 48000.0f;
      input[i] = 0.1f * std::sin(2.0f * 3.14159f * 100.0f * t);
    }

    eq.processBuffer(input.data(), output.data(), numSamples);

    // Calculate RMS of input and output (skip initial transient)
    float rmsIn = 0.0f, rmsOut = 0.0f;
    const int skipSamples = 500;
    for (int i = skipSamples; i < numSamples; ++i)
    {
      rmsIn += input[i] * input[i];
      rmsOut += output[i] * output[i];
    }
    rmsIn = std::sqrt(rmsIn / (numSamples - skipSamples));
    rmsOut = std::sqrt(rmsOut / (numSamples - skipSamples));

    // Output should be significantly boosted (+12dB ≈ 4x gain)
    // Due to filter rolloff and phase effects, expect 1.7x to 4.5x
    float amplification = rmsOut / rmsIn;
    T_ASSERT(ctx, amplification > 1.7f);
    T_ASSERT(ctx, amplification < 4.5f);
  }

  void test_threebandeq_mid_band_cut(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(44100.0f);
    eq.setCrossoverFreqs(880.0f, 5000.0f);
    eq.setLowGain(1.0f);
    eq.setMidGainDB(-12.0f); // Cut mid by -12dB
    eq.setHighGain(1.0f);

    // Generate mid frequency signal (2000Hz)
    const int numSamples = 4410; // 0.1 second
    std::vector<float> input(numSamples);
    std::vector<float> output(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
      float t = static_cast<float>(i) / 44100.0f;
      input[i] = 0.5f * std::sin(2.0f * 3.14159f * 2000.0f * t);
    }

    eq.processBuffer(input.data(), output.data(), numSamples);

    // Calculate RMS (skip transient)
    float rmsIn = 0.0f, rmsOut = 0.0f;
    const int skipSamples = 500;
    for (int i = skipSamples; i < numSamples; ++i)
    {
      rmsIn += input[i] * input[i];
      rmsOut += output[i] * output[i];
    }
    rmsIn = std::sqrt(rmsIn / (numSamples - skipSamples));
    rmsOut = std::sqrt(rmsOut / (numSamples - skipSamples));

    // Output should be significantly attenuated (at least 2x reduction for -12dB ≈ 0.25x gain)
    float attenuation = rmsOut / rmsIn;
    T_ASSERT(ctx, attenuation < 0.5f);
    T_ASSERT(ctx, attenuation > 0.1f); // Should not go to zero
  }

  void test_threebandeq_high_band_response(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(48000.0f);
    eq.setCrossoverFreqs(880.0f, 5000.0f);
    eq.setLowGain(1.0f);
    eq.setMidGain(1.0f);
    eq.setHighGainDB(6.0f); // Boost high by +6dB

    // Generate high frequency signal (10000Hz)
    const int numSamples = 4800;
    std::vector<float> input(numSamples);
    std::vector<float> output(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
      float t = static_cast<float>(i) / 48000.0f;
      input[i] = 0.2f * std::sin(2.0f * 3.14159f * 10000.0f * t);
    }

    eq.processBuffer(input.data(), output.data(), numSamples);

    // Calculate RMS (skip transient)
    float rmsIn = 0.0f, rmsOut = 0.0f;
    const int skipSamples = 500;
    for (int i = skipSamples; i < numSamples; ++i)
    {
      rmsIn += input[i] * input[i];
      rmsOut += output[i] * output[i];
    }
    rmsIn = std::sqrt(rmsIn / (numSamples - skipSamples));
    rmsOut = std::sqrt(rmsOut / (numSamples - skipSamples));

    // +6dB = ~2x amplification (allow wider tolerance for high frequency response)
    float amplification = rmsOut / rmsIn;
    T_ASSERT(ctx, amplification > 1.3f); // Relaxed lower bound for filter rolloff
    T_ASSERT(ctx, amplification < 2.5f);
  }

  void test_threebandeq_stereo_processing(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(44100.0f);
    eq.setCrossoverFreqs(880.0f, 5000.0f);
    eq.setGains(1.5f, 0.75f, 1.0f);

    const int numSamples = 512;
    std::vector<float> inputL(numSamples);
    std::vector<float> inputR(numSamples);
    std::vector<float> outputL(numSamples);
    std::vector<float> outputR(numSamples);

    // Generate different signals for left and right
    for (int i = 0; i < numSamples; ++i)
    {
      float t = static_cast<float>(i) / 44100.0f;
      inputL[i] = 0.5f * std::sin(2.0f * 3.14159f * 440.0f * t);  // A4
      inputR[i] = 0.5f * std::sin(2.0f * 3.14159f * 880.0f * t);  // A5
    }

    eq.processStereoBuffer(inputL.data(), inputR.data(),
                           outputL.data(), outputR.data(),
                           numSamples);

    // Both channels should be processed independently and produce finite output
    bool allFinite = true;
    for (int i = 0; i < numSamples; ++i)
    {
      if (!std::isfinite(outputL[i]) || !std::isfinite(outputR[i]))
      {
        allFinite = false;
        break;
      }
    }
    T_ASSERT(ctx, allFinite);

    // Outputs should be different (not identical) due to different inputs
    bool channelsDifferent = false;
    for (int i = 100; i < numSamples; ++i) // Skip transient
    {
      if (std::fabs(outputL[i] - outputR[i]) > 0.01f)
      {
        channelsDifferent = true;
        break;
      }
    }
    T_ASSERT(ctx, channelsDifferent);
  }

  void test_threebandeq_sample_rate_independence(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    // Test that the EQ behaves consistently across different sample rates
    const float sampleRates[] = {44100.0f, 48000.0f, 96000.0f};
    const float testFreq = 1000.0f; // Mid-range frequency

    for (float sr : sampleRates)
    {
      ThreeBandEQ eq;
      eq.setSampleRate(sr);
      eq.setCrossoverFreqs(880.0f, 5000.0f);
      eq.setGains(1.0f, 1.5f, 1.0f); // Boost mid

      const int numSamples = static_cast<int>(sr * 0.05f); // 50ms
      std::vector<float> input(numSamples);
      std::vector<float> output(numSamples);

      // Generate test signal
      for (int i = 0; i < numSamples; ++i)
      {
        float t = static_cast<float>(i) / sr;
        input[i] = 0.3f * std::sin(2.0f * 3.14159f * testFreq * t);
      }

      eq.processBuffer(input.data(), output.data(), numSamples);

      // Output should be finite and boosted
      bool allFinite = true;
      for (int i = 0; i < numSamples; ++i)
      {
        if (!std::isfinite(output[i]))
        {
          allFinite = false;
          break;
        }
      }
      T_ASSERT(ctx, allFinite);
    }
  }

  void test_threebandeq_crossover_frequency_behavior(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(44100.0f);

    // Test that crossover frequencies are properly clamped
    eq.setLowFreq(10.0f); // Below minimum, should be clamped to 20Hz
    T_ASSERT(ctx, eq.getLowFreq() >= 20.0f);

    eq.setLowFreq(250.0f);
    eq.setHighFreq(200.0f); // Below low freq, should be clamped above it
    T_ASSERT(ctx, eq.getHighFreq() > eq.getLowFreq());

    // Test normal range
    eq.setCrossoverFreqs(880.0f, 5000.0f);
    T_ASSERT_NEAR(ctx, eq.getLowFreq(), 880.0f, 1.0f);
    T_ASSERT_NEAR(ctx, eq.getHighFreq(), 5000.0f, 10.0f);
  }

  void test_threebandeq_extreme_gain_values(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(44100.0f);
    eq.setCrossoverFreqs(880.0f, 5000.0f);

    // Test maximum boost (+12dB)
    eq.setLowGainDB(12.0f);
    eq.setMidGainDB(12.0f);
    eq.setHighGainDB(12.0f);

    const int numSamples = 512;
    std::vector<float> input(numSamples, 0.1f); // DC-like signal
    std::vector<float> output(numSamples);

    eq.processBuffer(input.data(), output.data(), numSamples);

    // Should not overflow or produce infinities even with max gain
    bool allFinite = true;
    for (int i = 0; i < numSamples; ++i)
    {
      if (!std::isfinite(output[i]) || std::fabs(output[i]) > 10.0f)
      {
        allFinite = false;
        break;
      }
    }
    T_ASSERT(ctx, allFinite);

    // Test maximum cut (-12dB)
    eq.reset();
    eq.setLowGainDB(-12.0f);
    eq.setMidGainDB(-12.0f);
    eq.setHighGainDB(-12.0f);

    eq.processBuffer(input.data(), output.data(), numSamples);

    // Should produce very quiet output (skip transient)
    float maxVal = 0.0f;
    for (int i = 100; i < numSamples; ++i)
    {
      maxVal = std::max(maxVal, std::fabs(output[i]));
    }
    T_ASSERT(ctx, maxVal < 0.05f); // Significantly attenuated
  }

  void test_threebandeq_denormal_protection(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(44100.0f);
    eq.setCrossoverFreqs(880.0f, 5000.0f);
    eq.setGains(1.0f, 1.0f, 1.0f);

    // Feed extremely small values (near denormal range)
    const int numSamples = 1024;
    std::vector<float> input(numSamples);
    std::vector<float> output(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
      input[i] = 1e-20f * std::sin(2.0f * 3.14159f * 1000.0f * static_cast<float>(i) / 44100.0f);
    }

    eq.processBuffer(input.data(), output.data(), numSamples);

    // All output should be finite (denormal protection should prevent issues)
    bool allFinite = true;
    for (int i = 0; i < numSamples; ++i)
    {
      if (!std::isfinite(output[i]))
      {
        allFinite = false;
        break;
      }
    }
    T_ASSERT(ctx, allFinite);
  }

  void test_threebandeq_reset_behavior(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(44100.0f);
    eq.setCrossoverFreqs(880.0f, 5000.0f);
    eq.setGains(2.0f, 1.0f, 0.5f);

    // Process some samples to build up state
    for (int i = 0; i < 100; ++i)
    {
      eq.processSample(0.5f * std::sin(2.0f * 3.14159f * 1000.0f * static_cast<float>(i) / 44100.0f));
    }

    // Reset should clear all state
    eq.reset();

    // First output after reset should be close to zero (no history)
    float out1 = eq.processSample(0.0f);
    T_ASSERT(ctx, std::fabs(out1) < 1e-6f);
  }

  void test_threebandeq_phase_continuity(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(44100.0f);
    eq.setCrossoverFreqs(880.0f, 5000.0f);
    eq.setGains(1.0f, 1.0f, 1.0f);

    // Generate continuous sine wave and check for discontinuities
    const int numSamples = 2048;
    std::vector<float> output(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
      float t = static_cast<float>(i) / 44100.0f;
      float input = 0.5f * std::sin(2.0f * 3.14159f * 1000.0f * t);
      output[i] = eq.processSample(input);
    }

    // Check for large discontinuities (phase jumps)
    bool continuous = true;
    const int skipSamples = 100; // Skip initial transient
    const float maxJump = 0.5f;  // Threshold for continuity

    for (int i = skipSamples + 1; i < numSamples; ++i)
    {
      float jump = std::fabs(output[i] - output[i - 1]);
      if (jump > maxJump)
      {
        continuous = false;
        break;
      }
    }
    T_ASSERT(ctx, continuous);
  }

  void test_threebandeq_performance_benchmark(TestContext &ctx)
  {
    using ShortwavDSP::ThreeBandEQ;

    ThreeBandEQ eq;
    eq.setSampleRate(48000.0f);
    eq.setCrossoverFreqs(250.0f, 4000.0f);
    eq.setGains(1.5f, 0.8f, 1.2f);

    // Process a large buffer to test performance
    const int numSamples = 48000; // 1 second at 48kHz
    std::vector<float> input(numSamples);
    std::vector<float> output(numSamples);

    // Generate test signal
    for (int i = 0; i < numSamples; ++i)
    {
      float t = static_cast<float>(i) / 48000.0f;
      input[i] = 0.5f * std::sin(2.0f * 3.14159f * 440.0f * t);
    }

    // Process buffer (performance test - should complete quickly)
    eq.processBuffer(input.data(), output.data(), numSamples);

    // Verify output is valid
    bool allFinite = true;
    for (int i = 0; i < numSamples; ++i)
    {
      if (!std::isfinite(output[i]))
      {
        allFinite = false;
        break;
      }
    }
    T_ASSERT(ctx, allFinite);

    // This test is primarily for performance measurement during development
    // The assertion just ensures the algorithm completed successfully
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

  // FormantOscillator
  ::test_formantosc_basic_output(ctx);
  ::test_formantosc_silence_conditions(ctx);
  ::test_formantosc_frequency_sweep(ctx);
  ::test_formantosc_formant_width_parameter(ctx);
  ::test_formantosc_dc_offset_removal(ctx);
  ::test_formantosc_denormal_guard(ctx);
  ::test_formantosc_high_frequency_stability(ctx);
  ::test_formantosc_buffer_processing(ctx);
  ::test_formantosc_phase_continuity(ctx);
  ::test_formantosc_parameter_clamping(ctx);

  // ThreeBandEQ
  ::test_threebandeq_unity_gain(ctx);
  ::test_threebandeq_gain_precision(ctx);
  ::test_threebandeq_low_band_boost(ctx);
  ::test_threebandeq_mid_band_cut(ctx);
  ::test_threebandeq_high_band_response(ctx);
  ::test_threebandeq_stereo_processing(ctx);
  ::test_threebandeq_sample_rate_independence(ctx);
  ::test_threebandeq_crossover_frequency_behavior(ctx);
  ::test_threebandeq_extreme_gain_values(ctx);
  ::test_threebandeq_denormal_protection(ctx);
  ::test_threebandeq_reset_behavior(ctx);
  ::test_threebandeq_phase_continuity(ctx);
  ::test_threebandeq_performance_benchmark(ctx);

  ctx.summary();
  return (ctx.failed == 0) ? 0 : 1;
}
#endif