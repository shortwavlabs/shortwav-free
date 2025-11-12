#include "Waveshaper.hpp"

void Waveshaper::process(const ProcessArgs& args)
{
  // If no output or no input, early out
  if (!outputs[SIGNAL_OUTPUT].isConnected())
    return;

  const float inGain = params[INPUT_GAIN_PARAM].getValue();
  const float outGain = params[OUTPUT_GAIN_PARAM].getValue();
  const float orderF = params[ORDER_PARAM].getValue();
  const bool useSoftClip = params[SOFTCLIP_PARAM].getValue() >= 0.5f;

  // Configure order: round and clamp; 0 = bypass
  int order = (int)std::round(orderF);
  if (order <= 0)
    waveshaper.setOrder(0);
  else if ((std::size_t)order > MaxOrder)
    waveshaper.setOrder(MaxOrder);
  else
    waveshaper.setOrder((std::size_t)order);

  waveshaper.setUseSoftClipForInput(useSoftClip);
  waveshaper.setOutputGain(outGain);

  // Update first few harmonic coefficients from params.
  // Leave others as previously set (default 0) for stability.
  // Index: coeffs[n] is T_n.
  // We only expose up to T4 on the panel; higher orders remain configurable via ORDER_PARAM
  // but will be silent unless coefficients are changed in code later.
  waveshaper.setCoefficient(0, 0.f); // no DC by default
  waveshaper.setCoefficient(1, params[HARM1_PARAM].getValue());
  waveshaper.setCoefficient(2, params[HARM2_PARAM].getValue());
  waveshaper.setCoefficient(3, params[HARM3_PARAM].getValue());
  waveshaper.setCoefficient(4, params[HARM4_PARAM].getValue());

  // Single-sample (per-channel) processing.
  // This codebase is mono; follow RandomLfo style.
  float in = inputs[SIGNAL_INPUT].isConnected()
              ? inputs[SIGNAL_INPUT].getVoltage()
              : 0.f;

  // Normalize to [-1, 1] domain expected by ChebyshevWaveshaper:
  // Assume typical modular range +/-5V; use input gain for user adjustment.
  float x = (in * inGain) / 5.f;

  float y = waveshaper.processSample(x);

  // Map back to modular level.
  // Nominal scaling: +/-5V, then clamp to [-10V, +10V] via Rack SDK helper.
  float outV = y * 5.f;

  outV = clamp(outV, -10.f, 10.f);

  outputs[SIGNAL_OUTPUT].setVoltage(outV);
}

Model* modelWaveshaper = createModel<Waveshaper, WaveshaperWidget>("Waveshaper");