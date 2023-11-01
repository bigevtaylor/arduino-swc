/* Ford BA / BF Steering Wheel Control Adapter for Pioneer Stereo
  
This code currently only supports the functions that the wired remote tip
triggers. Later versions will cover the ring either with a diode configuration
or a dual digitpot.
   
This should work on Pioneer head units (Single DIN) that deliver both 5V & 3.3V
with the Wired Remote input.
 
*** NOTE : The 3.3V circuit on an Arduino is ONLY available when the unit is powered
via USB. If it is powered via VIN then a 5V power line to the digipot will work on a
Pioneer that delivers 3.3V via the WR.
 
I recommend leaving the USB free for debug / programming whilst installed in vehicle
and power the unit via another source.
 
Version 0.1 08/08/2020
Complete test in vehicle, all but previous track and power off functioning

Version 0.2 08/08/2020
Altered to catch release instead of press and supresss click to catch double-click

Version 0.3 09/08/2020
Cleaned up volume trigger, reset track -/+ trigger

Version 0.4 29/8/2020
Introduced dual digipot commands for tip & ring triggers, this enables BT phone commands

Version 1.0 11/10/2020
Final wire with power via VIN, step-down to ~7.5V. Re-calibrated virtual buttons as they were a bit off due to the new power source, I assume

Version 1.1 25/10/2020
Optimised button timing

Version 1.2 02/11/2020
Adjusted digipot value to reduce bleed into next function - seek & volume

Version 1.3 29/11/2020
Included Longhold and Release for VOL +/- and minor changes with move to AceButton 1.8

Version 1.4 1/11/2023
Re-calibrated the ladder, disabled a couple of functions due to moving head unit to a Pioneer SPH-DA306DAB. Will re-learn functions soon.

*/
 
#include <SPI.h>
#include <AceButton.h>
using namespace ace_button;

// Set the SWC input PIN
static const uint8_t BUTTON_PIN = A6;

// Set the digipot wiper
int csPin =  10;

// Set the wiper addresses for the digipot
int wiperTip = B00000000;
int wiperRing = B00010000;


// Set the digitpot resistor value for no button
int ground = 0;

// Create 4 AceButton objects, with their virtual pin number 0 to 3.
// Each virtual PIN relates to a resistance value on the ladder
static const uint8_t NUM_BUTTONS = 4;
static AceButton b0((uint8_t) 0);
static AceButton b1(1);
static AceButton b2(2);
static AceButton b3(3); 
static AceButton* const BUTTONS[NUM_BUTTONS] = {
    &b0, &b1, &b2, &b3,
};

// Define the ADC voltage levels for each button.
// For 4 buttons, we need 5 levels.
// These values are read from SWC buttons, '08 Ford BF
static const uint8_t NUM_LEVELS = NUM_BUTTONS + 1;
static const uint16_t LEVELS[NUM_LEVELS] = {
  0 /* VOL- */,
  295 /* VOL+ */,
  462 /* SEEK */,
  550 /* SOURCE */,
  664 /* Open */,
};

// The LadderButtonConfig constructor binds the AceButton to the
// LadderButtonConfig.
static LadderButtonConfig buttonConfig(
  BUTTON_PIN, NUM_LEVELS, LEVELS, NUM_BUTTONS, BUTTONS
);

void handleEvent(AceButton*, uint8_t, uint8_t);

void setup() {
  delay(1000); // some microcontrollers reboot twice
  Serial.begin(115200); 
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));
  
  // Don't use internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT);

// Configure the ButtonConfig with the event handler.
  buttonConfig.setEventHandler(handleEvent);
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);  
  buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

//  Ready digitpot, set wiper to no button
  pinMode(csPin, OUTPUT);
  SPI.begin();
  digitalWrite(csPin, LOW);
  SPI.transfer(wiperTip); // command
  SPI.transfer(ground); // value
  SPI.transfer(wiperRing); // command
  SPI.transfer(255); // float the ring circuit
  digitalWrite(csPin, HIGH);
  Serial.println(F("setup(): ready"));
}

void loop() {
  // Should be called every 4-5ms or faster, for the default debouncing time
  // of ~20ms.
buttonConfig.checkButtons();
  
}

