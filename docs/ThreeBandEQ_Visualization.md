# ThreeBandEQ Visualization Features

## Overview

The ThreeBandEQ module includes comprehensive real-time visual feedback to help you understand and adjust the frequency response of your audio. The visualization system consists of two main components:

1. **Frequency Response Display** - Shows the complete EQ curve across the audible spectrum
2. **Gain Meter Display** - Provides real-time gain level indicators with peak hold

---

## Frequency Response Display

### Description

The frequency response display is located at the top of the module and visualizes the EQ curve with a logarithmic frequency scale from 20 Hz to 20 kHz. This display updates in real-time as you adjust parameters or apply CV modulation.

### Visual Elements

#### Frequency Grid
- Vertical lines mark key frequencies: **100 Hz**, **1 kHz**, and **10 kHz**
- Frequency labels appear at the bottom of the display
- Logarithmic scale provides accurate representation of musical intervals

#### Gain Grid
- Horizontal lines mark gain levels: **-12 dB**, **-6 dB**, **0 dB**, **+6 dB**, **+12 dB**
- The **0 dB** line is thicker for easy reference (unity gain)
- Gain labels appear on the left side of the display

#### Frequency Band Regions
The display is divided into three color-coded regions corresponding to the three EQ bands:

- **Low Band** (Red tint) - Below the low crossover frequency (80-250 Hz)
- **Mid Band** (Green tint) - Between low and high crossover frequencies (1-4 kHz)
- **High Band** (Blue tint) - Above the high crossover frequency (8-20 kHz)

These colored regions shift dynamically as you adjust the crossover frequencies.

#### Response Curve
- **Golden/Orange curve** shows the combined frequency response
- Updates in real-time based on all three gain parameters
- Smooth transitions between frequency bands reflect the filter slopes
- Approximately 200 calculation points ensure smooth curve rendering

#### Crossover Markers
- **Red vertical lines** indicate the exact crossover frequencies
- Move dynamically with LOW FREQ and HIGH FREQ parameter adjustments
- Also respond to CV modulation of crossover frequencies

#### Gain Labels
Real-time numerical readouts appear at the top of the display:

- **L: [value]dB** - Low band gain (red text)
- **M: [value]dB** - Mid band gain (green text)
- **H: [value]dB** - High band gain (blue text)

These values include any CV modulation currently applied.

### Technical Details

#### Frequency Response Calculation
The display uses a **simplified approximation** of the actual filter response for visualization purposes. The approximation:

- Models the transition regions using sigmoid functions
- Transition width approximately 1.5 octaves
- Provides smooth blending between bands
- Updates at screen refresh rate (typically 60 fps)

**Note**: The actual DSP filter has a 24 dB/octave slope. The visualization approximates this for computational efficiency.

#### CV Modulation Integration
The display automatically incorporates CV inputs:

- **LOW FREQ CV**: 0-10V = 0-170 Hz offset (linear)
- **HIGH FREQ CV**: 0-10V = 0-3000 Hz offset (linear)
- **Gain CVs**: ±5V = ±12 dB (linear)

All modulated values are clamped to their valid parameter ranges.

---

## Gain Meter Display

### Description

Located below the frequency response display, the gain meter provides three vertical bar meters showing the current gain setting for each frequency band. Each meter includes peak hold functionality for easy monitoring of gain changes.

### Visual Elements

#### Three Vertical Meters
Each meter corresponds to one frequency band:

- **LOW** (Red) - Low band gain
- **MID** (Green) - Mid band gain
- **HIGH** (Blue) - High band gain

#### Meter Components

**Background Grid**
- Dark background with light border
- Horizontal line at center indicates **0 dB** (unity gain)
- Meter range: -12 dB (bottom) to +12 dB (top)

**Gain Bar**
- Colored vertical bar shows current gain level
- Gradient from darker (bottom) to brighter (top) for visual depth
- Bar height proportional to gain amount
- Extends above center for boost, below center for cut

**Peak Hold Indicator**
- White horizontal line shows the highest absolute gain value
- Holds position for **1.5 seconds** after peak occurs
- Automatically resets after hold time expires
- Tracks both positive and negative peaks

**Numerical Readout**
- Precise gain value displayed below each meter
- Format: **[+/-]X.X dB**
- Updates in real-time including CV modulation

### Meter Behavior

#### Real-Time Updates
- Meters respond instantly to parameter changes
- Display current value including CV modulation
- No smoothing or lag (for precise feedback)

#### Peak Hold System
The peak hold feature helps you monitor transient gain changes:

1. When gain increases or decreases, the peak marker updates
2. Peak marker holds at the extreme value for 1.5 seconds
3. After timeout, the marker resets to current value
4. Process repeats with next gain change

This is particularly useful when:
- Monitoring CV-modulated gain changes
- Comparing preset gain levels
- Ensuring gain adjustments stay within desired ranges

---

## Usage Tips

### EQ Curve Shaping

1. **Visual Feedback for Adjustments**
   - Watch the response curve as you adjust knobs
   - Observe how the curve smoothly transitions between bands
   - Use crossover markers to align frequency regions with your audio content

2. **Preset Comparison**
   - Select different presets from the context menu
   - Compare frequency response curves visually
   - Identify which preset best suits your audio material

