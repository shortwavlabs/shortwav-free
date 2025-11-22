#pragma once

#include "plugin.hpp"
#include "dsp/waveshaper.h"

// Waveshaper Module
// - One audio input
// - One audio output
// - Controls for:
//     * Input gain
//     * Output gain
//     * Waveshaper order
//     * Soft clip vs clamp
//     * Harmonic mix for a few Chebyshev terms

struct Waveshaper : Module
{
  enum ParamIds
  {
    INPUT_GAIN_PARAM,          // 0..2 (linear)
    OUTPUT_GAIN_PARAM,         // 0..2 (linear)
    ORDER_PARAM,               // 0..16 (0 = bypass)
    SOFTCLIP_PARAM,            // 0/1 toggle
    HARM1_PARAM,               // T1 coefficient
    HARM2_PARAM,               // T2 coefficient
    HARM3_PARAM,               // T3 coefficient
    HARM4_PARAM,               // T4 coefficient
    NUM_PARAMS
  };

  enum InputIds
  {
    SIGNAL_INPUT,
    NUM_INPUTS
  };

  enum OutputIds
  {
    SIGNAL_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds
  {
    NUM_LIGHTS
  };

  static constexpr std::size_t MaxOrder = 16u;

  ShortwavDSP::ChebyshevWaveshaper<MaxOrder> waveshaper;

  Waveshaper()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(INPUT_GAIN_PARAM, 0.f, 2.f, 1.f, "Input gain", "", 0.f, 1.f);
    configParam(OUTPUT_GAIN_PARAM, 0.f, 2.f, 1.f, "Output gain", "", 0.f, 1.f);

    // 0 = bypass via order 0, up to MaxOrder
    configParam(ORDER_PARAM, 0.f, 4.f, 4.f, "Chebyshev order");

    // 0 = hard clamp, 1 = soft-clip
    configParam(SOFTCLIP_PARAM, 0.f, 1.f, 1.f, "Soft clip");

    // Harmonic weights for first few Chebyshev terms.
    // Default to mostly linear: T1 = 1.0, others 0.0
    configParam(HARM1_PARAM, 0.f, 2.f, 1.f, "T1 weight");
    configParam(HARM2_PARAM, 0.f, 2.f, 0.f, "T2 weight");
    configParam(HARM3_PARAM, 0.f, 2.f, 0.f, "T3 weight");
    configParam(HARM4_PARAM, 0.f, 2.f, 0.f, "T4 weight");

    // Initialize default coefficients: T1(x) = x
    waveshaper.resetCoefficientsToLinear();
  }

  void process(const ProcessArgs &args) override;
};

