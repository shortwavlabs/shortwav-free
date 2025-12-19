

# soundhack MORPHAZENE

soundhack
MORPHAZENE

L[MAN]    R    L[MAN]    R

SENSE    TDE

MORPH    ARAANGE

MORPHED

GLST    SHOT

REC    SPLICE

MAKENOISE

---



2

# TABLE OF CONTENTS

- FCC
- Limited Warranty
- Installation
- Morphagene Overview
- Setting up for Recording
- Button Combinations
- Panel Controls
- Signal To Be Captured!
- Time Scales of the Morphagene:
    - Introduction
    - Reels
    - Splices
    - Shift
    - Gene-Size and Slide
    - Morph
- Types of Recording
- Sound DNA
- Sound On Sound and Vari-Speed
- Stopping Playback and Microsound
- Synchronizing the Morphagene
- Time Stretch
- Gene Shift
- Non Real-Time Functions:
    - Deleting Splice Markers and Splice Audio
    - Deleting All Splice Markers and Clearing the Reel
    - Reels and File Naming Convention
- Tips and Tricks
- Firmware Changelogs
- Patch Ideas

---



3

# FC                                                                                                                                                CE

This device complies with Part 15 of the FCC Rules. Operation is subject to the following two conditions: (1) this device may not cause harmful interference, and (2) this device must accept any interference received, including interference that may cause undesired operation.

Changes / modifications not approved by the Make Noise Co. could void the user's authority to operate the equipment.

This equipment has been tested and found to comply with the limits for a Class A digital device, pursuant to part 15 of the FCC Rules. These limits are designed to provide reasonable protection against harmful interference when the equipment is operated in a commercial environment. This equipment generates, uses, and can radiate radio frequency energy and, if not installed and used in accordance with the instruction manual, may cause harmful interference to radio communications.

makenoisemusic.com
Make Noise Co., 414 Haywood Road, Asheville, NC 28806

---



4

# LIMITED WARRANTY

Make Noise warrants this product to be free of defects in materials or construction for a period of one year from the date of purchase (proof of purchase/invoice required).

Malfunction resulting from wrong power supply voltages, backwards or reversed eurorack bus board cable connection, abuse of the product, removing knobs, changing faceplates, or any other causes determined by Make Noise to be the fault of the user are not covered by this warranty, and normal service rates will apply.

During the warranty period, any defective products will be repaired or replaced, at the option of Make Noise, on a return-to-Make Noise basis with the customer paying the transit cost to Make Noise.

Make Noise implies and accepts no responsibility for harm to person or apparatus caused through operation of this product.

Please contact technical@makenoisemusic.com with any questions, Return To Manufacturer Authorization, or any needs & comments.

http://www.makenoisemusic.com

[Waste bin symbol with crossed-out wheelie bin icon]

About This Manual:

Written by Tony Rolando and Walker Farrell
Illustrated by Lee Coleman and Lewis Dahm
Layout by Lewis Dahm

Development: Tom Erbe, Walker Farrell, Tony Rolando, and Matthew Sherwood
Hardware Design: Tony Rolando
DSP: Tom Erbe
Special Thanks to the Beta Testers!

---



5

# INSTALLATION

## Electrocution hazard!

Always turn the Eurorack case off and unplug the power cord before plugging or unplugging any Eurorack bus board connection cable. Do not touch any electrical terminals when attaching any Eurorack bus board cable.

The Make Noise Soundhack Morphagene is an electronic music module requiring 165mA of +12VDC and 20mA of -12VDC regulated voltage and a properly formatted distribution receptacle to operate. It must be properly installed into a Eurorack format modular synthesizer system case.

Go to http://www.makenoisemusic.com/ for examples of Eurorack Systems and Cases.

To install, find 20HP in your Eurorack synthesizer case, confirm proper installation of Eurorack bus board connector cable on backside of module (see picture below), plug the bus board connector cable into the Eurorack style bus board, minding the polarity so that the RED stripe on the cable is oriented to the NEGATIVE 12 Volt line on both the module and the bus board. On the Make Noise 6U or 3U Busboard, the negative 12 Volt line is indicated by the white stripe.

TOP

-12v

Please refer to your case manufacturer's specification for location of the negative supply.

---



6

# MORPHAGENE OVERVIEW

The Morphagene is a next generation Tape and Microsound music module that uses Reels, Splices, and Genes to create new sounds from those that already exist. It is informed by the worlds of Musique Concrète, where speed and direction variation were combined with creative tape splicing to pioneer new sounds, and Microsound, where computers allow for sound to be divided into pieces smaller than 1/10 of a second and manipulated like sub-atomic particles.

Having voltage control over every parameter, the Morphagene is most dynamic as a digital audio buffer for the modular synthesist. The Morphagene is comprised of a pair of tool-sets, which work well together: Tape Music Tools allow for sounds to be recorded on the fly, layered using the internal Sound On Sound (S.O.S) function, manually cut into pieces using the Splice function, and re-organized with the Organize control. Once it is spliced up, it is possible to create nearly infinite variations of the original loop by modulating the Organize parameter. Vari-Speed allows for the speed and direction of playback to be controlled continuously with one control signal.

Gene-Size and Slide make up the Microsound Tools. Gene-Size shrinks the Splice's "playback window" into gradually smaller pieces called Genes (aka particles, grains, granules). A control signal, such as the Wogglebug Smooth CV applied to Slide, moves through those pieces in a nonlinear fashion. Using Slide, random access of the audio buffer is possible. Morph allows additional textural control via the staggering or layering of successive Genes and, at extreme settings, randomized pitch up and panning. This yields interesting sonic results, particularly when combined with Functions such as Vari-Speed and Organize. It is possible to record Tape and Microsound manipulations into brand new Splices as if using two independent machines: one for playback and one to record. The Morphagene also allows for the storage and recall of multiple sound buffers in the form of Reels, stored as .wav files on a microSD card. Each Reel can be up to 2.9 minutes long and may consist of up to 300 Splices. The end result is a sampler/looper/audio buffer that is able to exist within a modular synthesizer system, and offer a vast amount of Real-Time sound manipulation in a fast and tactile way.

## Perspective

Early pioneers of electronic music did not have access to specialized gear with which to implement their ideas. Instead, they had to modify existing devices or create new devices in order to take steps forward into new musical possibilities. In many cases, this novelty extended beyond the instruments used, leading to the creation of new musical form and content that had not been previously imagined.

In the decades since, many new forms of music have been created, consumed, and eventually codified. Countless new instruments have been designed around these forms, so many that it is frequently possible to find and purchase a tool that is ready-made for any given goal in the known taxonomy of musical forms. Today, all you have to do is pick the characteristics you want in an instrument from a list, go to the nearest internet device, and find the ideal instrument in a database. Consequently, many designers are now making instruments which are fool-proof and which guarantee some specific musical result. Thus, making it easy to create the same music over and over again.

The Morphagene does not use this approach. It is a powerful and open-ended set of tools for the advanced creative synthesist, giving you the power and the responsibility to build the musical form of your choosing, or better yet, creating musical form from the ground up.

---



7

# SETTING UP FOR RECORDING

The Morphagene comes with a blank microSD card for storing sounds. No sounds are included with this card. You don't need somebody telling you what sounds are approved for use. The world of sound is larger than we could possibly imply with presets. But this does mean you will need to take some action to get your Morphagene to start doing something. This diagram shows the ideal "initialization" settings for the Morphagene, which results in any recording playing back as it was recorded.

The Morphagene is always modulating the sound based on the settings of the Panel Controls and any CV being applied. After recording modulations into a new Splice, refer to these "initialization" settings to set up the Morphagene for unmodulated 1/1 looping playback of your new recording.

SOUND SOURCE → OUTPUT

---



8

## Auto Leveling:

Once you have patched the sound source of your choice to the L and/or R Input(s), adjust the S.O.S. control to full counterclockwise to hear only the incoming audio. Hold the REC Button and press the Shift Button to perform Auto-Leveling, which analyzes the sound and adjusts its gain to the correct amplitude for use in the modular system. Holding this button combination for a few seconds produces a good snapshot of the signal dynamics and ideal level settings.

To Record into a new Splice, hold the REC Button and press the Splice Button. The REC Button lights to indicate Recording is happening. Press the REC Button again to stop.

