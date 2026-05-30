#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Wifi.h"
#include "MQTT.h"
#include "DataTransform.h"
#include "OTA.h"
#include "files.h"
#include "esp_task_wdt.h"

class ConnectionApp {
  private:
    typedef void (*ResetReason)(char* reason);
    ResetReason resetReason;
    char MAC[18], APSSID[32], APPWD[64], Gmail[50], Password[37], MQTT_Server[50], OTA_Server[50];
    uint16_t MQTT_Port, OTA_Port, currentTime, sunrise;
    uint8_t Mode;
    MQTT mqtt;
    DataTransform dataTransform;
    Ticker mainTicker;
    OTA ota;
    OneWire oneWire;
    DallasTemperature temperature;
    File fCA;
    bool mqttConnected = false, firstTime = true, otaStarted, firstCertificate = true, server_error = false;
    void sendCommand(const char* command, const char* payload);
    void setWDT(bool value);
  public:
    Wifi wifi;
    ConnectionApp(QueueHandle_t sender, QueueHandle_t receiver);
    bool doMainTimer = false;
    void init(char* ca);
    void MainTimer();
    void setResetReason(ResetReason interface);
    void wifiConnection(bool isConnected, const char* reason);
    void wifiRSSI(uint8_t quality);
    void WifiDataReceived(char* command, uint8_t* payload, uint16_t size);
    void MQTTConnected(bool isConnected);
    void MQTTReceived(char* topic, uint8_t* payload, uint16_t length);    
    void DataTransformListener(char* command, char* payload);
    void OTAListener(uint8_t step, uint8_t percent);
    void CAListener(uint8_t state, uint8_t data);
    void loop();
};
