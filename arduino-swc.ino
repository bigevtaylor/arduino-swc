#include <AceButton.h>
#include <SPI.h>
using namespace ace_button;

// Set the SWC input PIN
static const uint8_t BUTTON_PIN = A6;

// Set the digipot wiper
int csPin =  4;

// Set the address for the digipot
byte address = 0x00;
byte noButton = 255;

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
  SPI.transfer(noButton); // value
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
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(eventType);

// Volume Down
  if ((swButton = 0) && (swPress = AceButton::kEventClicked)) {
      /* digitalWrite(csPin, LOW);
      SPI.transfer(address);
      SPI.transfer(50); // 23kOhm
      Serial.println(" VOL DOWN"); // for debug
      delay(150); // Hold wiper resistance value for this time
      SPI.transfer(noButton); // Depress button to similate analogue
      Serial.println(" No Button"); // for debug
      digitalWrite(csPin, HIGH); */
      Serial.println(" VOL DOWN"); // for debug
      wrAction(50, 150);
     digitalWrite(csPin, HIGH);
  }

// Volume Up
  if ((swButton = 1) && (swPress = AceButton::kEventClicked)) {
      digitalWrite(csPin, LOW);
      SPI.transfer(address);
      SPI.transfer(35); // 16kOhm
      Serial.println(" VOL UP");
      delay(150);
      SPI.transfer(noButton);
      Serial.println(" No Button");
      digitalWrite(csPin, HIGH);
  }

// Next Track
  if ((swButton = 2) && (swPress = AceButton::kEventClicked)) {
      digitalWrite(csPin, LOW);
      SPI.transfer(address);
      SPI.transfer(19); // 8kOhm
      delay(150);
      SPI.transfer(noButton);
      digitalWrite(csPin, HIGH);
  }

// Previous Track
  if ((swButton = 2) && (swPress = AceButton::kEventDoubleClicked)) {
      digitalWrite(csPin, LOW);
      SPI.transfer(address);
      SPI.transfer(25); // 11kOhm
      delay(150);
      SPI.transfer(noButton);
      digitalWrite(csPin, HIGH);
  }

//  Source
  if ((swButton = 3) && (swPress = AceButton::kEventClicked)) {
      digitalWrite(csPin, LOW);
      SPI.transfer(address);
      SPI.transfer(2); // 1.2kOhm
      delay(150);
      SPI.transfer(noButton);
      digitalWrite(csPin, HIGH);
  }

//  Power
  if ((swButton = 3) && (swPress = AceButton::kEventLongPressed)) {
      digitalWrite(csPin, LOW);
      SPI.transfer(address);
      SPI.transfer(2); // 1.2kOhm, hold ~2secs
      delay(1500);
      SPI.transfer(noButton);
      digitalWrite(csPin, HIGH);
  }

}
