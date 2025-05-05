#include "CoolerButtons.h"

CoolerButtons::CoolerButtons() {
  for (uint8_t i=0; i<3; i++) {
    actions[i].pin = 100;
    actions[i].value = false;
  }
}

void CoolerButtons::setPorts(uint8_t w, uint8_t f, uint8_t s) {
  p_water = w;
  p_fan = f;
  p_speed = s;
}

void CoolerButtons::start(CoolerInterface listener) {
  interface = listener;
  pinMode(p_water, OUTPUT);
  digitalWrite(p_water, LOW);
  water = false;

  pinMode(p_fan, OUTPUT);
  digitalWrite(p_fan, LOW);
  fan = false;
  
  pinMode(p_speed, OUTPUT);
  digitalWrite(p_speed, LOW);
  speed = false;
}

void CoolerButtons::addAction(uint8_t pin, Led *led, bool value) {
  bool found = false;
  for (uint8_t i=0; i<3; i++)
    if (actions[i].pin == pin) {
      actions[i].led = led;
      actions[i].value = value;
      found = true;
      break;
    }

  if (!found) 
    for (uint8_t i=0; i<3; i++)
      if (actions[i].pin == 100) {
        actions[i].pin = pin;
        actions[i].value = value;
        actions[i].led = led;
        break;
      }
}

void CoolerButtons::setWater(bool value, Led *led) {
  addAction(p_water, led, value);
}

void CoolerButtons::setFan(bool value, Led *led) {
  addAction(p_fan, led, value);
}

void CoolerButtons::setSpeed(bool value, Led *led) {
  addAction(p_speed, led, value);
}

bool CoolerButtons::getWater() {
  return water;
}

bool CoolerButtons::getFan() {
  return fan;
}

bool CoolerButtons::getSpeed() {
  return speed;
}

void CoolerButtons::toggleWater(Led *led) {
  setWater(!water, led);
}

void CoolerButtons::toggleFan(Led *led) {
  setFan(!fan, led);
}

void CoolerButtons::toggleSpeed(Led *led) {
  setSpeed(!speed, led);
}

void CoolerButtons::loop() {
  if (millis() - lastAction >= 1000) {
    uint8_t a = 2, bb = 2, c = 2;
    for (uint8_t i=0; i<3; i++)
      if (actions[i].pin < 100) {
        digitalWrite(actions[i].pin, actions[i].value ? HIGH : LOW);
        if (actions[i].led != NULL) {
          if (actions[i].value)
            actions[i].led->on();
          else
            actions[i].led->off();
        }

        bool b = false;
        if (actions[i].pin == p_water) {
          b = (water != actions[i].value);
          water = actions[i].value;
          a = water ? 1 : 0;
        } else if (actions[i].pin == p_fan) {
          b = (fan != actions[i].value);
          fan = actions[i].value;
          bb = fan ? 1 : 0;
        } else if (actions[i].pin == p_speed) {
          b = (speed != actions[i].value);
          speed = actions[i].value;
          c = speed ? 1 : 0;
        }
        if ((interface != NULL) && (b))
          interface(a, bb, c);
        lastAction = millis();
        actions[i].pin = 100;
        break;
      }
  }
}
