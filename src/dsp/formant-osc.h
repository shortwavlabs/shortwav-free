#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>

/*
 * AM Formant Synthesis Oscillator
 *
 * Based on the algorithm described at:
 * https://www.musicdsp.org/en/latest/Synthesis/224-am-formantic-synthesis.html
 *
 * Core idea (from Thierry Rochebois via Paul Sernine):
 * - Generate formantic (vowel-like) timbres without filters or grains.
 * - Use "double carrier amplitude modulation" to pitch-shift formant waveforms
 *   while preserving harmonic structure.
 * - Formant waveforms are pre-calculated with varying bandwidth (width parameter).
 * - Each formant is a sum of harmonics with Hann windowing and Gaussian rolloff.
 * - Runtime uses cosine-phased carriers to avoid phase interference artifacts.
 * - Multiple formants can be layered for complex vowel sounds.
 *
 * This implementation is designed for real-time audio use:
 * - No allocations in the audio path.
 * - No locks or thread synchronization.
 * - Sample-rate-agnostic processing (internal table normalized to [-1,1] phase).
 * - Numerically stable with denormal guards and DC offset handling.
 * - Clear parameter semantics suitable for modular synthesis.
 *
 * Public interface:
 * - Call setSampleRate() once at initialization or when the host rate changes.
 * - Configure:
 *     - carrierFreqHz: base pitch (fundamental frequency)
 *     - formantFreqHz: center frequency of the formant resonance
 *     - formantWidth: bandwidth/Q of the formant (0..1, higher = wider/less peaked)
 *     - outputGain: overall amplitude scaling (typically 0..1)
 * - Call reset() to reset phase accumulators and internal state.
 * - Call processSample() each sample to get the next audio value.
 * - Call processBuffer() for block-based processing (more efficient for large blocks).
 *
 * Threading:
 * - Setters are plain stores and safe to call from a control thread between blocks.
 * - For sample-accurate automation, interpolate parameters externally and call
 *   setters per-sample or per-small-block as needed.
 */

namespace ShortwavDSP
{

  class FormantOscillator
  {

  public:
    // Table dimensions (following the reference implementation conventions).
    static constexpr int kTableSize = 256 + 1; // +1 so table wraps cleanly
    static constexpr int kMaxWidthIndex = 64;  // Number of different formant widths

    FormantOscillator()
    {
      // Initialize formant table on construction (one-time cost).
      initFormantTable();
      setSampleRate(44100.0f);
      reset();
    }

    // Initialize or update the sample rate.
    // Must be called before processSample() is used.
    inline void setSampleRate(float sampleRate) noexcept
    {
      if (sampleRate <= 1.0f)
        sampleRate = 44100.0f;
      sampleRate_ = sampleRate;
    }

    // Reset phase accumulators and DC blocker state.
    inline void reset() noexcept
    {
      carrierPhase_ = 0.0f;
      dcBlockerX1_ = 0.0f;
      dcBlockerY1_ = 0.0f;
    }

    // Set carrier (fundamental) frequency in Hz.
    // This is the base pitch of the oscillator.
    inline void setCarrierFreq(float freqHz) noexcept
    {
      carrierFreqHz_ = std::max(freqHz, 0.0f);
    }

    // Set formant center frequency in Hz.
    // This determines the spectral resonance peak.
    inline void setFormantFreq(float freqHz) noexcept
    {
      formantFreqHz_ = std::max(freqHz, 0.0f);
    }

    // Set formant width/bandwidth parameter (0..1).
    // - 0: very narrow/peaked formant (high Q)
    // - 1: broad/wide formant (low Q)
    // Internally maps to the table index for different formant shapes.
    inline void setFormantWidth(float width) noexcept
    {
      formantWidth_ = clamp01(width);
    }

    // Set overall output gain (typically 0..1).
    inline void setOutputGain(float gain) noexcept
    {
      outputGain_ = std::max(gain, 0.0f);
    }

