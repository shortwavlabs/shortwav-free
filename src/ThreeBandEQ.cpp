#include "ThreeBandEQ.hpp"

void ThreeBandEQ::process(const ProcessArgs &args)
{
  // Update bypass state from parameter
  bool currentBypass = params[BYPASS_PARAM].getValue() > 0.5f;
  if (currentBypass != bypassed)
  {
    bypassed = currentBypass;
    if (bypassed)
    {
      eq.reset(); // Clear state when bypassing
    }
  }

  // Update bypass LED
  lights[BYPASS_LIGHT].setBrightness(bypassed ? 1.0f : 0.0f);

  // Get base parameter values
  float lowFreq = params[LOW_FREQ_PARAM].getValue();
  float highFreq = params[HIGH_FREQ_PARAM].getValue();
  float lowGainDB = params[LOW_GAIN_PARAM].getValue();
  float midGainDB = params[MID_GAIN_PARAM].getValue();
  float highGainDB = params[HIGH_GAIN_PARAM].getValue();

  // Apply CV modulation to crossover frequencies
  // CV is 0-10V, map to frequency range with exponential scaling for musical response
  if (inputs[LOW_FREQ_CV_INPUT].isConnected())
  {
    float cv = inputs[LOW_FREQ_CV_INPUT].getVoltage();
    // Map 0-10V to 0-1 range, then scale exponentially
    float cvNorm = clamp(cv / 10.0f, 0.0f, 1.0f);
    float freqRange = 250.0f - 80.0f; // 170 Hz range
    lowFreq = 80.0f + cvNorm * freqRange;
  }

  if (inputs[HIGH_FREQ_CV_INPUT].isConnected())
  {
    float cv = inputs[HIGH_FREQ_CV_INPUT].getVoltage();
    float cvNorm = clamp(cv / 10.0f, 0.0f, 1.0f);
    float freqRange = 4000.0f - 1000.0f; // 3000 Hz range
    highFreq = 1000.0f + cvNorm * freqRange;
  }

  // Apply CV modulation to gains
  // CV is -5V to +5V bipolar, maps to -12dB to +12dB
  if (inputs[LOW_GAIN_CV_INPUT].isConnected())
  {
    float cv = inputs[LOW_GAIN_CV_INPUT].getVoltage();
    // Map -5V to +5V -> -12dB to +12dB
    float cvDB = clamp(cv * 2.4f, -12.0f, 12.0f); // 2.4 = 12/5
    lowGainDB = clamp(lowGainDB + cvDB, -12.0f, 12.0f);
  }

  if (inputs[MID_GAIN_CV_INPUT].isConnected())
  {
    float cv = inputs[MID_GAIN_CV_INPUT].getVoltage();
    float cvDB = clamp(cv * 2.4f, -12.0f, 12.0f);
    midGainDB = clamp(midGainDB + cvDB, -12.0f, 12.0f);
  }

  if (inputs[HIGH_GAIN_CV_INPUT].isConnected())
  {
    float cv = inputs[HIGH_GAIN_CV_INPUT].getVoltage();
    float cvDB = clamp(cv * 2.4f, -12.0f, 12.0f);
    highGainDB = clamp(highGainDB + cvDB, -12.0f, 12.0f);
  }

  // Update EQ parameters
  eq.setCrossoverFreqs(lowFreq, highFreq);
  eq.setGainsDB(lowGainDB, midGainDB, highGainDB);

  // Process audio
  bool leftConnected = inputs[AUDIO_L_INPUT].isConnected();
  bool rightConnected = inputs[AUDIO_R_INPUT].isConnected();

  if (!leftConnected && !rightConnected)
  {
    // No inputs connected, output silence
    outputs[AUDIO_L_OUTPUT].setVoltage(0.0f);
    outputs[AUDIO_R_OUTPUT].setVoltage(0.0f);
    return;
  }

  // Get input voltages (Rack uses -10V to +10V for audio)
  float leftIn = leftConnected ? inputs[AUDIO_L_INPUT].getVoltage() : 0.0f;
  float rightIn = rightConnected ? inputs[AUDIO_R_INPUT].getVoltage() : leftIn; // Mono to stereo

  // If bypassed, pass audio through without processing
  if (bypassed)
  {
    outputs[AUDIO_L_OUTPUT].setVoltage(leftIn);
    outputs[AUDIO_R_OUTPUT].setVoltage(rightIn);
    return;
  }

  // Scale from Rack voltage (-10V to +10V) to DSP normalized range (-1 to +1)
  const float rackToNorm = 0.1f; // 1/10
  const float normToRack = 10.0f;

  float leftNorm = leftIn * rackToNorm;
  float rightNorm = rightIn * rackToNorm;

  // Process through equalizer
  eq.processStereoSample(leftNorm, rightNorm);

  // Scale back to Rack voltage range and output
  float leftOut = leftNorm * normToRack;
  float rightOut = rightNorm * normToRack;

  // Soft clipping to prevent harsh digital clipping
  // (EQ boost can increase levels significantly)
  leftOut = clamp(leftOut, -10.0f, 10.0f);
  rightOut = clamp(rightOut, -10.0f, 10.0f);

  outputs[AUDIO_L_OUTPUT].setVoltage(leftOut);
  outputs[AUDIO_R_OUTPUT].setVoltage(rightOut);
}

// Create the model (required for plugin registration)
Model *modelThreeBandEQ = createModel<ThreeBandEQ, ThreeBandEQWidget>("ThreeBandEQ");