After recording (or deleting), the microSD Card is busy writing, as indicated by the ashing Shift Button. All parameters are active and ready for use except Reel Mode and the Delete routines, which could be delayed. **Do not remove microSD Card while it is busy, as indicated by the Shift Button Flashing.**

Now, turn the S.O.S. panel control full Clockwise to hear the playback of what you have recorded.

*NOTE: Morphagene is always storing your latest recordings and splices to the microSD Card. If you do not want to write over a card, remove it once you have loaded the desired Reel. If there is already a recording present, you can clear all the audio from the Reel by holding the Shift Button, then holding the REC Button for 3 seconds.*

## Synchronizing Recording:

Patching a Clock source to the Clock Input performs several tasks which are in some cases dependent upon other settings on the Morphagene. During the Recording process, a Clock patched to the Clock Input synchronizes the REC Button, allowing for Recording Start and Stop times to be synchronized with an External Clock. The REC Button strobes after it is pressed in order to indicate that the Morphagene is waiting for the next Clock cycle to start Recording. On the next Clock pulse received, the procedure is completed, the REC Button lights in order to indicate recording has started. After the REC Button is pressed again, it waits for a Clock Pulse, and turns off to indicate recording has stopped.

---



9

# BUTTON COMBINATIONS

Below is a list of all the real-time procedures performed using button press combinations on the Morphagene. Note that single presses of the REC, Splice, and Shift buttons are all that is needed for a great deal of performative functionality. Additional "combos" allow much more detailed editing, recall, etc. but are not necessary for the practical use of the module.

**Note: Single button press actions take place upon release of the button except for Record Stop. For example, Record Into Same Splice begins when the REC Button is released, not the moment it is pressed. This allows for multi-button combinations to work without accidentally initiating the single-button action.**

## Function / / Button Combination

**Auto Leveling:** Hold REC Button, Press Shift Button. Enables Auto Leveling. Measures incoming signal and sets gain to Normalize while Recording. Option: Continue to hold the REC Button for a "Rolling Listen."

**Mount microSD Card:** Press Shift Button. Loads inserted microSD when unmounted.

**Enter Reel Mode:** Hold Splice Button, Press REC Button: Enters Reel Mode (microSD must be mounted)
- **Organize Panel Control and CV:** Selects Reel. The last Reel (Pink/White pulsing) creates a new Reel upon exiting Reel mode
- **Hold Splice Button, Press REC Button:** Exits Reel mode
- **Hold Shift, Press REC Button:** Deletes current Reel including all Splices and audio. Deleting last Reel exits Reel Mode and creates new Reel.
- **Removing microSD:** Exits Reel Mode

**Record into Current Splice:** Press REC Button. Records into current Splice. If no Splices are present, records into a new Splice. During recording, Press REC Button again to stop active recording. Note: if the total Reel length of 2.9 minutes is reached, recording stops automatically.

**Record into New Splice:** Hold REC Button, Press Splice Button. Records into new Splice at end of Reel.

**Create New Splice Marker:** Press Splice Button. Creates new Splice Marker at current playback location.

**Increment Splice:** Press Shift Button. Increments to next Splice.

**Delete Splice Marker:** Hold Shift Button, Press Splice Button. Deletes Splice Marker, joining current Splice with next Splice.

**Delete All Splice Markers:** Hold Shift Button, Press+Hold Splice Button for 3 seconds. Deletes all Splice Markers, leaving entire Reel loaded in a single Splice.

**Delete Splice Audio:** Hold Shift Button, Press REC Button. Deletes Splice Audio.

**Clear Reel:** Hold Shift Button, Press+Hold REC Button for 3 seconds. Deletes all Splice Markers, clears all Splice audio, empties current Reel.

*Note: With a Clock patched to the Play Input, cannot Delete Splice Marker, Splice Audio, All Splice Markers, or All Splice Audio. With a Clock patched to Shift Input, it is difficult to reliably Record. When alternately recording and deleting audio, it is a good idea to wait for the Shift Button to stop flashing before proceeding.*

---



10

# PANEL CONTROLS

1. **Audio In L (Mono) and R:** Line Level to Modular level signal. AC coupled. No analog input gain/ attenuation control. Signals may be leveled using Automatic Leveling (see Page 7).
2. **Sound On Sound CV IN:** Uni-polar control input for the Sound On Sound (S.O.S.) parameter. Range 0V to 8V, linear response. Normalized to +8V.
3. **Sound On Sound (S.O.S.) Combo Pot:** Sets mix of previously recorded Loop with Live Signal input allowing for Sound on Sound. With nothing patched to S.O.S. CV IN, works as standard panel control for S.O.S. parameter. With Signal patched to S.O.S. CV IN, works as attenuator for that signal.
4. **Audio Out L (Mono) and R:** Typically 10Vpp; AC coupled.
5. **Gene-Size Panel Control:** Manual unipolar control which nondestructively changes the "playback window" from the full Splice when full counter clockwise, to extremely short (potentially inaudible) at full clockwise.
6. **Gene-Size CV IN Attenuator:** Bi-polar attenuator for Gene-Size.
7. **Gene-Size CV IN:** Bi-polar control input which sets Gene-Size. Range 0V to +8V.
8. **Var-Speed Activity Windows:** Indicates Speed, Direction, and Morph settings.
9. **Vari-Speed Bipolar Panel Control:** Manual bipolar speed and direction control. When set to 12:00, playback is stopped. Turning clockwise from 12:00 increases playback speed in forward direction. Turning counter clockwise from 12:00 increases playback speed in reverse direction.
10. **Vari-Speed CV Input Attenuverter:** Bipolar attenuator for Vari-Speed CV Input.
11. **Vari-Speed CV Input:** Bipolar speed and direction control input. Range +/-4V.
12. **Morph CV Input:** Unity level, uni-polar CV input which sets Morph level. Range 0V to +5V.
13. **Morph Panel Control:** Manual unipolar control which allows Genes to be staggered, stacked, and layered.

---



11

# PANEL CONTROLS

1. **CLK Input:** Synchronizes REC, Gene-Size, and Morph (see Page 28). Expects Clock or Gate of at least 2.5V in amplitude.
2. **CV Output:** Control signal representing the average energy of the signals at the Audio Outs (see Page 28). Range 0V to +8V DC.
3. **CV Output Activity Window:** Visual indication of the CV Output signal.
4. **Play Gate Input:** Gate HIGH triggers or re-triggers playback from start of currently selected Splice. If held HIGH, Splice Loops. If held LOW, playback stops at End of Splice. This input is normalled High, so with nothing patched to Play, the Splice loops. Slide offsets the Play Reset/ Start point.
5. **REC Gate Input:** Clock or gate input for Record. Toggles Record ON/OFF. Expects Clock or Gate of at least 2.5V.
6. **REC Button w/ Illumination:** Starts and stops recording. Lights to indicate that recording is happening. Press to Record into same Splice or to initiate Time Lag Accumulation. The process starts upon the release of the REC Button. Hold REC Button and Press Splice Button to Record into New Splice. While recording, press the REC Button to stop the Recording process. Also used for Button Combinations (see Page 8).
7. **Reel Activity Window:** Visual indication of currently selected Reel. Flashes to indicate signal at CLK IN and while in Reel Select Mode. Strobes at High Gene-Size and Morph values to indicate excitement.
8. **Splice Button w/ Illumination:** Press to drop Splice marker on captured sound. Lights to indicate End of Splice/Gene. Also used for Button Combinations (see Page 9).
9. **End of Gene/ Splice Gate Output:** Outputs Gate at the end of each Splice and/or Gene. 0 to 10Vpp (see Page 28).
10. **Splice Gate Input:** Places Splice marker on captured sound based on incoming Gate HIGH. Expects Clock or Gate signal of at least 2.5V.
11. **Splice Activity Window:** Visual indication of currently selected Splice. Flashes to indicate Erase Splice, Erase Splice Audio, Erase All Splices, and Erase All Audio.

---



12

# PANEL CONTROLS

[A control panel diagram with numbered labels 1-8 pointing to various controls]

