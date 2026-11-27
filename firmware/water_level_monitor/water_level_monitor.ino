/*
  ESP32 Water Level Monitor
  Author: Sharanakumar Gadag

  POWER (HW-688 buck converter, 12V -> 5V)
    Buck 5V   pin -> ESP32 VIN
    Buck GND  pin -> ESP32 GND
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Buck converter pins (power only, no GPIO needed)
// 5V  -> ESP32 VIN
// GND -> ESP32 GND

// Sensor pins (from schematic)
const uint8_t SENSOR_PINS[4]  = {25, 26, 27, 32};   // J2, J5, J3, J6
const char*   SENSOR_NAMES[4] = {"S1 (J2)", "S2 (J5)", "S3 (J3)", "S4 (J6)"};

// 1.3" I2C OLED (SH1106), SDA=21, SCL=22, address 0x3C
Adafruit_SH1106G display(128, 64, &Wire, -1);

const unsigned long DEBOUNCE_MS = 50;
bool          state[4]      = {false, false, false, false};  // true = water detected
bool          lastRaw[4]    = {false, false, false, false};
unsigned long lastChange[4] = {0, 0, 0, 0};

void drawDisplay(int level) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(0, 0);
  display.print("Water Level Monitor");

  for (int i = 0; i < 4; i++) {
    display.setCursor(0, 14 + i * 10);
    display.print(SENSOR_NAMES[i]);
    display.print(": ");
    display.print(state[i] ? "WET" : "DRY");
  }

  display.setCursor(90, 56);
  display.print("L:");
  display.print(level);
  display.display();
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) pinMode(SENSOR_PINS[i], INPUT_PULLUP);

  Wire.begin(21, 22);
  if (!display.begin(0x3C, true)) {
    Serial.println("OLED not found");
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  unsigned long now = millis();

  for (int i = 0; i < 4; i++) {
    bool raw = (digitalRead(SENSOR_PINS[i]) == LOW);   // LOW = water detected
    if (raw != lastRaw[i]) {
      lastRaw[i] = raw;
      lastChange[i] = now;
    }
    if ((now - lastChange[i]) > DEBOUNCE_MS) state[i] = raw;
  }

  int level = 0;
  for (int i = 0; i < 4; i++) if (state[i]) level++;

  Serial.printf("S1:%d S2:%d S3:%d S4:%d  -> Level %d/4\n",
                state[0], state[1], state[2], state[3], level);
  drawDisplay(level);
  delay(200);
}