#ifndef LED_h
#define LED_h
#include <Arduino.h>

class Led {
  private:
    void _on();
    void _off();
    bool revert, debug;
    uint8_t index, state, step, save_Step, save_Value;
    uint16_t onTime, offTime, save_On, save_Off, after, brightness;
    uint32_t lastTime;
  public:
    Led();
    Led(uint8_t pin, uint8_t index);
//    void setPin(uint8_t pin, uint8_t index);
    void setPin(uint8_t pin, uint8_t index, bool debug_ = false);
    void init();
    void toggle();
    void saveState();
    void restoreState();
    void setState(uint8_t value);
    void setBlinking(uint16_t onTime, uint16_t offTime);
    void setRevert(bool value);
    void setBrightness(uint16_t value);
    void loop();
    void on();
    void on(uint16_t after);
    void off();
    void off(uint16_t after);
};
#endif