1. **Slide Panel Control:** Manual control for traveling through the captured sound. Allows for scrubbing of the recorded material and offsetting the Play Reset/Start point. This parameter is affected by the Gene-Size setting.
2. **Slide CV Input Attenuverter:** Bipolar attenuator for Slide.
3. **Slide CV Input:** Uni-polar control input for Slide. Range 0V to +8V.
4. **Organize Panel Control:** Manual unipolar control that selects next Splice to be played. The currently selected Splice or Gene plays to the end before the next Splice is selected. In Reel Mode, used to select the Reel.
5. **Organize CV Input:** Uni-polar control signal input for Organize. Range 0V to +5V. Unpatch when in Reel Mode to select Reel.
6. **Shift Gate Input:** Input to increment Splice selection. Expects clock or Gate of at least 2.5V in amplitude.
7. **Shift Button w/ illumination:** Increments Splice selection. Lights on to indicate microSD Card is mounted. If SD Card is not mounted on insertion, press Shift Button to mount inserted SD Card. Flashes to indicate "SD CARD BUSY!" Also used for Button Combinations. Do not remove microSD Card while it is busy, as indicated by the Shift Button Flashing.
8. **microSD Card Slot:** For use of FAT32 formatted microSD cards only. If inserted, microSD Card is mounted as soon as it is recognized or when you press Shift Button. The Splice Button lights on to indicate the microSD card is mounted. Do not remove microSD Card while it is busy, as indicated by the Shift Button Flashing.

---



13

# SIGNAL TO BE CAPTURED!

The Signal Inputs accommodate modular synthesizer signals, as well as line level sources. Signals are leveled using Automatic Leveling (Hold REC Button, Press Shift Button) so that the resulting output is around 10Vpp.

Early electronic music composers often recorded pure sine waves to tape at different frequencies and amplitudes, editing and splicing sounds into musical phrases; however, this was largely because there did not exist the means to control electronic sounds. Laboratory instruments were being used as musical instruments but lacked the vast controls needed for musical expression. The arrival of the VCO, VCA, and Sequencer made complete control of Frequency and Amplitude possible on a time scale determined by the composer.

Previous tools for recording in the modular domain have often had lower-quality results than when playing back pure tones from a synthesizer. However, the Morphagene features high fidelity playback even when under heavy modulation, making early Tape Music processes feasible again.

You may find that the most interesting source material for the Morphagene is not your synthesizer, but instead any sound that surrounds you. So grab a microphone or patch into the internet and find some seemingly-mundane sound to massage into a gorgeous, jumbled up symphony of Micronoise!

## Recording Time and Quality

The record and playback frequency of the Morphagene is kept at a constant of 48 kHz with 32-bit dynamic range. This quality is maintained even when modulating playback speed or granulating the audio. Each Reel loaded to the internal memory of the Morphagene can hold roughly 2.9 minutes of quality stereo audio and can handle modulation of extreme speed and intensity with instantaneous results. Additional buffers or "Reels" can be manually loaded from the microSD card. **Morphagene is always storing your latest recordings and Splices to the microSD Card. If you do not want to write over a card, remove it once you have loaded the desired Reel.**

## Vari-Speed

The Vari-Speed control plays back at original speed and pitch when set to about 9:30 (Reverse) or 2:30 (Forward). The respective Vari-Speed Activity Window lights Green when this playback speed is set.

*Note: The Vari-Speed control does not affect the record speed in any way, only the playback speed. This makes it useful for doing overdubs of new material over pitch-shifted or reversed instances of previously-recorded sound.*

The Morphagene is always modulating the sound based on the settings of the panel controls and any CV being applied. After recording modulations into a new Splice, refer to the "initialization" settings on p. 7 to see how to set up the Morphagene to do unmodulated 1/1 looping playback of your new recording.

## Signal OUT

Any modular system with a Morphagene installed is at a great advantage, as many Analog Synthesis techniques and processes work well with the Morphagene.

Amplitude Modulation, Echo, Reverb are commonly-suggested post-processing techniques in Microsound and Granular Synthesis. Since the Morphagene is a stereo device, consider recording different sound material into each of the two inputs and sending each output to independent locations for parallel processing.

---



14

# TIME SCALES OF THE MORPHAGENE

The Morphagene operates on several Time Scales, allowing the synthesist to explore sound and time at multiple levels and in great detail. The power is so great that a full view of all time scales at once can be daunting, even for those musicians with a deep understanding of tape and Microsound techniques. In the coming pages, you will find a visual guide to the various timescales to help you intuit the way the controls work together to explore the sound that is held within the Morphagene. This should serve as a roadmap of sorts: a handy reference for finding your way through the forest. Thorough text descriptions of controls and processes follow.

In the book *Microsound*, Curtis Roads identifies the following nine timescales of music. Numbers four through six are accessible by the Morphagene:

1. **Infinite:** The ideal time span of mathematical durations such as the infinite sine waves of classical Fourier analysis.
2. **Supra:** A time scale beyond that of an individual composition and extending into months, years, decades, and centuries.
3. **Macro:** The time scale of overall musical architecture or form, measured in minutes or hours (or in extreme cases, days).
4. **Meso:** Divisions of form. Groupings of sound objects into hierarchies of phrase structures of various sizes, measured in minutes or seconds. This time scale is represented in the Morphagene by the Reel and/or Splice.
5. **Sound Object:** A basic unit of musical structure, generalizing the traditional concept of note to include complex and mutating sound events on a time scale ranging from a fraction of a second to several seconds. This time scale is represented in the Morphagene by the Splice and/or Gene.
6. **Micro:** Sound particles on a time scale that extends down to the threshold of auditory perception (measured in thousandths of a second or milliseconds). This time scale is represented in the Morphagene by the Splice and/or Gene.
7. **Sample:** The atomic level of digital audio systems: individual binary samples or numerical amplitude values, one following another at a fixed time interval. The period between samples is measured in millionths of a second (microseconds).
8. **Subsample:** Fluctuations on a time scale too brief to be properly recorded or perceived, measured in billionths of a second (nanoseconds) or less.
9. **Infinitesimal:** The ideal time span of mathematical durations such as the infinitely brief delta functions.

---



15

# REELS

**Note: The following diagrams all assume nothing is patched into the Play Input so the Morphogene is playing back continuously. To hear the entire Splice, turn Gene-Size full counterclockwise. Diagrams not scale. If no microSD Card is present, then only one Reel may be created and Reel mode cannot be entered.**

Reels are stored on the MicroSD card. The Reel Activity Window displays Reels in the following color order:

[Eight circular icons shown in sequence: blue, green, light green, yellow, orange, red, pink, white, with a note indicating "Additional Reels repeat in this color order..."]

The current Reel is loaded on the Morphagene, or a new Reel is created. Starting with an Empty Reel, hold the REC Button and press the Splice Button to start the recording process.

[Diagram showing two buttons labeled "REC" and "SPLICE" with curved lines connecting them]

[Blue circular icon followed by a waveform display showing audio content]

**Maximum length: 2.9 minutes**

# SPLICES

Reels can be divided with a maximum of 299 Splice Markers, resulting in a maximum of 300 possible Splices.

[Waveform display showing four colored sections: blue, green, light green, yellow, and orange, with labels below:]

Splice Marker 1 | Splice Marker 2 | Splice Marker 3 | Splice Marker 4

The current Splice is indicated by the Splice Activity Window:

[Diagram showing a "SPLICE" button icon on the left, and a waveform display with multiple colored sections, with an indicator pointing to the yellow/orange section]

---



16

## SPLICES, CONT.

If a Reel is under 2.9 minutes long...

<table>
  <tbody>
    <tr>
        <td>Splice Marker 1</td>
<td>Splice Marker 2</td>
<td>Splice Marker 3</td>
    </tr>
<tr>
        <td>38 seconds</td>
        <td colspan="2"></td>
    </tr>
  </tbody>
</table>

... You can record additional Splices onto the end, up to 2.9 minutes total. These Splices may consist of entirely new material, overdubs, manipulations of the current Splice, or any combination thereof.

<table>
  <tbody>
    <tr>
        <td>Splice Marker 1</td>
<td>Splice Marker 2</td>
<td>Splice Marker 3</td>
<td>Splice Marker 4</td>
<td>Splice Marker 5</td>
    </tr>
<tr>
        <td>50 seconds</td>
        <td colspan="4"></td>
    </tr>
  </tbody>
</table>

Splices are always evenly distributed through the Organize Panel Control, regardless of their relative length.

---



17

## SHIFT

The Shift Button and/or Gate Input can also be used to Shift from one Splice to the next.

**Note: Organize Panel Control always takes priority. In other words, changes to the Organize Panel Control override the Shift Button and Gate Input.**

