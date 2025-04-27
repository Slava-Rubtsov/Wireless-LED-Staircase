#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#define PIR_PIN 25
#define BT_DEVICE_NAME "LEDController"  // Bluetooth name of the LED controller

unsigned long lastCheck = 0;
const unsigned long checkInterval = 5000; // check every 5 seconds

void connectToServer() {
  Serial.println("Attempting to connect to LEDController...");

  if (SerialBT.connect(BT_DEVICE_NAME)) {
    Serial.println("Connected to LEDController!");
  } else {
    Serial.println("Failed to connect. Retrying in 5 seconds...");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  SerialBT.begin("PIRSensor", true); // true = client mode

  connectToServer();
}

void loop() {
  // Check connection every 5 seconds
  if (millis() - lastCheck > checkInterval) {
    lastCheck = millis();
    if (!SerialBT.connected()) {
      Serial.println("Disconnected. Reconnecting...");
      SerialBT.disconnect();  // Clean disconnect if needed
      connectToServer();
    }
  }

  // PIR sensor reading and sending
  if (SerialBT.connected() && digitalRead(PIR_PIN) == HIGH) {
    SerialBT.write('B'); // or 'A' depending on which PIR
    Serial.println("Sent 'A' to LEDController");
    delay(1000); // debounce and avoid spamming
  }
}
