#pragma once

#include "plugin.hpp"
#include "dsp/3-band-eq.h"
#include "ThreeBandEQDisplay.hpp"

struct ThreeBandEQ : Module
{
  enum ParamIds
  {
    LOW_FREQ_PARAM,  // Low/Mid crossover frequency (80-250 Hz)
    HIGH_FREQ_PARAM, // Mid/High crossover frequency (1-4 kHz)
    LOW_GAIN_PARAM,  // Low band gain (-12 to +12 dB)
    MID_GAIN_PARAM,  // Mid band gain (-12 to +12 dB)
    HIGH_GAIN_PARAM, // High band gain (-12 to +12 dB)
    BYPASS_PARAM,    // Bypass switch (0 = active, 1 = bypass)
    NUM_PARAMS
  };

  enum InputIds
  {
    AUDIO_L_INPUT,
    AUDIO_R_INPUT,
    LOW_FREQ_CV_INPUT,
    HIGH_FREQ_CV_INPUT,
    LOW_GAIN_CV_INPUT,
    MID_GAIN_CV_INPUT,
    HIGH_GAIN_CV_INPUT,
    NUM_INPUTS
  };

  enum OutputIds
  {
    AUDIO_L_OUTPUT,
    AUDIO_R_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds
  {
    BYPASS_LIGHT,
    NUM_LIGHTS
  };

  ShortwavDSP::ThreeBandEQ eq;
  bool bypassed = false;

  ThreeBandEQ()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // Low/Mid crossover frequency: 80-250 Hz (default 150 Hz)
    configParam(LOW_FREQ_PARAM, 80.f, 250.f, 150.f, "Low Freq", " Hz");

    // Mid/High crossover frequency: 1000-4000 Hz (default 2500 Hz)
    configParam(HIGH_FREQ_PARAM, 1000.f, 4000.f, 2500.f, "High Freq", " Hz");

    // Gain parameters: -12 to +12 dB
    configParam(LOW_GAIN_PARAM, -12.f, 12.f, 0.f, "Low Gain", " dB");
    configParam(MID_GAIN_PARAM, -12.f, 12.f, 0.f, "Mid Gain", " dB");
    configParam(HIGH_GAIN_PARAM, -12.f, 12.f, 0.f, "High Gain", " dB");

    // Bypass switch
    configParam(BYPASS_PARAM, 0.f, 1.f, 0.f, "Bypass");

    // Configure inputs
    configInput(AUDIO_L_INPUT, "Audio L");
    configInput(AUDIO_R_INPUT, "Audio R");
    configInput(LOW_FREQ_CV_INPUT, "Low Freq CV");
    configInput(HIGH_FREQ_CV_INPUT, "High Freq CV");
    configInput(LOW_GAIN_CV_INPUT, "Low Gain CV");
    configInput(MID_GAIN_CV_INPUT, "Mid Gain CV");
    configInput(HIGH_GAIN_CV_INPUT, "High Gain CV");

    // Configure outputs
    configOutput(AUDIO_L_OUTPUT, "Audio L");
    configOutput(AUDIO_R_OUTPUT, "Audio R");

    onSampleRateChange();
  }

  void onSampleRateChange() override
  {
    float sr = APP->engine->getSampleRate();
    eq.setSampleRate(sr);
  }

  void onReset() override
  {
    eq.reset();
    bypassed = false;
  }

  void process(const ProcessArgs &args) override;

  json_t *dataToJson() override
  {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "bypassed", json_boolean(bypassed));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override
  {
    json_t *bypassedJ = json_object_get(rootJ, "bypassed");
    if (bypassedJ)
      bypassed = json_boolean_value(bypassedJ);
  }
};

