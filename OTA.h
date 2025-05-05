#include <Arduino.h>
#include <Update.h>
#include <ArduinoJson.h>

class OTA {
  private:
    typedef void (*OTAListener) (uint8_t step, uint8_t percent);
    uint16_t packetCounter;
    uint32_t packetTotal;
    OTAListener otaListener;

  public:
  
    OTA() {}

    void setOTAListener(OTAListener interface) { otaListener = interface; }

    void abort() {
      Update.abort();
    }

    void execute(const char* command, uint8_t* payload, uint16_t size) {
      if (strcmp(command, "Update.Begin") == 0) {
        JsonDocument doc;
        deserializeJson(doc, (char*) payload);
        packetTotal = doc["Size"].as<uint32_t>();
        if (Update.begin(packetTotal)) {
          Serial.println(F("OTA is started"));
          if (otaListener != NULL)
            otaListener(1, 0);
        }
        
        else {
          Serial.println(F("OTA has not enough space"));
          if (otaListener != NULL)
            otaListener(1, 1);
        }
        packetTotal = (packetTotal / 2000) + 1;
        packetCounter = 0;
      }
      
      else if (strcmp(command, "Update.Write") == 0) {
        if (Update.write(payload, size) != size) {
          Update.printError(Serial);
          if (otaListener != NULL)
            otaListener(2, 200);
        }
        
        else {
          packetCounter++;
          Serial.print(".");
          if (packetCounter % 30 == 0) {
            uint8_t percent = (uint8_t) ((packetCounter * 100) / packetTotal);
            Serial.printf("(%c%d)\n", '%', percent);
            if (otaListener != NULL)
              otaListener(2, percent);
          }
        }
      }
      
      else if (strcmp(command, "Update.End") == 0) {
        Serial.println(F("(%100)\nApplying new firmware..."));
        if (otaListener != NULL)
          otaListener(2, 100);
        if (Update.end(true)) {
          Serial.println(F("OTA is end"));
          if (Update.isFinished()) {
            Serial.println("Finished successfuly");
            if (otaListener != NULL)
              otaListener(3, 0);
          }
          
          else {
            Serial.println("Something is went wrong");
            if (otaListener != NULL)
              otaListener(3, 2);
          }
        }
        
        else {
          Update.printError(Serial);
          Serial.printf("Error on ending %s\n", String(Update.getError()));
          if (otaListener != NULL)
              otaListener(3, 1);
        }   
      }
      
      else if (strcmp(command, "Update.Failed") == 0) {
        Serial.println("Code is not valid on the server");
        if (otaListener != NULL)
          otaListener(1, 2);
      }
    }
};