[RANGE control dial and waveform display showing blue section at start, followed by lighter colored sections]

[Hand pressing SHIFT button illustration]

[RANGE control dial and waveform display showing green section in middle, with lighter colored sections before and after]

To override a Shift, adjust the Organize Panel Control to the desired Splice.

[RANGE control dial and waveform display showing green section in middle, with lighter colored sections before and after]

[RANGE control dial and waveform display showing blue section at start, followed by lighter colored sections]

---



18

# GENE SIZE AND SLIDE

Gene-Size and Slide determine *how much* and *which portion* of the Splice are playing back. With Gene-Size full Counterclockwise, the entire Splice plays and loops.

![GENE-SIZE diagram showing full waveform playback]

As Gene-Size is turned up, the portion of the selected Splice you hear playing back gradually is getting smaller.

![GENE-SIZE diagram showing reduced playback portion]

![GENE-SIZE diagram showing further reduced playback portion]

The Slide Panel Control and CV Input determines where in the selected Splice the first Gene will begin playing. In other words, Slide offsets the start point of the Genes.

**Note: If Gene Size is set full CCW to play the full Splice, Slide still offsets the start point. This allows audio scrubbing, changes the Play Reset/Start Point, and the temporal location of the EOSG.**

![SLIDE diagram showing offset at beginning]

![SLIDE diagram showing offset at middle]

![SLIDE diagram showing offset at end]

**Note: Slide and Vari-Speed are continuously-variable controls (as opposed to a stepped selection like the Organize Panel Control).**

---



19

# MORPH

Morph controls the amount of playback overlap there is between the end of a Gene instance and the beginning of the next. In other words, the Morph Control offsets the start point of the new Gene playback instance the moment the current instance ends.

**MORPH: No Clock patched to CLK IN**

**MORPH: Clock patched to CLK IN**

With Morph fully counterclockwise, a short gap of silence is heard before the start of each subsequent Gene. Gene Gaps and Overlaps turn the Morph Activity Window (i.e. Opposing Vari-Speed Activity Window) Red.

**MORPH**

⊕

**SILENCE**

**GENE GAPS/OVERLAPS**

**FORWARD PLAYBACK**

At around 8:30, the gap of silence decreases to zero and the Genes loop seamlessly with no gap or overlap. This state is indicated by the Morph Activity Window (the Opposing Vari-Speed LED) turning Amber.

**MORPH**

⊕

**MORPH**

**FORWARD PLAYBACK**

---



20

## MORPH, CONT.

Turning the Morph Panel Control up further causes the new Gene instance to start before the previous one has ended, causing a playback overlap of multiple Gene instances. Gene Overlaps are indicated by the Red Morph Activity Window.

MORPH

OFFSET X

GENE GAPS/
OVERLAPS

FORWARD
PLAYBACK

After about 12:00 on the Morph Panel Control, the overlap is more than one half of the full Gene length and a third instance of the Gene is added.

MORPH

OFFSET X

OFFSET Y

---



21

## MORPH, CONT.

At about 1:00, the start positions of all three instances of the Gene are equally spaced from one another and panning is introduced.

MORPH

**PANNING**

**PANNING**

Turning the Morph Panel Control beyond 2:30 introduces a 4th Gene overlap, pitch up variations, and ???*

MORPH

**PANNING/ PITCH/???**

**PANNING/ PITCH/???**

**PANNING/ PITCH/???**

* Not for laboratory use


---



22

# TYPES OF RECORDING

Recording and playback are independent in the Morphagene. In other words, Recording always takes place at a constant speed and direction regardless of modulation that might be happening to previously-recorded material. Thus, any audible modulations are preserved in the resulting recording. What you hear is what you get. This is like having two machines, one for modulated playback, and another for recording the modulated machine.

The REC Button toggles Record On/Off. While sound is coming into the Input(s), hold the REC Button and press the Shift Button to perform Auto Input Leveling (set the Inputs to an appropriate gain for the source material).

---



23

There are three types of Recording:

## 1. Initial Recording

Creates the first Splice in a new Reel. Typically, the S.O.S. Panel Control is set full counterclockwise to allow the incoming audio to record at full amplitude.

Starting with an Empty Reel, hold the REC Button and press the Splice Button to start the recording process. Press the REC Button again to stop the active recording process. Alternatively, a single Gate/Trigger initiates the recording process while a second, single Gate stops the active recording. Note: If the end of the Reel (~2.9 minutes total) is reached before the user stops recording, Record automatically stops and Reel and Splice Activity Windows ash.

## 2. Time Lag Accumulation

Records over a previously-recorded Splice and has the capability to record continuously, even as the Splice loops and replays. The recording may consist of audio from the currently-selected Splice on the Reel, new audio from the inputs, or any combination thereof, as set by the S.O.S. control.

Starting from a previously-recorded Splice, pressing the REC Button once starts the recording process and pressing the REC Button again stops the recording process. Alternatively, sending a single Gate/Trigger to the Record Gate Input initiates the recording process and a second Gate/Trigger stops the active recording. For overdubbing or Time Lag Accumulation, the recording continues even if the end of the Splice is reached. Recording stops when either the REC Button is pressed again or a Gate/Trigger is received in the REC Input. During TLA, all audible manipulations of S.O.S., Vari-Speed, and other panel controls are reected in the next playthrough of the currently-selected Splice. Using the Organize and/or Shift panel control to navigate to other Splices during TLA adds for addition of their audio to the Splice that is being Recorded into. The Splice that is is active when Recording starts is the only Splice that is Recorded into during TLA. TLA with Vari-Speed set to be above or below 1/1 (green) or MORPH greater than 2:30 (randomized upward pitching) causes sounds to be pitch shifted until they are eventually outside the frequency range of the device. Using Time Lag Accumulation with Vari-Speed Forward less that 1/1 Playback Speed eventually results in loss of signal.

## 3. Record into New Splice

Adds a new Splice of any length (up to 2.9 minutes) to the end of the Reel. This new Splice may be filled with audio from any other Splices on the Reel, new audio from the input(s), or any combination thereof, as set by the S.O.S. control. Use the Organize control to navigate to other Splices during Record into New Splice which adds their audio to the new Splice.

Starting from a previously-recorded Splice, hold the REC Button and press the Splice Button to record into a new Splice. Recording continues until the REC Button is pressed again (or a Gate is received in the REC Gate IN), or until the end of the Reel is reached. During Record into New Splice, all audible manipulations of S.O.S., Vari-Speed, and other panel controls are reected in the newly-created Splice.

The Morphagene begins looping the recording immediately upon completion of the record cycle (assuming the Play Input is held High, which is the default state when there is nothing patched to the Play Input). It is necessary to adjust the S.O.S. control so that the desired source (or blend of sources) appears at the Signal Outputs.

**Morphagene is always storing your latest recordings and splices to the microSD Card. If you do not want to write over a card, remove it once you have loaded the desired Reel. Do not remove the microSD Card while it is busy, as indicated by Shift Button Flashing. To record the process of Shifting Splices, you must turn off or unpatch the clock patched to the Shift Input before you start recording. Once recording has started, it is fine to clock via Shift Input. Alternately, you could use Organize to change Splices while recording.**

---



24

# SOUND DNA

## Replicating Genes: Morph

The MORPH parameter controls the amount of overlap from one Gene to the next. At full counter clockwise, there is a small gap between Genes for a pointillist effect. At around 9:00, Morph is set as a seamless loop, i.e. "1/1" (the opposing Vari-Speed Activity Window lights Amber when Morph is set to an integer). Turning Morph further clockwise overlaps multiple instances of the Gene beyond 1/1, up to as many as 3/1. Turning the Morph control beyond 3/1 causes additional upward pitch randomization of Genes and Panning. The Morph control works the same regardless of the size of the Gene, all the way up to a full Splice of any length including the full Reel, up to 2.9 minutes in length. The Morphagene uses Dynamic Enveloping to achieve smoothing of the audible glitches that result from performing these particle physics experiments with audio signals.

## Traversing Sound on a Genetic Level in a Nonlinear Way

As interesting as the single Gene may sound while modulated, you may find that exploring the many possible Gene windows in a given Splice via modulation of the Slide parameter is even more thrilling. The sonic contrast of the different Genes and the order in which they are heard generates a wealth of new sounds from your source material. Slide immediately moves the playback window, so modulating with Stepped Voltages often results in fast, hard, timbral changes as you move from one cluster of samples to another. Continuous signals, such as CV from MATHS or the Morphagene's own CV Output, are very good modulators for Slide.

