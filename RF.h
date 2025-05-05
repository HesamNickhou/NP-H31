// #define RF315
#ifndef _RF
#define _RF
#include <Arduino.h>

static struct RFd {
  uint8_t 
    bitCount,
    count = 0, 
    pin;
  uint16_t 
    high, 
    low;
  uint32_t 
    receiving = 0, 
    last      = 0, 
    t         = 0, 
    code      = 0;
  bool
    reset     = false, 
    available = false;
} rf433, rf315;

static void IRAM_ATTR receiver433() {
  rf433.t = micros() - rf433.last;
  rf433.reset = false;
  if (rf433.t > 100) {
    if (digitalRead(rf433.pin) == LOW)
      rf433.high = rf433.t;
    else if (rf433.high > 0) {
      rf433.low = rf433.t;
      rf433.receiving <<= 1;
      rf433.count++;
      if (rf433.high > rf433.low)
        rf433.receiving |= 1;
      else if (rf433.low > (rf433.high * 4))
        rf433.reset = true;
      if (rf433.count == 24) {
        rf433.code      = rf433.receiving;
        rf433.available = true;
        rf433.reset     = true;
      }
    }
    else
      rf433.reset = true;
  }
  else
    rf433.reset = true;

  if (rf433.reset) {
    rf433.count     = 0;
    rf433.receiving = 0;
    rf433.high      = 0;
    rf433.low       = 0;
  }
  rf433.last = micros();
}

#ifdef RF315
static void IRAM_ATTR receiver315() {
  rf315.t = micros() - rf315.last;
  rf315.reset = false;
  if (rf315.t > 100) {
    if (digitalRead(rf315.pin) == LOW)
      rf315.high = rf315.t;
    else if (rf315.high > 0) {
      rf315.low = rf315.t;
      rf315.receiving <<= 1;
      rf315.count++;
      if (rf315.high > rf315.low)
        rf315.receiving |= 1;
      else if (rf315.low > (rf315.high * 4))
        rf315.reset = true;
      if (rf315.count == 24) {
        rf315.code      = rf315.receiving;
        rf315.available = true;
        rf315.reset     = true;
      }
    }
    else
      rf315.reset = true;
  }
  else
    rf315.reset = true;

  if (rf315.reset) {
    rf315.count     = 0;
    rf315.receiving = 0;
    rf315.high      = 0;
    rf315.low       = 0;
  }
  rf315.last = micros();
}
#endif

class RF {
  private:
    typedef void (*Callback433)(uint32_t data);
    Callback433 callback433;
    uint8_t trn433, repeat433;
    uint32_t preCode433, preTime433;

    struct TLastCode {
      uint32_t code, time;
    } lastCodes[5];
    bool getLastTime(uint32_t code) {
      uint8_t i=0;
      while (i < 5) {
        if ((millis() - lastCodes[i].time) >= 1000) {
          lastCodes[i].code = 0;
          lastCodes[i].time = 0;
        }
        else if (lastCodes[i].code == code) {
          bool b = (millis() - lastCodes[i].time) >= 1000;
          lastCodes[i].time = millis();
          return b;
        }
        i++;
      }
      
      i = 0;
      while (i < 5) {
        if (lastCodes[i].code == 0) {
          lastCodes[i].code = code;
          lastCodes[i].time = millis();
          return true;
        }
        i++;
      }
      return false;
    }

#ifdef RF315
    typedef void (*Callback315)(uint32_t data);
    Callback315 callback315;
    uint8_t trn315, repeat315;
    uint32_t preCode315, preTime315;
#endif

  public:
    RF() {
      callback433  = NULL;
      preTime433   = 0;
      preCode433   = 0;
      trn433       = 100;

#ifdef RF315
      callback315  = NULL;
      preTime315   = 0;
      preCode315   = 0;
      trn315       = 100;
#endif
    }
    
    void setTransmitter(uint8_t pin, uint8_t nRepeat) {
      trn433 = pin;
      pinMode(trn433, OUTPUT);
      digitalWrite(trn433, LOW);
      repeat433 = nRepeat;
    }

    void setReceiver(uint8_t pin, Callback433 listener) {
      callback433 = listener;
      rf433.pin   = pin;
      attachInterrupt(rf433.pin, receiver433, CHANGE);
    }
    
    void transmit(uint32_t code) { 
      if (trn433 >= 100)
        return;
      digitalWrite(trn433, LOW);
      for (uint8_t r=0; r<repeat433; r++) {
        for (uint8_t i=24; i>0; i--) {
          if (code & (1L << (i-1))) {
            digitalWrite(trn433, HIGH);
            delayMicroseconds(1050);
            digitalWrite(trn433, LOW);
            delayMicroseconds(350);
          }
          else {
            digitalWrite(trn433, HIGH);
            delayMicroseconds(350);
            digitalWrite(trn433, LOW);
            delayMicroseconds(1050);
          }
        }
        digitalWrite(trn433, HIGH);
        delayMicroseconds(350);
        digitalWrite(trn433, LOW);
        delayMicroseconds(10850);
      }
      digitalWrite(trn433, LOW);
    }
    
#ifdef RF315
    void setReceiver315(uint8_t pin, Callback315 listener) {
      callback315 = listener;
      rf315.pin   = pin;
      attachInterrupt(rf315.pin, receiver315, CHANGE);
    }

    void setTransmitter315(uint8_t pin, uint8_t nRepeat) {
      trn315 = pin;
      pinMode(trn315, OUTPUT);
      digitalWrite(trn315, LOW);
      repeat315 = nRepeat;
    }

    void transmit315(uint32_t code) { 
      if (trn315 >= 100)
        return;
      digitalWrite(trn315, LOW);
      for (uint8_t r=0; r<repeat315; r++) {
        for (uint8_t i=24; i>0; i--) {
          if (code & (1L << (i-1))) {
            digitalWrite(trn315, HIGH);
            delayMicroseconds(1050);
            digitalWrite(trn315, LOW);
            delayMicroseconds(350);
          }
          else {
            digitalWrite(trn315, HIGH);
            delayMicroseconds(350);
            digitalWrite(trn315, LOW);
            delayMicroseconds(1050);
          }
        }
        digitalWrite(trn315, HIGH);
        delayMicroseconds(350);
        digitalWrite(trn315, LOW);
        delayMicroseconds(10850);
      }
      digitalWrite(trn315, LOW);
    }
#endif

    void loop() {
      if (rf433.available) {
        /*if (rf433.code != preCode433) {
          preCode433 = rf433.code;
          if (callback433 != NULL)
            callback433(rf433.code);
        }
        preTime433 = millis();*/
        preCode433 = rf433.code;
        if ((getLastTime(preCode433)) && (callback433 != NULL))
          callback433(preCode433);
        rf433.available = false;
      }

      /*if (preTime433 > 0)
        if ((millis() - preTime433) > 1000) {
          preCode433 = 0;
          preTime433 = 0;
        }*/

#ifdef RF315
      if (rf315.available) {
        if (rf315.code != preCode315) {
          preCode315 = rf315.code;
          if (callback315 != NULL)
            callback315(rf315.code);
        }
        preTime315 = millis();
        rf315.available = false;
      }

      if (preTime315 > 0)
        if ((millis() - preTime315) > 1000) {
          preCode315 = 0;
          preTime315 = 0;
        }
#endif
    }
};
#endif