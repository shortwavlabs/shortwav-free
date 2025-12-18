#pragma once

#include "plugin.hpp"
#include "dsp/wav-player.h"
#include <thread>
#include <atomic>

// Forward declare display widget
struct WaveformDisplay;

struct WavPlayer : Module
{
  enum ParamIds
  {
    PLAY_BUTTON_PARAM,      // Play/pause trigger
    STOP_BUTTON_PARAM,      // Stop playback
    LOOP_MODE_PARAM,        // Loop mode selector (0=off, 1=forward, 2=pingpong)
    REVERSE_PARAM,          // Reverse playback toggle
    SPEED_PARAM,            // Playback speed control (-2 to +2 octaves)
    PITCH_PARAM,            // Pitch shift control (-12 to +12 semitones)
    VOLUME_PARAM,           // Output volume
    ZOOM_PARAM,             // Waveform zoom level
    NUM_SLICES_PARAM,       // Number of slices (1=off, 2-32)
    SLICE_SELECT_PARAM,     // Manual slice selector
    TRIGGER_MODE_PARAM,     // Trigger mode (0=edge, 1=gate)
    INTERP_QUALITY_PARAM,   // Interpolation quality
    NUM_PARAMS
  };

  enum InputIds
  {
    TRIGGER_INPUT,          // Trigger/gate input
    SLICE_CV_INPUT,         // CV input for slice selection (0-10V)
    SPEED_CV_INPUT,         // CV modulation for speed
    PITCH_CV_INPUT,         // CV modulation for pitch
    NUM_INPUTS
  };

  enum OutputIds
  {
    AUDIO_OUTPUT_L,         // Left audio output
    AUDIO_OUTPUT_R,         // Right audio output
    NUM_OUTPUTS
  };

  enum LightIds
  {
    PLAY_LIGHT,             // Play button LED
    ENUMS(SLICE_LIGHTS, 32), // Slice indicator LEDs
    NUM_LIGHTS
  };

  // DSP engine
  ShortwavDSP::WavPlayer player;

  // File loading state (thread-safe)
  std::atomic<bool> fileLoading_{false};
  std::atomic<bool> fileLoaded_{false};
  std::atomic<float> loadProgress_{0.0f};
  std::string filePath_;
  std::string fileName_;
  std::mutex fileMutex_;

  // Slice management
  struct SliceInfo
  {
    size_t startSample;
    size_t endSample;
    int order;              // For slice reordering
  };
  std::vector<SliceInfo> slices_;
  int currentSlice_ = 0;
  std::mutex sliceMutex_;

  // Trigger state
  dsp::SchmittTrigger playTrigger_;
  dsp::SchmittTrigger stopTrigger_;
  dsp::SchmittTrigger externalTrigger_;
  bool lastTriggerState_ = false;

  // Waveform display reference (set by widget)
  WaveformDisplay* waveformDisplay_ = nullptr;

  WavPlayer()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // Transport controls
    configParam(PLAY_BUTTON_PARAM, 0.f, 1.f, 0.f, "Play/Pause");
    configParam(STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "Stop");
    
    // Loop and reverse
    configParam(LOOP_MODE_PARAM, 0.f, 2.f, 0.f, "Loop Mode");
    configParam(REVERSE_PARAM, 0.f, 1.f, 0.f, "Reverse");

    // Speed: -2 to +2 octaves (0.25x to 4x)
    configParam(SPEED_PARAM, -2.f, 2.f, 0.f, "Speed", " oct");
    
    // Pitch: -12 to +12 semitones
    configParam(PITCH_PARAM, -12.f, 12.f, 0.f, "Pitch", " st");
    
    // Volume: 0 to 2 (unity at 1.0)
    configParam(VOLUME_PARAM, 0.f, 2.f, 1.f, "Volume");
    
    // Zoom: 1x to 100x
    configParam(ZOOM_PARAM, 0.f, 1.f, 0.f, "Zoom");

    // Slicing
    configParam(NUM_SLICES_PARAM, 1.f, 32.f, 1.f, "Slices");
    configParam(SLICE_SELECT_PARAM, 0.f, 31.f, 0.f, "Slice");

    // Trigger mode
    configParam(TRIGGER_MODE_PARAM, 0.f, 1.f, 0.f, "Trigger Mode");

