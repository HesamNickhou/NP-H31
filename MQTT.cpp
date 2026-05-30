#include "MQTT.h"

MQTT::MQTT() {
  connectionListener = NULL;  
  startMQTT = false;
  preConnect = false;
  preState = -1000;
}

void MQTT::setCACert(char* ca) { wifiClient.setCACert(ca); }
void MQTT::init() {
  File f = SPIFFS.open("/certificate.ca", "r");
  /*if (f) {
    uint16_t i = 0;
    while (f.available())
      ca[i++] = f.read();
    ca[i] = NULL;
    f.close();
    wifiClient.setCACert(ca);
    
  } else
    Serial.println("Certificate isn't exists");*/
  
  client.setBufferSize(4096);
  client.setKeepAlive(60);
  client.setClient(wifiClient);
  
}

void MQTT::printCause(int state) {
  if (state != 0)
    Serial.print(F("MQTT is "));
  switch (state) {
    case -4: Serial.println(F("CONNECTION_TIMEOUT - the server didn't respond within the keepalive time")); break;
    case -3: Serial.println(F("CONNECTION_LOST - the network connection was broken")); break;
    case -2: Serial.println(F("CONNECT_FAILED - the network connection failed")); break;
    case -1: Serial.println(F("DISCONNECTED - the client is disconnected cleanly")); break;
    case 0: /*Serial.println(F("CONNECTED - the client is connected"));*/ break;
    case 1: Serial.println(F("CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT")); break;
    case 2: Serial.println(F("CONNECT_BAD_CLIENT_ID - the server rejected the client identifier")); break;
    case 3: Serial.println(F("CONNECT_UNAVAILABLE - the server was unable to accept the connection")); break;
    case 4: Serial.println(F("CONNECT_BAD_CREDENTIALS - the username/password were rejected")); break;
    case 5: Serial.println(F("CONNECT_UNAUTHORIZED - the client was not authorized to connect")); break;
    default: Serial.println(F("Unknown error!"));
  }
}

void MQTT::setCallBack(CallBack callBack) {
  client.setCallback(callBack);
}

void MQTT::_Connect() {
  startMQTT = true;
  if (client.connect(mac, this->gmail, this->password, willTopic, 0, false, mac, false)) {
    sprintf(topicSend, "Asaneh/%s/NP-H31/%s/App", this->gmail, this->mac);

    client.subscribe("Asaneh/Time");
    char topicReceive[82];
    sprintf(topicReceive, "Asaneh/%s/WhoIs", this->gmail);
    client.subscribe(topicReceive);

    sprintf(topicReceive, "Asaneh/%s/NP-H31/%s/Gateway/+", this->gmail, this->mac);
    client.subscribe(topicReceive);
    client.subscribe("Asaneh/Server/+");
  }
  
  else if (connectionListener && (preState != client.state())) {
    preState = client.state();
    connectionListener(client.connected());
    printCause(preState);
    char because[1024];
    wifiClient.lastError(because, 1024);
    Serial.printf("\tWifi Client Secure is %s because %s\n", wifiClient.connected() ? "Connected" : "Disconnected", because);
  }
  lastTime = millis();
}

void MQTT::Connect(const char* server, uint16_t port, const char* mac, const char* gmail, const char* password) {
  client.setServer(server, port);
  strcpy(this->mac, mac);
  strcpy(this->gmail, gmail);
  strcpy(this->password, password);
  sprintf(willTopic, "Asaneh/%s/IamGone", this->gmail);
  _Connect();
}

void MQTT::send(const char* command, const char* data) {
  char topic[1024];
  if (strcmp(command, "Iam") == 0)
    sprintf(topic, "Asaneh/%s/Iam", gmail);
  else
    sprintf(topic, "%s/%s", topicSend, command);
  client.publish(topic, data);
}

void MQTT::setOnMQTTConnection(ConnectionListener listener) {
  connectionListener = listener;
}

void MQTT::loop() {
  if (startMQTT) {
    bool connected = client.loop();  
    if (preConnect != connected) {
      preConnect = connected;
      preState = client.state();
      connectionListener(preConnect);
      printCause(preState);
    }
    
    if (!connected)
      if (millis() - lastTime >= 15000) {
        lastTime = millis();
        wifiClient.setInsecure();
        _Connect();
      }
  }
}
