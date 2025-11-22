#pragma once

#include <cmath>
#include <algorithm>
#include <cstddef>
#include <cstdint>

/*
 * Three-Band Equalizer
 *
 * High-quality 3-band equalizer with adjustable crossover frequencies and gains.
 * 
 * Based on the algorithm from:
 * https://www.musicdsp.org/en/latest/Filters/236-3-band-equaliser.html
 * Original algorithm by Paul Kellet, implementation by Neil C / Etanza Systems
 *
 * Features:
 *  - Three frequency bands: Low, Mid, High
 *  - Adjustable crossover frequencies (low/mid and mid/high boundaries)
 *  - Independent gain control for each band (-12dB to +12dB)
 *  - Stereo processing support
 *  - Real-time safe (no allocations, no locks)
 *  - Denormal protection
 *  - Sample rate independent
 *
 * Architecture:
 *  - Two 4-pole cascaded single-pole filters (24dB/octave)
 *  - Filter #1: Lowpass (extracts low band)
 *  - Filter #2: Highpass (extracts high band)
 *  - Mid band computed as: input - (low + high)
 *
 * Frequency Ranges (typical):
 *  - Low band:  0 Hz to lowFreq (80-250 Hz recommended)
 *  - Mid band:  lowFreq to highFreq (1-4 kHz recommended)
 *  - High band: highFreq to Nyquist (8-20 kHz recommended)
 *
 * Usage:
 *  ThreeBandEQ eq;
 *  eq.setSampleRate(48000.0f);
 *  eq.setLowFreq(880.0f);      // Low/Mid crossover
 *  eq.setHighFreq(5000.0f);    // Mid/High crossover
 *  eq.setLowGain(1.5f);        // +3.5 dB boost
 *  eq.setMidGain(0.75f);       // -2.5 dB cut
 *  eq.setHighGain(1.0f);       // 0 dB (unity)
 *  
 *  float out = eq.processSample(input);
 *  // or
 *  eq.processBuffer(inL, inR, outL, outR, numSamples);
 */

namespace ShortwavDSP
{

  namespace detail
  {
    // Convert dB to linear gain: dB = 20 * log10(gain) => gain = 10^(dB/20)
    inline float dBToGain(float dB) noexcept
    {
      return std::pow(10.0f, dB / 20.0f);
    }

    // Convert linear gain to dB: dB = 20 * log10(gain)
    inline float gainToDB(float gain) noexcept
    {
      return 20.0f * std::log10(std::max(gain, 1e-10f));
    }

    // Clamp value to range
    inline float clamp(float value, float minVal, float maxVal) noexcept
    {
      return std::max(minVal, std::min(maxVal, value));
    }

    // Denormal fix constant (very small amount to prevent denormals)
    constexpr float kVSA = 1.0f / 4294967295.0f;
  }

  //------------------------------------------------------------------------------
  // ThreeBandEQ - Single channel state
  //------------------------------------------------------------------------------

  class ThreeBandEQChannel
  {
  public:
    ThreeBandEQChannel() noexcept
    {
      reset();
    }

    // Reset all filter state to zero
    void reset() noexcept
    {
      // Filter #1 poles (lowpass)
      f1p0_ = 0.0f;
      f1p1_ = 0.0f;
      f1p2_ = 0.0f;
      f1p3_ = 0.0f;

      // Filter #2 poles (highpass)
      f2p0_ = 0.0f;
      f2p1_ = 0.0f;
      f2p2_ = 0.0f;
      f2p3_ = 0.0f;

      // Sample history buffer (for highpass calculation)
      sdm1_ = 0.0f;
      sdm2_ = 0.0f;
      sdm3_ = 0.0f;
    }

