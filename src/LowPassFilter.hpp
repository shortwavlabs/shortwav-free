#pragma once

#include "plugin.hpp"
#include "dsp/low-pass.h"

// LowPassFilter Module
// - Moog-style 4-pole (24dB/oct) resonant low-pass filter
// - Stereo processing with separate filter instances per channel
// - Controls for:
//     * Cutoff frequency (20Hz - Nyquist/2)
//     * Resonance (0.0 = none, 1.0 = self-oscillation)
//     * CV modulation for cutoff and resonance

struct LowPassFilter : Module
{
  enum ParamIds
  {
    CUTOFF_PARAM,    // Cutoff frequency (Hz, exponential)
    RESONANCE_PARAM, // Resonance amount 0..1
    NUM_PARAMS
  };

  enum InputIds
  {
    CUTOFF_CV_INPUT,
    RESONANCE_CV_INPUT,
    AUDIO_INPUT_L,
    AUDIO_INPUT_R,
    NUM_INPUTS
  };

  enum OutputIds
  {
    AUDIO_OUTPUT_L,
    AUDIO_OUTPUT_R,
    NUM_OUTPUTS
  };

  enum LightIds
  {
    NUM_LIGHTS
  };

  ShortwavDSP::MoogLowPassFilter filterL;
  ShortwavDSP::MoogLowPassFilter filterR;

  LowPassFilter()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // Cutoff: exponential range from 20Hz to 20kHz
    // Use exponential scaling for musical frequency control
    configParam(CUTOFF_PARAM, std::log2(20.f), std::log2(20000.f), std::log2(1000.f), "Cutoff", " Hz", 2.f, 1.f);
    
    // Resonance: linear 0..1
    configParam(RESONANCE_PARAM, 0.f, 1.f, 0.0f, "Resonance");

    // Input/output configuration
    configInput(CUTOFF_CV_INPUT, "Cutoff CV (1V/oct)");
    configInput(RESONANCE_CV_INPUT, "Resonance CV (0-10V)");
    configInput(AUDIO_INPUT_L, "Audio L");
    configInput(AUDIO_INPUT_R, "Audio R");
    configOutput(AUDIO_OUTPUT_L, "Audio L");
    configOutput(AUDIO_OUTPUT_R, "Audio R");

    onSampleRateChange();
  }

  void onSampleRateChange() override
  {
    float sr = APP->engine->getSampleRate();
    filterL.setSampleRate(sr);
    filterR.setSampleRate(sr);
  }

  void onReset() override
  {
    filterL.reset();
    filterR.reset();
  }

  void process(const ProcessArgs &args) override;
};

struct LowPassFilterWidget : ModuleWidget
{
  LowPassFilterWidget(LowPassFilter *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/6HP_BLANK.svg")));

    // Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Layout: top to bottom
    // - Cutoff knob + CV input
    // - Resonance knob + CV input
    // - Audio inputs (L/R)
    // - Audio outputs (L/R)

    float knobX = 15.f;
    float cvX = 55.f;
    float y = 50.f;

    // Cutoff control
    addParam(createParam<RoundLargeBlackKnob>(Vec(knobX, y), module, LowPassFilter::CUTOFF_PARAM));
    addInput(createInput<PJ301MPort>(Vec(cvX, y + 5), module, LowPassFilter::CUTOFF_CV_INPUT));
    
    y += 55.f;
    
    // Resonance control
    addParam(createParam<RoundLargeBlackKnob>(Vec(knobX, y), module, LowPassFilter::RESONANCE_PARAM));
    addInput(createInput<PJ301MPort>(Vec(cvX, y + 5), module, LowPassFilter::RESONANCE_CV_INPUT));

    y += 70.f;

    // Audio inputs
    addInput(createInput<PJ301MPort>(Vec(10, y), module, LowPassFilter::AUDIO_INPUT_L));
    addInput(createInput<PJ301MPort>(Vec(50, y), module, LowPassFilter::AUDIO_INPUT_R));

    y += 45.f;

    // Audio outputs
    addOutput(createOutput<PJ301MPort>(Vec(10, y), module, LowPassFilter::AUDIO_OUTPUT_L));
    addOutput(createOutput<PJ301MPort>(Vec(50, y), module, LowPassFilter::AUDIO_OUTPUT_R));
  }

  void appendContextMenu(Menu *menu) override
  {
    LowPassFilter *module = dynamic_cast<LowPassFilter *>(this->module);
    if (!module)
      return;

    menu->addChild(new MenuEntry);
    menu->addChild(createMenuLabel("Low-Pass Filter"));

    menu->addChild(new MenuEntry);

    struct PresetItem : MenuItem
    {
      LowPassFilter *module;
      int preset;
      void onAction(const event::Action &e) override
      {
        switch (preset)
        {
        case 0: // Subtle (Low resonance)
          module->params[LowPassFilter::CUTOFF_PARAM].setValue(std::log2(1000.f));
          module->params[LowPassFilter::RESONANCE_PARAM].setValue(0.2f);
          break;
        case 1: // Moderate (Mid resonance)
          module->params[LowPassFilter::CUTOFF_PARAM].setValue(std::log2(800.f));
          module->params[LowPassFilter::RESONANCE_PARAM].setValue(0.5f);
          break;
        case 2: // Resonant (High resonance)
          module->params[LowPassFilter::CUTOFF_PARAM].setValue(std::log2(1200.f));
          module->params[LowPassFilter::RESONANCE_PARAM].setValue(0.8f);
          break;
        case 3: // Self-Oscillating
          module->params[LowPassFilter::CUTOFF_PARAM].setValue(std::log2(440.f));
          module->params[LowPassFilter::RESONANCE_PARAM].setValue(1.0f);
          break;
        }
        // Reset filter state to prevent clicks
        module->filterL.reset();
        module->filterR.reset();
      }
    };

    PresetItem *subtleItem = new PresetItem;
    subtleItem->text = "Subtle";
    subtleItem->module = module;
    subtleItem->preset = 0;
    menu->addChild(subtleItem);

    PresetItem *moderateItem = new PresetItem;
    moderateItem->text = "Moderate";
    moderateItem->module = module;
    moderateItem->preset = 1;
    menu->addChild(moderateItem);

    PresetItem *resonantItem = new PresetItem;
    resonantItem->text = "Resonant";
    resonantItem->module = module;
    resonantItem->preset = 2;
    menu->addChild(resonantItem);

    PresetItem *selfOscItem = new PresetItem;
    selfOscItem->text = "Self-Oscillating";
    selfOscItem->module = module;
    selfOscItem->preset = 3;
    menu->addChild(selfOscItem);
  }
};