    // Interpolation quality
    configParam(INTERP_QUALITY_PARAM, 0.f, 2.f, 2.f, "Quality");

    // Configure inputs
    configInput(TRIGGER_INPUT, "Trigger/Gate");
    configInput(SLICE_CV_INPUT, "Slice CV");
    configInput(SPEED_CV_INPUT, "Speed CV");
    configInput(PITCH_CV_INPUT, "Pitch CV");

    // Configure outputs
    configOutput(AUDIO_OUTPUT_L, "Audio L");
    configOutput(AUDIO_OUTPUT_R, "Audio R");

    onSampleRateChange();
  }

  ~WavPlayer()
  {
    // Ensure clean shutdown
    player.stop();
    player.unload();
  }

  void onSampleRateChange() override
  {
    float sr = APP->engine->getSampleRate();
    player.setSampleRate(sr);
  }

  void process(const ProcessArgs& args) override;

  // Internal helper methods
  void updatePlayerParameters();
  void handleTriggerInput();

  // File loading (async, thread-safe)
  void loadFileAsync(const std::string& path)
  {
    if (fileLoading_.load())
    {
      return; // Already loading
    }

    fileLoading_.store(true);
    fileLoaded_.store(false);
    loadProgress_.store(0.0f);

    // Launch loading thread
    std::thread([this, path]() {
      std::lock_guard<std::mutex> lock(fileMutex_);
      
      loadProgress_.store(0.2f);
      auto result = player.loadFile(path.c_str());
      loadProgress_.store(0.8f);

      if (result == ShortwavDSP::WavError::None)
      {
        filePath_ = path;
        // Extract filename from path
        size_t lastSlash = path.find_last_of("/\\");
        fileName_ = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
        
        fileLoaded_.store(true);
        updateSlices();
      }
      else
      {
        WARN("Failed to load WAV file: %s", ShortwavDSP::wavErrorToString(result));
        fileLoaded_.store(false);
      }

      loadProgress_.store(1.0f);
      fileLoading_.store(false);
    }).detach();
  }

  // Update slice boundaries based on NUM_SLICES_PARAM
  void updateSlices()
  {
    std::lock_guard<std::mutex> lock(sliceMutex_);
    
    int numSlices = static_cast<int>(params[NUM_SLICES_PARAM].getValue());
    numSlices = clamp(numSlices, 1, 32);

    slices_.clear();
    
    if (numSlices <= 1 || !fileLoaded_.load())
    {
      return; // No slicing
    }

    size_t totalSamples = player.getNumSamples();
    size_t samplesPerSlice = totalSamples / numSlices;

    for (int i = 0; i < numSlices; ++i)
    {
      SliceInfo slice;
      slice.startSample = i * samplesPerSlice;
      slice.endSample = (i == numSlices - 1) ? totalSamples : (i + 1) * samplesPerSlice;
      slice.order = i;
      slices_.push_back(slice);
    }
  }

  // Get current slice based on CV input or manual selection
  int getCurrentSlice()
  {
    std::lock_guard<std::mutex> lock(sliceMutex_);
    
    if (slices_.empty())
    {
      return -1; // No slicing active
    }

    int sliceIdx = 0;

    if (inputs[SLICE_CV_INPUT].isConnected())
    {
      // Map 0-10V to slice indices
      float cv = clamp(inputs[SLICE_CV_INPUT].getVoltage(), 0.f, 10.f);
      sliceIdx = static_cast<int>(cv / 10.f * slices_.size());
      sliceIdx = clamp(sliceIdx, 0, static_cast<int>(slices_.size()) - 1);
    }
    else
    {
      sliceIdx = static_cast<int>(params[SLICE_SELECT_PARAM].getValue());
      sliceIdx = clamp(sliceIdx, 0, static_cast<int>(slices_.size()) - 1);
    }

    return sliceIdx;
  }

  // Trigger slice playback
  void triggerSlice(int sliceIdx)
  {
    std::lock_guard<std::mutex> lock(sliceMutex_);
    
    if (sliceIdx < 0 || sliceIdx >= static_cast<int>(slices_.size()))
    {
      // Play full file
      bool isReverse = player.getReverse();
      if (isReverse)
      {
        // Seek to end of file when reversed
        player.seekToSample(player.getNumSamples() - 1);
      }
      else
      {
        player.seek(0.0f);
      }
      player.play();
      return;
    }

    const SliceInfo& slice = slices_[sliceIdx];
    bool isReverse = player.getReverse();
    
    if (isReverse)
    {
      // When reversed, start from the end of the slice
      player.seekToSample(slice.endSample - 1);
    }
    else
    {
      // Normal: start from the beginning of the slice
      player.seekToSample(slice.startSample);
    }
    
    player.play();
    currentSlice_ = sliceIdx;
  }

  // Check if playback should stop at slice boundary
  bool checkSliceBoundary()
  {
    std::lock_guard<std::mutex> lock(sliceMutex_);
    
    if (slices_.empty() || currentSlice_ < 0)
    {
      return false;
    }

    if (currentSlice_ >= static_cast<int>(slices_.size()))
    {
      return false;
    }

    const SliceInfo& slice = slices_[currentSlice_];
    double currentPos = player.getPlaybackPositionSamples();
    bool isReverse = player.getReverse();

    // Check if we've passed the boundary of the current slice
    if (isReverse)
    {
      // When reversed, stop if we go before the start
      if (currentPos < static_cast<double>(slice.startSample))
      {
        player.stop();
        return true;
      }
    }
    else
    {
      // Normal: stop if we go past the end
      if (currentPos >= static_cast<double>(slice.endSample))
      {
        player.stop();
        return true;
      }
    }

    return false;
  }

  // JSON state save/load
  json_t* dataToJson() override
  {
    json_t* rootJ = json_object();

    // Save file path
    if (!filePath_.empty())
    {
      json_object_set_new(rootJ, "filePath", json_string(filePath_.c_str()));
    }

    // Save slice order (for reordering feature)
    json_t* sliceOrderJ = json_array();
    {
      std::lock_guard<std::mutex> lock(sliceMutex_);
      for (const auto& slice : slices_)
      {
        json_array_append_new(sliceOrderJ, json_integer(slice.order));
      }
    }
    json_object_set_new(rootJ, "sliceOrder", sliceOrderJ);

    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override
  {
    // Load file path
    json_t* filePathJ = json_object_get(rootJ, "filePath");
    if (filePathJ)
    {
      std::string path = json_string_value(filePathJ);
      if (!path.empty())
      {
        loadFileAsync(path);
      }
    }

    // Load slice order
    json_t* sliceOrderJ = json_object_get(rootJ, "sliceOrder");
    if (sliceOrderJ && json_is_array(sliceOrderJ))
    {
      std::lock_guard<std::mutex> lock(sliceMutex_);
      size_t numSlices = json_array_size(sliceOrderJ);
      for (size_t i = 0; i < numSlices && i < slices_.size(); ++i)
      {
        json_t* orderJ = json_array_get(sliceOrderJ, i);
        if (orderJ)
        {
          slices_[i].order = json_integer_value(orderJ);
        }
      }
    }
  }
};