## Chronological Inspection of the Sound DNA

With Morph set to be counterclockwise from 12:00 and Gene-Size set to be smaller than the whole Splice, you may use the CLK IN to play through Genes in Chronological order. This is called Synchronous Granulation. At the rising edge of each Clock or Gate, the Morphagene jumps to the next Gene and plays that Gene at the rate and direction determined by Vari-Speed until the next Clock or Gate arrives at the CLK Input. Modulating Vari-Speed and Gene-Size while clocking through the Genes is very pleasing. This is also useful for syncing timbral shifts or for performing crude Time Stretching.

## Gene Identity

The Gene-Size parameter is continuous with the Gene shrinking gradually and smoothly as the parameter increases. The designated Gene Size is a function of time--not of sample length. This behavior allows Genes that have loose relation or no relation at all to the rhythm of captured sound and therefore, the creation of new rhythms. Additionally, it means that a change in Vari-Speed (unless stopped entirely) does not change the temporal length of the Gene, allowing EOSG to be used as a steady clock even if Vari-Speed is being modulated.

---



25

# SOUND-ON-SOUND AND VARI-SPEED

Once the initial recording is made, the Record function then allows for Punch In/Out within the loop and layering of other sounds. The Sound On Sound Control sets the balance of the Live Signal and previously-recorded Loop Signal. While the Morphagene does not record in reverse, it does playback in reverse while recording for Sound On Sound. Therefore, it is possible to have two sounds playing in opposite directions. Once S.O.S. is complete, remember to adjust the S.O.S. panel control so that the desired source (or blend of sources) appears at the Signal Outputs.

*Note: The position of the Vari-Speed Control affects only playback, not recording. In order to hear the unaltered loop, once S.O.S. is complete, set Vari-Speed back to the 1:1 (Green Activity Window) setting around 2:30.*

## Splice

Typically, Splices are made in the meso and sound object scales, but it is possible to Splice into the Microsound range. Organize and Shift treat Splices differently from how the Slide treats the Genes, since Organize waits until the current Splice/Gene reaches the end before moving to the next-selected Splice. A world of micromontage awaits.

## Re-Organizing Splices

Like Shift, Organize is not instantaneous. Turning the panel control, modulating Organize, or pressing/gating Shift causes the Activity Window to change color in order to indicates newly-found Splice; however, the newly-found Splice is not selected for playback until the currently-playing Splice reaches its end. At which point, the End Of Splice Gate Output generates a Gate and the newly-selected Splice begins playing. The Organize parameter is designed for sequential control sources, such as René or Pressure Points. The range is smaller than the other control inputs, reflecting the typical 5Vpp range of analog sequencers.

## Splice Manipulation

Splice Markers and audio can also be removed from the Reel. To remove a Splice Marker, hold the Shift Button and press the Splice Button. This removes the "next" Splice point. For example, if you are on Splice 3 and remove the next Splice point, Splices 3 and 4 merge together. If you want to remove ALL Splice Markers and make the Reel into a single, large Splice, while holding the Shift Button, hold the Splice Button for 3 seconds.

To delete the Splice and its audio, hold the Shift Button and press the REC Button. If you want to remove ALL Splices (i.e. clear the current Reel), while holding the Shift Button, hold the REC Button for 3 seconds.

---



26

## Speed Variation, Direction Variation

There is one control associated with both direction and speed, and that control is Vari-Speed. Also there are Activity Window indicators, which show direction and speed of playback. At 12:00, Vari-Speed is at 0 and playback stops and the associated Activity Windows are lit red. Turning clockwise from 12:00 increases playback speed in the forward direction. Turning counter-clockwise from 12:00 also increases the playback speed but in the reverse direction. During playback, the Activity Window on the right indicates forward playback and the Activity Window on the left indicates reverse playback. When playback is at the original record speed, the associated Activity Window lights Green. At one octave up, the Activity Window lights Baby Blue. At one octave down, the Activity Window lights Peach.

*Note: The opposite side of the Vari-Speed Activity Window indicates the Morph setting (see the section on Morph for more detail).*

Vari-Speed has a wide range with increased resolution toward the center of its range, giving the control a nice feel when "braking" playback with a smooth control voltage. Vari-Speed has as a wider range slowing down than when speeding up, capable of about 12 semitones up, 26 down. For "Wow & Flutter" effects, modulate with a greatly-attenuated Smooth Random voltage (such as Wogglebug Woggle CV), setting the on board attenuator nearly to 12:00 (nulled). Sequencing the speed of playback is also very pleasing when sequencing the Organize parameter.

---



27

# STOPPING PLAYBACK AND MICROSOUND

## Stopping Playback, Starting Playback, Re-Triggering Sounds

When Vari-Speed is at 12:00, the Morphagene slows to a halt. When Vari-Speed is set to greater or less than 12:00, playback starts from where it was halted.

The Play IN also stops and starts playback, but in a very different way. At the end of each Gene or Splice, the Morphagene looks at the Play In. If it is high, then Morphagene continues to play, if it is low, Morphagene stops at the end of the Gene or Splice. In this way, the Play Input, always plays the Gene or Splice from the beginning, as defined by the position of the Slide panel control. The Play Input may also be used to re-trigger sounds, since it is essentially looking for a change in state, from low to high. To achieve classic retrigger effects, send repeat Gates to Play while Vari-Speed is set for playback. Play and Vari-Speed respect one another, so for example, if Vari-Speed is set to 12:00, a Gate at the Play IN does not trigger playback, since the playback speed is effectively zero.

## Microsound

Microsound includes sounds shorter than musical notes and yet longer than single samples. The sound is essentially a cluster of samples contained in an amplitude envelope. The Morphagene offers two methods "Micromontage" and "Granulation."

Micromontage is done manually using the Splice function to cut the sound into pieces and the Organize parameter to asynchronously play through those pieces. It is an interesting process that allows for a great deal of variation, but it may requires patience, since the sound is cut up manually to achieve such variation. While modulating Organize and/or Slide, some quasi-random Micromontage may be performed automatically by patching the EOSG output to the Splice input. This technique should be performed carefully, as it could either result in reaching the maximum number of Splices quickly or more Splices being placed on one end of a Reel than on the other.

## Granulation

Granulation is the automatic splicing of the sound. It is done without regard to source material and is a linear, machine-like process, dividing the audio buffer into progressively-smaller pieces. Since the individual Samples that comprise a digitally-recorded sound make up the DNA of that sound, we refer to these small clusters of samples as Genes. If Gene-Size is set full counterclockwise, the Gene length is identical to that of the full Splice. If not set full counterclockwise, the Gene-Size is set to a constant length in time, as opposed to in samples. Another way to put this is that the temporal length of the Gene is independent of Vari-Speed.

At a certain point on the Gene-Size panel control, the size of the Gene is so small that it is heard as a click. This is still useful because there are many ways to vary the timbre of a click and when hundreds of clicks are heard one after the other, the ear perceives them as a tone with varied timbre (Slide) and pitch (Vari-Speed). Also try manipulating the Morph parameter to stagger, layer, and spread these clicks.

Granulation is an automatic, Real-Time process, and therefore, it may be modulated by control signals within the modular system. Additionally, the Clock Input allows for strict, synchronous playback of the resulting pieces, which is useful for Time Stretch and other effects.

---



28

# SYNCHRONIZING THE MORPHAGENE

The Morphagene has four timing inputs and one timing output. The Play, Rec, Shift, and Clock Inputs are all useful for synchronizing the Morphagene. The EOSG Gate OUT is useful for synchronizing other events to the Morphagene.

## CLK IN

Patching a Clock source to the Clock Input performs several tasks which are in some cases dependent upon other settings on the Morphagene. During the Recording process, a Clock patched to the CLK Input synchronizes the REC Button, allowing for Recording Start and Stop times to be synchronized with an External Clock. The REC Button ashes after it is pressed in order to indicate that the Morphagene is waiting for the next Clock cycle to start Recording. On the next Clock received, the procedure is completed, the REC Button lights in order to indicate recording has started and then turns off to indicate Recording has stopped.

## Gene Shift

