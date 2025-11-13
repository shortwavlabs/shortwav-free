#include "Drift.hpp"

void Drift::process(const ProcessArgs &args)
{
  const float in = inputs[IN_INPUT].getVoltage();

  // Map parameters to DriftGenerator.

  // Depth shaping:
  // Use a gentle exponential curve so low knob positions are still audible
  // and high positions ramp up more strongly:
  //
  //   depthEff = (DEPTH^2) * kMaxDriftVolts
  //
  // This preserves determinism and smoothness while making the control
  // musically useful across its full travel.
  const float depthParam = params[DEPTH_PARAM].getValue();
  const float shapedDepth = depthParam * depthParam; // quadratic curve
  const float effectiveDepthVolts = shapedDepth * kMaxDriftVolts;

  // Rate directly mapped in Hz; DriftGenerator clamps internally.
  const float uiRate = params[RATE_PARAM].getValue();

  // Configure underlying generator.
  drift.setDepth(effectiveDepthVolts);
  drift.setRateHz(uiRate);

  // Per-sample drift (sample-accurate).
  const float d = drift.next();

  // Apply drift as an additive modulation in volts.
  const float out = in + d;

  outputs[OUT_OUTPUT].setVoltage(out);
}

// Instantiate the VCV Rack model for the Drift module.
Model *modelDrift = createModel<Drift, DriftWidget>("Drift");