    // Process a single sample
    // Returns the equalized output
    float processSample(float sample, float lf, float hf, float lg, float mg, float hg) noexcept
    {
      // Filter #1 (lowpass) - 4 cascaded single-pole filters
      // Each stage: y[n] = y[n-1] + lf * (x[n] - y[n-1])
      f1p0_ += (lf * (sample - f1p0_)) + detail::kVSA;
      f1p1_ += (lf * (f1p0_ - f1p1_));
      f1p2_ += (lf * (f1p1_ - f1p2_));
      f1p3_ += (lf * (f1p2_ - f1p3_));

      const float l = f1p3_;

      // Filter #2 (highpass) - 4 cascaded single-pole filters
      // High component extracted from delayed input minus lowpass
      f2p0_ += (hf * (sample - f2p0_)) + detail::kVSA;
      f2p1_ += (hf * (f2p0_ - f2p1_));
      f2p2_ += (hf * (f2p1_ - f2p2_));
      f2p3_ += (hf * (f2p2_ - f2p3_));

      const float h = sdm3_ - f2p3_;

      // Calculate midrange (original signal minus low and high components)
      const float m = sdm3_ - (h + l);

      // Scale by gains
      const float lOut = l * lg;
      const float mOut = m * mg;
      const float hOut = h * hg;

      // Shuffle history buffer (3-sample delay for highpass)
      sdm3_ = sdm2_;
      sdm2_ = sdm1_;
      sdm1_ = sample;

      // Combine bands and return
      return lOut + mOut + hOut;
    }

  private:
    // Filter #1 state (lowpass)
    float f1p0_, f1p1_, f1p2_, f1p3_;

    // Filter #2 state (highpass)
    float f2p0_, f2p1_, f2p2_, f2p3_;

    // Sample delay memory (for highpass calculation)
    float sdm1_, sdm2_, sdm3_;
  };

  //------------------------------------------------------------------------------
  // ThreeBandEQ - Main equalizer class
  //------------------------------------------------------------------------------

  class ThreeBandEQ
  {
  public:
    ThreeBandEQ() noexcept
        : sampleRate_(44100.0f),
          lowFreq_(880.0f),
          highFreq_(5000.0f),
          lowGain_(1.0f),
          midGain_(1.0f),
          highGain_(1.0f)
    {
      updateFilterCoefficients();
    }

    //--------------------------------------------------------------------------
    // Configuration
    //--------------------------------------------------------------------------

    // Set sample rate (call once at initialization or when rate changes)
    void setSampleRate(float sampleRate) noexcept
    {
      sampleRate_ = std::max(1.0f, sampleRate);
      updateFilterCoefficients();
    }

    // Set low/mid crossover frequency (Hz)
    // Recommended range: 80-250 Hz
    void setLowFreq(float freq) noexcept
    {
      lowFreq_ = detail::clamp(freq, 20.0f, sampleRate_ * 0.4f);
      updateFilterCoefficients();
    }

    // Set mid/high crossover frequency (Hz)
    // Recommended range: 1000-4000 Hz (1-4 kHz)
    void setHighFreq(float freq) noexcept
    {
      highFreq_ = detail::clamp(freq, lowFreq_ + 100.0f, sampleRate_ * 0.45f);
      updateFilterCoefficients();
    }

    // Set crossover frequencies in one call
    void setCrossoverFreqs(float lowFreq, float highFreq) noexcept
    {
      lowFreq_ = detail::clamp(lowFreq, 20.0f, sampleRate_ * 0.4f);
      highFreq_ = detail::clamp(highFreq, lowFreq_ + 100.0f, sampleRate_ * 0.45f);
      updateFilterCoefficients();
    }

    // Set low band gain (linear)
    // Use 1.0 for unity, >1.0 for boost, <1.0 for cut
    // Range: 0.25 (-12dB) to 4.0 (+12dB) recommended
    void setLowGain(float gain) noexcept
    {
      lowGain_ = detail::clamp(gain, 0.0f, 10.0f);
    }

    // Set mid band gain (linear)
    void setMidGain(float gain) noexcept
    {
      midGain_ = detail::clamp(gain, 0.0f, 10.0f);
    }

    // Set high band gain (linear)
    void setHighGain(float gain) noexcept
    {
      highGain_ = detail::clamp(gain, 0.0f, 10.0f);
    }

    // Set low band gain in dB
    // Range: -12dB to +12dB recommended
    void setLowGainDB(float dB) noexcept
    {
      setLowGain(detail::dBToGain(detail::clamp(dB, -24.0f, 24.0f)));
    }

    // Set mid band gain in dB
    void setMidGainDB(float dB) noexcept
    {
      setMidGain(detail::dBToGain(detail::clamp(dB, -24.0f, 24.0f)));
    }

    // Set high band gain in dB
    void setHighGainDB(float dB) noexcept
    {
      setHighGain(detail::dBToGain(detail::clamp(dB, -24.0f, 24.0f)));
    }

    // Set all gains at once (linear)
    void setGains(float lowGain, float midGain, float highGain) noexcept
    {
      setLowGain(lowGain);
      setMidGain(midGain);
      setHighGain(highGain);
    }

