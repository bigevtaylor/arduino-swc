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

Version 1.5 28/08/2026
Non-blocking digipot pulses - removed delay() from wrTip/wrRing so button
debouncing is never stalled; a timed release is polled from loop() instead.
Modernised for AceButton 1.10.1 (named event constants, nullptr ButtonConfig,
dropped the unused RepeatPress feature, 5ms sampling rate-limit). Button-feel
timing moved into setup() - VOL +/- and Seek hold now engages at 500ms, was 1000.
The wait for a USB serial host now times out after ~2s so the unit still boots
in the car with nothing attached.
Needs re-test in vehicle: the old calibration assumed loop() froze for 50ms on
every button action, which no longer happens.

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
// Each virtual PIN relates to a resistance value on the ladder.
// The nullptr buttonConfig suppresses the unused System ButtonConfig
// (saves ~30 bytes flash + 26 bytes RAM); LadderButtonConfig binds them below.
static const uint8_t NUM_BUTTONS = 4;
static AceButton b0(nullptr, 0);
static AceButton b1(nullptr, 1);
static AceButton b2(nullptr, 2);
static AceButton b3(nullptr, 3);
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
  // Wait for the USB serial host (Leonardo/Micro), but give up after ~2s so the
  // adapter still boots in the car with nothing attached.
  while (!Serial && millis() < 3000);
  Serial.println(F("setup(): begin"));
  
  // Don't use internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT);

// Configure the ButtonConfig with the event handler.
  buttonConfig.setEventHandler(handleEvent);
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  // Button-feel timing. Tune here rather than editing the library.
  // longPressDelay is global - it also sets the Seek (button 2) hold time.
  buttonConfig.setLongPressDelay(500);   // default 1000; hold time for VOL +/- and Seek
  buttonConfig.setClickDelay(200);       // default 200;  max press still counted as a click
  buttonConfig.setDoubleClickDelay(400); // default 400;  track +/- disambiguation window

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

// --- Non-blocking wired-remote pulse engine --------------------------------
// A "button press" to the head unit is a resistance held on the digipot for a
// short time. Instead of blocking loop() with delay() (which freezes
// AceButton's debouncing), set the wiper now, record when to release it, and
// let releasePulse() - polled from loop() - do the release.

enum WrPlug : uint8_t { WR_TIP, WR_RING };

bool     wrPending   = false;   // a timed pulse is waiting to be released
uint32_t wrReleaseAt = 0;       // millis() deadline for the pending pulse
WrPlug   wrPlug      = WR_TIP;  // which plug the current action drives

// One framed write to the digipot: CS low, command byte, value byte, CS high.
void digipotWrite(uint8_t addr, uint8_t value) {
  digitalWrite(csPin, LOW);
  SPI.transfer(addr);
  SPI.transfer(value);
  digitalWrite(csPin, HIGH);
}

// End the current action: tip back to ground; if the ring was in use, float it.
void wrEnd() {
  digipotWrite(wiperTip, ground);
  if (wrPlug == WR_RING) digipotWrite(wiperRing, 255); // float the ring circuit
  wrPending = false;
}

// Polled from loop(): release a timed pulse once its deadline passes.
// The signed comparison stays correct across the millis() rollover.
void releasePulse() {
  if (wrPending && (int32_t) (millis() - wrReleaseAt) >= 0) wrEnd();
}

void loop() {
  // Rate-limit checkButtons() to ~5ms: analogRead() is slow and the debounce
  // time is ~20ms. (uint16_t) cast required on 32-bit cores, harmless on AVR.
  // DO NOT use delay() here.
  static uint16_t prev = millis();
  uint16_t now = millis();
  if ((uint16_t) (now - prev) >= 5) {
    prev = now;
    buttonConfig.checkButtons();
  }
  releasePulse(); // every iteration - keeps the pulse width accurate
}

