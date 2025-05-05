#include <Arduino.h>
#include "Led.h"

class CoolerButtons {
  private:
    typedef void (*CoolerInterface)(uint8_t water, uint8_t fan, uint8_t speed);
    struct TAction {
      Led *led;
      uint8_t pin;
      bool value;
    };
    TAction actions[3];
    uint8_t p_water, p_fan, p_speed;
    bool water, fan, speed;
    uint32_t lastAction = 0;
    CoolerInterface interface = NULL;
    void addAction(uint8_t pin, Led *led, bool value);
  public:
    CoolerButtons();
    void start(CoolerInterface listener);
    void setPorts(uint8_t w, uint8_t f, uint8_t s);
    void setWater(bool value, Led *led);
    void setFan(bool value, Led *led);
    void setSpeed(bool value, Led *led);
    bool getWater();    
    bool getFan();    
    bool getSpeed();
    void toggleWater(Led *led);
    void toggleFan(Led *led);
    void toggleSpeed(Led *led);
    void loop();
};
