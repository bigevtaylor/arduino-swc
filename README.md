Car Audio Steering Wheel Control interface using Arduino.  

Requires AceButton https://github.com/bxparks/AceButton  

**Ford BA / BF Steering Wheel Control Adapter for Pioneer Stereo**  
  
This code currently only supports the functions that the wired remote tip triggers. Later versions will cover the ring either with a diode configuration  
or a dual digitpot.  

The bulk of this has been built off the knowledge found in the following articles and forum posts;  

http://www.jvde.net/node/7  
https://forum.arduino.cc/index.php?topic=230068.0  
  
This should work on Pioneer head units (Single DIN) that deliver both 5V & 3.3V via the Wired Remote input.  
  
*NOTE : The 3.3V circuit on an Arduino is ONLY available when the unit is powered via USB. If it is powered via VIN then a 5V power line to the digipot will work on a Pioneer that delivers 3.3V via the WR.*
  
I recommend leaving the USB free for debug / programming whilst installed in vehicle and power the unit via another source.  

Current button configuration:  

Function | Button | Action
-------- | ------ | -------
Volume Down | VOL- | Press
Volume Up | VOL+ | Press
Next Track / Program | Seek | Press
Previous Track / Program | Seek | Double-press
Source | Source | Press
Voice Control | Source | Long-press
Hang-up | Source | Double-press

*Version 0.1 08/08/2020*  
Complete test in vehicle, all but previous track and power off functioning  

*Version 0.2 08/08/2020*  
Altered to catch release instead of press and supresss click to catch double-click  

*Version 0.3 09/08/2020*  
Cleaned up volume trigger, reset track -/+ trigger  

*Version 0.4 29/08/2020*  
Introduced dual digipot commands for tip & ring triggers, this enables BT phone commands  

*Version 1.0 11/10/2020*
Final wire with power via VIN, step-down to ~7.5V. Re-calibrated virtual buttons as they were a bit off due to the new power source, I assume

*Version 1.1 25/20/2020*
Optimised button timing