When Gene-Size is smaller than the whole Splice, and Morph is set to 2/1 or below, the CLK input shifts/increments through the individual Genes of your sound. Gene-shifting takes place immediately upon reception of a clock pulse at the input, resulting in changes with a directly audible relation to the pulse stream.

## Time Stretch/Compression

When Gene-Size is smaller than the whole Splice, and Morph is set to greater than 2/1, the CLK input drives a Time-Stretch or Time-Compression process. During Time Stretch/Compression, the Vari-Speed control can be used to change the "pitch" of playback without affecting its speed. Time Stretch/Compression requires a clock rate of one clock every 3.5 seconds or faster. Time Compression hits its maximum speed with a clock rate of about 18Hz, above which it does not compress time any further.

## PLAY Input

Patching a Clock or Gate signal to the Play Input allows for triggering and retriggering of the Gene or Splice at the rate of the Clock or Gate. This input may be used like the Reset input on an LFO. Try patching a division of the master clock so that the loop is periodically pulled back into sync with the rest of the patch. Note: the Start point is selected by the Slide parameter. For example, you could also Mult your clock to a sequencer or Wogglebug and sequence Slide in order to start the Splice at a new point with each Gate received at the Play input.

## SHIFT Input

The Shift Input makes it possible to synchronize Splice incrementation with an external event. This means it is possible to change from one loop or sound to the next in time with some other event occurring in your patch. The most traditional use is to Splice up a percussive loop. Then, use a Gate Sequencer or Clock source patched to the Shift and Play Inputs in order to step through these Splices.

---



29

## SHIFT Input

The Shift Input makes it possible to synchronize Splice incrementation with an external event. This means it is possible to change from one loop or sound to the next in time with some other event occurring in your patch. The most traditional use is to Splice up a percussive loop. Then, use a Gate Sequencer or Clock source patched to the Shift and Play Inputs in order to step through these Splices.

## REC Input

Patching a Clock or Gate signal to the REC Input allows for recording and applying Sound On Sound at the rate of a Clock or Gate.

## Control Outputs: CV and EOSG

In addition to the audio outputs, the Morphagene also includes two control signal outputs. The CV Output is the product of an Envelope Follower, so that the output voltage goes higher as the output level gets louder. It can be used to control characteristics of the sound being fed into the Morphagene, to process of the sound at the output (such as through Echophon or Erbe-Verb), or to patch directly into the Morphagene's CV inputs. The CV Out's shape and amplitude is highly dependent on the sound in the currently playing Splice or Gene. Higher Morph values often stabilize the CV Out to some degree.

The End Of Splice Gate outputs a trigger at the end of the playback window, useful for syncing other things in the system to the Morphagene. It is affected by the Gene-Size and Morph parameters, both of which cause the EOSG to fire more frequently as their values increase.

---



30

# TIME STRETCH

With an External Clock source patched the CLK Input, setting the Morph panel control to 11:00 changes the Morph Activity Window to Blue in order to indicate Time Stretch.

[Image shows a modular synthesizer panel with various knobs, inputs, and outputs. An arrow labeled "OUTPUT" points to the top right, and an arrow labeled "CLOCK SOURCE" points to the left side of the panel. The center of the panel features a circular display with concentric rings in blue and green.]

In this mode, the External Clock source synchronizes the currently-selected Splice with each Clock or Gate:

[Diagram showing timing relationship between CLOCK and SPLICE signals. The CLOCK signal shows 4 evenly-spaced pulses. The SPLICE signal shows a continuous waveform pattern synchronized with the clock pulses, with white arrows indicating direction of movement within blue horizontal bars.]

Increasing the External Clock Rate results in Time Shifting of the currently-selected Splice:

[Diagram showing timing relationship with faster clock rate. The CLOCK signal shows 5 pulses in a shorter time period. The SPLICE signal shows a compressed waveform pattern with white arrows indicating direction of movement within blue horizontal bars.]

Decreasing the External Clock Rate results in Time Stretch of the currently-selected Splice:

[Diagram showing timing relationship with slower clock rate. The CLOCK signal shows 4 pulses spread over a longer time period. The SPLICE signal shows an expanded waveform pattern with white arrows indicating direction of movement within blue horizontal bars.]

---



31

# GENE SHIFT

With an External Clock source patched to the CLK Input, setting the Morph panel control to 11:00 or lower changes the Morph Activity Window to Red in order to indicate Gene Shift.

**OUTPUT**

**CLOCK SOURCE**

In this mode, the External Clock increments Genes with each Clock or Gate:

**CLOCK**

**SPLICE**

**CLOCK**

**SPLICE**

**CLOCK**

**SPLICE**

---



32

# NON REAL-TIME FUNCTIONS

Once you are familiar with recording, playback, and the many real-time modulations of the Morphagene, you may want to dig into the deeper non-realtime functions. When dealing with large numbers of Splices, these more abstract processes can be useful, but they are by no means necessary for enjoyment of the module.

## Deleting Splice Markers

As you recall, Splice Markers are created by Pressing (and releasing) the Splice Button or sending a gate to the Splice input. It is possible to remove a Splice Marker by holding the Shift Button and then pressing the Splice Button. This merges the currently selected Splice with the next one.

*Note: Alternately recording and deleting Splices while the SD Card is busy (SHIFT Button Flashing) may result in losing recorded material.*

[Diagram showing two hand gestures: one holding the SHIFT button, plus sign, and another pressing the SPLICE button]

[Waveform diagram showing audio segments in different colors (blue, green, yellow, orange) with splice markers]

<table>
  <tbody>
    <tr>
        <td>Reel Start</td>
<td>Splice Marker 1</td>
<td>Splice Marker 2</td>
<td>Splice Marker 3</td>
<td>Splice Marker 4</td>
    </tr>
  </tbody>
</table>

[Second waveform diagram showing the result after merging splice markers, with fewer markers]

<table>
  <tbody>
    <tr>
        <td>Reel Start</td>
<td>Splice Marker 1</td>
<td>Splice Marker 2</td>
<td>Splice Marker 3</td>
    </tr>
  </tbody>
</table>



---



33

## Deleting Splice Audio

It is also possible to delete the whole region (including audio) for the currently-selected Splice, by holding Shift and pressing the REC Button.

[Diagram showing two hands: Left hand holding SHIFT key, plus sign, right hand pressing REC button]

[Waveform diagram showing audio regions in different colors (blue, green, orange) with labels:]
- Reel Start
- Splice Marker 1
- Splice Marker 2
- Splice Marker 3

[Arrow pointing down with circular icon]

[Second waveform diagram showing the result after deletion, with fewer regions:]
- Reel Start
- Splice Marker 1
- Splice Marker 2

*Note: Alternately recording and deleting Splices while the SD Card is busy (SHIFT Button Flashing) may result in losing recorded materal.*

[Circular arrow icon at bottom of page]

---



34

## Deleting All Splice Markers

To delete all Splice markers in the Reel (and preserve audio content), hold SHIFT and press the SPLICE button.

<table>
  <tbody>
    <tr>
        <td>Reel Start</td>
<td>Splice Marker 1</td>
<td>Splice Marker 2</td>
<td>Splice Marker 3</td>
    </tr>
  </tbody>
</table>

HOLD + PRESS AND HOLD 3 SEC

## Clearing the Reel by Deleting all Splices and Audio

It is also possible to delete the whole region (including audio) for the currently-selected Splice, by holding Shift and pressing the REC Button.

HOLD + PRESS AND HOLD 3 SEC

---



35

# Reels

Reels are collections of audio up to ~2.9 minutes in length. Each Reel can be divided into up to 300 Splices using the Splice controls. One Reel is active on the Morphagene at any given time. Additional Reels (and their Splice Markers) are stored on the included microSD Card in the form of standard .wav files.

MG is always storing your latest recordings and splices to the microSD Card. If you do not want to write over a card, remove it once you have loaded the desired Reel. Do not remove microSD Card while it is busy, as indicated by the Shift Button Flashing.

*Note: If no microSD Card is present, then only one Reel may be created, Reel mode cannot be entered.*

To select a Reel, enter Reel Mode by holding the Splice Button and pressing the REC Button. While using Reel Mode, it is important to remove or turn off modulation to REC, Splice, Shift, and Organize.

[Diagram showing two hands: left hand labeled "PRESS" pressing the REC button, right hand labeled "HOLD" holding the SPLICE button]

