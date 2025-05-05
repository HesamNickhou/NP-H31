#ifndef FILE_h
#define FILE_h

#include <Arduino.h>
#include "SPIFFS.h"

class myFile {
  private:
  public:
    myFile() {}
    bool fRead(const char* fileName, char* buffer, uint16_t size) {
      uint16_t counter = 0;
      buffer[0] = '\0';
      if (!SPIFFS.exists(fileName))
        return false;
      else {
        File file = SPIFFS.open(fileName, FILE_READ);
        if (file) {
          while (file.available() && counter < size)
            buffer[counter++] = char(file.read());
          buffer[counter] = NULL;
          file.close();
          return true;
        }
        else
          return false;
      }
    }
    
    void fWrite(const char* fileName, const char* data) {
      File file = SPIFFS.open(fileName, FILE_WRITE);
      file.print(data);
      file.close();
    }
    
    void fDelete(const char* fileName) {
      SPIFFS.remove(fileName);
    }
};

#endif
