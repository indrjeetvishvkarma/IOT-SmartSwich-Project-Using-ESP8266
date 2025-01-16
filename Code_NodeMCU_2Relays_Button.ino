#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
//this code by INDRJEET 

// WiFi Credentials
const char* ssid = "WIFI name";       // Replace with your WiFi SSID
const char* password = "password"; // Replace with your WiFi password

// Sinric Pro Credentials
const char* appKey = "Aap key";    // Replace with your Sinric Pro App Key
const char* secretKey = "SecretKey"; // Replace with your Sinric Pro Secret Key
const char* switch1_ID = "Swich1ID"; // Replace with Switch 1 ID
const char* switch2_ID = "Swich2ID"; // Replace with Switch 2 ID
 
// Relay and Switch Pins
#define RelayPin1 12  // GPIO D6
#define RelayPin2 4  // GPIO D2
#define SwitchPin1 13  // GPIO D7
#define SwitchPin2 0   // GPIO D3

// Variables to track relay states
bool relayState1 = false;
bool relayState2 = false;

// Debounce variables
const int debounceDelay = 200;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
bool previousSwitchState1 = HIGH;
bool previousSwitchState2 = HIGH;

// Functions to control relays
void setRelayState(int relayPin, bool state) {
  digitalWrite(relayPin, state ? LOW : HIGH);
}

// Sinric Pro callbacks
bool onPowerState1(const String &deviceId, bool &state) {
  relayState1 = state;
  setRelayState(RelayPin1, state);
  Serial.printf("Switch 1 %s\n", state ? "ON" : "OFF");
  return true; // Report success
}

bool onPowerState2(const String &deviceId, bool &state) {
  relayState2 = state;
  setRelayState(RelayPin2, state);
  Serial.printf("Switch 2 %s\n", state ? "ON" : "OFF");
  return true; // Report success
}

// Manual control for switches
void handleManualControl() {
  unsigned long currentMillis = millis();

  // Handle Switch 1
  if (digitalRead(SwitchPin1) == LOW && previousSwitchState1 == HIGH) {
    if (currentMillis - lastDebounceTime1 >= debounceDelay) {
      relayState1 = !relayState1;
      setRelayState(RelayPin1, relayState1);
      SinricProSwitch& mySwitch1 = SinricPro[switch1_ID];
      mySwitch1.sendPowerStateEvent(relayState1); // Sync state with Sinric Pro
      lastDebounceTime1 = currentMillis;
    }
  }
  previousSwitchState1 = digitalRead(SwitchPin1);

  // Handle Switch 2
  if (digitalRead(SwitchPin2) == LOW && previousSwitchState2 == HIGH) {
    if (currentMillis - lastDebounceTime2 >= debounceDelay) {
      relayState2 = !relayState2;
      setRelayState(RelayPin2, relayState2);
      SinricProSwitch& mySwitch2 = SinricPro[switch2_ID];
      mySwitch2.sendPowerStateEvent(relayState2); // Sync state with Sinric Pro
      lastDebounceTime2 = currentMillis;
    }
  }
  previousSwitchState2 = digitalRead(SwitchPin2);
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("Setting up...");

  // Setup relay pins
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);

  // Setup switch pins
  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Setup Sinric Pro
  SinricProSwitch& mySwitch1 = SinricPro[switch1_ID];
  mySwitch1.onPowerState(onPowerState1);

  SinricProSwitch& mySwitch2 = SinricPro[switch2_ID];
  mySwitch2.onPowerState(onPowerState2);

  SinricPro.begin(appKey, secretKey);
}

void loop() {
  SinricPro.handle();
  handleManualControl();
}
