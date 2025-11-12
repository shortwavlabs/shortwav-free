#include "RandomLfo.hpp"

void RandomLfo::process(const ProcessArgs &args)
{
  // Pull base params.
  float rate = params[RATE_PARAM].getValue();
  float depth = params[DEPTH_PARAM].getValue();
  float smooth = params[SMOOTH_PARAM].getValue();
  bool bipolar = params[BIPOLAR_PARAM].getValue() >= 0.5f;

  // Apply simple CV modulation where available.
  // Assumptions:
  // - RATE_CV_INPUT: 1V/oct style modulation around the param (clamped reasonable).
  // - DEPTH_CV_INPUT: 0-10V mapped to 0..1 additive.
  // - SMOOTH_CV_INPUT: 0-10V mapped to 0..1 additive.
  if (inputs[RATE_CV_INPUT].isConnected()) {
    // Map 0-10V to a multiplicative factor ~[0.25,4] around base rate.
    float v = clamp(inputs[RATE_CV_INPUT].getVoltage(), -10.f, 10.f);
    float factor = std::pow(2.f, v / 5.f); // +/-5V = +/- one octave
    rate *= factor;
  }

  if (inputs[DEPTH_CV_INPUT].isConnected()) {
    float v = clamp(inputs[DEPTH_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
    depth *= v;
  }

  if (inputs[SMOOTH_CV_INPUT].isConnected()) {
    float v = clamp(inputs[SMOOTH_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
    // Blend parameter and CV for stability.
    smooth = clamp(smooth * 0.5f + v * 0.5f, 0.f, 1.f);
  }

  // Clamp and apply to LFO engine.
  rate = clamp(rate, 0.0f, 40.f); // hard guard
  depth = clamp(depth, 0.f, 1.f);
  smooth = clamp(smooth, 0.f, 1.f);

  lfo.setRate(rate);
  lfo.setDepth(depth);
  lfo.setSmooth(smooth);
  lfo.setBipolar(bipolar);

  // Generate one sample per audio sample; Rack drives process() at audio rate.
  float value = lfo.processSample();

  // Map to a useful voltage range for modulation:
  // - Bipolar: [-5V, +5V]
  // - Unipolar: [0V, +10V]
  float outV = 0.f;
  if (bipolar) {
    outV = value * 5.f; // value already in [-1,1]*depth
  } else {
    outV = value * 10.f; // value in [0,1]*depth
  }

  outputs[LFO_OUTPUT].setVoltage(outV);
}

Model *modelRandomlfo = createModel<RandomLfo, RandomLfoWidget>("RandomLfo");

