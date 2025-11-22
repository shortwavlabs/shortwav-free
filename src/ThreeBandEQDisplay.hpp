#pragma once

#include "plugin.hpp"
#include "dsp/3-band-eq.h"
#include <cmath>

// Forward declaration
struct ThreeBandEQ;

// Frequency Response Display Widget
// Visualizes the EQ curve with low/mid/high band response
struct EQFrequencyResponseDisplay : TransparentWidget
{
  Module *module = nullptr;
  
  // Frequency range for display (20 Hz - 20 kHz)
  const float minFreq = 20.f;
  const float maxFreq = 20000.f;
  
  // Number of points to calculate for curve
  const int numPoints = 200;

  void drawLayer(const DrawArgs &args, int layer) override
  {
    if (layer != 1)
      return;
    
    if (!module)
    {
      drawPlaceholder(args);
      return;
    }

    NVGcontext *vg = args.vg;
    
    // Get current EQ parameters using enum values 0-5
    float lowFreq = module->params[0].getValue();  // LOW_FREQ_PARAM
    float highFreq = module->params[1].getValue(); // HIGH_FREQ_PARAM
    float lowGain = module->params[2].getValue();  // LOW_GAIN_PARAM
    float midGain = module->params[3].getValue();  // MID_GAIN_PARAM
    float highGain = module->params[4].getValue(); // HIGH_GAIN_PARAM
    
    // Apply CV modulation if connected
    if (module->inputs[2].isConnected()) // LOW_FREQ_CV_INPUT
    {
      float cv = module->inputs[2].getVoltage();
      lowFreq += cv * 17.f; // 0-10V = 0-170Hz range
      lowFreq = clamp(lowFreq, 80.f, 250.f);
    }
    
    if (module->inputs[3].isConnected()) // HIGH_FREQ_CV_INPUT
    {
      float cv = module->inputs[3].getVoltage();
      highFreq += cv * 300.f; // 0-10V = 0-3000Hz range
      highFreq = clamp(highFreq, 1000.f, 4000.f);
    }
    
    if (module->inputs[4].isConnected()) // LOW_GAIN_CV_INPUT
    {
      float cv = module->inputs[4].getVoltage();
      lowGain += cv * 2.4f; // ±5V = ±12dB
      lowGain = clamp(lowGain, -12.f, 12.f);
    }
    
    if (module->inputs[5].isConnected()) // MID_GAIN_CV_INPUT
    {
      float cv = module->inputs[5].getVoltage();
      midGain += cv * 2.4f;
      midGain = clamp(midGain, -12.f, 12.f);
    }
    
    if (module->inputs[6].isConnected()) // HIGH_GAIN_CV_INPUT
    {
      float cv = module->inputs[6].getVoltage();
      highGain += cv * 2.4f;
      highGain = clamp(highGain, -12.f, 12.f);
    }

    // Draw background
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 200));
    nvgFill(vg);

    // Draw frequency grid lines
    drawFrequencyGrid(vg);
    
    // Draw gain grid lines
    drawGainGrid(vg);
    
    // Draw frequency band regions
    drawBandRegions(vg, lowFreq, highFreq);
    
    // Draw frequency response curve
    drawResponseCurve(vg, lowFreq, highFreq, lowGain, midGain, highGain);
    
    // Draw crossover frequency markers
    drawCrossoverMarkers(vg, lowFreq, highFreq);
    
    // Draw gain labels
    drawGainLabels(vg, lowGain, midGain, highGain);
  }

  void drawPlaceholder(const DrawArgs &args)
  {
    NVGcontext *vg = args.vg;
    
    // Dark background
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 200));
    nvgFill(vg);
    
    // "EQ Display" text
    nvgFontSize(vg, 12);
    nvgFontFaceId(vg, APP->window->uiFont->handle);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, nvgRGBA(150, 150, 150, 255));
    nvgText(vg, box.size.x * 0.5f, box.size.y * 0.5f, "EQ Response", NULL);
  }

  void drawFrequencyGrid(NVGcontext *vg)
  {
    // Draw vertical lines at key frequencies: 100Hz, 1kHz, 10kHz
    float frequencies[] = {100.f, 1000.f, 10000.f};
    
    nvgStrokeColor(vg, nvgRGBA(60, 60, 60, 255));
    nvgStrokeWidth(vg, 1.0f);
    
    for (float freq : frequencies)
    {
      float x = freqToX(freq);
      nvgBeginPath(vg);
      nvgMoveTo(vg, x, 0);
      nvgLineTo(vg, x, box.size.y);
      nvgStroke(vg);
      
      // Label
      nvgFontSize(vg, 8);
      nvgFontFaceId(vg, APP->window->uiFont->handle);
      nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
      nvgFillColor(vg, nvgRGBA(120, 120, 120, 255));
      
      std::string label;
      if (freq >= 1000.f)
        label = string::f("%.0fk", freq / 1000.f);
      else
        label = string::f("%.0f", freq);
      
      nvgText(vg, x, box.size.y - 12, label.c_str(), NULL);
    }
  }

  void drawGainGrid(NVGcontext *vg)
  {
    // Draw horizontal lines at 0dB, ±6dB, ±12dB
    float gains[] = {-12.f, -6.f, 0.f, 6.f, 12.f};
    
    for (float gain : gains)
    {
      float y = gainToY(gain);
      
      // Thicker line for 0dB
      if (std::abs(gain) < 0.1f)
      {
        nvgStrokeColor(vg, nvgRGBA(100, 100, 100, 255));
        nvgStrokeWidth(vg, 1.5f);
      }
      else
      {
        nvgStrokeColor(vg, nvgRGBA(50, 50, 50, 255));
        nvgStrokeWidth(vg, 1.0f);
      }
      
      nvgBeginPath(vg);
      nvgMoveTo(vg, 0, y);
      nvgLineTo(vg, box.size.x, y);
      nvgStroke(vg);
      
      // Label
      nvgFontSize(vg, 8);
      nvgFontFaceId(vg, APP->window->uiFont->handle);
      nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
      nvgFillColor(vg, nvgRGBA(120, 120, 120, 255));
      nvgText(vg, 2, y, string::f("%+.0f", gain).c_str(), NULL);
    }
  }

  void drawBandRegions(NVGcontext *vg, float lowFreq, float highFreq)
  {
    float lowX = freqToX(lowFreq);
    float highX = freqToX(highFreq);
    
    // Low band (red tint)
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, lowX, box.size.y);
    nvgFillColor(vg, nvgRGBA(80, 20, 20, 40));
    nvgFill(vg);
    
    // Mid band (green tint)
    nvgBeginPath(vg);
    nvgRect(vg, lowX, 0, highX - lowX, box.size.y);
    nvgFillColor(vg, nvgRGBA(20, 80, 20, 40));
    nvgFill(vg);
    
    // High band (blue tint)
    nvgBeginPath(vg);
    nvgRect(vg, highX, 0, box.size.x - highX, box.size.y);
    nvgFillColor(vg, nvgRGBA(20, 20, 80, 40));
    nvgFill(vg);
  }

  void drawResponseCurve(NVGcontext *vg, float lowFreq, float highFreq, 
                         float lowGain, float midGain, float highGain)
  {
    nvgBeginPath(vg);
    
    bool firstPoint = true;
    for (int i = 0; i < numPoints; i++)
    {
      float t = (float)i / (numPoints - 1);
      float freq = minFreq * std::pow(maxFreq / minFreq, t);
      float gain = approximateGainAtFreq(freq, lowFreq, highFreq, lowGain, midGain, highGain);
      
      float x = freqToX(freq);
      float y = gainToY(gain);
      
      if (firstPoint)
      {
        nvgMoveTo(vg, x, y);
        firstPoint = false;
      }
      else
      {
        nvgLineTo(vg, x, y);
      }
    }
    
    nvgStrokeColor(vg, nvgRGBA(255, 200, 100, 255));
    nvgStrokeWidth(vg, 2.5f);
    nvgStroke(vg);
  }

  void drawCrossoverMarkers(NVGcontext *vg, float lowFreq, float highFreq)
  {
    // Draw crossover frequency markers
    nvgStrokeColor(vg, nvgRGBA(255, 100, 100, 200));
    nvgStrokeWidth(vg, 2.0f);
    
    float lowX = freqToX(lowFreq);
    nvgBeginPath(vg);
    nvgMoveTo(vg, lowX, 0);
    nvgLineTo(vg, lowX, box.size.y);
    nvgStroke(vg);
    
    float highX = freqToX(highFreq);
    nvgBeginPath(vg);
    nvgMoveTo(vg, highX, 0);
    nvgLineTo(vg, highX, box.size.y);
    nvgStroke(vg);
  }

  void drawGainLabels(NVGcontext *vg, float lowGain, float midGain, float highGain)
  {
    nvgFontSize(vg, 10);
    nvgFontFaceId(vg, APP->window->uiFont->handle);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    
    // Low band label
    nvgFillColor(vg, nvgRGBA(255, 100, 100, 255));
    nvgText(vg, box.size.x * 0.15f, 2, string::f("L: %+.1fdB", lowGain).c_str(), NULL);
    
    // Mid band label
    nvgFillColor(vg, nvgRGBA(100, 255, 100, 255));
    nvgText(vg, box.size.x * 0.5f, 2, string::f("M: %+.1fdB", midGain).c_str(), NULL);
    
    // High band label
    nvgFillColor(vg, nvgRGBA(100, 100, 255, 255));
    nvgText(vg, box.size.x * 0.85f, 2, string::f("H: %+.1fdB", highGain).c_str(), NULL);
  }

  // Convert frequency to X position (logarithmic scale)
  float freqToX(float freq)
  {
    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    float logFreq = std::log10(freq);
    return ((logFreq - logMin) / (logMax - logMin)) * box.size.x;
  }

  // Convert gain (dB) to Y position
  float gainToY(float gainDB)
  {
    float centerY = box.size.y * 0.5f;
    float scale = box.size.y / 28.f; // ±14dB vertical range
    return centerY - (gainDB * scale);
  }

  // Approximate gain at a specific frequency
  // This is a simplified approximation for visualization
  float approximateGainAtFreq(float freq, float lowFreq, float highFreq,
                               float lowGain, float midGain, float highGain)
  {
    // Transition width factor (octaves)
    float transitionWidth = 1.5f;
    
    // Calculate distance from crossover points
    float lowDist = std::log2(freq / lowFreq);
    float highDist = std::log2(freq / highFreq);
    
    // Low band influence (below lowFreq)
    float lowInfluence = 1.0f / (1.0f + std::exp(lowDist / transitionWidth));
    
    // High band influence (above highFreq)
    float highInfluence = 1.0f / (1.0f + std::exp(-highDist / transitionWidth));
    
    // Mid band influence (between crossovers)
    float midInfluence = 1.0f - lowInfluence - highInfluence;
    midInfluence = std::max(0.0f, midInfluence);
    
    // Combine gains
    float totalGain = lowGain * lowInfluence + 
                      midGain * midInfluence + 
                      highGain * highInfluence;
    
    return totalGain;
  }
};