// Waveform display widget
struct WaveformDisplay : TransparentWidget
{
  WavPlayer* module = nullptr;
  float zoom = 1.0f;
  float scrollPos = 0.0f;

  WaveformDisplay()
  {
    box.size = Vec(300, 100);
  }

  void drawLayer(const DrawArgs& args, int layer) override
  {
    if (layer != 1)
      return;

    if (!module || !module->fileLoaded_.load())
    {
      // Draw empty state
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
      nvgFillColor(args.vg, nvgRGBA(20, 20, 20, 255));
      nvgFill(args.vg);

      nvgFontSize(args.vg, 12);
      nvgFontFaceId(args.vg, APP->window->uiFont->handle);
      nvgFillColor(args.vg, nvgRGBA(150, 150, 150, 255));
      nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
      nvgText(args.vg, box.size.x * 0.5f, box.size.y * 0.5f, "No file loaded", NULL);
      return;
    }

    // Background
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(args.vg, nvgRGBA(10, 10, 10, 255));
    nvgFill(args.vg);

    // Draw waveform
    drawWaveform(args);

    // Draw slice boundaries
    drawSliceBoundaries(args);

    // Draw playback position
    drawPlaybackPosition(args);

    // Border
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
    nvgStrokeColor(args.vg, nvgRGBA(100, 100, 100, 255));
    nvgStrokeWidth(args.vg, 1.0f);
    nvgStroke(args.vg);
  }

