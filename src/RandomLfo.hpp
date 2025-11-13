#pragma once

#include "plugin.hpp"

#include "dsp/random-lfo.h"

struct RandomLfo : Module
{
  enum ParamIds
  {
    RATE_PARAM,    // Hz
    DEPTH_PARAM,   // 0..1
    SMOOTH_PARAM,  // 0..1
    BIPOLAR_PARAM, // 0 = unipolar, 1 = bipolar
    NUM_PARAMS
  };

  enum InputIds
  {
    RATE_CV_INPUT,
    DEPTH_CV_INPUT,
    SMOOTH_CV_INPUT,
    NUM_INPUTS
  };

  enum OutputIds
  {
    LFO_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds
  {
    NUM_LIGHTS
  };

  ShortwavDSP::RandomLFO lfo;

  RandomLfo()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // Parameter ranges tuned for typical random LFO usage.
    // Rate: 0.01 Hz to 20 Hz
    configParam(RATE_PARAM, 0.01f, 20.f, 1.f, "Rate", " Hz");
    // Depth: 0..1 scalar
    configParam(DEPTH_PARAM, 0.f, 1.f, 1.f, "Depth");
    // Smoothness: 0..1 (higher = smoother / more correlated)
    configParam(SMOOTH_PARAM, 0.f, 1.f, 0.75f, "Smooth");
    // Bipolar toggle: 0 = unipolar [0,1], 1 = bipolar [-1,1]
    configParam(BIPOLAR_PARAM, 0.f, 1.f, 1.f, "Bipolar");

    onSampleRateChange();
  }

  void onSampleRateChange() override
  {
    float sr = APP->engine->getSampleRate();
    lfo.setSampleRate(sr);
    // Seed based on module id pointer for deterministic but distinct instances
    lfo.seed(reinterpret_cast<uint64_t>(this) & 0xFFFFFFFFu);
  }

  void process(const ProcessArgs &args) override;
};

struct RandomLfoWidget : ModuleWidget
{
  RandomLfoWidget(RandomLfo *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/3HP_BLANK.svg")));

    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParam<RoundLargeBlackKnob>(Vec(5, 20), module, RandomLfo::RATE_PARAM));
    addParam(createParam<RoundLargeBlackKnob>(Vec(5, 60), module, RandomLfo::DEPTH_PARAM));
    addParam(createParam<RoundLargeBlackKnob>(Vec(5, 100), module, RandomLfo::SMOOTH_PARAM));
    addParam(createParam<CKSS>(Vec(15, 160), module, RandomLfo::BIPOLAR_PARAM));

    addOutput(createOutput<PJ301MPort>(Vec(10, 220), module, RandomLfo::LFO_OUTPUT));
  }
};
