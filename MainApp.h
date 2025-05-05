#include <Arduino.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "myFile.h"
#include "Button.h"
#include "RF.h"
#include "CoolerButtons.h"
#include "Led.h"
#include "Beep.h"
#include "DataTransform.h"

#define VERSION_CODE 32
#define VERSION "1403.05.07"

class MainApp {
  private:
    typedef void (*ResetReason)(char* reason);
    ResetReason resetReason;
    struct Settings {
      char APSSID[32];
      char APPWD[64];
      uint8_t Mode;
      char Gmail[50];
      char Password[37];
      char MQTT_Server[50];
      char OTA_Server[50];
      uint16_t MQTT_Port;
      uint16_t OTA_Port;
    };

    struct TThermostat {
      uint8_t water, fan, speed, threshold;
    };

    struct TPeriodic {
      uint16_t startTime, endTime;
      uint8_t period1, period2, action1, action2, turn, counter;
    };

    struct TTimeLeft {
      uint8_t action;
      uint16_t time;
    };
    
    struct TSchedule {
      uint16_t startTime, endTime;
      uint8_t action, days;
    };

    struct TAutomatic {
      bool enabled;
      uint8_t scheduleCount, mode;
      TThermostat thermostat;  //Mode == 1;
      TPeriodic period;        //Mode == 2;
      TSchedule schedules[21]; //Mode == 3;
      TTimeLeft timeLeft;      //Mode == 4;
    } Automatic;

    struct TRemote {
      uint32_t waterON, waterOFF;
      uint32_t fanON, fanOFF;
      uint32_t speedON, speedOFF;
      uint32_t automaticON, automaticOFF;
    } remote;

    int64_t tlReminder = 0, autoRestart, autoRestartCounter;
    char MAC[18], IP[16];
    Led ledNetwork, ledWater, ledFan, ledSpeed, ledAutomatic, ledRF;
    Button touchWater, touchFan, touchSpeed, touchAutomatic;
    Beep beep;
    Settings setting;
    Ticker rfTicker, mainTicker;
    myFile file;
    CoolerButtons coolerButton;
    DataTransform dataTransform;

    uint8_t rf_index = 0, keepLastButtons = 0, preTemp, biasA, biasB, biasC, biasD;
    uint16_t soundVolume, brightness;
    uint8_t preWater, preFan, preSpeed, wifiReboot;
    bool pair_remote = false, parentalLock = false, internalThermo = true, revert;
    uint32_t rWater = 0, rFan = 0, rSpeed = 0, rAutomatic = 0;

    void internalRestart();
    bool process(const char* command, const char* payload, char* response);
    void writeTimer();
    void readTimer();
    void writeRemote();
    void readRemote();
    void writeButtons();
    void readButtons();
    void writeAutomatic();
    void readAutomatic();
    void writeConfig();
    void readConfig();    
    void writeSettings();
    void readSettings();

    void sendCommand(const char* command, const char* payload);
    void setAutomatic(bool status, bool send);
    void sendStatus(bool automatic);
    void sendStatus(uint8_t water, uint8_t fan, uint8_t speed, bool automatic);
    float getTemperature();
    void wifiConnection(bool isConnected, const char* ip, const char* reason);
    void MQTTConnected(bool isConnected);
    void MainTimer(uint8_t temperature, uint8_t dayOfWeek, int time);
    uint8_t getRealTemperature(uint8_t temp);
    uint8_t getRealTemperature();
    void JsonToAutomatic(JsonObject jo);
    void AutomaticToJson(JsonObject jo);
    void sortAutomatic();
  public:
    MainApp(QueueHandle_t sender, QueueHandle_t receiver);
    bool doMainTimer = false, doRFTicker = false, doRestart = false;
    void init();
    void setResetReason(ResetReason interface);
    void DataTransformListener(char* command, char* payload);
    void rfReceived(uint32_t code);
    void waterClick(uint8_t step);
    void fanClick(uint8_t step);
    void speedClick(uint8_t step);
    void automaticClick(uint8_t step);
    void coolerButtonCallback(uint8_t water, uint8_t fan, uint8_t speed);
    void AbortLearning();
    void loop();
};