The Reel Activity Window ashes to indicate the Reel mode is active. While in Reel mode, the Organize control and CV Inputs are used to select the Reel that is loaded into the Morphagene. Reels are color-coded at the Reel Activity Window and with S.O.S. set clockwise, the output of the Morphagene is a brief preview of the highlighted Reel. Select the Reel for loading and exiting Reel mode with the same button combination, holding Splice and Pressing Record.

The final Reel (selected by the Organize Panel Control at full clockwise) ashes Pink and White and has no audio preview. Exit Reel Mode with this Reel selected to create a new Reel. Up to 32 Reels may exist on a single microSD Card.

While in Reel mode, Reels can also be deleted by holding Shift and pressing Record.

[Diagram showing two hands with a plus sign between them: left hand labeled "HOLD" holding the SHIFT button, right hand labeled "PRESS" pressing the REC button]

If only one Reel is present and it is deleted, a new Reel is automatically created and Reel Mode is exited. While in Reel Mode, remove modulation to REC, Splice, Shift, and Organize.

---



36

# File Naming Convention

Reels take the form of 32-bit float, 48000Hz stereo-only .wav files. Any program that can export such files can create Reels for the Morphagene, or be used to edit Reels that have been created by the Morphagene. Examples include Audacity and Reaper. Splice Markers take the form of standard audio markers in the .wav file, which can be created, moved, and deleted using Reaper and other software.

The Morphagene expects to find the 1-32 Reels in the Root directory of a FAT32-formatted microSD Card. The Reels are named as follows: mg1.wav, mg2.wav, mg3.wav, ...mg9.wav, mga.wav, mgb.wav, …mgw.wav. Note: each file must be 2.9 minutes or less and stereo in order for the Morphagene to recognize and load the files.

---



37

# TIPS AND TRICKS

- Don't forget to Auto-Level when switching from line to modular level sources or vice-versa. When performing Auto-Level, it is possible to hold the REC Button and tap the Shift Button while Recording to let the Morphagene continue "listening" to the source material.

- The Morphagene is always modulating the sound based on the settings of the Panel Controls and any CV being applied. After recording modulations into a new Splice, refer to the "initialization" settings on p.7 to see how to set the Morphagene to unmodulated 1/1 looping playback of your new recording.

- Recording and playback are independent in the Morphagene. In other words, Recording always takes place at a constant speed and direction regardless of modulation that might be happening to the previously-recorded material. Thus, any audible modulations are preserved in the resulting recording, but do not affect the recording of the Live sound. What you hear is what you get.

- The Morph control affects a number of characteristics of the sound, including loop length (counterclockwise settings adds a gap between one Gene/Splice and the next) and the frequency of the EOSG trigger output (as Morph goes up, more Genes are present and the trigger fires more frequently).

- During Reel Selection, the Organize CV input is still active. For best results, remove signals patched to REC, Splice, Shift, and Organize while using Reel Mode.

- With a Clock patched/Modulating Shift at Shift In, hold the Shift Button to momentarily stop the clocking or modulation.

- Sound On Sound (S.O.S.) may be misused as a Voltage Controlled Cross-fader (between Live and Loop) or VCA for Loop (no Live Signal Patched).

- Using Time Lag Accumalation with Vari-Speed Forward less that 1/1 Playback Speed eventually results in loss of signal.

- Copying Reels: (Thanks, Tony Rolando) When you mount an SD Card, the current Reel is copied to the newly-mounted SD Card. Use this to create copies of the Reel. Mount SD Card. Load Reel the to be copied. Remove SD Card. Wait for Shift Button to be OFF. Insert SD Card and press Shift to mount. Currently loaded Reel will be coped to the SD Card.

---



38

# MG137 CHANGELOG

1. Improves Vari-Speed response for slow modulation rates.

2. Improves SD Card file format for more compatibility with software .wav file formats.

3. Doubles total record time per Reel. Upgraded memory performance offers an increase from 1.45 minutes to 2.9 minutes of record time. There is no change in audio fidelity, which remains at 48kHz 32-bit.

If you are using Splices that are over 1 minute long, the Slide parameter is scanning across a massive amount of material. Therefore, the longer the Splice, the longer it takes to get to where you are Sliding toward. Imagine a tape machine shuttling forward or backward to get to your desired destination. No, we cannot change this, as the MG is already scanning as fast as possible. The new double record time firmware makes this even more of a factor.

Read and Write times will be MUCH longer when you have a Full 2.9 minute reel. For example, if you decided to erase a 5 second Splice in the middle of a 2.9 minute reel, it would take twice as long to write as before because the MG must re-write the entire 2.9 minute reel. Keep in mind, all of this is happening in the background and should not cause too much hassle. That being said, when the Shift Button is ashing, it is best to not perform more SD Card actions. For most granular processing, Splices of 10 seconds or less are recommended.

So why even bother increasing record time to 2.9 minutes? Well, it allows for storing even more Splices on a Reel and it also makes it possible to record long portions of a patch for later arranging within your DAW. You could potentially capture a patch including modulations of a shorter Splice (say 10 seconds) in order to retrieve the sound from the SD Card and compile it into a larger work using your DAW.

4. Increased max number of Splices to 300.

---



39

# MG155 CHANGELOG

1. Minor Bug Fixes
2. User-definable options for many new alternate behaviors

A Morphagene running this firmware will automatically create (or modify the existing) options.txt file in mounted SD Card's root directory. This file has always contained information about the last-used Auto-Level settings and Reel, and now adds a number of user-definable firmware options. To make use of the options.txt file you will need to remove the SD Card from your Morphagene and mount it in a computer. Navigate to the options.txt file and click on it it to open.

Each option appears on its own line of text and contains the option name, the current setting (represented by a number), then a comment describing the option. For example:

```
omod 0 //Organize option: 0 organize at end of gene, 1 organize immediately
```

The option name is "omod", the current setting is 0, and the comment tells us that the "0" setting means the Organize control will wait for the end of the Gene before moving to the new Splice and the "1" setting will move to a new Splice immediately.

To edit, simply change the number 0 to any of the listed values (some options have two, some have three), save the file to the SD card, and then re-mount in the SD Card in the Morphagene.

So we could edit the above line to show:

```
omod 1 //Organize option: 0 organize at end of gene, 1 organize immediately
```

When the Morphagene mounts this card its Organize behavior will be changed so that Organize is read immediately regardless of the playback position.

It is very important that only the option number (see below in bold) is changed. No other text in the options.txt file should be changed.

```
omod 1 //Organize option: 0 organize at end of gene, 1 organize immediately
```

There is only one options.txt file per SD Card. If you need to have different Morphagene firmware configurations on hand at all times, you will need to have different SD Cards with those options.txt files on them along with any reels you wish to use.

The options are all explained briefly in the options.txt file. Note that in order to use the 1V/ Octave option you will need to adjust the Vari-Speed attenuverter to get good response.

*Note: The "0" setting for all options is the default setting. Thus if you do not edit the options file, you will not engage any of the new options: they are totally optional.*

---



40

Example contents of options.txt:

```
6 46345 0
//
// firmware version 155
//
// 0 option is default
vsop 0 ///Varispeed option: 0 bidirectional classic, 1 bidirectional 1 v/oct, 2 positive only - 1
v/oct.
inop 0 //Input option: 0 record SOS mix, 1 record input only
pmin 0 //Phase/position modulation: 0 no phase modulation, 1 phase playback modulation on right
signal input when no signal on left input
omod 0 //Organize option: 0 organize at end of gene, 1 organize immediately
gnsm 0 //Gene smooth: 0 classic, 1 smooth gene window
rsop 0 //Record option: 0 record + splice = record new splice, record = record current splice; 1
record + splice = record current splice, record = record new splice
pmod 0 //Play option: 0 classic, 1 momentary, 2 trigger loop
ckop 0 //Clock control option: 0 hybrid gene shift time stretch, 1 gene shift only, 2 time stretch
only
cvop 0 //CV out: 0 envelope follow, 1 ramp gene
```

Here are some more details and possible uses for each of the new options.

## Vari-Speed Option (vsop):

Set to 0 for original behavior (bi-directional with smooth motion).

Set to 1 for new bidirectional v/oct behavior. With this option, if you carefully adjust the input attenuverter to the correct setting, the Vari-Speed input will track 1 volt per octave.