struct WaveshaperWidget : ModuleWidget
{
  WaveshaperWidget(Waveshaper* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/9HP_BLANK.svg")));

    // Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Layout: follow RandomLfo style; 3HP is tight, but keep consistent vertically.
    // Top: input/output gain, order, soft-clip; middle: harmonics; bottom: jacks.

    float y = 20.f;
    addParam(createParam<RoundLargeBlackKnob>(Vec(5, y), module, Waveshaper::INPUT_GAIN_PARAM));
    y += 40.f;
    addParam(createParam<RoundLargeBlackKnob>(Vec(5, y), module, Waveshaper::OUTPUT_GAIN_PARAM));
    y += 45.f;
    addParam(createParam<RoundBlackSnapKnob>(Vec(10, y), module, Waveshaper::ORDER_PARAM));
    y += 40.f;
    addParam(createParam<CKSS>(Vec(15, y + 20.f), module, Waveshaper::SOFTCLIP_PARAM));
    y += 40.f;

    float y2 = 20.f;
    // Harmonic knobs (small, stacked)
    addParam(createParam<RoundLargeBlackKnob>(Vec(50, y2), module, Waveshaper::HARM1_PARAM));
    y2 += 40.f;
    addParam(createParam<RoundLargeBlackKnob>(Vec(50, y2), module, Waveshaper::HARM2_PARAM));
    y2 += 40.f;
    addParam(createParam<RoundLargeBlackKnob>(Vec(50, y2), module, Waveshaper::HARM3_PARAM));
    y2 += 40.f;
    addParam(createParam<RoundLargeBlackKnob>(Vec(50, y2), module, Waveshaper::HARM4_PARAM));

    // I/O jacks
    addInput(createInput<PJ301MPort>(Vec(10, 320), module, Waveshaper::SIGNAL_INPUT));
    addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Waveshaper::SIGNAL_OUTPUT));
  }

  void appendContextMenu(Menu *menu) override
  {
    Waveshaper *module = dynamic_cast<Waveshaper *>(this->module);

    menu->addChild(new MenuEntry);
    menu->addChild(createMenuLabel("Waveshaper"));

    menu->addChild(new MenuEntry);

    struct PresetItem : MenuItem
    {
      Waveshaper *module;
      int preset;
      void onAction(const event::Action &e) override
      {
        switch (preset)
        {
        case 0: // Clean (Linear)
          module->params[Waveshaper::INPUT_GAIN_PARAM].setValue(1.f);
          module->params[Waveshaper::OUTPUT_GAIN_PARAM].setValue(1.f);
          module->params[Waveshaper::ORDER_PARAM].setValue(1.f);
          module->params[Waveshaper::SOFTCLIP_PARAM].setValue(1.f);
          module->params[Waveshaper::HARM1_PARAM].setValue(1.f);
          module->params[Waveshaper::HARM2_PARAM].setValue(0.f);
          module->params[Waveshaper::HARM3_PARAM].setValue(0.f);
          module->params[Waveshaper::HARM4_PARAM].setValue(0.f);
          break;
        case 1: // Soft Overdrive
          module->params[Waveshaper::INPUT_GAIN_PARAM].setValue(1.5f);
          module->params[Waveshaper::OUTPUT_GAIN_PARAM].setValue(0.8f);
          module->params[Waveshaper::ORDER_PARAM].setValue(3.f);
          module->params[Waveshaper::SOFTCLIP_PARAM].setValue(1.f);
          module->params[Waveshaper::HARM1_PARAM].setValue(1.f);
          module->params[Waveshaper::HARM2_PARAM].setValue(0.3f);
          module->params[Waveshaper::HARM3_PARAM].setValue(0.1f);
          module->params[Waveshaper::HARM4_PARAM].setValue(0.f);
          break;
        case 2: // Hard Distortion
          module->params[Waveshaper::INPUT_GAIN_PARAM].setValue(2.f);
          module->params[Waveshaper::OUTPUT_GAIN_PARAM].setValue(0.6f);
          module->params[Waveshaper::ORDER_PARAM].setValue(4.f);
          module->params[Waveshaper::SOFTCLIP_PARAM].setValue(0.f);
          module->params[Waveshaper::HARM1_PARAM].setValue(0.8f);
          module->params[Waveshaper::HARM2_PARAM].setValue(0.6f);
          module->params[Waveshaper::HARM3_PARAM].setValue(0.4f);
          module->params[Waveshaper::HARM4_PARAM].setValue(0.2f);
          break;
        case 3: // Fuzz
          module->params[Waveshaper::INPUT_GAIN_PARAM].setValue(2.f);
          module->params[Waveshaper::OUTPUT_GAIN_PARAM].setValue(0.7f);
          module->params[Waveshaper::ORDER_PARAM].setValue(4.f);
          module->params[Waveshaper::SOFTCLIP_PARAM].setValue(0.f);
          module->params[Waveshaper::HARM1_PARAM].setValue(0.5f);
          module->params[Waveshaper::HARM2_PARAM].setValue(1.f);
          module->params[Waveshaper::HARM3_PARAM].setValue(0.8f);
          module->params[Waveshaper::HARM4_PARAM].setValue(0.6f);
          break;
        case 4: // Subtle Warmth
          module->params[Waveshaper::INPUT_GAIN_PARAM].setValue(1.2f);
          module->params[Waveshaper::OUTPUT_GAIN_PARAM].setValue(0.9f);
          module->params[Waveshaper::ORDER_PARAM].setValue(2.f);
          module->params[Waveshaper::SOFTCLIP_PARAM].setValue(1.f);
          module->params[Waveshaper::HARM1_PARAM].setValue(1.f);
          module->params[Waveshaper::HARM2_PARAM].setValue(0.15f);
          module->params[Waveshaper::HARM3_PARAM].setValue(0.f);
          module->params[Waveshaper::HARM4_PARAM].setValue(0.f);
          break;
        case 5: // Octave Up
          module->params[Waveshaper::INPUT_GAIN_PARAM].setValue(1.f);
          module->params[Waveshaper::OUTPUT_GAIN_PARAM].setValue(1.f);
          module->params[Waveshaper::ORDER_PARAM].setValue(2.f);
          module->params[Waveshaper::SOFTCLIP_PARAM].setValue(1.f);
          module->params[Waveshaper::HARM1_PARAM].setValue(0.3f);
          module->params[Waveshaper::HARM2_PARAM].setValue(1.f);
          module->params[Waveshaper::HARM3_PARAM].setValue(0.f);
          module->params[Waveshaper::HARM4_PARAM].setValue(0.f);
          break;
        }
      }
    };

    menu->addChild(createMenuLabel("Presets"));

    const char *presetNames[] = {
        "Clean (Linear)",
        "Soft Overdrive",
        "Hard Distortion",
        "Fuzz",
        "Subtle Warmth",
        "Octave Up"};

    for (int i = 0; i < 6; ++i)
    {
      PresetItem *presetItem = createMenuItem<PresetItem>(presetNames[i]);
      presetItem->module = module;
      presetItem->preset = i;
      menu->addChild(presetItem);
    }
  }
};
