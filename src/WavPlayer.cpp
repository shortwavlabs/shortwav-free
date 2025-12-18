#include "WavPlayer.hpp"
#include <osdialog.h>

void WavPlayer::process(const ProcessArgs& args)
{
  // Handle transport button triggers
  if (playTrigger_.process(params[PLAY_BUTTON_PARAM].getValue()))
  {
    if (player.isPlaying())
    {
      player.pause();
    }
    else
    {
      player.play();
    }
  }

  if (stopTrigger_.process(params[STOP_BUTTON_PARAM].getValue()))
  {
    player.stop();
    currentSlice_ = 0;
  }

  // Update player parameters
  updatePlayerParameters();

  // Handle external trigger input
  handleTriggerInput();

  // Process audio if file is loaded
  if (fileLoaded_.load() && player.isPlaying())
  {
    // Check slice boundary if slicing is active
    if (!slices_.empty())
    {
      checkSliceBoundary();
      
      // Update current slice based on CV input (for LED display)
      int selectedSlice = getCurrentSlice();
      if (selectedSlice >= 0 && selectedSlice < static_cast<int>(slices_.size()))
      {
        currentSlice_ = selectedSlice;
      }
    }

    // Generate stereo output
    float left, right;
    player.processSampleStereo(left, right);

    outputs[AUDIO_OUTPUT_L].setVoltage(left * 5.0f); // Scale to Eurorack levels
    outputs[AUDIO_OUTPUT_R].setVoltage(right * 5.0f);
  }
  else
  {
    outputs[AUDIO_OUTPUT_L].setVoltage(0.0f);
    outputs[AUDIO_OUTPUT_R].setVoltage(0.0f);
  }

  // Update play LED
  lights[PLAY_LIGHT].setBrightness(player.isPlaying() ? 1.0f : 0.0f);

  // Update slice LEDs
  {
    std::lock_guard<std::mutex> lock(sliceMutex_);
    for (int i = 0; i < 32; ++i)
    {
      if (i < static_cast<int>(slices_.size()) && i == currentSlice_)
      {
        lights[SLICE_LIGHTS + i].setBrightness(1.0f);
      }
      else
      {
        lights[SLICE_LIGHTS + i].setBrightness(0.0f);
      }
    }
  }
}

void WavPlayer::updatePlayerParameters()
{
  // Loop mode: 0=off, 1=forward, 2=pingpong
  int loopMode = static_cast<int>(params[LOOP_MODE_PARAM].getValue());
  switch (loopMode)
  {
  case 0:
    player.setLoopMode(ShortwavDSP::LoopMode::Off);
    break;
  case 1:
    player.setLoopMode(ShortwavDSP::LoopMode::Forward);
    break;
  case 2:
    player.setLoopMode(ShortwavDSP::LoopMode::PingPong);
    break;
  }

  // Reverse
  bool reverse = params[REVERSE_PARAM].getValue() >= 0.5f;
  player.setReverse(reverse);

  // Speed: convert octaves to ratio
  float speedOct = params[SPEED_PARAM].getValue();
  if (inputs[SPEED_CV_INPUT].isConnected())
  {
    float cv = clamp(inputs[SPEED_CV_INPUT].getVoltage(), -10.f, 10.f);
    speedOct += cv; // 1V/oct
  }
  float speedRatio = std::pow(2.0f, speedOct);
  player.setSpeed(speedRatio);

  // Pitch: convert semitones to ratio
  float pitchSt = params[PITCH_PARAM].getValue();
  if (inputs[PITCH_CV_INPUT].isConnected())
  {
    float cv = clamp(inputs[PITCH_CV_INPUT].getVoltage(), -10.f, 10.f);
    pitchSt += cv * 1.2f; // ~1V/oct (12 semitones per volt)
  }
  float pitchRatio = std::pow(2.0f, pitchSt / 12.0f);
  player.setPitch(pitchRatio);

  // Volume
  float volume = params[VOLUME_PARAM].getValue();
  player.setVolume(volume);

  // Interpolation quality
  int quality = static_cast<int>(params[INTERP_QUALITY_PARAM].getValue());
  switch (quality)
  {
  case 0:
    player.setInterpolationQuality(ShortwavDSP::InterpolationQuality::None);
    break;
  case 1:
    player.setInterpolationQuality(ShortwavDSP::InterpolationQuality::Linear);
    break;
  case 2:
  default:
    player.setInterpolationQuality(ShortwavDSP::InterpolationQuality::Cubic);
    break;
  }

  // Check if slice count changed
  static int lastNumSlices = 1;
  int numSlices = static_cast<int>(params[NUM_SLICES_PARAM].getValue());
  if (numSlices != lastNumSlices)
  {
    updateSlices();
    lastNumSlices = numSlices;
    
    // Update slice selector max value to match number of slices
    if (paramQuantities[SLICE_SELECT_PARAM])
    {
      paramQuantities[SLICE_SELECT_PARAM]->maxValue = std::max(0.f, (float)(numSlices - 1));
      // Clamp current value if it's now out of range
      if (params[SLICE_SELECT_PARAM].getValue() > paramQuantities[SLICE_SELECT_PARAM]->maxValue)
      {
        params[SLICE_SELECT_PARAM].setValue(paramQuantities[SLICE_SELECT_PARAM]->maxValue);
      }
    }
  }
}

