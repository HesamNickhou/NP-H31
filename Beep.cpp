#include "Beep.h"

Beep::Beep() {
  repeat = false;
  step = 0;
}

Beep::Beep(uint8_t pin, uint8_t idx) {
  this->index = idx;
  ledcAttachPin(pin, index);
  ledcSetup(index, 1500, 8);
  volume = 256;
  repeat = false;
  step = 0;
}

void Beep::setVolume(uint16_t value) {
  volume = value;
}

void Beep::setPin(uint8_t pin, uint8_t idx) {
  this->index = idx;
  ledcAttachPin(pin, index);
  ledcSetup(index, 1500, 8);
}

void Beep::beep(uint16_t on) {
  beep(on, 0, 0, 0, 0);
}

void Beep::beep(uint16_t on1, uint16_t off1, uint16_t on2) {
  beep(on1, off1, on2, 0, 0);
}

void Beep::beep(uint16_t on1, uint16_t off1, uint16_t on2, uint16_t off2, uint16_t on3) {
  repeat = false;
  this->on1 = on1;
  this->off1 = off1;
  this->on2 = on2;
  this->off2 = off2;
  this->on3 = on3;
  step = 1;
  lastTime = millis();
}

void Beep::alarm(uint16_t on, uint16_t off) {
  ron = on;
  roff = off;
  this->on1 = on;
  this->off1 = off;
  this->on2 = 0;
  this->off2 = 0;
  this->on3 = 0;
  repeat = true;
  step = 1;
  lastTime = millis();
}

void Beep::loop() {
  if (step == 1) {
    ledcWrite(index, volume);
    if (millis() - lastTime >= on1) {
      on1 = repeat ? ron : 0;
      step = 2;
      lastTime = millis();
    }
  }
  
  else if (step == 2) {
    ledcWrite(index, volume);
    if (millis() - lastTime >= off1){
      off1 = repeat ? roff : 0;
      step = repeat ? 1 : 3;
      lastTime = millis();
    }
  }
  
  else if (step == 3) {
    ledcWrite(index, volume);
    if (millis() - lastTime >= on2) {
      on2 = 0;
      step = 4;
      lastTime = millis();
    }
  }
  
  else if (step == 4) {
    ledcWrite(index, 0);
    if (millis() - lastTime >= off2) {
      off2 = 0;
      step = 5;
      lastTime = millis();
    }
  }
  
  else if (step == 5) {
    ledcWrite(index, volume);
    if (millis() - lastTime >= on3) {
      ledcWrite(index, 0);
      on3 = 0;
      step = 0;
      lastTime = millis();
    }
  }
}
