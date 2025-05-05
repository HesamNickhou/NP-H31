#include "Led.h"
#include "Beep.h"
#include <Arduino.h>

class Button {
  private:
    typedef void (*TouchListener)(uint8_t state);
    uint8_t pin, pressValue, lastReading, step = 1;
    bool changeState;
    uint32_t lastDebounceTime, lastTime;
    
    TouchListener touchListener;
  public:
    Button();
    Button(uint8_t active, uint8_t pin);
    void init();
    void setParameters(uint8_t active, uint8_t pin);
    void setOnTouchListener(TouchListener listener);
    void loop();
};
