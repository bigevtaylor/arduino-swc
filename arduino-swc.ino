/* Ford BA / BF Steering Wheel Control Adapter for Pioneer Stereo
 * Version 0.1 08/11/2020
 */
 
#include <AceButton.h>
#include <SPI.h>
using namespace ace_button;

// Set the SWC input PIN
static const uint8_t BUTTON_PIN = A6;

// Set the digipot wiper
int csPin =  10;

// Set the address for the digipot
int address = 0x00;
int noButton = 255;

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
// These values are read from SWC buttons, '06 Ford BA/BF
static const uint8_t NUM_LEVELS = NUM_BUTTONS + 1;
static const uint16_t LEVELS[NUM_LEVELS] = {
  6 /* VOL- */,
  275 /* VOL+ */,
  560 /* SEEK */,
  825 /* SOURCE */,
  1023 /* 100%, 10-bit ADC, open circuit */,
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
  while (! Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));

  // Define output PIN here
  
  // Don't use internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT);

  // Configure the ButtonConfig with the event handler, enable all higher
  // level events and supress events
//  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig.setEventHandler(handleEvent);
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  // Ready digitpot, set wiper to no button
  pinMode(csPin, OUTPUT);
  SPI.begin();
  digitalWrite(csPin, LOW);
  SPI.transfer(address); // command
  SPI.transfer(255); // value
  digitalWrite(csPin, HIGH);
  Serial.println(" Ready");
}

void loop() {
  // Should be called every 4-5ms or faster, for the default debouncing time
  // of ~20ms.
buttonConfig.checkButtons();
  
}

void wrAction(int digiValue, int delayMs) {
      digitalWrite(csPin, LOW);
      SPI.transfer(address);
      SPI.transfer(digiValue);
      Serial.println(" Button Press"); // for debug
      delay(delayMs);
      SPI.transfer(address);
      SPI.transfer(noButton);
      Serial.println(" Button Release"); // for debug
      digitalWrite(csPin, HIGH);
}

// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t /* buttonState */) {

  uint8_t swButton = button->getPin();
  uint8_t swPress = (eventType);

  // Print out a message for all analogue events for debug
  Serial.print(F("handleEvent(): "));
  Serial.print(F("virtualPin: "));
  Serial.print(swButton);
  Serial.print(F("; eventType: "));
  Serial.println(swPress);

// Volume Down
  if ((swButton == 0) && (swPress == 0)) {
      Serial.println(" VOL DOWN"); // for debug
      wrAction(50, 150); // 23kOhm
      digitalWrite(csPin, HIGH);
  }

// Volume Up
  if ((swButton == 1) && (swPress == 0)) {
      Serial.println(" VOL UP");
      wrAction(35, 150); // 24kOhm
      digitalWrite(csPin, HIGH);
  }

// Next Track
  if ((swButton == 2) && (swPress == 1)) {
      Serial.println(" Next Track");
      wrAction(19, 150); // 82kOhm
      digitalWrite(csPin, HIGH);
  }

// Previous Track
  if ((swButton == 2) && (swPress == 3)) {
      Serial.println(" Previous Track");
      wrAction(25, 150); // 11kOhm
      digitalWrite(csPin, HIGH);

  }

//  Source
  if ((swButton == 3) && (swPress == 0)) {
      Serial.println(" Source");
      wrAction(2, 150); // 1.2kOhm
      digitalWrite(csPin, HIGH);

  } 

//  Power
  /*  Serial.println(" Power");
      wrAction(2, 1500); // 1.2kOhm held for 2 secs
      digitalWrite(csPin, HIGH);

  } */

}
