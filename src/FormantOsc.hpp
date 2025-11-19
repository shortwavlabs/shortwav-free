#pragma once

#include "plugin.hpp"

#include "dsp/formant-osc.h"

struct FormantOsc : Module
{
  enum ParamIds
  {
    CARRIER_FREQ_PARAM, // Hz (base pitch)
    FORMANT_FREQ_PARAM, // Hz (formant center)
    FORMANT_WIDTH_PARAM, // 0..1 (formant bandwidth/Q)
    OUTPUT_GAIN_PARAM,  // 0..1
    NUM_PARAMS
  };

  enum InputIds
  {
    CARRIER_FREQ_CV_INPUT,
    FORMANT_FREQ_CV_INPUT,
    FORMANT_WIDTH_CV_INPUT,
    NUM_INPUTS
  };

  enum OutputIds
  {
    AUDIO_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds
  {
    NUM_LIGHTS
  };

  ShortwavDSP::FormantOscillator osc;

  FormantOsc()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // Parameter ranges tuned for formant synthesis.
    // Carrier frequency: C2 (65.4 Hz) to C7 (2093 Hz), default A3 (220 Hz)
    configParam(CARRIER_FREQ_PARAM, 65.4f, 2093.0f, 220.0f, "Carrier Frequency", " Hz");
    
    // Formant frequency: typical vowel formant range 200-4000 Hz, default 800 Hz
    configParam(FORMANT_FREQ_PARAM, 200.0f, 4000.0f, 800.0f, "Formant Frequency", " Hz");
    
    // Formant width: 0..1 scalar (0 = narrow/peaked, 1 = wide/broad)
    configParam(FORMANT_WIDTH_PARAM, 0.0f, 1.0f, 0.3f, "Formant Width");
    
    // Output gain: 0..1 scalar
    configParam(OUTPUT_GAIN_PARAM, 0.0f, 1.0f, 0.5f, "Output Gain");

    onSampleRateChange();
  }

  void onSampleRateChange() override
  {
    float sr = APP->engine->getSampleRate();
    osc.setSampleRate(sr);
  }

  void process(const ProcessArgs &args) override;
};

struct FormantOscWidget : ModuleWidget
{
  FormantOscWidget(FormantOsc *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/3HP_BLANK.svg")));

    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Knobs for main parameters
    addParam(createParam<RoundLargeBlackKnob>(Vec(5, 40), module, FormantOsc::CARRIER_FREQ_PARAM));
    addParam(createParam<RoundLargeBlackKnob>(Vec(5, 100), module, FormantOsc::FORMANT_FREQ_PARAM));
    addParam(createParam<RoundLargeBlackKnob>(Vec(5, 160), module, FormantOsc::FORMANT_WIDTH_PARAM));
    addParam(createParam<RoundBlackKnob>(Vec(10, 220), module, FormantOsc::OUTPUT_GAIN_PARAM));

    // CV inputs
    addInput(createInput<PJ301MPort>(Vec(10, 260), module, FormantOsc::CARRIER_FREQ_CV_INPUT));
    addInput(createInput<PJ301MPort>(Vec(10, 290), module, FormantOsc::FORMANT_FREQ_CV_INPUT));
    addInput(createInput<PJ301MPort>(Vec(10, 320), module, FormantOsc::FORMANT_WIDTH_CV_INPUT));

    // Audio output
    addOutput(createOutput<PJ301MPort>(Vec(10, 350), module, FormantOsc::AUDIO_OUTPUT));
  }
};