  void drawWaveform(const DrawArgs& args)
  {
    size_t numSamples = module->player.getNumSamples();
    if (numSamples == 0)
      return;

    uint16_t numChannels = module->player.getNumChannels();
    float zoomLevel = std::pow(10.0f, module->params[WavPlayer::ZOOM_PARAM].getValue() * 2.0f);
    
    size_t visibleSamples = static_cast<size_t>(numSamples / zoomLevel);
    size_t startSample = static_cast<size_t>(scrollPos * numSamples);
    size_t endSample = std::min(startSample + visibleSamples, numSamples);

    if (startSample >= endSample)
      return;

    // Downsample for display
    size_t displayPoints = static_cast<size_t>(box.size.x);
    size_t samplesPerPoint = std::max<size_t>(1, (endSample - startSample) / displayPoints);

    nvgBeginPath(args.vg);
    
    for (size_t i = 0; i < displayPoints; ++i)
    {
      size_t sampleStart = startSample + i * samplesPerPoint;
      size_t sampleEnd = std::min(sampleStart + samplesPerPoint, endSample);
      
      if (sampleStart >= numSamples)
        break;

      // Find min/max in this range
      float minVal = 0.0f, maxVal = 0.0f;
      for (size_t s = sampleStart; s < sampleEnd; ++s)
      {
        float val = 0.0f;
        for (uint16_t c = 0; c < numChannels; ++c)
        {
          val += module->player.getRawSample(s, c);
        }
        val /= numChannels; // Average channels

        minVal = std::min(minVal, val);
        maxVal = std::max(maxVal, val);
      }

      float x = (float)i / displayPoints * box.size.x;
      float yMin = (1.0f - (minVal + 1.0f) * 0.5f) * box.size.y;
      float yMax = (1.0f - (maxVal + 1.0f) * 0.5f) * box.size.y;

      if (i == 0)
      {
        nvgMoveTo(args.vg, x, (yMin + yMax) * 0.5f);
      }
      
      nvgLineTo(args.vg, x, yMin);
      nvgLineTo(args.vg, x, yMax);
    }

    nvgStrokeColor(args.vg, nvgRGBA(0, 200, 255, 200));
    nvgStrokeWidth(args.vg, 1.0f);
    nvgStroke(args.vg);
  }

  void drawSliceBoundaries(const DrawArgs& args)
  {
    std::lock_guard<std::mutex> lock(module->sliceMutex_);
    
    if (module->slices_.empty())
      return;

    size_t numSamples = module->player.getNumSamples();
    if (numSamples == 0)
      return;

    nvgStrokeColor(args.vg, nvgRGBA(255, 255, 0, 150));
    nvgStrokeWidth(args.vg, 1.0f);

    for (const auto& slice : module->slices_)
    {
      float x = ((float)slice.startSample / numSamples) * box.size.x;
      
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, x, 0);
      nvgLineTo(args.vg, x, box.size.y);
      nvgStroke(args.vg);
    }
  }

  void drawPlaybackPosition(const DrawArgs& args)
  {
    if (!module->player.isPlaying())
      return;

    float pos = module->player.getPlaybackPosition();
    float x = pos * box.size.x;

    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, x, 0);
    nvgLineTo(args.vg, x, box.size.y);
    nvgStrokeColor(args.vg, nvgRGBA(255, 100, 100, 255));
    nvgStrokeWidth(args.vg, 2.0f);
    nvgStroke(args.vg);
  }

  void onButton(const event::Button& e) override
  {
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT)
    {
      // Seek on click
      if (module && module->fileLoaded_.load())
      {
        float pos = e.pos.x / box.size.x;
        module->player.seek(clamp(pos, 0.0f, 1.0f));
      }
      e.consume(this);
    }
    TransparentWidget::onButton(e);
  }
};

struct WavPlayerWidget : ModuleWidget
{
  WavPlayerWidget(WavPlayer* module);
  void appendContextMenu(Menu* menu) override;
};