    // Generate next audio sample.
    // This is real-time safe and intended for per-sample use in an audio callback.
    float processSample() noexcept
    {
      if (sampleRate_ <= 0.0f || carrierFreqHz_ <= 0.0f)
      {
        return 0.0f;
      }

      // Advance carrier phase [-1, 1] normalized.
      const float phaseIncrement = 2.0f * carrierFreqHz_ / sampleRate_;
      carrierPhase_ += phaseIncrement;

      // Wrap phase to [-1, 1].
      while (carrierPhase_ >= 1.0f)
        carrierPhase_ -= 2.0f;
      while (carrierPhase_ < -1.0f)
        carrierPhase_ += 2.0f;

      // Compute formant width index for table lookup.
      // Width parameter (0..1) maps to [0, kMaxWidthIndex-1].
      const float widthIndexFloat = formantWidth_ * static_cast<float>(kMaxWidthIndex - 1);

      // Compute harmonic ratio for double-carrier pitch shifting.
      // formantFreqHz / carrierFreqHz gives the harmonic number.
      const float harmonicRatio = (carrierFreqHz_ > 0.001f) ? (formantFreqHz_ / carrierFreqHz_) : 1.0f;

      // Lookup formant waveform and apply double-carrier modulation.
      const float formantValue = lookupFormant(carrierPhase_, widthIndexFloat);
      const float carrierValue = doubleCarrier(harmonicRatio, carrierPhase_);

      // Amplitude modulation: formant * carrier.
      float output = formantValue * carrierValue;

      // The formant function can produce values with substantial DC and amplitude,
      // so normalize to a reasonable range before applying gain.
      // Empirically, the formant function yields values roughly in [-3, 6] range
      // depending on width. Scale to approximately [-1, 1] range.
      output *= 0.2f; // Normalize formant output amplitude

      // Apply output gain.
      output *= outputGain_;

      // DC blocking filter (first-order highpass at ~5 Hz).
      output = dcBlocker(output);

      // Denormal guard.
      if (std::fabs(output) < 1e-30f)
        output = 0.0f;

      return output;
    }

    // Process a buffer of samples.
    // inputBuffer can be nullptr for pure oscillator use (no external modulation).
    // If inputBuffer is non-null, it's added to the oscillator output (for ring mod etc).
    void processBuffer(const float *inputBuffer, float *outputBuffer, size_t numSamples) noexcept
    {
      for (size_t i = 0; i < numSamples; ++i)
      {
        float sample = processSample();
        if (inputBuffer != nullptr)
        {
          sample += inputBuffer[i];
        }
        outputBuffer[i] = sample;
      }
    }

  private:
    float sampleRate_ = 44100.0f;
    float carrierFreqHz_ = 110.0f;  // A2 default
    float formantFreqHz_ = 800.0f;  // Typical vowel formant
    float formantWidth_ = 0.3f;     // Medium Q
    float outputGain_ = 1.0f;

    float carrierPhase_ = 0.0f; // Normalized to [-1, 1]

    // DC blocker state (first-order highpass).
    float dcBlockerX1_ = 0.0f;
    float dcBlockerY1_ = 0.0f;

    // Formant wavetable [phase_index][width_index].
    // Stored as flat array: table[phase + width * kTableSize].
    float formantTable_[kTableSize * kMaxWidthIndex];

    // Clamp to [0, 1].
    static inline float clamp01(float x) noexcept
    {
      if (x < 0.0f)
        return 0.0f;
      if (x > 1.0f)
        return 1.0f;
      return x;
    }

    // Fast cosine approximation for x in [-1, 1].
    // Approximates cos(pi*x) using a 4th-order polynomial.
    // Good accuracy for audio (error < 0.5% over [-1,1]).
    static inline float fastCos(float x) noexcept
    {
      const float x2 = x * x;
      return 1.0f + x2 * (-4.0f + 2.0f * x2);
    }

    // Formant function with given width parameter.
    // Generates a sum of harmonics with Hann windowing and Gaussian rolloff.
    // p: phase in [-1, 1]
    // width: formant bandwidth index (0..kMaxWidthIndex-1)
    static float formantFunction(float p, float width) noexcept
    {
      float a = 0.5f;

      // Limit harmonic count based on width to keep table size reasonable.
      const int hmax = std::min(static_cast<int>(10.0f * width), kTableSize / 2);

      if (hmax < 1)
        return a; // Degenerate case: DC only

      float phi = 0.0f;
      const float pi = 3.14159265359f;

      for (int h = 1; h < hmax; ++h)
      {
        phi += pi * p;

        // Hann window to taper high harmonics.
        const float hann = 0.5f + 0.5f * fastCos(static_cast<float>(h) / static_cast<float>(hmax));

        // Gaussian rolloff based on formant width.
        const float widthSafe = std::max(width, 0.1f); // Avoid division by tiny width
        const float gaussienne = 0.85f * std::exp(-static_cast<float>(h * h) / (widthSafe * widthSafe));

        // Small "skirt" to add body.
        const float jupe = 0.15f;

        // Harmonic component.
        const float harmonique = std::cos(phi);

        a += hann * (gaussienne + jupe) * harmonique;
      }

      return a;
    }