void WavPlayer::handleTriggerInput()
{
  if (!inputs[TRIGGER_INPUT].isConnected())
  {
    return;
  }

  float triggerVoltage = inputs[TRIGGER_INPUT].getVoltage();
  bool triggerMode = params[TRIGGER_MODE_PARAM].getValue() >= 0.5f; // 0=edge, 1=gate

  if (triggerMode)
  {
    // Gate mode: play while high, stop while low
    bool triggerHigh = triggerVoltage >= 1.0f;
    if (triggerHigh && !lastTriggerState_)
    {
      // Rising edge: start playback
      int slice = getCurrentSlice();
      triggerSlice(slice);
    }
    else if (!triggerHigh && lastTriggerState_)
    {
      // Falling edge: stop playback
      player.stop();
    }
    lastTriggerState_ = triggerHigh;
  }
  else
  {
    // Edge mode: trigger on rising edge only
    if (externalTrigger_.process(triggerVoltage))
    {
      int slice = getCurrentSlice();
      triggerSlice(slice);
    }
  }
}

WavPlayerWidget::WavPlayerWidget(WavPlayer* module)
{
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WavPlayer.svg")));

  // Screws
  addChild(createWidget<ScrewSilver>(Vec(0, 0)));
  addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  // Waveform display (larger for 22HP panel)
  WaveformDisplay* display = new WaveformDisplay();
  display->module = module;
  display->box.pos = Vec(10, 30);
  display->box.size = Vec(box.size.x - 20, 105);
  addChild(display);
  if (module)
  {
    module->waveformDisplay_ = display;
  }

  // Transport controls (row 1)
  float yPos = 150;
  addParam(createParam<LEDButton>(Vec(17, yPos), module, WavPlayer::PLAY_BUTTON_PARAM));
  addChild(createLight<MediumLight<GreenLight>>(Vec(21, yPos + 4), module, WavPlayer::PLAY_LIGHT));
  addParam(createParam<LEDButton>(Vec(52, yPos), module, WavPlayer::STOP_BUTTON_PARAM));
  
  // Loop and reverse switches
  addParam(createParam<CKSSThree>(Vec(95, yPos + 2), module, WavPlayer::LOOP_MODE_PARAM));
  addParam(createParam<CKSS>(Vec(145, yPos + 2), module, WavPlayer::REVERSE_PARAM));

  // Speed, pitch, and volume knobs (row 2)
  yPos = 190;
  addParam(createParam<RoundBlackKnob>(Vec(13, yPos), module, WavPlayer::SPEED_PARAM));
  addParam(createParam<RoundBlackKnob>(Vec(68, yPos), module, WavPlayer::PITCH_PARAM));
  addParam(createParam<RoundBlackKnob>(Vec(123, yPos), module, WavPlayer::VOLUME_PARAM));

  // CV inputs for speed, pitch, zoom (row 3)
  yPos = 238;
  addInput(createInput<PJ301MPort>(Vec(18, yPos), module, WavPlayer::SPEED_CV_INPUT));
  addInput(createInput<PJ301MPort>(Vec(73, yPos), module, WavPlayer::PITCH_CV_INPUT));
  addParam(createParam<RoundSmallBlackKnob>(Vec(128, yPos + 3), module, WavPlayer::ZOOM_PARAM));

  // Slice controls (row 4)
  yPos = 280;
  addParam(createParam<RoundBlackSnapKnob>(Vec(13, yPos), module, WavPlayer::NUM_SLICES_PARAM));
  addParam(createParam<RoundBlackSnapKnob>(Vec(68, yPos), module, WavPlayer::SLICE_SELECT_PARAM));

  // Slice CV and trigger inputs (row 5)
  yPos = 328;
  addInput(createInput<PJ301MPort>(Vec(18, yPos), module, WavPlayer::SLICE_CV_INPUT));
  addInput(createInput<PJ301MPort>(Vec(73, yPos), module, WavPlayer::TRIGGER_INPUT));
  
  // Trigger mode and interpolation switches
  addParam(createParam<CKSS>(Vec(130, yPos + 3), module, WavPlayer::TRIGGER_MODE_PARAM));
  addParam(createParam<CKSSThree>(Vec(168, yPos + 1), module, WavPlayer::INTERP_QUALITY_PARAM));

  // Audio outputs (row 6)
  yPos = 365;
  addOutput(createOutput<PJ301MPort>(Vec(18, yPos), module, WavPlayer::AUDIO_OUTPUT_L));
  addOutput(createOutput<PJ301MPort>(Vec(73, yPos), module, WavPlayer::AUDIO_OUTPUT_R));
}