struct ThreeBandEQWidget : ModuleWidget
{
  ThreeBandEQWidget(ThreeBandEQ *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/22HP_BLANK.svg")));

    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Frequency Response Display (top visualization)
    EQFrequencyResponseDisplay *freqDisplay = new EQFrequencyResponseDisplay();
    freqDisplay->box.pos = Vec(20, 20);
    freqDisplay->box.size = Vec(200, 80);
    freqDisplay->module = module;
    addChild(freqDisplay);

    // Gain Meter Display (middle visualization)
    // EQGainMeterDisplay *gainDisplay = new EQGainMeterDisplay();
    // gainDisplay->box.pos = Vec(85, 110);
    // gainDisplay->box.size = Vec(200, 60);
    // gainDisplay->module = module;
    // addChild(gainDisplay);

    // Crossover frequency knobs
    addParam(createParam<RoundLargeBlackKnob>(Vec(25, 210), module, ThreeBandEQ::LOW_FREQ_PARAM));
    addParam(createParam<RoundLargeBlackKnob>(Vec(75, 210), module, ThreeBandEQ::HIGH_FREQ_PARAM));

    // Gain knobs (Low, Mid, High)
    addParam(createParam<RoundLargeBlackKnob>(Vec(25, 125), module, ThreeBandEQ::LOW_GAIN_PARAM));
    addParam(createParam<RoundLargeBlackKnob>(Vec(75, 125), module, ThreeBandEQ::MID_GAIN_PARAM));
    addParam(createParam<RoundLargeBlackKnob>(Vec(125, 125), module, ThreeBandEQ::HIGH_GAIN_PARAM));

    // Bypass button
    addParam(createParam<CKSS>(Vec(185, 125), module, ThreeBandEQ::BYPASS_PARAM));
    addChild(createLight<MediumLight<RedLight>>(Vec(190, 155), module, ThreeBandEQ::BYPASS_LIGHT));

    // CV inputs (left column)
    addInput(createInput<PJ301MPort>(Vec(30, 260), module, ThreeBandEQ::LOW_FREQ_CV_INPUT));
    addInput(createInput<PJ301MPort>(Vec(80, 260), module, ThreeBandEQ::HIGH_FREQ_CV_INPUT));

    addInput(createInput<PJ301MPort>(Vec(30, 170), module, ThreeBandEQ::LOW_GAIN_CV_INPUT));
    addInput(createInput<PJ301MPort>(Vec(80, 170), module, ThreeBandEQ::MID_GAIN_CV_INPUT));
    addInput(createInput<PJ301MPort>(Vec(130, 170), module, ThreeBandEQ::HIGH_GAIN_CV_INPUT));

    // Audio inputs (bottom)
    addInput(createInput<PJ301MPort>(Vec(20, 300), module, ThreeBandEQ::AUDIO_L_INPUT));
    addInput(createInput<PJ301MPort>(Vec(20, 335), module, ThreeBandEQ::AUDIO_R_INPUT));

    // Audio outputs (bottom)
    addOutput(createOutput<PJ301MPort>(Vec(195, 300), module, ThreeBandEQ::AUDIO_L_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(195, 335), module, ThreeBandEQ::AUDIO_R_OUTPUT));
  }

  void appendContextMenu(Menu *menu) override
  {
    ThreeBandEQ *module = dynamic_cast<ThreeBandEQ *>(this->module);

    menu->addChild(new MenuEntry);
    menu->addChild(createMenuLabel("3-Band Equalizer"));

    struct BypassItem : MenuItem
    {
      ThreeBandEQ *module;
      void onAction(const event::Action &e) override
      {
        module->bypassed = !module->bypassed;
      }
      void step() override
      {
        rightText = module->bypassed ? "✔" : "";
        MenuItem::step();
      }
    };

    BypassItem *bypassItem = createMenuItem<BypassItem>("Bypass");
    bypassItem->module = module;
    menu->addChild(bypassItem);

    menu->addChild(new MenuEntry);

    struct PresetItem : MenuItem
    {
      ThreeBandEQ *module;
      int preset;
      void onAction(const event::Action &e) override
      {
        switch (preset)
        {
        case 0: // Flat (Unity)
          module->params[ThreeBandEQ::LOW_GAIN_PARAM].setValue(0.f);
          module->params[ThreeBandEQ::MID_GAIN_PARAM].setValue(0.f);
          module->params[ThreeBandEQ::HIGH_GAIN_PARAM].setValue(0.f);
          break;
        case 1: // Bass Boost
          module->params[ThreeBandEQ::LOW_GAIN_PARAM].setValue(6.f);
          module->params[ThreeBandEQ::MID_GAIN_PARAM].setValue(0.f);
          module->params[ThreeBandEQ::HIGH_GAIN_PARAM].setValue(-3.f);
          break;
        case 2: // Vocal Enhance
          module->params[ThreeBandEQ::LOW_GAIN_PARAM].setValue(-3.f);
          module->params[ThreeBandEQ::MID_GAIN_PARAM].setValue(6.f);
          module->params[ThreeBandEQ::HIGH_GAIN_PARAM].setValue(3.f);
          break;
        case 3: // Bright
          module->params[ThreeBandEQ::LOW_GAIN_PARAM].setValue(-3.f);
          module->params[ThreeBandEQ::MID_GAIN_PARAM].setValue(0.f);
          module->params[ThreeBandEQ::HIGH_GAIN_PARAM].setValue(6.f);
          break;
        case 4: // Warm
          module->params[ThreeBandEQ::LOW_GAIN_PARAM].setValue(4.f);
          module->params[ThreeBandEQ::MID_GAIN_PARAM].setValue(2.f);
          module->params[ThreeBandEQ::HIGH_GAIN_PARAM].setValue(-2.f);
          break;
        case 5: // Smiley (V-shape)
          module->params[ThreeBandEQ::LOW_GAIN_PARAM].setValue(6.f);
          module->params[ThreeBandEQ::MID_GAIN_PARAM].setValue(-6.f);
          module->params[ThreeBandEQ::HIGH_GAIN_PARAM].setValue(6.f);
          break;
        }
      }
    };

    menu->addChild(createMenuLabel("Presets"));

    const char *presetNames[] = {
        "Flat (Unity)",
        "Bass Boost",
        "Vocal Enhance",
        "Bright",
        "Warm",
        "Smiley (V-shape)"};

    for (int i = 0; i < 6; ++i)
    {
      PresetItem *presetItem = createMenuItem<PresetItem>(presetNames[i]);
      presetItem->module = module;
      presetItem->preset = i;
      menu->addChild(presetItem);
    }
  }
};
