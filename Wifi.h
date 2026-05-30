#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

class Wifi {
  private:
    typedef void (*ConnectionListener) (bool isConnected, const char* reason);
    typedef void (*DataListener) (char* command, uint8_t* payload, uint16_t size);
    typedef void (*ConnectedDevice) (uint8_t num);
    typedef void (*RSSIListener) (uint8_t quality);
    typedef void (*CAListener) (uint8_t state, uint8_t data);
    typedef void (*RestartListener) ();

    const char* reasons[9] PROGMEM = {
        "No Shield",
        "Idle",
        "No SSID available",
        "Scan is completed",
        "Connected",
        "Connection Failed",
        "Connection is lost",
        "Disconnected",
        "Unknown status"
      };
    unsigned long lastTime, rssiTime;
    uint16_t i, count, len;
    uint8_t payload[2000], step, status, mode, connectedNo, cb, wifiReboot, caState;
    char command[50];
    bool initialized, inReading, doStop;
    WiFiServer server;
    WiFiClient client;
    IPAddress remote;

    ConnectedDevice connectedDevice;
    ConnectionListener connectionListener;
    DataListener dataListener;
    RSSIListener rssiListener;
    RestartListener restartListener;
    CAListener         caListener;

    const char* getCause(uint8_t status) {
      switch (status) {
        case WL_NO_SHIELD:       return reasons[0]; break;
        case WL_IDLE_STATUS:     return reasons[1]; break;
        case WL_NO_SSID_AVAIL:   return reasons[2]; break;
        case WL_SCAN_COMPLETED:  return reasons[3]; break;
        case WL_CONNECTED:       return reasons[4]; break;
        case WL_CONNECT_FAILED:  return reasons[5]; break;
        case WL_CONNECTION_LOST: return reasons[6]; break;
        case WL_DISCONNECTED:    return reasons[7]; break;
        default:                 return reasons[8];
      }
    }

    bool readCommand() {
      bool result = false;
      if (step == 1) {
        if (cb == 10) {
          command[len] = NULL;
          step = 2;
          i = 0;
          caState = (strcmp(command, "Certificate") == 0);
          result = true;
        }
        else
          command[len++] = cb;
      }
      return result;
    }

    bool readLength() {
      if (step == 2) {
        if (cb == 10) {
          payload[i] = NULL;
          step = 3;
          len = atoi((char*)payload);
          if ((len == 0) && (i > 0)) {
            inReading = false;
            if (dataListener != NULL)
              dataListener(command, payload, i);
            step = 1;
            len  = 0;
          }
          i = 0;
          return true;
        }

        else
          payload[i++] = cb;
      }
      return (step == 2);
    }

    bool isCA() {
      if (caState > 0) { //Certificate is incoming...
        if (caState == 1) { //Create file
          caState = 2;
          i++;
          caListener(1, cb);
        }

        else if (caState == 2) { //Write byte to file
          caListener(2, cb);
          if (++i >= len) {
            caState = 0;
            caListener(3, 0);
            i    = 0;
            len  = 0;
            step = 1;
          }
        }
        return true;
      }
      return false;
    }

    bool readPayload() {
      bool result = false;
      if (step == 3) {
        if (len > 0) {
          payload[i++] = cb;
          if (i == len) {
            inReading = false;
            if (dataListener != NULL) 
              dataListener(command, payload, len);
            step   = 1;
            i      = 0;
            len    = 0;
            result = true;
          }
        }
        
        else {
          inReading = false;
          if (dataListener != NULL)
            dataListener(command, payload, len);
          step   = 1;
          len    = 0;
          i      = 0;
          result = true;
        }
      }
      return result;
    }
  
  public:
  
    Wifi():server(5000) {
      step        = 1;
      len         = 0;
      i           = 0;
      connectedNo = 0;
      mode        = 1;
      initialized = false;
      inReading   = false;
      doStop      = false;
      rssiTime    = 0;
      wifiReboot  = 15;
      lastTime    = 1;
      caState     = 0;
      remote      = IPAddress(0,0,0,0);
    }

