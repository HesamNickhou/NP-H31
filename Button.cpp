#include "Button.h"

Button::Button() {
  touchListener = NULL;
  pressValue = LOW;
}

Button::Button(uint8_t active, uint8_t pin) {
  Button();
  this->pressValue = active;
  this->pin = pin;
  init();
}

void Button::setParameters(uint8_t active, uint8_t pin) {
  this->pressValue = active;
  this->pin = pin;
  init();
}

void Button::init() {
  lastTime = 0;
  lastDebounceTime = 0;
  pinMode(pin, INPUT_PULLUP);
  lastReading = digitalRead(pin);
  step = 0;
}

void Button::loop() {
  uint8_t newReading = digitalRead(pin);
  if (newReading != lastReading) {
    lastDebounceTime = millis();
    changeState = true;
  }
  if (changeState)
    if (millis() - lastDebounceTime > 50) {
      changeState = false;
      lastTime = millis();
      if (touchListener != NULL)
        if (newReading == pressValue) {
          step = 1;
          touchListener(1);
        } else if (step == 1) {
          step = 0;
          touchListener(2);
        }
    }

  if (step == 1 && (millis() - lastTime >= 3000)) {
    step = 0;
    if (touchListener != NULL)
      touchListener(3);
  }
  lastReading = newReading;
}

void Button::setOnTouchListener(TouchListener listener) {
  touchListener = listener;
}
