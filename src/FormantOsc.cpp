#include "FormantOsc.hpp"

void FormantOsc::process(const ProcessArgs &args)
{
  // Pull base parameter values.
  float carrierFreq = params[CARRIER_FREQ_PARAM].getValue();
  float formantFreq = params[FORMANT_FREQ_PARAM].getValue();
  float formantWidth = params[FORMANT_WIDTH_PARAM].getValue();
  float outputGain = params[OUTPUT_GAIN_PARAM].getValue();

  // Apply CV modulation where available.
  // Assumptions for CV inputs:
  // - CARRIER_FREQ_CV_INPUT: 1V/oct pitch modulation
  // - FORMANT_FREQ_CV_INPUT: 1V/oct frequency modulation
  // - FORMANT_WIDTH_CV_INPUT: 0-10V mapped to 0..1 additive modulation

  if (inputs[CARRIER_FREQ_CV_INPUT].isConnected())
  {
    // 1V/oct: for each volt, multiply frequency by 2^(V/1).
    float cv = clamp(inputs[CARRIER_FREQ_CV_INPUT].getVoltage(), -10.0f, 10.0f);
    float factor = std::pow(2.0f, cv);
    carrierFreq *= factor;
  }

  if (inputs[FORMANT_FREQ_CV_INPUT].isConnected())
  {
    // 1V/oct: for each volt, multiply frequency by 2^(V/1).
    float cv = clamp(inputs[FORMANT_FREQ_CV_INPUT].getVoltage(), -10.0f, 10.0f);
    float factor = std::pow(2.0f, cv);
    formantFreq *= factor;
  }

  if (inputs[FORMANT_WIDTH_CV_INPUT].isConnected())
  {
    // 0-10V mapped to 0..1, additive with parameter.
    float cv = clamp(inputs[FORMANT_WIDTH_CV_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
    formantWidth = clamp(formantWidth + cv * 0.5f, 0.0f, 1.0f);
  }

  // Clamp frequencies to reasonable ranges for stability.
  carrierFreq = clamp(carrierFreq, 0.0f, 20000.0f);
  formantFreq = clamp(formantFreq, 0.0f, 20000.0f);
  formantWidth = clamp(formantWidth, 0.0f, 1.0f);
  outputGain = clamp(outputGain, 0.0f, 2.0f);

  // Apply to oscillator engine.
  osc.setCarrierFreq(carrierFreq);
  osc.setFormantFreq(formantFreq);
  osc.setFormantWidth(formantWidth);
  osc.setOutputGain(outputGain);

  // Generate one audio sample per process() call; Rack drives this at audio rate.
  float sample = osc.processSample();

  // Map to audio output voltage range: typical ±5V for audio in Rack.
  float outV = sample * 5.0f;

  outputs[AUDIO_OUTPUT].setVoltage(outV);
}

Model *modelFormantOsc = createModel<FormantOsc, FormantOscWidget>("FormantOsc");