    // Set all gains at once (dB)
    void setGainsDB(float lowDB, float midDB, float highDB) noexcept
    {
      setLowGainDB(lowDB);
      setMidGainDB(midDB);
      setHighGainDB(highDB);
    }

    //--------------------------------------------------------------------------
    // Getters
    //--------------------------------------------------------------------------

    float getSampleRate() const noexcept { return sampleRate_; }
    float getLowFreq() const noexcept { return lowFreq_; }
    float getHighFreq() const noexcept { return highFreq_; }
    float getLowGain() const noexcept { return lowGain_; }
    float getMidGain() const noexcept { return midGain_; }
    float getHighGain() const noexcept { return highGain_; }
    float getLowGainDB() const noexcept { return detail::gainToDB(lowGain_); }
    float getMidGainDB() const noexcept { return detail::gainToDB(midGain_); }
    float getHighGainDB() const noexcept { return detail::gainToDB(highGain_); }

    //--------------------------------------------------------------------------
    // Reset
    //--------------------------------------------------------------------------

    // Reset all filter state (call when starting/stopping audio or on glitches)
    void reset() noexcept
    {
      leftChannel_.reset();
      rightChannel_.reset();
    }

    //--------------------------------------------------------------------------
    // Processing
    //--------------------------------------------------------------------------

    // Process a single mono sample
    float processSample(float sample) noexcept
    {
      return leftChannel_.processSample(sample, lf_, hf_, lowGain_, midGain_, highGain_);
    }

    // Process a single stereo sample pair
    void processStereoSample(float &left, float &right) noexcept
    {
      left = leftChannel_.processSample(left, lf_, hf_, lowGain_, midGain_, highGain_);
      right = rightChannel_.processSample(right, lf_, hf_, lowGain_, midGain_, highGain_);
    }

    // Process a buffer of mono samples
    void processBuffer(const float *input, float *output, size_t numSamples) noexcept
    {
      for (size_t i = 0; i < numSamples; ++i)
      {
        output[i] = processSample(input[i]);
      }
    }

    // Process a buffer of interleaved stereo samples
    void processStereoBufferInterleaved(const float *input, float *output, size_t numFrames) noexcept
    {
      for (size_t i = 0; i < numFrames; ++i)
      {
        const size_t idx = i * 2;
        float left = input[idx];
        float right = input[idx + 1];
        processStereoSample(left, right);
        output[idx] = left;
        output[idx + 1] = right;
      }
    }

    // Process separate left/right buffers (non-interleaved stereo)
    void processStereoBuffer(const float *inputL, const float *inputR,
                             float *outputL, float *outputR,
                             size_t numSamples) noexcept
    {
      for (size_t i = 0; i < numSamples; ++i)
      {
        outputL[i] = leftChannel_.processSample(inputL[i], lf_, hf_, lowGain_, midGain_, highGain_);
        outputR[i] = rightChannel_.processSample(inputR[i], lf_, hf_, lowGain_, midGain_, highGain_);
      }
    }

  private:
    // Update filter coefficients based on current frequencies and sample rate
    void updateFilterCoefficients() noexcept
    {
      // Calculate filter cutoff frequencies using prewarped frequency mapping
      // lf and hf are the normalized cutoff frequencies for single-pole filters
      // Formula: 2 * sin(PI * (freq / sampleRate))
      const float pi = 3.14159265358979323846f;
      lf_ = 2.0f * std::sin(pi * (lowFreq_ / sampleRate_));
      hf_ = 2.0f * std::sin(pi * (highFreq_ / sampleRate_));

      // Clamp to valid range (0, 2) for stability
      lf_ = detail::clamp(lf_, 0.0001f, 1.99f);
      hf_ = detail::clamp(hf_, 0.0001f, 1.99f);
    }

    // Configuration
    float sampleRate_;
    float lowFreq_;  // Low/Mid crossover
    float highFreq_; // Mid/High crossover

    // Gains (linear)
    float lowGain_;
    float midGain_;
    float highGain_;

    // Filter coefficients (computed from frequencies)
    float lf_; // Lowpass coefficient
    float hf_; // Highpass coefficient

    // Channel state (stereo)
    ThreeBandEQChannel leftChannel_;
    ThreeBandEQChannel rightChannel_;
  };

} // namespace ShortwavDSP