Set to 2 for new single-direction v/oct behavior. With this alternate behavior, playback happens only in the forward direction, with STOP achieved by setting Vari-Speed fully counterclockwise. In this mode, there is significantly more precision on the control, especially in lower octaves, and it can go slower before stopping. If you carefully adjust the input attenuverter to the correct setting, the Vari-Speed input will track 1 volt per octave.

## Input option (inop):

Set to 0 for original behavior, where the Morphagene records the mix of input and Reel playback as set by the SOS control and CV.

Set to 1 for new behavior. With this alternate behavior, the Morphagene will always record only what is at the input regardless of the SOS setting (the SOS control still does have an effect on playback). This makes layered overdubs impossible, but opens up some new possibilities. For example, when recording into a new Splice this can be useful as it will allow you to run the Morphagene's outputs through effects and other processing, then back into the inputs, and record the result for variation.

With the input option set to 1, the behavior of Time Lag Accumulation is also significantly changed. Instead of accumulating, we now have the buffer replaced at the end of each Splice length. Thus it becomes a single-re-peat microsound-capable delay. With longer delay times this can be used for adding texture to an input signal. With very short delay times it could be used for functionally immediate pitch-shifting and granulation. Medium delay times allow stutter and beat-repeat effects especially when modulating SOS.

---



41

## Phase/Position Modulation option (pmin):

Set to 0 for original behavior, in which the Left and Right inputs are always standard audio inputs.

Set to 1 to enable an optional audio rate phase/position modulation using the Right audio input of the Morphagene. The Right input will phase modulate the sound coming out of the Morphagene. If there is a signal in the Left input jack, then phase modulation will not happen (note: if you have a signal in the Left input jack and remove it, it will take a few seconds for the Morphagene to confirm this and begin using the Right input for modulation). This way you can easily switch to new input sources for either or both inputs when you want to record something. Try tuning the modulator to a frequency harmonically related to the input... or perhaps using one of the outputs as its own modulator.

## Organize option (omod):

Set to 0 for original behavior, where Organize and SHIFT changes take effect at the end of the current Gene or Splice.

Set to 1 to make Organize and SHIFT work immediately regardless of the position of the playback head. With this option enabled, Organize and SHIFT will be more active/immediate. Note that the Organize and SHIFT controls do not have any additional enveloping involved, so depending on material this option could result in audible clicks. Take advantage of this for effect if you like.

## Gene Smooth option (gnsm):

Set to 0 for default Gene windowing. Default gene windowing is only long enough to suppress clicks, not long enough to have any other audible effect.

Set to 1 for a more pronounced windowing of Genes. At low Gene Size settings, enabling this behavior results in slight fading out/in when looping. With smaller Genes it results in a different, more "liquid" microsound texture.

## Record option (rsop):

Set to 0 for original behavior, in which a press/release of RECORD begins recording into the current splice, and REC+SPLICE begins recording into a new Splice.

Set to 1 to reverse the button combos for Record into Same Splice, and Record into new Splice. When set to 1, this option makes it so that a press/release of RECORD starts recording into a new Splice. Thus with rmod set to 1 it is possible to automate the recording of new Splices, by patching a gate to the Record input. One possible use for this is to achieve an instantly glitched version of whatever you are recording: send a fast stream of random gates to RECORD, then remove all the Splice points. With this setting (and always), remember that the max total record time is 2.9 minutes, and the max number of Splices is 300.

---



42

## Play option (pmod):

The pmod option sets the behavior of the PLAY gate input. The PLAY input is normaled high, so the Morphagene will simply loop when the PLAY input is unpatched, regardless of the setting of this option.

Set to 0 for original behavior, where the Morphagene looks at the PLAY input at the end of the Gene or Splice to decide whether to continue playing. If high, it continues playing. If low, it waits for the input to go high before playing again.

Set to 1 for Momentary Play input, which will cause the Morphagene to start playing immediately upon gate high, and stop immediately on gate low. This could be used for tighter timing or other effects.

Set to 2 for Trigger Loop, which will cause the Morphagene to jump to the start point set by Slide and Organize, immediately upon the rising edge of a gate. Unlike default behavior, the Morphagene will continue playing at all times. Achieve interesting rhythms by exploring the relation between the tempo of the PLAY gate and the Gene Size. One thing to keep in mind is that in Trigger Loop mode there is no way to stop playback using the PLAY input. Of course if you are looking for silence it is always possible to set Vari-Speed to STOP, set SOS counter-clockwise with nothing in the input, navigate to a Splice filled with silence, patch the outputs to VCAs, or turn off your system and go to bed.

## Clock Control option (ckop):

Set to 0 for original behavior, where the Clock input's behavior depends on the setting of MORPH (Gene shift at 2/1 or below, Time Stretch above)

Set to 1 to make Clock input Shift Genes regardless of Morph setting.

Set to 2 to make Clock input drive Time Stretch regardless of Morph setting.

## CV Output option (cvop):

Set to 0 for original behavior, where the CV output is an envelope follower.

Set this to 1 to change the CV Output to a Ramp wave that is timed to the current Gene Size. This synced function generator can be used as a continuous timing signal with a different character from the EOSG.

---



43

# MG157 CHANGELOG

1. Minor Bug Fixes
2. The playback gain is adjusted by 2/3 (-3.5 dB). This gain is before TLA, so it will affect that function.

# MG203 CHANGELOG

- New options added. They are on the "options.txt" file which is found on the root directory of any microSD card that has been mounted in a Morphagene. See the MG155 changelog for more information on this file.

- The new options mcr1, mcr2, and mcr3 allow user-definable Morph Chord Ratios (detuning of 2nd through 4th Genes with Morph at its highest settings). Historical settings have been 2.00000 (+1 octave), 3.00000 (+1 octave & 5th), and 4.0000 (+2 octaves). Now you can set any ratio from 0.06250 (-4 octaves) to 16.00000 (+4 octaves). Ratios are accurate to five decimals. You can also make one or more individual Genes play in reverse with this setting by using negative numbers. A value of 1.00000 results in unison (no detuning). A list of 12-TET ratios is provided as comments in the text file for easy copy/paste.

- The Auto-level procedure has been replaced with an improved input gain staging, accessed with a 4-step gain selection (blue-greenorange-purple). To use, Hold Record, and Press Shift to step through the gain settings: Blue, low -3db, Green, modular level, Orange medium +6db, Purple high +12db.

- Playback gain is reverted to pre-mg157 level to improve TLA and Record-Into-New-Splice functions.

- Internal processing improved to work with (2) and (3) to significantly improve signal-to-noise ratio on input and playback.

- Minor bug fixes.

---



44

# PATCH IDEAS

## Synchronizing Recording

During the Recording process, a Clock patched to the CLK Input synchronizes the REC Button, synchronizing the Recording Start and Stop times with an External Clock. The REC Button flashes after it is pressed in order to indicate that the Morphagene is waiting for the next clock cycle to start Recording. On the next Clock or Gate received, the procedure is completed, the REC Button lights in order to indicate recording has started and then turns off to indicate recording has stopped, ending recording sync.

## Combining Splices into a New Splice

While Recording into New Splice, all audible manipulations of S.O.S., Vari-Speed, and other panel controls are reflected in the newly-created Splice. Using the Organize panel control to navigate to different Splices during Record into New Splice combines the Splices' audio into the New Splice. After Recording stops, it is possible to delete the previous Splices (see Button Combinations, Page 9).

## Tron Bike Race

Patch the Ring Mod Output of the Wogglebug to the Erbe Verb. With the Erbe Verb's Mix Panel Control set to Wet, set size and Decay Pane Controls to around 1:00. Record a sample to a new Splice or Reel. While recording, use the Wogglebug's Freeze button to manually sustain certain sounds. After the Reel is created, turn Morphagene's Sound On Sound Panel Control fully clockwise and set the Morphagene to 1:1 playback by using the Vari-Speed Playback Control. Set Morph to fully clockwise to observe the Random Pitching and Panning of the layered Genes to get the sounds of the bike race started.

## Single-Repeat Microsound-capable Delay and Pitch Shifter
*(Requires firmware MG155 or above)*

With Input option (inop) set to 1 on options.txt, create an empty Splice of the delay length you want, then patch a sound source to one or both inputs and begin TLA. Use SOS as "Wet/Dry" control. Vari-Speed, Morph, Gene Size, Slide will all have near-immediate effect. Very short delay times will have a different "flavor" from medium or long times.