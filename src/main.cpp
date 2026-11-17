#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1106G display(128, 64, &Wire, -1);
uint32_t nextFrame = 0, nextLook = 0, nextBlink = 2500, blinkStart = 0;
bool blinking = false, ready = false;
int gaze = 0, target = 0;
int oldGaze = 999, oldLeft = -1, oldRight = -1, oldMood = -1;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.beginTransmission(0x3C);
  if (Wire.endTransmission() != 0) {
    Serial.println("OLED not found at 0x3C. Check power, SDA and SCL.");
    return;
  }
  ready = display.begin(0x3C, true);
  if (!ready) Serial.println("OLED memory allocation failed.");
}

void loop() {
  if (!ready) { delay(100); return; }
  uint32_t now = millis();
  if ((int32_t)(now - nextFrame) < 0) { delay(1); return; }
  nextFrame = now + 50;
  if ((int32_t)(now - nextLook) >= 0) {
    target = random(-10, 11);
    nextLook = now + random(1000, 2400);
  }
  if (gaze < target) ++gaze;
  if (gaze > target) --gaze;
  if (!blinking && (int32_t)(now - nextBlink) >= 0) {
    blinking = true;
    blinkStart = now;
  }
  int openness = 100;
  if (blinking) {
    uint32_t elapsed = now - blinkStart;
    if (elapsed < 100) openness = 100 - elapsed;
    else if (elapsed < 200) openness = elapsed - 100;
    else {
      blinking = false;
      nextBlink = now + random(2200, 4800);
    }
  }
  int mood = (now / 6000) % 4;
  int lh = mood == 2 ? 14 : 32;
  int rh = mood == 3 ? 22 : lh;
  lh = max(2, lh * openness / 100);
  rh = max(2, rh * openness / 100);
  if (gaze == oldGaze && lh == oldLeft && rh == oldRight && mood == oldMood) return;
  oldGaze = gaze; oldLeft = lh; oldRight = rh; oldMood = mood;
  display.clearDisplay();
  int leftX = 20 + gaze, rightX = 74 + gaze;
  display.fillRoundRect(leftX, 32 - lh / 2, 34, lh, min(8, lh / 2), 1);
  display.fillRoundRect(rightX, 32 - rh / 2, 34, rh, min(8, rh / 2), 1);
  
  if (mood == 1 && openness > 50) {
    display.fillRoundRect(leftX - 2, 32, 38, 28, 12, 0);
    display.fillRoundRect(rightX - 2, 32, 38, 28, 12, 0);
  }
  display.display();
}