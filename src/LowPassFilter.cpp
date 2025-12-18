#include "LowPassFilter.hpp"

void LowPassFilter::process(const ProcessArgs &args)
{
  // Get base parameter values
  // Cutoff is stored as log2(Hz), so convert back to linear Hz
  float cutoffHz = std::pow(2.f, params[CUTOFF_PARAM].getValue());
  float resonance = params[RESONANCE_PARAM].getValue();

  // Apply CV modulation
  // Cutoff CV: 1V/oct standard (additive in log space)
  if (inputs[CUTOFF_CV_INPUT].isConnected())
  {
    float cvVoltage = inputs[CUTOFF_CV_INPUT].getVoltage();
    // Each volt adds one octave: multiply frequency by 2^(V/1)
    cutoffHz *= std::pow(2.f, cvVoltage);
  }

  // Resonance CV: 0-10V mapped to 0..1 (multiplicative)
  if (inputs[RESONANCE_CV_INPUT].isConnected())
  {
    float cvVoltage = clamp(inputs[RESONANCE_CV_INPUT].getVoltage(), 0.f, 10.f);
    resonance = clamp(resonance * (cvVoltage / 10.f), 0.f, 1.f);
  }

  // Update filter parameters
  // (Clamping happens inside the filter setters)
  filterL.setCutoff(cutoffHz);
  filterL.setResonance(resonance);
  filterR.setCutoff(cutoffHz);
  filterR.setResonance(resonance);

  // Process audio
  // Left channel
  if (inputs[AUDIO_INPUT_L].isConnected() && outputs[AUDIO_OUTPUT_L].isConnected())
  {
    float inputL = inputs[AUDIO_INPUT_L].getVoltage();
    float outputL = filterL.processSample(inputL);
    outputs[AUDIO_OUTPUT_L].setVoltage(outputL);
  }
  else if (outputs[AUDIO_OUTPUT_L].isConnected())
  {
    // No input connected - output silence
    outputs[AUDIO_OUTPUT_L].setVoltage(0.f);
  }

  // Right channel (or copy left if only left input is connected)
  if (outputs[AUDIO_OUTPUT_R].isConnected())
  {
    if (inputs[AUDIO_INPUT_R].isConnected())
    {
      // True stereo: process right input through right filter
      float inputR = inputs[AUDIO_INPUT_R].getVoltage();
      float outputR = filterR.processSample(inputR);
      outputs[AUDIO_OUTPUT_R].setVoltage(outputR);
    }
    else if (inputs[AUDIO_INPUT_L].isConnected())
    {
      // Mono to stereo: copy left input to right channel
      float inputL = inputs[AUDIO_INPUT_L].getVoltage();
      float outputR = filterR.processSample(inputL);
      outputs[AUDIO_OUTPUT_R].setVoltage(outputR);
    }
    else
    {
      // No input - output silence
      outputs[AUDIO_OUTPUT_R].setVoltage(0.f);
    }
  }
}

Model *modelLowPassFilter = createModel<LowPassFilter, LowPassFilterWidget>("LowPassFilter");
