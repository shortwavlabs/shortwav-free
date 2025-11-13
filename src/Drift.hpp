#pragma once

#include "plugin.hpp"
#include "dsp/drift.h"

// Drift Module
// - One audio/CV input ("In").
// - One audio/CV output ("Out").
// - Applies a smooth analog-style drift (ShortwavDSP::DriftGenerator) as an
//   additive modulation: Out = In + Drift.
// - Parameters:
//     * DEPTH_PARAM : 0..1, scales DriftGenerator depth (intensity).
//     * RATE_PARAM  : 0.001..2 Hz, maps to DriftGenerator rateHz.
//
// Design:
// - Follows patterns from RandomLfo and Waveshaper modules.
// - Real-time safe: no allocations or locks in process().
// - Deterministic: seeds DriftGenerator using the module instance pointer.
// - Sample-rate aware: updates DriftGenerator on sample-rate change.

struct Drift : Module
{
  enum ParamIds
  {
    DEPTH_PARAM, // 0..1 (normalized drift depth)
    RATE_PARAM,  // Hz, mapped onto underlying DriftGenerator rate
    NUM_PARAMS
  };

  enum InputIds
  {
    IN_INPUT,
    NUM_INPUTS
  };

  enum OutputIds
  {
    OUT_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds
  {
    NUM_LIGHTS
  };

  ShortwavDSP::DriftGenerator drift;

  // Maximum drift amplitude (in volts) applied at DEPTH_PARAM == 1.
  // Chosen to be clearly audible for both CV and audio-rate signals while
  static constexpr float kMaxDriftVolts = .5f;

  Drift()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // Depth:
    // 0.0 = no drift (bypass-like),
    // 1.0 = full configured drift depth (mapped to kMaxDriftVolts).
    configParam(DEPTH_PARAM,
                0.0f,
                10.0f,
                5.0f,
                "Drift depth",
                "",
                0.f,
                1.f);

    // Rate in Hz:
    // 0.001 Hz  (~1000 s) up to 2 Hz.
    // Default 0.1 Hz: very slow analog-style movement.
    configParam(RATE_PARAM,
                0.001f,
                2.0f,
                0.25f,
                "Drift rate",
                " Hz");

    onSampleRateChange();
  }

  void onSampleRateChange() override
  {
    const float sr = APP->engine->getSampleRate();
    drift.setSampleRate(sr);

    // Seed per-instance deterministically based on address,
    // consistent with RandomLfo style.
    const uint32_t seed =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this) & 0xFFFFFFFFu);
    drift.seed(seed);

    // Reset to zero drift at SR change for determinism.
    drift.reset(0.0f);
  }

  void process(const ProcessArgs &args) override;
};

struct DriftWidget : ModuleWidget
{
  DriftWidget(Drift *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/3HP_BLANK.svg")));

    // Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(
        Vec(box.size.x - RACK_GRID_WIDTH,
            RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Layout: similar density/ordering to RandomLfo/Waveshaper.

    // Rate knob (top)
    addParam(createParam<RoundLargeBlackKnob>(
        Vec(5.f, 25.f),
        module,
        Drift::RATE_PARAM));

    // Depth knob (middle)
    addParam(createParam<RoundLargeBlackKnob>(
        Vec(5.f, 75.f),
        module,
        Drift::DEPTH_PARAM));

    // I/O jacks (bottom)
    addInput(createInput<PJ301MPort>(
        Vec(10.f, 140.f),
        module,
        Drift::IN_INPUT));

    addOutput(createOutput<PJ301MPort>(
        Vec(10.f, 190.f),
        module,
        Drift::OUT_OUTPUT));
  }
};

// Model declaration for registration in plugin.cpp.
extern Model *modelDrift;