// In the function for the tip commands we only change resistance for the 
// wiper where the tip is connected. The ring floats and is ignored by the 
// head unit
void wrTip(int digiValue, int delayMs) {
      digitalWrite(csPin, LOW);
      SPI.transfer(wiperTip);
      SPI.transfer(digiValue);
      //Serial.println(" Button Press"); // for debug
      delay(delayMs);
      SPI.transfer(wiperTip);
      SPI.transfer(ground);
      //Serial.println(" Button Release TIP"); // for debug
      digitalWrite(csPin, HIGH);
}

void wrTipHold(int digiValue) {
      digitalWrite(csPin, LOW);
      SPI.transfer(wiperTip);
      SPI.transfer(digiValue);
      //Serial.println(" Hold TIP"); // for debug
      digitalWrite(csPin, HIGH);
}

void wrTipRelease() {
      digitalWrite(csPin, LOW);
      SPI.transfer(wiperTip);
      SPI.transfer(ground);
      //Serial.println(" Button Release TIP"); // for debug
      digitalWrite(csPin, HIGH);
}

// For ring functions the wiper for the tip is where resistance is applied
// in combination with taking the ring to ground
void wrRing(int digiValue, int delayMs) {
      digitalWrite(csPin, LOW);
      SPI.transfer(wiperRing);
      SPI.transfer(ground);
      //Serial.println(" Select Ring, ground"); // for debug
      SPI.transfer(wiperTip);
      SPI.transfer(digiValue);
      //Serial.println(" Select Tip, button"); // for debug
      delay(delayMs);
      SPI.transfer(wiperTip);
      SPI.transfer(ground);
      SPI.transfer(wiperRing);
      SPI.transfer(255); // float
      //Serial.println(" Button Release Ring"); // for debug
      digitalWrite(csPin, HIGH);
}

// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t /* buttonState */) {

  uint8_t swButton = button->getPin();
  uint8_t swPress = (eventType);

 /*
  // Print out a message for all analogue events for debug
  Serial.print(F("handleEvent(): "));
  Serial.print(F("virtualPin: "));
  Serial.print(swButton);
  Serial.print(F("; eventType: "));
  Serial.println(swPress);
 */

// Volume Down, Single Click
  if ((swButton == 0) && (swPress == 0)) {
      ///Serial.println(" VOL DOWN Click"); // for debug
      wrTip(55, 50); // 24kOhm
  }

// Volume Down, Hold
  if ((swButton == 0) && (swPress == 4)) {
      //Serial.println(" VOL DOWN Hold"); // for debug
      wrTipHold(55); // 24kOhm
  }

// Volume Down, Release Hold
  if ((swButton == 0) && (swPress == 6)) {
      //Serial.println(" VOL DOWN Release Hold"); // for debug
      wrTipRelease();
  }

// Volume Up, Single Click
// A pressed event is the most responsive for volume control
  if ((swButton == 1) && (swPress == 0)) {
      //Serial.println(" VOL UP");
      wrTip(42, 50); // 16kOhm
  }

// Volume Up, Hold
  if ((swButton == 1) && (swPress == 4)) {
      //Serial.println(" VOL UP Hold");
      wrTipHold(42); // 16kOhm
  }

// Volume Up, Release
  if ((swButton == 1) && (swPress == 6)) {
      //Serial.println(" VOL UP Release Hold");
      wrTipRelease();
  }

/*
* Relase is suppressed and clicked is delayed, these events catch click and 
* double-click with a reasonale delay for the track & seek -/+ functions
* and source button functions. A slight delay in these functions is acceptable UX.
* At least, I think so anyway :)
*/

// Next Track
  if ((swButton == 2) && (swPress == 2)) {
      //Serial.println(" Next Track");
      wrTip(19, 50); // 8kOhm
  }

// Previous Track
  if ((swButton == 2) && (swPress == 3)) {
      //Serial.println(" Previous Track");
      wrTip(27, 50); // 11,25kOhm
  }

//  Seek Series (DAB or Radio)
  if ((swButton == 2) && (swPress == 4)) {
      //Serial.println(" Voice Control");
      wrTip(100, 50); // 62,75kOhm
    }

//  Source
  if ((swButton == 3) && (swPress == 2)) {
      //Serial.println(" Source");
      wrTip(2, 50); // 1.2kOhm
  } 

//  Voice Control
//  if ((swButton == 3) && (swPress == 4)) {
//      Serial.println(" Voice Control");
//      wrRing(232, 1000); // 38-88kOhm hold
//    }

//  Hang-up
  if ((swButton == 3) && (swPress == 3)) {
      Serial.println(" Hang-up");
      wrRing(14, 50); // 5kOhm
    }
}