    void getMAC(char* mac) {
      uint8_t MAC[6];
      WiFi.macAddress(MAC);
      sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", 
        MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
    }

    void init(const char* ssid, const char* password, uint8_t _mode) {
      mode = _mode;
      char name[14];
      uint8_t MAC[6];
      WiFi.macAddress(MAC);
      sprintf(name, "ASANEH_%02X%02X%02X", MAC[3], MAC[4], MAC[5]);
      if (mode == 1)
        WiFi.softAP(name);
      else {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
      }
      ArduinoOTA.setHostname(name);
      ArduinoOTA.setPassword("f8r8zc0m");
      ArduinoOTA.begin();
      server.begin();      
      initialized = true;
      len = 0;
    }

    bool mdns(const char* type) {
      bool success = false;
      uint8_t MAC[6];
      char mac[18];
      WiFi.macAddress(MAC);
      sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
      if (MDNS.begin(mac))
        if (MDNS.addService("asaneh_offline", "tcp", 5000)) {
          MDNS.addServiceTxt("asaneh_offline", "tcp", "type", type);
          success = true;
        }
      return success;
    }

    void startServer() {
      server.begin();
      initialized = true;
    }

    void send(const char* command, const char* payload, const char* server, uint16_t port) {
      if (!client.connect(server, port))
        Serial.printf("Can't connect to %s:%d\n", server, 50000);
      else {
        ArduinoOTA.end();
        client.printf("%s#%s", command, payload);
      }
    }

    void send(const char* command, const char* payload) {
      if (initialized && (remote != IPAddress(0,0,0,0))) {
        if (!client.connected()) {
          if (!client.connect(remote, 5000)) {
            Serial.println("Can't send data over TCP");
            remote = IPAddress(0,0,0,0);
            return;
          }
        }

        client.println(command);
        client.println(payload);
        doStop = true;
      }
        
    }

    void stop() { client.stop(); }

    void setWifiReboot(uint8_t value) { wifiReboot = value; }
    void setOnWifiConnection(ConnectionListener listener) { connectionListener = listener; }    
    void setOnDataReceived(DataListener listener) { dataListener = listener; }    
    void setOnConnectedDevice(ConnectedDevice interface) { connectedDevice = interface; }
    void setOnRSSIListener(RSSIListener interface) { rssiListener = interface; }
    void setOnRestartListener(RestartListener interface) { restartListener = interface; }
    void setCAListener(CAListener interface)              { caListener         = interface; }
  
    void loop() {
      if (mode > 1)
        if (WiFi.status() != status) {
          status = WiFi.status();
          if (status == WL_CONNECTED) {
            connectionListener(true, getCause(status));
            lastTime = 0;
          }
          
          else if (status != WL_NO_SHIELD) {
            connectionListener(false, getCause(status));
            if (lastTime == 0)
              lastTime = millis();
          }
        }

      if (lastTime > 10)
        if (wifiReboot > 0)
          if ((millis() - lastTime) > (wifiReboot * 60000)) {
            restartListener();
            lastTime = 0;
          }

      if (WiFi.status() == WL_CONNECTED)
        if (millis() - rssiTime >= 1000) {
          if (rssiListener != NULL) {
            if (WiFi.RSSI() < -75)
              rssiListener(0);
            else if (WiFi.RSSI() < -40)
              rssiListener(1);
            else
              rssiListener(2);
          }
          rssiTime = millis();
        }

      if (mode == 1)
        if (WiFi.softAPgetStationNum() != connectedNo) {
          connectedNo = WiFi.softAPgetStationNum();
          if (connectedDevice != NULL)
            connectedDevice(connectedNo);
        }
        
      ArduinoOTA.handle();
      
      if (client.connected()) {
        remote = client.remoteIP();
        while (client.available()) {
          inReading = true;
          cb = client.read();
          if (readCommand()) continue;
          if (readLength())  continue;
          if (isCA())        continue;
          if (readPayload()) break;
        }

        if (!inReading && doStop) {
          doStop = false;
          client.stop();
        }
      }
      
      else
        client = server.available();
    }
};