// In the tip commands we only change resistance on the wiper where the tip is
// connected. The ring floats and is ignored by the head unit.
// Momentary: released by releasePulse() after delayMs.
void wrTip(int digiValue, int delayMs) {
  if (wrPending) wrEnd();               // cleanly close any in-flight pulse first
  digipotWrite(wiperTip, digiValue);
  wrPlug = WR_TIP;
  wrReleaseAt = millis() + (uint16_t) delayMs;
  wrPending = true;
}

// Engage the tip and hold until wrTipRelease() (VOL +/- hold).
void wrTipHold(int digiValue) {
  if (wrPending) wrEnd();
  digipotWrite(wiperTip, digiValue);
  wrPlug = WR_TIP;
  wrPending = false;                    // held until the button's release event
}

void wrTipRelease() {
  wrEnd();
}

// For ring functions the tip wiper carries the resistance while the ring is
// taken to ground. Momentary: released by releasePulse() after delayMs.
void wrRing(int digiValue, int delayMs) {
  if (wrPending) wrEnd();
  digipotWrite(wiperRing, ground);
  digipotWrite(wiperTip, digiValue);
  wrPlug = WR_RING;
  wrReleaseAt = millis() + (uint16_t) delayMs;
  wrPending = true;
}

// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t /* buttonState */) {

  uint8_t swButton = button->getPin();
  uint8_t swPress = eventType;

 /*
  // Print out a message for all analogue events for debug
  Serial.print(F("handleEvent(): "));
  Serial.print(F("virtualPin: "));
  Serial.print(swButton);
  Serial.print(F("; eventType: "));
  Serial.println(AceButton::eventName(swPress));
 */

// Volume Down, Press
  if ((swButton == 0) && (swPress == AceButton::kEventPressed)) {
      ///Serial.println(" VOL DOWN Click"); // for debug
      wrTip(55, 50); // 24kOhm
  }

// Volume Down, Hold
  if ((swButton == 0) && (swPress == AceButton::kEventLongPressed)) {
      //Serial.println(" VOL DOWN Hold"); // for debug
      wrTipHold(55); // 24kOhm
  }

// Volume Down, Release Hold
  if ((swButton == 0) && (swPress == AceButton::kEventLongReleased)) {
      //Serial.println(" VOL DOWN Release Hold"); // for debug
      wrTipRelease();
  }

// Volume Up, Press
// A pressed event is the most responsive for volume control
  if ((swButton == 1) && (swPress == AceButton::kEventPressed)) {
      //Serial.println(" VOL UP");
      wrTip(42, 50); // 16kOhm
  }

// Volume Up, Hold
  if ((swButton == 1) && (swPress == AceButton::kEventLongPressed)) {
      //Serial.println(" VOL UP Hold");
      wrTipHold(42); // 16kOhm
  }

// Volume Up, Release
  if ((swButton == 1) && (swPress == AceButton::kEventLongReleased)) {
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
  if ((swButton == 2) && (swPress == AceButton::kEventClicked)) {
      //Serial.println(" Next Track");
      wrTip(19, 50); // 8kOhm
  }

// Previous Track
  if ((swButton == 2) && (swPress == AceButton::kEventDoubleClicked)) {
      //Serial.println(" Previous Track");
      wrTip(27, 50); // 11,25kOhm
  }

//  Seek Series (DAB or Radio)
  if ((swButton == 2) && (swPress == AceButton::kEventLongPressed)) {
      //Serial.println(" Voice Control");
      wrTip(100, 50); // 62,75kOhm
    }

//  Source
  if ((swButton == 3) && (swPress == AceButton::kEventClicked)) {
      //Serial.println(" Source");
      wrTip(2, 50); // 1.2kOhm
  }

//  Voice Control
//  if ((swButton == 3) && (swPress == AceButton::kEventLongPressed)) {
//      Serial.println(" Voice Control");
//      wrRing(232, 1000); // 38-88kOhm hold
//    }

//  Hang-up
  if ((swButton == 3) && (swPress == AceButton::kEventDoubleClicked)) {
      Serial.println(" Hang-up");
      wrRing(14, 50); // 5kOhm
    }
}