3. **CV Modulation Monitoring**
   - Patch CV sources to gain or frequency inputs
   - Watch the curve animate in real-time
   - Observe how dynamic modulation affects the frequency spectrum

### Gain Monitoring

1. **Checking Gain Ranges**
   - Use meters to ensure gains stay within musical ranges
   - Peak hold indicators help catch extreme values
   - Numerical readouts provide precise control

2. **Balancing Bands**
   - Compare relative gain levels across bands visually
   - Adjust to achieve desired frequency balance
   - Monitor for excessive boosting/cutting

3. **Dynamic Range Awareness**
   - Peak hold shows maximum applied gain
   - Helps prevent over-processing
   - Useful for automation and CV modulation scenarios

---

## Keyboard Shortcuts & Interaction

### Display Interaction
- The visualization displays are **non-interactive** (display-only)
- All parameter adjustments use the knobs and CV inputs
- No mouse interaction with the displays themselves

### Context Menu Integration
- Right-click on module for presets and bypass
- Preset changes are immediately reflected in visualizations
- Bypass mode does not affect visualization (shows active settings)

---

## Performance Considerations

### CPU Usage
The visualization system is optimized for minimal CPU overhead:

- Displays render at screen refresh rate only
- No audio-rate calculations
- Efficient curve approximation algorithm
- Typical overhead: **< 0.1% CPU** on modern systems

### Optimization Tips
1. The displays only redraw when visible on screen
2. Calculations are skipped when module is not in view
3. No performance impact on audio processing

---

## Technical Implementation

### Architecture

**File Structure**
```
src/ThreeBandEQDisplay.hpp  - Visualization widget classes
src/ThreeBandEQ.hpp         - Main module (includes display widgets)
```

**Widget Classes**
- `EQFrequencyResponseDisplay` - Inherits from `TransparentWidget`
- `EQGainMeterDisplay` - Inherits from `TransparentWidget`

### Rendering Pipeline

1. **drawLayer() Method** - Called by VCV Rack render engine
2. **Parameter Reading** - Fetch current values from module
3. **CV Integration** - Apply CV modulation to display values
4. **NanoVG Drawing** - Render using NanoVG graphics context
5. **Screen Update** - Vsync'd refresh (typically 60 Hz)

### Coordinate Systems

**Frequency Response Display**
- X-axis: Logarithmic frequency scale (20 Hz - 20 kHz)
- Y-axis: Linear gain scale (-14 dB to +14 dB visible range)
- Dimensions: 200 × 80 pixels

**Gain Meter Display**
- Three meters, each approximately 66 × 60 pixels
- Linear vertical scale from -12 dB to +12 dB
- Total dimensions: 200 × 60 pixels

### Color Palette

**Frequency Response**
- Background: `rgba(0, 0, 0, 200)` - Semi-transparent black
- Response curve: `rgba(255, 200, 100, 255)` - Golden orange
- Crossover lines: `rgba(255, 100, 100, 200)` - Red
- Grid lines: Various grays (50-120 RGB values)

**Gain Meters**
- Low band: `rgb(255, 100, 100)` - Red
- Mid band: `rgb(100, 255, 100)` - Green
- High band: `rgb(100, 100, 255)` - Blue
- Peak hold: `rgba(255, 255, 255, 200)` - White

---

## Future Enhancements

Potential future additions to the visualization system:

1. **Spectrum Analyzer**
   - Real-time FFT of input/output signal
   - Overlaid on frequency response display
   - Compare actual spectrum to EQ curve

2. **Phase Response Display**
   - Show phase shift across frequency bands
   - Useful for parallel processing scenarios
   - Toggle between magnitude and phase

3. **Interactive Curve Editing**
   - Click and drag on response curve to adjust gains
   - Direct manipulation of crossover frequencies
   - Gesture-based preset recall

4. **Stereo Correlation Meter**
   - Visualize stereo field
   - Monitor left/right channel differences
   - Detect phase issues

5. **Historical Gain Display**
   - Graph gain changes over time
   - Useful for parameter animation
   - CV modulation waveform display

---

## Troubleshooting

### Display Not Showing
- Ensure module is properly loaded (not bypassed)
- Check that panel size is 18HP (visualizations require this width)
- Try reloading the module

### Curve Looks Incorrect
- Remember the display uses an approximation algorithm
- Actual DSP filter response may differ slightly
- For precise measurements, use external spectrum analyzer

### Performance Issues
- Visualizations are optimized but use some GPU resources
- On older systems, consider hiding module when not in use
- VCV Rack's frame rate limiter helps maintain performance

### Peak Hold Not Working
- Peak hold requires gain changes to trigger
- Hold time is 1.5 seconds - may need to wait for reset
- Ensure CV inputs are properly connected if using modulation

---

## Conclusion

The ThreeBandEQ visualization system provides comprehensive visual feedback for precise EQ adjustments. The frequency response display offers an intuitive overview of your EQ curve, while the gain meters provide detailed monitoring of each band. Together, these tools make it easier to achieve professional-sounding results and understand the impact of your EQ decisions.

For more information on the DSP implementation and parameter ranges, refer to the main **ThreeBandEQ.md** documentation.