// Gain Level Meter Widget
// Shows real-time gain levels for each band with peak hold
struct EQGainMeterDisplay : TransparentWidget
{
  Module *module = nullptr;
  
  // Peak hold values and timers
  float lowPeak = 0.f;
  float midPeak = 0.f;
  float highPeak = 0.f;
  float peakHoldTime = 1.5f; // seconds
  float lowPeakTimer = 0.f;
  float midPeakTimer = 0.f;
  float highPeakTimer = 0.f;
  
  // RMS values for smoothing
  float lowRMS = 0.f;
  float midRMS = 0.f;
  float highRMS = 0.f;
  
  void step() override
  {
    TransparentWidget::step();
    
    if (!module)
      return;
    
    float sampleTime = APP->engine->getSampleTime();
    
    // Decay peak hold timers
    if (lowPeakTimer > 0.f)
      lowPeakTimer -= sampleTime;
    if (midPeakTimer > 0.f)
      midPeakTimer -= sampleTime;
    if (highPeakTimer > 0.f)
      highPeakTimer -= sampleTime;
    
    // Reset peaks when timer expires
    if (lowPeakTimer <= 0.f)
      lowPeak = 0.f;
    if (midPeakTimer <= 0.f)
      midPeak = 0.f;
    if (highPeakTimer <= 0.f)
      highPeak = 0.f;
  }

