#ifndef BEEP_h
#define BEEP_h
#include <Arduino.h>

class Beep {
  private:
    uint8_t index, step;
    uint16_t on1, off1, on2, off2, on3, off3, ron, roff, volume;
    bool repeat;
    uint32_t lastTime;
  public:
    Beep();
    Beep(uint8_t pin, uint8_t index);
    void setPin(uint8_t pin, uint8_t index);
    void setVolume(uint16_t value);
    void beep(uint16_t on);
    void beep(uint16_t on1, uint16_t off, uint16_t on2);
    void beep(uint16_t on1, uint16_t off1, uint16_t on2, uint16_t off2, uint16_t on3);
    void alarm(uint16_t on, uint16_t off);
    void loop();
};
#endif
