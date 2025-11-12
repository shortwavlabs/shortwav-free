# ShortwavFree Copilot Instructions

## Project Overview
ShortwavFree is a Rack modular synthesizer plugin (version 2.0.0) developed by Shortwav Labs. Currently contains one module: **RandomLfo** – a smooth random LFO generator for CV modulation.

**Key Architecture**: Plugin built on Rack-SDK framework. Single module with DSP engine, UI widget, and parameter/port definitions.

---

## Directory Structure & Key Files

```
src/
  plugin.cpp           # Plugin initialization (registers modules)
  plugin.hpp           # Global plugin declarations & extern declarations
  RandomLfo.cpp        # Module processor logic (CV modulation, output conversion)
  RandomLfo.hpp        # Module definition: params, inputs, outputs, widget UI
  dsp/random-lfo.h     # DSP core: second-order smoothing system for random targets
res/                   # SVG graphics (panels, knobs); currently "3HP.svg"
plugin.json            # Plugin metadata (name, version, author, module definitions)
Makefile               # Rack plugin build system (wraps Rack-SDK's plugin.mk)
{build,clean,install}.sh  # Shell shortcuts to Makefile targets
```

---

## Critical Architecture Patterns

### 1. **Module Lifecycle & Plugin Registration**
- **plugin.cpp** calls `p->addModel(modelRandomlfo)` in `init()`. This registers the module with Rack at startup.
- **plugin.hpp** declares `extern Model *modelRandomlfo` (defined in RandomLfo.cpp).
- Adding a new module: create `NewModule.hpp/.cpp`, declare `extern Model *modelNewModule` in plugin.hpp, add `p->addModel(modelNewModule)` in plugin.cpp's `init()`.

### 2. **Module Structure (RandomLfo Example)**
Modules inherit from `Module` and define:
- **ParamIds** enum: knobs/switches (e.g., RATE_PARAM, BIPOLAR_PARAM)
- **InputIds** enum: CV/audio inputs (e.g., RATE_CV_INPUT)
- **OutputIds** enum: audio/CV outputs (e.g., LFO_OUTPUT)
- **LightIds** enum: indicator LEDs (empty for RandomLfo)
- **Config** in constructor via `config(numParams, numInputs, numOutputs, numLights)` + `configParam()` calls
- **process()** method: runs at audio rate, reads inputs/params, writes outputs

### 3. **Parameter & CV Modulation Pattern**
In `RandomLfo::process()`:
- Read base param value: `params[RATE_PARAM].getValue()`
- Check if input connected: `inputs[RATE_CV_INPUT].isConnected()`
- Apply CV as multiplicative (rate) or additive (depth/smooth) modulation
- **Voltage conventions**: 1V/oct for frequency, 0-10V for normalized parameters
- **Always clamp** values after CV blending to prevent DSP artifacts
- Output voltage scaling: bipolar→[-5V,+5V], unipolar→[0V,+10V]

### 4. **DSP Engine Integration (random-lfo.h)**
- **No allocations, no locks**: real-time safe; suitable for per-sample processing
- Core algorithm: second-order critically-damped system tracking random targets at configurable rate
- Public interface:
  - `setSampleRate(sr)` – call once on init or rate change
  - `setRate(hz)` – how often new random targets spawn
  - `setDepth(0..1)` – output gain
  - `setSmooth(0..1)` – damping/correlation (0=sharp, 1=smooth)
  - `setBipolar(bool)` – output range
  - `seed(uint32_t)` – for deterministic but distinct instances
  - `processSample()` – call per audio sample; returns float in [-1,1] or [0,1]
- **Internal RNG**: fast LCG, seeded with module pointer for determinism across instances

### 5. **UI Widget Pattern (RandomLfoWidget)**
- Inherits from `ModuleWidget`; call `setModule(module)` in constructor
- Load SVG panel: `APP->window->loadSvg(asset::plugin(pluginInstance, "res/3HP.svg"))`
- Add visual elements: screws, knobs, ports, lights via `addChild()` and `addParam()`/`addInput()` helpers
- Positions are in pixel units; use `Vec(x, y)` and standard Rack component dimensions

---

## Build & Deployment

### Local Build
```bash
./build.sh              # Builds distribution package
./clean.sh              # Cleans build artifacts
./install.sh            # Installs to local Rack plugin directory
```

**Under the hood**: All call `RACK_DIR=./dep/Rack-SDK make [target]`. Requires:
- Rack-SDK in `dep/Rack-SDK/` (fetched separately or via submodule)
- C++17 compiler (g++/clang)
- Make

### plugin.json Metadata
- `slug`: identifier (used in file paths, URLs)
- `version`: semantic versioning
- `modules[].slug`, `.name`, `.description`, `.tags`: Rack browser integration

---

## Common Development Patterns & Conventions

### DSP Precision
- Use `float` (32-bit) for all audio; DSP classes assume this
- Avoid `double` except for phase accumulators if 32-bit causes beating artifacts
- Clamp CV inputs immediately after reading to prevent NaN/inf propagation

### Parameter Naming & Defaults
- Use descriptive names with units in `configParam()` labels: `"Rate", " Hz"`
- Respect Rack conventions: frequency in Hz, amplitude 0-1 or 0-10V mapped to 0-1, bipolar/unipolar toggles at 0.f/1.f
- Set sensible defaults in `configParam(id, min, max, default, label, unit)` – RandomLfo uses 0.75f for smoothness, 1.f for depth

### Sample Rate Changes
- Override `onSampleRateChange()` in Module constructor or processor if DSP depends on SR
- RandomLfo seeds its RNG here and calls `lfo.setSampleRate()`
- Use `APP->engine->getSampleRate()` to query current rate

### Memory & Real-Time Safety
- No `new`/`delete` in audio path; use stack allocation or member fields
- RandomLfo embeds `ShortwavDSP::RandomLFO lfo` as a member (stack-allocated)
- No mutex locks in `process()`; shared state must be atomic or lock-free

---

## Adding a New Module

1. **Create header** (`src/NewModule.hpp`): Define struct inheriting `Module`, declare ParamIds/InputIds/OutputIds, implement UI widget
2. **Create implementation** (`src/NewModule.cpp`): Implement `process()` method, call `createModel<>()` at end, export `extern Model *modelNewModule`
3. **Update plugin.hpp**: Add `extern Model *modelNewModule`
4. **Update plugin.cpp**: Add `p->addModel(modelNewModule)` in `init()`
5. **Update plugin.json**: Add module metadata under `modules[]`
6. **Add resources**: SVG panel files in `res/` as needed

---

## Debugging Tips

- **Build errors**: Check `RACK_DIR` path in Makefile or shell scripts; ensure Rack-SDK is present
- **Module not loading**: Verify `slug` matches in both RandomLfo.cpp and plugin.json
- **Audio clicks/pops**: Likely DC bias or discontinuities; add small smoothing filter or check CV clipping
- **Visual corruption**: Check SVG path and pixel coordinates in widget (should align with panel dimensions)

---

## References
- **Rack API**: https://vcvrack.com/manual/PluginDevelopment.html
- **DSP Algorithm**: https://www.musicdsp.org/en/latest/Synthesis/269-smooth-random-lfo-generator.html (basis for RandomLFO class)