void WavPlayerWidget::appendContextMenu(Menu* menu)
{
  WavPlayer* module = dynamic_cast<WavPlayer*>(this->module);
  if (!module)
    return;

  menu->addChild(new MenuEntry);
  menu->addChild(createMenuLabel("WAV Player"));

  // Load file menu item
  struct LoadFileItem : MenuItem
  {
    WavPlayer* module;
    void onAction(const event::Action& e) override
    {
      osdialog_filters* filters = osdialog_filters_parse("WAV files:wav,WAV");
      
      char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
      if (path)
      {
        module->loadFileAsync(path);
        std::free(path);
      }
      osdialog_filters_free(filters);
    }
  };

  LoadFileItem* loadItem = new LoadFileItem();
  loadItem->text = "Load WAV file...";
  loadItem->module = module;
  menu->addChild(loadItem);

  // Show current file
  if (!module->fileName_.empty())
  {
    menu->addChild(createMenuLabel("File: " + module->fileName_));
    
    if (module->fileLoaded_.load())
    {
      char info[256];
      snprintf(info, sizeof(info), "%.1fs, %dHz, %dch, %dbit",
               module->player.getDurationSeconds(),
               module->player.getFileSampleRate(),
               module->player.getNumChannels(),
               module->player.getBitsPerSample());
      menu->addChild(createMenuLabel(info));
    }
  }

  // Clear file menu item
  struct ClearFileItem : MenuItem
  {
    WavPlayer* module;
    void onAction(const event::Action& e) override
    {
      module->player.stop();
      module->player.unload();
      module->fileLoaded_.store(false);
      module->filePath_.clear();
      module->fileName_.clear();
      module->slices_.clear();
    }
  };

  if (module->fileLoaded_.load())
  {
    ClearFileItem* clearItem = new ClearFileItem();
    clearItem->text = "Clear file";
    clearItem->module = module;
    menu->addChild(clearItem);
  }

  menu->addChild(new MenuEntry);

  // Slice reorder submenu (advanced feature)
  struct SliceReorderMenu : MenuItem
  {
    WavPlayer* module;
    Menu* createChildMenu() override
    {
      Menu* menu = new Menu();
      
      std::lock_guard<std::mutex> lock(module->sliceMutex_);
      
      if (module->slices_.empty())
      {
        menu->addChild(createMenuLabel("No slices"));
        return menu;
      }

      for (size_t i = 0; i < module->slices_.size(); ++i)
      {
        char label[32];
        snprintf(label, sizeof(label), "Slice %zu (order: %d)", i, module->slices_[i].order);
        menu->addChild(createMenuLabel(label));
      }

      return menu;
    }
  };

  if (!module->slices_.empty())
  {
    SliceReorderMenu* reorderMenu = new SliceReorderMenu();
    reorderMenu->text = "Slice order";
    reorderMenu->rightText = RIGHT_ARROW;
    reorderMenu->module = module;
    menu->addChild(reorderMenu);
  }
}

Model* modelWavPlayer = createModel<WavPlayer, WavPlayerWidget>("WavPlayer");
