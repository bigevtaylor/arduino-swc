Car Audio Steering Wheel Control interface using Arduino.  

Requires AceButton https://github.com/bxparks/AceButton  

**Ford BA / BF Steering Wheel Control Adapter for Pioneer Stereo**  
  
This code currently only supports the functions that the wired remote tip triggers.

The bulk of this has been built off the knowledge found in the following articles and forum posts;  

http://www.jvde.net/node/7  
https://forum.arduino.cc/index.php?topic=230068.0  
  
This should work on Pioneer head units (Single DIN) that deliver both 5V & 3.3V via the Wired Remote input.  
  
*NOTE : The 3.3V circuit on an Arduino is ONLY available when the unit is powered via USB. If it is powered via VIN then a 5V power line to the digipot will work on a Pioneer that delivers 3.3V via the WR.*
  
I recommend leaving the USB free for debug / programming whilst installed in vehicle and power the unit via another source.  

Current button configuration:  

Function | Button | Action
-------- | ------ | -------
Volume Down | VOL- | Press & Hold
Volume Up | VOL+ | Press & Hold
Next Track / Program | Seek | Press
Previous Track / Program | Seek | Double-press
Play / Pause* | Seek | Long-press
Source | Source | Press
Hang-up | Source | Double-press
Voice Control | Source | Long-press

* Play/Pause only operates in APP mode on the Pioneer

*Digipot values to Pioneer functions (Pioneer MVH-S305BT)*

Function | Resistance | Digipot Value | Time | Plug
-------- | ---------- | ------------- | ---- | ----
Volume Up | 16k | 33 - 46 | 50ms | Tip
Volume Down | 24k | 47 - 72 | 50ms | Tip
Track + / Seek + | 8,2k | 17 - 22 | 50ms | Tip
Track - / Seek - | 11,25k | 23 -32 | 50ms | Tip
Source | 1,2k | 1 - 5 | 50ms | Tip
Power | 1,2k | 1 -5 | 1000ms | Ring
Band | 62,75k | 88 - 212 | 50ms | Ring
Play / Pause | 62,75k | 88 - 212 | 50ms | Tip
Display | 5,75 | 12 - 16 | 50ms | Tip
Mute | 3,5 | 6 - 11 | 50ms | Tip
Voice Control | 6,7k | 98 - 232 | 1000ms | Ring
Hang-up | 5,5k | 14 | 50ms | Tip

*Note, **Band** in some modes can result in Play / Pause and **Voice Control** is dependant on Pioneer model and mobile phone*


*Version 0.1 08/08/2020*  
Complete test in vehicle, all but previous track and power off functioning  

*Version 0.2 08/08/2020*  
Altered to catch release instead of press and supresss click to catch double-click  

*Version 0.3 09/08/2020*  
Cleaned up volume trigger, reset track -/+ trigger  

*Version 0.4 29/08/2020*  
Introduced dual digipot commands for tip & ring triggers, this enables BT phone commands  

*Version 1.0 11/10/2020*
Final wire with power via VIN, step-down to ~ 7.5V. Re-calibrated virtual buttons as they were a bit off due to the new power source, I assume

*Version 1.1 25/20/2020*
Optimised button timing

*Version 1.2 02/11/2020*
Adjusted digipot value to reduce bleed into next function - seek & volume

*Version 1.3 11/12/2020*
Included Longhold and Release for VOL +/- and minor changes with move to AceButton 1.8

*Version 1.4 1/11/2023*
Re-calibrated the ladder, disabled a couple of functions due to moving head unit to a Pioneer SPH-DA306DAB. Will re-learn functions soon.