  void drawLayer(const DrawArgs &args, int layer) override
  {
    if (layer != 1)
      return;
    
    if (!module)
    {
      drawPlaceholder(args);
      return;
    }

    NVGcontext *vg = args.vg;
    
    // Get current gain settings
    float lowGain = module->params[2].getValue();  // LOW_GAIN_PARAM
    float midGain = module->params[3].getValue();  // MID_GAIN_PARAM
    float highGain = module->params[4].getValue(); // HIGH_GAIN_PARAM
    
    // Apply CV modulation if connected
    if (module->inputs[4].isConnected()) // LOW_GAIN_CV_INPUT
    {
      float cv = module->inputs[4].getVoltage();
      lowGain += cv * 2.4f;
      lowGain = clamp(lowGain, -12.f, 12.f);
    }
    
    if (module->inputs[5].isConnected()) // MID_GAIN_CV_INPUT
    {
      float cv = module->inputs[5].getVoltage();
      midGain += cv * 2.4f;
      midGain = clamp(midGain, -12.f, 12.f);
    }
    
    if (module->inputs[6].isConnected()) // HIGH_GAIN_CV_INPUT
    {
      float cv = module->inputs[6].getVoltage();
      highGain += cv * 2.4f;
      highGain = clamp(highGain, -12.f, 12.f);
    }
    
    // Update peak values
    updatePeaks(lowGain, midGain, highGain);

    // Draw background
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 200));
    nvgFill(vg);
    
    // Draw meters
    float meterWidth = (box.size.x - 12.f) / 3.f;
    float meterHeight = box.size.y - 20.f;
    
    drawMeter(vg, 4.f, 15.f, meterWidth, meterHeight, lowGain, lowPeak, 
              nvgRGBA(255, 100, 100, 255), "LOW");
    drawMeter(vg, 4.f + meterWidth + 2.f, 15.f, meterWidth, meterHeight, midGain, midPeak,
              nvgRGBA(100, 255, 100, 255), "MID");
    drawMeter(vg, 4.f + (meterWidth + 2.f) * 2.f, 15.f, meterWidth, meterHeight, highGain, highPeak,
              nvgRGBA(100, 100, 255, 255), "HIGH");
  }

  void drawPlaceholder(const DrawArgs &args)
  {
    NVGcontext *vg = args.vg;
    
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 200));
    nvgFill(vg);
    
    nvgFontSize(vg, 10);
    nvgFontFaceId(vg, APP->window->uiFont->handle);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, nvgRGBA(150, 150, 150, 255));
    nvgText(vg, box.size.x * 0.5f, box.size.y * 0.5f, "Gain Meters", NULL);
  }

  void drawMeter(NVGcontext *vg, float x, float y, float width, float height,
                 float gain, float peak, NVGcolor color, const char *label)
  {
    // Draw meter background
    nvgBeginPath(vg);
    nvgRect(vg, x, y, width, height);
    nvgFillColor(vg, nvgRGBA(40, 40, 40, 255));
    nvgFill(vg);
    
    nvgStrokeColor(vg, nvgRGBA(80, 80, 80, 255));
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);
    
    // Draw center line (0dB)
    float centerY = y + height * 0.5f;
    nvgBeginPath(vg);
    nvgMoveTo(vg, x, centerY);
    nvgLineTo(vg, x + width, centerY);
    nvgStrokeColor(vg, nvgRGBA(100, 100, 100, 255));
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);
    
    // Calculate bar height
    float normalizedGain = (gain + 12.f) / 24.f; // -12 to +12 -> 0 to 1
    normalizedGain = clamp(normalizedGain, 0.f, 1.f);
    
    float barHeight = height * normalizedGain;
    float barY = y + height - barHeight;
    
    // Draw gain bar
    nvgBeginPath(vg);
    nvgRect(vg, x + 2, barY, width - 4, barHeight);
    
    // Color gradient based on gain level
    NVGcolor topColor = color;
    NVGcolor bottomColor = nvgRGBA(color.r * 255 * 0.5f, color.g * 255 * 0.5f, color.b * 255 * 0.5f, 255);
    NVGpaint paint = nvgLinearGradient(vg, x, barY, x, y + height, topColor, bottomColor);
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    
    // Draw peak indicator
    if (peak > -12.f)
    {
      float normalizedPeak = (peak + 12.f) / 24.f;
      normalizedPeak = clamp(normalizedPeak, 0.f, 1.f);
      float peakY = y + height * (1.f - normalizedPeak);
      
      nvgBeginPath(vg);
      nvgRect(vg, x, peakY - 1, width, 2);
      nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
      nvgFill(vg);
    }
    
    // Draw label
    nvgFontSize(vg, 8);
    nvgFontFaceId(vg, APP->window->uiFont->handle);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    nvgFillColor(vg, color);
    nvgText(vg, x + width * 0.5f, y - 12, label, NULL);
    
    // Draw value
    nvgFontSize(vg, 7);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
    nvgFillColor(vg, nvgRGBA(200, 200, 200, 255));
    nvgText(vg, x + width * 0.5f, y + height + 10, string::f("%+.1f", gain).c_str(), NULL);
  }

  void updatePeaks(float lowGain, float midGain, float highGain)
  {
    // Update low band peak
    if (std::abs(lowGain) > std::abs(lowPeak))
    {
      lowPeak = lowGain;
      lowPeakTimer = peakHoldTime;
    }
    
    // Update mid band peak
    if (std::abs(midGain) > std::abs(midPeak))
    {
      midPeak = midGain;
      midPeakTimer = peakHoldTime;
    }
    
    // Update high band peak
    if (std::abs(highGain) > std::abs(highPeak))
    {
      highPeak = highGain;
      highPeakTimer = peakHoldTime;
    }
  }
};
