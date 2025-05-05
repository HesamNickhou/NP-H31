#include "Led.h"

Led::Led() {}

Led::Led(uint8_t pin, uint8_t index) {
  debug = false;
  ledcAttachPin(pin, index);
  ledcSetup(pin, 12000, 8);
  this->index = index;
  init();
}

void Led::setPin(uint8_t pin, uint8_t index, bool debug_) {
  debug = debug_;
  ledcAttachPin(pin, index);
  ledcSetup(index, 12000, 8);
  this->index = index;
  init();
}

void Led::init() {
  step = 0;
  onTime = 0;
  offTime = 0;
  lastTime = 0;
  after = 0;
  brightness = 256;
  revert = false;
  setState(brightness);
}

void Led::setRevert(bool value) {
  this->revert = value;
  if (revert) {
    state = 1 - state;
    setState(state);
  }
}

void Led::setBrightness(uint16_t value) {
  brightness = value;
  setState(state);
}

void Led::toggle() {
  setState(1 - state);
}

void Led::setState(uint8_t value) {
  state = value;
  if (debug)
    Serial.printf("LED: state = %s\n", state == HIGH ? "HIGH" : "LOW");
  ledcWrite(index, state == HIGH ? brightness : 0);
}

void Led::on() {
  step = 0;
  setState(revert ? LOW : HIGH);
}

void Led::on(uint16_t _after) {
  on();
  off(_after);  
}

void Led::off() {
  step = 0;
  setState(revert ? HIGH : LOW);
}

void Led::off(uint16_t after) {
  step = 0;
  lastTime = millis();
  this->after = after;
}

void Led::_on() {
  setState(revert ? LOW : HIGH);
}

void Led::_off() {
  setState(revert ? HIGH : LOW);
}

void Led::saveState() {
  save_On = onTime;
  save_Off = offTime;
  save_Step = step;
  save_Value = state;
}

void Led::restoreState() {
  if (save_Step == 0)
    setState(save_Value);
  else
    setBlinking(save_On, save_Off);
}

void Led::setBlinking(uint16_t time1, uint16_t time2) {
  this->onTime = time1;
  this->offTime = time2;
  if (onTime > 0) {
    _on();
    lastTime = millis();
    step = 1;
  } else {
    step = 0;
    off();
  }
}

void Led::loop() {
  if (step == 0) {
    if (after > 0)
      if ((millis() - lastTime) >= after) {
        off();
        after = 0;
      }
  } else if (step == 1) {
    if (millis() - lastTime >= onTime) {
      step = 2;
      _off();
      lastTime = millis();
    }
  } else if (step == 2) {
    if (millis() - lastTime >= offTime) {
      step = 1;
      _on();
      lastTime = millis();
    }
  }
}