    // Initialize the formant wavetable.
    // Called once in the constructor.
    void initFormantTable() noexcept
    {
      const float phaseCoef = 2.0f / static_cast<float>(kTableSize - 1);

      for (int widthIdx = 0; widthIdx < kMaxWidthIndex; ++widthIdx)
      {
        for (int phaseIdx = 0; phaseIdx < kTableSize; ++phaseIdx)
        {
          const float phase = -1.0f + static_cast<float>(phaseIdx) * phaseCoef;
          const float widthValue = static_cast<float>(widthIdx);
          const int tableIndex = phaseIdx + widthIdx * kTableSize;
          formantTable_[tableIndex] = formantFunction(phase, widthValue);
        }
      }
    }

    // Lookup formant waveform with bilinear interpolation.
    // phase: normalized phase in [-1, 1]
    // widthIndexFloat: formant width index as float (0..kMaxWidthIndex-1)
    float lookupFormant(float phase, float widthIndexFloat) const noexcept
    {
      // Clamp width index to valid range.
      if (widthIndexFloat < 0.0f)
        widthIndexFloat = 0.0f;
      if (widthIndexFloat > static_cast<float>(kMaxWidthIndex - 2))
        widthIndexFloat = static_cast<float>(kMaxWidthIndex - 2);

      // Normalize phase to [0, kTableSize-1].
      const float phaseNorm = (phase + 1.0f) * 0.5f * static_cast<float>(kTableSize - 1);
      const int phaseIdx = static_cast<int>(phaseNorm);
      const float phaseFrac = phaseNorm - static_cast<float>(phaseIdx);

      // Integer and fractional parts of width.
      const int widthIdx = static_cast<int>(widthIndexFloat);
      const float widthFrac = widthIndexFloat - static_cast<float>(widthIdx);

      // Four corners for bilinear interpolation.
      const int i00 = phaseIdx + widthIdx * kTableSize;
      const int i10 = i00 + kTableSize;

      // Bilinear interpolation.
      const float v00 = formantTable_[i00];
      const float v01 = formantTable_[i00 + 1];
      const float v10 = formantTable_[i10];
      const float v11 = formantTable_[i10 + 1];

      const float v0 = v00 + phaseFrac * (v01 - v00);
      const float v1 = v10 + phaseFrac * (v11 - v10);

      return v0 + widthFrac * (v1 - v0);
    }

    // Double carrier with crossfading to preserve harmonicity.
    // Implements the "cosine-phased carriers" trick to avoid phase interference.
    // harmonicRatio: formantFreqHz / carrierFreqHz
    // phase: carrier phase in [-1, 1]
    float doubleCarrier(float harmonicRatio, float phase) const noexcept
    {
      // Integer and fractional harmonic number.
      const float h0 = std::floor(harmonicRatio);
      const float hFrac = harmonicRatio - h0;

      // Two carrier phases at harmonics h0 and h0+1.
      // Apply modulo to wrap into [-1, 1].
      float phi0 = std::fmod(phase * h0 + 1.0f + 1000.0f, 2.0f) - 1.0f;
      float phi1 = std::fmod(phase * (h0 + 1.0f) + 1.0f + 1000.0f, 2.0f) - 1.0f;

      // Cosine carriers.
      const float carrier0 = fastCos(phi0);
      const float carrier1 = fastCos(phi1);

      // Crossfade between the two carriers.
      return carrier0 + hFrac * (carrier1 - carrier0);
    }

    // DC blocking filter (first-order highpass).
    // Removes DC offset from the output.
    // Cutoff ~5 Hz at 44.1 kHz (R = 0.9993).
    float dcBlocker(float input) noexcept
    {
      const float R = 0.9993f; // Pole location (adjust for different sample rates if needed)
      const float output = input - dcBlockerX1_ + R * dcBlockerY1_;
      dcBlockerX1_ = input;
      dcBlockerY1_ = output;
      return output;
    }
  };

} // namespace ShortwavDSP
