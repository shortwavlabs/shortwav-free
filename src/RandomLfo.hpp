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

  void appendContextMenu(Menu *menu) override
  {
    RandomLfo *module = dynamic_cast<RandomLfo *>(this->module);

    menu->addChild(new MenuEntry);
    menu->addChild(createMenuLabel("Random LFO"));

    menu->addChild(new MenuEntry);

    struct PresetItem : MenuItem
    {
      RandomLfo *module;
      int preset;
      void onAction(const event::Action &e) override
      {
        switch (preset)
        {
        case 0: // Slow & Smooth
          module->params[RandomLfo::RATE_PARAM].setValue(0.25f);
          module->params[RandomLfo::DEPTH_PARAM].setValue(1.f);
          module->params[RandomLfo::SMOOTH_PARAM].setValue(0.9f);
          module->params[RandomLfo::BIPOLAR_PARAM].setValue(1.f);
          break;
        case 1: // Sample & Hold
          module->params[RandomLfo::RATE_PARAM].setValue(2.f);
          module->params[RandomLfo::DEPTH_PARAM].setValue(1.f);
          module->params[RandomLfo::SMOOTH_PARAM].setValue(0.f);
          module->params[RandomLfo::BIPOLAR_PARAM].setValue(1.f);
          break;
        case 2: // Smooth Random
          module->params[RandomLfo::RATE_PARAM].setValue(1.f);
          module->params[RandomLfo::DEPTH_PARAM].setValue(1.f);
          module->params[RandomLfo::SMOOTH_PARAM].setValue(0.75f);
          module->params[RandomLfo::BIPOLAR_PARAM].setValue(1.f);
          break;
        case 3: // Fast Wobble
          module->params[RandomLfo::RATE_PARAM].setValue(5.f);
          module->params[RandomLfo::DEPTH_PARAM].setValue(0.7f);
          module->params[RandomLfo::SMOOTH_PARAM].setValue(0.5f);
          module->params[RandomLfo::BIPOLAR_PARAM].setValue(1.f);
          break;
        case 4: // Unipolar Smooth
          module->params[RandomLfo::RATE_PARAM].setValue(0.5f);
          module->params[RandomLfo::DEPTH_PARAM].setValue(1.f);
          module->params[RandomLfo::SMOOTH_PARAM].setValue(0.8f);
          module->params[RandomLfo::BIPOLAR_PARAM].setValue(0.f);
          break;
        case 5: // Glitchy
          module->params[RandomLfo::RATE_PARAM].setValue(10.f);
          module->params[RandomLfo::DEPTH_PARAM].setValue(1.f);
          module->params[RandomLfo::SMOOTH_PARAM].setValue(0.1f);
          module->params[RandomLfo::BIPOLAR_PARAM].setValue(1.f);
          break;
        }
      }
    };

    menu->addChild(createMenuLabel("Presets"));

    const char *presetNames[] = {
        "Slow & Smooth",
        "Sample & Hold",
        "Smooth Random",
        "Fast Wobble",
        "Unipolar Smooth",
        "Glitchy"};

    for (int i = 0; i < 6; ++i)
    {
      PresetItem *presetItem = createMenuItem<PresetItem>(presetNames[i]);
      presetItem->module = module;
      presetItem->preset = i;
      menu->addChild(presetItem);
    }
  }
};
