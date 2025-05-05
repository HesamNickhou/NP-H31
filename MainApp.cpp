#include "MainApp.h"

MainApp *toThis;
RF rf;

static void shortcut_WaterClicked(uint8_t step)                             { toThis->waterClick(step); }
static void shortcut_FanClicked(uint8_t step)                               { toThis->fanClick(step); }
static void shortcut_SpeedClicked(uint8_t step)                             { toThis->speedClick(step); }
static void shortcut_AutomaticClicked(uint8_t step)                         { toThis->automaticClick(step); }
static void shortcut_CoolerButton(uint8_t water, uint8_t fan, uint8_t speed)      { toThis->coolerButtonCallback(water, fan, speed); }
static void shortcut_rfReceived(uint32_t code)                           { toThis->rfReceived(code); }
static void shortcut_DataTransformListener(char* command, char* payload) { toThis->DataTransformListener(command, payload); }

MainApp::MainApp(QueueHandle_t send, QueueHandle_t receive) {
  toThis = this;
  dataTransform.setName("Core");
  dataTransform.setSendQueue(send);
  dataTransform.setReceiveQueue(receive);
  dataTransform.setDataListener(shortcut_DataTransformListener);

  touchWater.setParameters(LOW, 35);
  touchFan.setParameters(LOW, 39);
  touchSpeed.setParameters(LOW, 36);
  touchAutomatic.setParameters(LOW, 34);

  ledNetwork.setPin(12, 0);
  ledNetwork.on(100);

  ledWater.setPin(26, 1);
  ledFan.setPin(19, 2);
  ledSpeed.setPin(25, 3);
  ledAutomatic.setPin(21, 4);
  ledRF.setPin(14, 5);

  soundVolume = 256;
  brightness = 256;
  beep.setPin(18, 6);

  coolerButton.setPorts(27, 17, 5);
  coolerButton.start(shortcut_CoolerButton);
  preWater = coolerButton.getWater() ? 1 : 0;
  preFan   = coolerButton.getFan() ? 1 : 0;
  preSpeed = coolerButton.getSpeed() ? 1 : 0;
  if (coolerButton.getWater())
    ledWater.on();
  else
    ledWater.off();

  if (coolerButton.getFan())
    ledFan.on();
  else
    ledFan.off();

  if (coolerButton.getSpeed())
    ledSpeed.on();
  else
    ledSpeed.off();

  strcpy(setting.APSSID, "");
  strcpy(setting.APPWD, "");
  setting.Mode = 1;
  strcpy(setting.Gmail, "");
  strcpy(setting.Password, "");
  strcpy(setting.MQTT_Server, "asaniot2.com");
  setting.MQTT_Port = 8883;
  strcpy(setting.OTA_Server, "asaniot2.net");
  setting.OTA_Port = 50000;
  strcpy(IP, "192.168.4.1");

  keepLastButtons = 0;

  Automatic.enabled = false;
  Automatic.mode = 1;
  Automatic.thermostat.water = 31;
  Automatic.thermostat.fan = 30;
  Automatic.thermostat.speed = 33;
  Automatic.thermostat.threshold = 2;
  Automatic.period.period1 = 60;
  Automatic.period.period2 = 30;
  Automatic.period.action1 = B0000111;
  Automatic.period.action2 = B0000000;
  Automatic.period.startTime = 0;
  Automatic.period.endTime = 1439;
  Automatic.period.turn = 1;
  Automatic.period.counter = Automatic.period.period1;
  Automatic.scheduleCount = 0;
  Automatic.timeLeft.time = 60;
  Automatic.timeLeft.action = B00000000;

  biasA = 10;
  biasB = 2;
  biasC = 2;
  biasD = 1;
  preTemp = 0;
  wifiReboot = 15;
}

void MainApp::setResetReason(ResetReason interface) { resetReason = interface; }

void MainApp::coolerButtonCallback(uint8_t water, uint8_t fan, uint8_t speed) {
  if ((water != 2) || (fan != 2) || (speed != 2))
    sendStatus(water, fan, speed, Automatic.enabled);
  bool bWater = ((water < 2) && ((keepLastButtons >> 4) & 0x03));
  bool bFan   = ((fan   < 2) && ((keepLastButtons >> 2) & 0x03));
  bool bSpeed = ((speed < 2) && ((keepLastButtons >> 0) & 0x03));
  if (bWater || bFan || bSpeed) {
    preWater = (water < 2) ? water : preWater;
    preFan   = (fan   < 2) ? fan   : preFan;
    preSpeed = (speed < 2) ? speed : preSpeed;
    
    JsonDocument doc;
    doc["Water"] = preWater;
    doc["Fan"]   = preFan;
    doc["Speed"] = preSpeed;
    char buffer[100];
    serializeJson(doc, buffer);
    file.fWrite("/buttons.dat", buffer);
  }
}

void MainApp::waterClick(uint8_t step) {
  if (step == 1) {
    if (!parentalLock)
      ledWater.toggle();
    else
      beep.beep(100, 50, 100);
  }
  
  else if (step == 2) {
    if (!parentalLock) {
      coolerButton.toggleWater(&ledWater);
      beep.beep(100);
      setAutomatic(/*Value=*/ false, /*Send=*/ false);
    }
  }
  
  else if (step == 3) {
    if (coolerButton.getWater())
      ledWater.on();
    else
      ledWater.off();
    parentalLock = !parentalLock;
    if (parentalLock)
      beep.beep(1000);
    else
      beep.beep(200, 200, 200);
  }
}

void MainApp::fanClick(uint8_t step) {
  if (step == 1) {
    if (!parentalLock)
      ledFan.toggle();
    else
      beep.beep(100, 50, 100);
  }
  
  else if (step == 2) {
    if (!parentalLock) {
      coolerButton.toggleFan(&ledFan);
      beep.beep(100);
      setAutomatic(/*Value=*/ false, /*Send=*/ false);
    }  
  }
  
  else if (step = 3) {
    if (!parentalLock) {
      if (coolerButton.getFan())
          ledFan.on();
        else
          ledFan.off();
    }
  }
}

void MainApp::speedClick(uint8_t step) {
  if (step == 1) {
    if (!parentalLock)
      ledSpeed.toggle();
    else
      beep.beep(100, 50, 100);
  }

  else if (step == 2) {
    if (!parentalLock) {
      coolerButton.toggleSpeed(&ledSpeed);
      beep.beep(100);
      setAutomatic(/*Value=*/ false, /*Send=*/ false);
    }
  }

  else if (step == 3) {
    if (!parentalLock) {
      beep.beep(100);
      if (coolerButton.getSpeed())
        ledSpeed.on();
      else
        ledSpeed.off();

      if (remote.waterON     || remote.waterOFF ||
          remote.fanON       || remote.fanOFF   ||
          remote.speedON     || remote.speedOFF ||
          remote.automaticON || remote.automaticOFF) {
          beep.beep(1000);
          remote.waterON      = 0;
          remote.waterOFF     = 0;
          remote.fanON        = 0;
          remote.fanOFF       = 0;
          remote.speedON      = 0;
          remote.speedOFF     = 0;
          remote.automaticON  = 0;
          remote.automaticOFF = 0;
          writeRemote();
      }

      else {
        beep.beep(100);
        pair_remote = true;
        rf_index = 9;
        rfTicker.once(60, []{ toThis->doRFTicker = true; });
        ledRF.on();
      }
    } else
      beep.beep(100, 50, 100);
  }
}

void MainApp::automaticClick(uint8_t step) {
  if (step == 1) {
    if (!parentalLock)
      ledAutomatic.toggle();
    else
      beep.beep(100, 50, 100);
  } 
  
  else if (step == 2) {
    if (!parentalLock) {
      setAutomatic(!Automatic.enabled, true);
      beep.beep(100);
    }
  } 
  
  else if (step == 3) {
    if (!parentalLock) {
      beep.beep(200, 200, 200, 200, 200);
      strcpy(setting.APSSID, MAC);
      strcpy(setting.APPWD, "");
      setting.Mode = 1;
      strcpy(setting.Gmail, "");
      strcpy(setting.Password, "");
      writeSettings();
      ESP.restart();
    }
  }
}

void MainApp::setAutomatic(bool status, bool send) {
  bool changed = Automatic.enabled != status;
  Automatic.enabled = status;
    
  if (Automatic.enabled)
    if (Automatic.mode == 2) { //Periodic;
      Automatic.period.turn = 2;
      Automatic.period.counter = 1;
    }
    
    else if (Automatic.mode == 4) { //TimeLeft
      tlReminder = Automatic.timeLeft.time;
    }

  if (changed)
    writeAutomatic();
  if (Automatic.enabled)
    ledAutomatic.on();
  else
    ledAutomatic.off();
  if (send && changed)
    sendStatus(Automatic.enabled);
}

void MainApp::sendStatus(bool automatic) {
  sendStatus(2, 2, 2, automatic);
}

void MainApp::sendStatus(uint8_t water, uint8_t fan, uint8_t speed, bool automatic) {
  JsonDocument doc;
  char payload[512];
  doc["Water"]     = water;
  doc["Fan"]       = fan;
  doc["Speed"]     = speed;
  doc["Automatic"] = automatic;
  serializeJson(doc, payload);
  sendCommand("Status", payload);  
}

void getCodes(uint32_t code, uint32_t *result) {
  result[0] = 0;
  result[1] = 0;
  result[2] = 0;
  result[3] = 0;  
  uint32_t lsb = code & 0x00000f;
  uint32_t msb = code & 0xfffff0;
  if ((lsb == 1) or (lsb == 2) or (lsb == 4) or (lsb == 8)) {
    result[0] = msb | 0x000001;
    result[1] = msb | 0x000002;
    result[2] = msb | 0x000004;
    result[3] = msb | 0x000008;
  }
}

void MainApp::AbortLearning() {
  doRFTicker = false;
  beep.beep(500, 100, 500, 100, 500);
  pair_remote = false;
  sendCommand("AbortLearning", "");
}

void MainApp::rfReceived(uint32_t code) {
  rfTicker.detach();
  if (pair_remote) {
    bool error = false;
    char _status[30];

    uint32_t codes[4];
    if (rf_index == 9) {
      getCodes(code, codes);
      if (error == false and codes[0] == 0) {
        error = true;
        strcpy(_status, "Invalid Remote");
      }
    }
    
    if (error == false) {
      if (remote.waterON == code     || remote.waterOFF == code || 
          remote.fanON == code       || remote.fanOFF == code   ||
          remote.speedON == code     || remote.speedOFF == code ||
          remote.automaticON == code || remote.automaticOFF == code)
        error = true;

      if (rf_index == 9)
        for (uint8_t j=0; j<4; j++)
          if (remote.waterON == codes[j]     || remote.waterOFF == codes[j] || 
              remote.fanON == codes[j]       || remote.fanOFF == codes[j]   ||
              remote.speedON == codes[j]     || remote.speedOFF == codes[j] ||
              remote.automaticON == codes[j] || remote.automaticOFF == codes[j]) {
            error = true;
            break;
          }
      if (error)
        strcpy(_status, "Repeated Remote");
    }

    JsonDocument doc;
    char buffer[128];
    pair_remote = false;
    if (error == true) {
        doc["Status"] = _status;
        serializeJson(doc, buffer);
        sendCommand("AddRemote", buffer);
    }
    
    else {
           if (rf_index == 1) remote.waterON      = code;
      else if (rf_index == 2) remote.waterOFF     = code;
      else if (rf_index == 3) remote.fanON        = code;
      else if (rf_index == 4) remote.fanOFF       = code;
      else if (rf_index == 5) remote.speedON      = code;
      else if (rf_index == 6) remote.speedOFF     = code;
      else if (rf_index == 7) remote.automaticON  = code;
      else if (rf_index == 8) remote.automaticOFF = code;
      else if (rf_index == 9) {
        remote.waterON      = codes[0];
        remote.waterOFF     = remote.waterON;
        remote.fanON        = codes[1];
        remote.fanOFF       = remote.fanON;
        remote.speedON      = codes[2];
        remote.speedOFF     = remote.speedON;
        remote.automaticON  = codes[3];
        remote.automaticOFF = remote.automaticON;
      }
      writeRemote();
      doc["Status"] = "Successful";
      doc["Index"]  = rf_index;
      serializeJson(doc, buffer);
      sendCommand("AddRemote", buffer);      
    }
    ledRF.off();
    beep.beep(100, 50, 100);
  } 
  
  else {
    bool found = true, automatic = true;
    if (code == remote.waterON) {
      if (remote.waterOFF == remote.waterON)
        coolerButton.toggleWater(&ledWater);
      else
        coolerButton.setWater(true, &ledWater);
    }
    else if (code == remote.waterOFF)
      coolerButton.setWater(false, &ledWater);
      
    else if (code == remote.fanON) {
      if (remote.fanON == remote.fanOFF)
        coolerButton.toggleFan(&ledFan);
      else
        coolerButton.setFan(true, &ledFan);
    }
    else if (code == remote.fanOFF)
      coolerButton.setFan(false, &ledFan);
      
    else if (code == remote.speedON) {
      if (remote.speedON == remote.speedOFF)
        coolerButton.toggleSpeed(&ledSpeed);
      else
        coolerButton.setSpeed(true, &ledSpeed);
    } else if (code == remote.speedOFF)
      coolerButton.setSpeed(false, &ledSpeed);
      
    else if (code == remote.automaticON) {
      if (remote.automaticON == remote.automaticOFF)
        setAutomatic(!Automatic.enabled, true);
      else
        setAutomatic(true, true);
      automatic = false;
    }
    else if (code == remote.automaticOFF) {
      setAutomatic(false, true);
      automatic = false;
    }
    else
      found = false;
    if (found) {
      ledRF.on(100);
      if (automatic)
        setAutomatic(false, false);
    }
  }
}

void MainApp::JsonToAutomatic(JsonObject doc) {
  Automatic.enabled = doc["Enabled"].as<bool>();
  Automatic.mode    = doc["Mode"].as<uint8_t>();
  
  JsonObject jo = doc["T"].as<JsonObject>();
  Automatic.thermostat.water     = jo["W"].as<uint8_t>();
  Automatic.thermostat.fan       = jo["F"].as<uint8_t>();
  Automatic.thermostat.speed     = jo["S"].as<uint8_t>();
  Automatic.thermostat.threshold = jo["T"].as<uint8_t>();

  jo = doc["P"].as<JsonObject>();
  Automatic.period.period1   = jo["P1"].as<uint8_t>();
  Automatic.period.period2   = jo["P2"].as<uint8_t>();
  Automatic.period.action1   = jo["A1"].as<uint8_t>();
  Automatic.period.action2   = jo["A2"].as<uint8_t>();
  Automatic.period.startTime = jo["ST"].as<uint16_t>();
  Automatic.period.endTime   = jo["ET"].as<uint16_t>();
  Automatic.period.counter   = 1;
  Automatic.period.turn      = 2;

  JsonArray ja = doc["S"].as<JsonArray>();
  Automatic.scheduleCount = 0;
  for (uint8_t i=0; i<ja.size(); i++) {
    jo = ja[i].as<JsonObject>();
    Automatic.schedules[i].startTime = jo["ST"].as<uint16_t>();
    Automatic.schedules[i].endTime   = jo["ET"].as<uint16_t>();
    Automatic.schedules[i].action    = jo["A"].as<uint8_t>();
    Automatic.schedules[i].days      = jo["D"].as<uint8_t>();
    Automatic.scheduleCount++;
  }

  jo = doc["TL"].as<JsonObject>();
  Automatic.timeLeft.time   = jo["T"].as<uint16_t>();
  Automatic.timeLeft.action = jo["A"].as<uint8_t>();
}

void MainApp::AutomaticToJson(JsonObject doc) {
  doc["Enabled"] = Automatic.enabled;
  doc["Mode"]    = Automatic.mode;

  JsonObject jo = doc["T"].to<JsonObject>();
  jo["W"] = Automatic.thermostat.water;
  jo["F"] = Automatic.thermostat.fan;
  jo["S"] = Automatic.thermostat.speed;
  jo["T"] = Automatic.thermostat.threshold;

  jo = doc["P"].to<JsonObject>();
  jo["P1"] = Automatic.period.period1;
  jo["P2"] = Automatic.period.period2;
  jo["A1"] = Automatic.period.action1;
  jo["A2"] = Automatic.period.action2;
  jo["ST"] = Automatic.period.startTime;
  jo["ET"] = Automatic.period.endTime;
  jo["T"]  = Automatic.period.turn;
  jo["C"]  = Automatic.period.counter;
  
  JsonArray ja = doc["S"].to<JsonArray>();
  for (uint8_t i=0; i<Automatic.scheduleCount; i++) {
    jo = ja.add<JsonObject>();
    jo["ST"] = Automatic.schedules[i].startTime;
    jo["ET"] = Automatic.schedules[i].endTime;
    jo["A"]  = Automatic.schedules[i].action;
    jo["D"]  = Automatic.schedules[i].days;
  }

  jo = doc["TL"].to<JsonObject>();
  jo["T"] = Automatic.timeLeft.time;
  jo["A"] = Automatic.timeLeft.action;
}

void MainApp::readRemote() {
  JsonDocument doc;
  char buffer[512];
  bool successful = false;
  if (file.fRead("/remote.dat", buffer, 512)) {
    DeserializationError error = deserializeJson(doc, buffer);
    if (!error) {
      remote.waterON =      doc.containsKey("WaterON")      ? doc["WaterON"].as<uint32_t>() :      0;
      remote.waterOFF =     doc.containsKey("WaterOFF")     ? doc["WaterOFF"].as<uint32_t>() :     0;
      remote.fanON =        doc.containsKey("FanON")        ? doc["FanON"].as<uint32_t>() :        0;
      remote.fanOFF =       doc.containsKey("FanOFF")       ? doc["FanOFF"].as<uint32_t>() :       0;
      remote.speedON =      doc.containsKey("SpeedON")      ? doc["SpeedON"].as<uint32_t>() :      0;
      remote.speedOFF =     doc.containsKey("SpeedOFF")     ? doc["SpeedOFF"].as<uint32_t>() :     0;
      remote.automaticON =  doc.containsKey("AutomaticON")  ? doc["AutomaticON"].as<uint32_t>() :  0;
      remote.automaticOFF = doc.containsKey("AutomaticOFF") ? doc["AutomaticOFF"].as<uint32_t>() : 0;
      successful = true;
    }
  }
  if (!successful)
    writeRemote();
}

void MainApp::writeRemote() {
  JsonDocument doc;
  doc["WaterON"]      = remote.waterON;
  doc["WaterOFF"]     = remote.waterOFF;
  doc["FanON"]        = remote.fanON;
  doc["FanOFF"]       = remote.fanOFF;
  doc["SpeedON"]      = remote.speedON;
  doc["SpeedOFF"]     = remote.speedOFF;
  doc["AutomaticON"]  = remote.automaticON;
  doc["AutomaticOFF"] = remote.automaticOFF;
  char buffer[512];
  serializeJson(doc, buffer);
  file.fWrite("/remote.dat", buffer);
}

void MainApp::readSettings(){
  JsonDocument doc;
  char buffer[1024];
  bool successful = false;
  if (file.fRead("/settings.dat", buffer, 1024)) {
    DeserializationError err = deserializeJson(doc, buffer);
    if (!err) {
      strcpy(setting.APSSID, doc.containsKey("APSSID") ? doc["APSSID"].as<const char*>() : "");
      strcpy(setting.APPWD, doc.containsKey("APPWD") ? doc["APPWD"].as<const char*>() : "");
      setting.Mode = doc.containsKey("Mode") ? doc["Mode"].as<uint8_t>() : 1;
      strcpy(setting.Gmail, doc.containsKey("Gmail") ? doc["Gmail"].as<const char*>() : "");
      strcpy(setting.Password, doc.containsKey("Password") ? doc["Password"].as<const char*>() : "");
      strcpy(setting.MQTT_Server, doc.containsKey("MQTTServer") ? doc["MQTTServer"].as<const char*>() : "asaniot2.com");
      setting.MQTT_Port = doc.containsKey("MQTTPort") ? doc["MQTTPort"].as<uint16_t>() : 8883;
      strcpy(setting.OTA_Server, doc.containsKey("OTAServer") ? doc["OTAServer"].as<const char*>() : "asaniot2.net");
      setting.OTA_Port = doc.containsKey("OTAPort") ? doc["OTAPort"].as<uint16_t>() : 50000;
      successful = true;
    }
  }
  if (!successful)
    writeSettings();
}

void MainApp::writeSettings() {
  JsonDocument doc;
  doc["APSSID"]     = setting.APSSID;
  doc["APPWD"]      = setting.APPWD;
  doc["Mode"]       = setting.Mode;
  doc["Gmail"]      = setting.Gmail;
  doc["Password"]   = setting.Password;
  doc["MQTTServer"] = setting.MQTT_Server;
  doc["MQTTPort"]   = setting.MQTT_Port;
  doc["OTAServer"]  = setting.OTA_Server;
  doc["OTAPort"]    = setting.OTA_Port;
  
  char data[1024];
  serializeJson(doc, data);
  file.fWrite("/settings.dat", data);
}

void MainApp::writeAutomatic() {
  JsonDocument doc;
  AutomaticToJson(doc.to<JsonObject>()); 
  char buffer[1024];
  serializeJson(doc, buffer);
  file.fWrite("/automatic.dat", buffer);
}

void MainApp::readAutomatic() {
  JsonDocument doc;
  char buffer[1024];
  bool successful = false;
  if (file.fRead("/automatic.dat", buffer, 1024)) {
    DeserializationError error = deserializeJson(doc, buffer);
    if (!error) {
      JsonToAutomatic(doc.as<JsonObject>());
      successful = true;
    }
  }
  if (!successful)
    writeAutomatic();
}

void MainApp::writeConfig() {
  JsonDocument doc;
  doc["InternalThermo"]  = internalThermo;
  doc["KeepLastButtons"] = keepLastButtons;
  doc["BiasA"]           = biasA;
  doc["BiasB"]           = biasB;
  doc["BiasC"]           = biasC;
  doc["BiasD"]           = biasD;
  doc["Sound"]           = soundVolume;
  doc["Brightness"]      = brightness;
  doc["Revert"]          = revert;
  doc["WifiReboot"]      = wifiReboot;
  doc["AutoRestart"]     = autoRestart;
  char buffer[200];
  serializeJson(doc, buffer);
  file.fWrite("/config.dat", buffer);
}

void MainApp::readConfig() {
  char buffer[200];
  bool successful = false;
  if (file.fRead("/config.dat", buffer, 200)) {
    JsonDocument doc;        
    DeserializationError error = deserializeJson(doc, buffer);
    if (!error) {
      internalThermo  = doc.containsKey("InternalThermo") ? doc["InternalThermo"].as<bool>() : true;
      keepLastButtons = doc.containsKey("KeepLastButtons") ? doc["KeepLastButtons"].as<uint8_t>() : 0;
      biasA           = doc.containsKey("BiasA") ? doc["BiasA"].as<uint8_t>() : 10;
      biasB           = doc.containsKey("BiasB") ? doc["BiasB"].as<uint8_t>() : 2;
      biasC           = doc.containsKey("BiasC") ? doc["BiasC"].as<uint8_t>() : 2;
      biasD           = doc.containsKey("BiasD") ? doc["BiasD"].as<uint8_t>() : 1;
      soundVolume     = doc.containsKey("Sound") ? doc["Sound"].as<uint16_t>() : 256;
      brightness      = doc.containsKey("Brightness") ? doc["Brightness"].as<uint16_t>() : 256;
      revert          = doc.containsKey("Revert") ? doc["Revert"].as<bool>() : true;
      wifiReboot      = doc.containsKey("WifiReboot") ? doc["WifiReboot"].as<uint8_t>() : 15;
      autoRestart     = doc.containsKey("AutoRestart") ? doc["AutoRestart"].as<uint16_t>() : 7200;
      successful      = true;
    }
  }
  if (!successful)
    writeConfig();
}

void MainApp::readButtons() {
  char buffer[100];
  JsonDocument doc;
  
  if (file.fRead("/restart.dat", buffer, 100)) {
    DeserializationError error = deserializeJson(doc, buffer);
    if (!error) {
      coolerButton.setWater(doc["Water"].as<bool>() ? 1 : 0, &ledWater);
      coolerButton.setFan(  doc["Fan"].as<bool>()   ? 1 : 0, &ledFan);
      coolerButton.setSpeed(doc["Speed"].as<bool>() ? 1 : 0, &ledSpeed);
      setAutomatic(         doc["Automatic"].as<bool>(), false);
    }
    file.fDelete("/restart.dat");
  }
  
  else if (file.fRead("/buttons.dat", buffer, 100)) {    
    DeserializationError error = deserializeJson(doc, buffer);
    if (!error) {
      uint8_t water = ((keepLastButtons & B00110000) >> 4);
      uint8_t fan   = ((keepLastButtons & B00001100) >> 2);
      uint8_t speed = ((keepLastButtons & B00000011) >> 0);
      if (water < 2)
        coolerButton.setWater(water == 1, &ledWater);
      else if (water == 2)
        coolerButton.setWater(doc.containsKey("Water") ? doc["Water"].as<bool>() : false, &ledWater);

      if (fan < 2)
        coolerButton.setFan(fan == 1, &ledFan);
      else if (fan == 2)
        coolerButton.setFan(doc.containsKey("Fan") ? doc["Fan"].as<bool>() : false, &ledFan);

      if (speed < 2)
        coolerButton.setSpeed(speed == 1, &ledSpeed);
      else if (speed == 2)
        coolerButton.setSpeed(doc.containsKey("Speed") ? doc["Speed"].as<bool>() : false, &ledSpeed);
    }
  }
}

void MainApp::wifiConnection(bool isConnected, const char* ip, const char* reason) {
  if (isConnected) {
    beep.beep(100);
    Serial.printf("\n\tConnected to %s\n", setting.APSSID);
    Serial.print("\n\tIP = ");
    Serial.println(ip);
    ledNetwork.setBlinking(50, 2000);
    dataTransform.sendData("Internal.MQTT.Connect", "");
    JsonDocument doc;
    doc["Bias"] = getRealTemperature();
    char buffer[100];
    serializeJson(doc, buffer);
    dataTransform.sendData("TempTime", buffer);
  }
  
  else {
    beep.beep(50, 20, 50);
    Serial.printf("\n\tDisconnected from %s - %s\n", setting.APSSID, reason);
    ledNetwork.setBlinking(300, 300);
  }
}

void MainApp::MQTTConnected(bool isConnected) {
  if (!isConnected)
    ledNetwork.setBlinking(50, 2000);   
  else {
    ledNetwork.on();
    Serial.println("\n\tMQTT is connected");
  }
}

uint8_t MainApp::getRealTemperature() {
  uint8_t temp = 0;
  if (internalThermo) {
    temp += biasA;
    temp += coolerButton.getWater() ? biasB : 0;
    temp += coolerButton.getFan() ? biasC : 0;
    temp += coolerButton.getSpeed() ? biasD : 0;
  }
  return temp;
}

void MainApp::sortAutomatic() {
  for (uint8_t i=0; i<Automatic.scheduleCount - 1; i++)
    for (uint8_t j=i; j<Automatic.scheduleCount; j++)
      if (Automatic.schedules[i].startTime > Automatic.schedules[j].startTime) {
        uint16_t start = Automatic.schedules[i].startTime;
        uint16_t end   = Automatic.schedules[i].endTime;
        uint8_t action = Automatic.schedules[i].action;
        uint8_t days   = Automatic.schedules[i].days;

        Automatic.schedules[i].startTime = Automatic.schedules[j].startTime;
        Automatic.schedules[i].endTime   = Automatic.schedules[j].endTime;
        Automatic.schedules[i].action    = Automatic.schedules[j].action;
        Automatic.schedules[i].days      = Automatic.schedules[j].days;

        Automatic.schedules[j].startTime = start;
        Automatic.schedules[j].endTime   = end;
        Automatic.schedules[j].action    = action;
        Automatic.schedules[j].days      = days;
      }
}

uint8_t MainApp::getRealTemperature(uint8_t temp) {
  temp -= getRealTemperature();
  return temp;
}

void MainApp::internalRestart() {
  JsonDocument doc;
  char buffer[100];
  doc["Water"]     = coolerButton.getWater();
  doc["Fan"]       = coolerButton.getFan();
  doc["Speed"]     = coolerButton.getSpeed();
  doc["Automatic"] = Automatic.enabled;
  serializeJson(doc, buffer);
  file.fWrite("/restart.dat", buffer);
  ESP.restart();
}

void MainApp::MainTimer(uint8_t temp, uint8_t dayOfWeek, int time) {
  temp = getRealTemperature(temp);
  if (temp != preTemp) {
    preTemp = temp;
    char buffer[100];
    sprintf(buffer, "{\"Bias\":%d}", getRealTemperature());
    dataTransform.sendData("TempTime", buffer);
  }

  if (autoRestartCounter > 0) {
    autoRestartCounter--;
    if (autoRestartCounter == 0)
      internalRestart();
  }

  if (Automatic.enabled) {
    if (Automatic.mode == 1) {        // Thermostat
      if (temp >= (Automatic.thermostat.water))                                       coolerButton.setWater(true, &ledWater);
      else if (temp <= (Automatic.thermostat.water - Automatic.thermostat.threshold)) coolerButton.setWater(false, &ledWater);
      if (temp >= (Automatic.thermostat.fan))                                         coolerButton.setFan(true, &ledFan);
      else if (temp <= (Automatic.thermostat.fan - Automatic.thermostat.threshold))   coolerButton.setFan(false, &ledFan);
      if (temp >= (Automatic.thermostat.speed))                                       coolerButton.setSpeed(true, &ledSpeed);
      else if (temp <= (Automatic.thermostat.speed - Automatic.thermostat.threshold)) coolerButton.setSpeed(false, &ledSpeed);
    } 

    else if (Automatic.mode == 2 && time>0) { // Period
      uint16_t st = Automatic.period.startTime;
      uint16_t et = Automatic.period.endTime;
      bool allow = (st <= et) ? ((st <= time) && (time < et)) : ((st <= time) || (time < et));
      
      if (allow) {
        Automatic.period.counter--;
        if (Automatic.period.counter == 0) {
          uint8_t w, f, s;
          if (Automatic.period.turn == 1) {
            Automatic.period.turn = 2;
            Automatic.period.counter = Automatic.period.period2;
            w = (Automatic.period.action2 & B00110000) >> 4;
            f = (Automatic.period.action2 & B00001100) >> 2;
            s = (Automatic.period.action2 & B00000011);
          }
          
          else if (Automatic.period.turn == 2) {
            Automatic.period.turn = 1;
            Automatic.period.counter = Automatic.period.period1;
            w = (Automatic.period.action1 & B00110000) >> 4;
            f = (Automatic.period.action1 & B00001100) >> 2;
            s = (Automatic.period.action1 & B00000011);
          }
          if (w < 2) coolerButton.setWater(w == 1, &ledWater);
          if (f < 2) coolerButton.setFan(f == 1, &ledFan);
          if (s < 2) coolerButton.setSpeed(s == 1, &ledSpeed);
        }
      }
    } 

    else if (Automatic.mode == 3 && time>0) { // Schedule
      uint8_t action = 0;
      for (uint8_t i=0; i<Automatic.scheduleCount; i++) {
        if (bitRead(Automatic.schedules[i].days, 7 - dayOfWeek) == 1) {
          uint16_t st = Automatic.schedules[i].startTime;
          uint16_t et = Automatic.schedules[i].endTime;
          bool allow = (st <= et) ? ((st <= time) && (time < et)) : ((st <= time) || (time < et));
            
          if (allow) {
            action = Automatic.schedules[i].action;
            break;
          }
        }
      }
      uint8_t w = ((action & B00110000) >> 4);
      uint8_t f = ((action & B00001100) >> 2);
      uint8_t s = ((action & B00000011) >> 0);
      if (w < 2) coolerButton.setWater(w == 1, &ledWater);
      if (f < 2) coolerButton.setFan(f == 1, &ledFan);
      if (s < 2) coolerButton.setSpeed(s == 1, &ledSpeed);
    }

    else if (Automatic.mode == 4) { // TimeLeft
      tlReminder--;
      if (tlReminder == 0) {
        uint8_t w = (Automatic.timeLeft.action & B00110000) >> 4;
        uint8_t f = (Automatic.timeLeft.action & B00001100) >> 2;
        uint8_t s = (Automatic.timeLeft.action & B00000011);
        if (w < 2) coolerButton.setWater(w == 1, &ledWater);
        if (f < 2) coolerButton.setFan(f == 1, &ledFan);
        if (s < 2) coolerButton.setSpeed(s == 1, &ledSpeed);
      }
    }
  }
}

void MainApp::DataTransformListener(char* command, char* payload) {
  if (strcmp(command, "Internal.WIFI.MAC") == 0) {
    strcpy(MAC, payload);
    JsonDocument doc;
    doc["APSSID"]     = setting.APSSID;
    doc["APPWD"]      = setting.APPWD;
    doc["Gmail"]      = setting.Gmail;
    doc["Password"]   = setting.Password;
    doc["Mode"]       = setting.Mode;
    doc["MQTTServer"] = setting.MQTT_Server;
    doc["MQTTPort"]   = setting.MQTT_Port;
    doc["OTAServer"]  = setting.OTA_Server;
    doc["OTAPort"]    = setting.OTA_Port;
    doc["WifiReboot"] = wifiReboot;
    char buffer[1024];
    serializeJson(doc, buffer);
    dataTransform.sendData("Internal.Setting", buffer);    
    Serial.printf("\n\t%s\n", MAC);
  }
  
  else if (strcmp(command, "Internal.WIFI.Connection") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload);
    wifiConnection(doc["Status"].as<bool>(), doc["IP"].as<const char*>(), doc["Reason"].as<const char*>());
  }

  else if (strcmp(command, "Internal.WIFI.Restart") == 0) 
    internalRestart();
  
  else if (strcmp(command, "Internal.MQTT.Connection") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload);
    MQTTConnected(doc["Connected"].as<bool>());
  }
  
  else if (strcmp(command, "Internal.MainTimer") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload);
    MainTimer(doc["Temperature"].as<uint8_t>(), doc["DayOfWeek"].as<uint8_t>(), 
      doc.containsKey("Time") ? doc["Time"].as<int>() : -1);
  }
  
  else {
    char response[4000];
    beep.beep(50);
    Serial.printf(">> %s = %s\n", command, payload);
    if (process(command, payload, response)) {
      if (strcmp(command, "WhoIs") == 0)
        strcpy(command, "Iam");
      dataTransform.sendData(command, response);
    }
  }
}

void MainApp::sendCommand(const char* command, const char* payload) {
  dataTransform.sendData(command, payload);
}

void checkVersion() {
  File f = SPIFFS.open("/version.dat", "r");
  uint8_t A = 0, B = 0, C = 0, D = 0;
  if (f) {    
    if (f.available()) A = f.read();
    if (f.available()) B = f.read();
    if (f.available()) C = f.read();
    if (f.available()) D = f.read();
    uint32_t crCode = (A << 24) + (B << 16) + (C << 8) + D;
    if (crCode < VERSION_CODE) {
      Serial.println("\n\t***New firmware from OTA server***");
      A = (uint8_t)(VERSION_CODE >> 24);
      B = (uint8_t)(VERSION_CODE >> 16);
      C = (uint8_t)(VERSION_CODE >> 8);
      D = (uint8_t) VERSION_CODE;
      f = SPIFFS.open("/version.dat", "w");
      f.write(A);
      f.write(B);
      f.write(C);
      f.write(D);
    }
  } else {
    A = (uint8_t)(VERSION_CODE >> 24);
    B = (uint8_t)(VERSION_CODE >> 16);
    C = (uint8_t)(VERSION_CODE >> 8);
    D = (uint8_t) VERSION_CODE;
    f = SPIFFS.open("/version.dat", "w");
    f.write(A);
    f.write(B);
    f.write(C);
    f.write(D);
  }
}

void MainApp::init() {
  checkVersion();
  Serial.printf("\n\tVersion: %s(%d)\n", VERSION, VERSION_CODE);

  readSettings();
  readConfig();
  readAutomatic();  
  readRemote();
  readButtons();

  autoRestartCounter = autoRestart;
  touchWater.setOnTouchListener(shortcut_WaterClicked);
  touchFan.setOnTouchListener(shortcut_FanClicked);
  touchSpeed.setOnTouchListener(shortcut_SpeedClicked);
  touchAutomatic.setOnTouchListener(shortcut_AutomaticClicked);
  if (Automatic.enabled)
    ledAutomatic.on();
  else
    ledAutomatic.off();
  rf.setReceiver(4, shortcut_rfReceived);

  ledWater.setBrightness(brightness);
  ledFan.setBrightness(brightness);
  ledSpeed.setBrightness(brightness);
  ledAutomatic.setBrightness(brightness);
  ledNetwork.setBrightness(brightness);
  ledRF.setBrightness(brightness);

  ledWater.setRevert(revert);
  ledFan.setRevert(revert);
  ledSpeed.setRevert(revert);
  ledAutomatic.setRevert(revert);
  
  beep.setVolume(soundVolume);
  beep.beep(500);
}

void MainApp::loop() {
  touchWater.loop();
  touchFan.loop();
  touchSpeed.loop();
  touchAutomatic.loop();
  ledNetwork.loop();
  ledWater.loop();
  ledFan.loop();
  ledSpeed.loop();
  ledAutomatic.loop();
  ledRF.loop();
  beep.loop();

  coolerButton.loop();
  rf.loop();
  dataTransform.loop();
    
  if (doRFTicker)
    AbortLearning();
}

bool MainApp::process(const char* command, const char* payload, char* response) {
  bool result = true;
  strcpy(response, payload);
  JsonDocument doc;
  if (strcmp(command, "SetWater") == 0) {
    deserializeJson(doc, payload);
    coolerButton.setWater(doc["Value"].as<bool>(), &ledWater);
  } 
  
  else if (strcmp(command, "SetFan") == 0) {
    deserializeJson(doc, payload);
    coolerButton.setFan(doc["Value"].as<bool>(), &ledFan);
  } 
  
  else if (strcmp(command, "SetSpeed") == 0) {
    deserializeJson(doc, payload);
    coolerButton.setSpeed(doc["Value"].as<bool>(), &ledSpeed);
  } 
  
  else if (strcmp(command, "SetStatus") == 0) {
    deserializeJson(doc, payload);
    coolerButton.setWater(doc["Water"].as<bool>(), &ledWater);
    coolerButton.setFan(doc["Fan"].as<bool>(), &ledFan);
    coolerButton.setSpeed(doc["Speed"].as<bool>(), &ledSpeed);
    strcpy(response, "Ok");
  } 
  
  else if (strcmp(command, "GetStatus") == 0) {
    doc["Water"]     = coolerButton.getWater() ? 1 : 0;
    doc["Fan"]       = coolerButton.getFan()   ? 1 : 0;
    doc["Speed"]     = coolerButton.getSpeed() ? 1 : 0;
    doc["Automatic"] = Automatic.enabled;
    serializeJson(doc, response, 512);
  } 
  
  else if (strcmp(command, "Automatic.Enable") == 0) {
    deserializeJson(doc, payload);
    setAutomatic(doc["Enable"].as<bool>(), true);
  } 
  
  else if (strcmp(command, "Automatic.Mode") == 0) {
    deserializeJson(doc, payload);
    Automatic.mode = doc["Mode"].as<uint8_t>();
    writeAutomatic();  
  } 
  
  else if (strcmp(command, "Automatic.Set") == 0) {
    deserializeJson(doc, payload);
    JsonToAutomatic(doc.as<JsonObject>());
    file.fWrite("/automatic.dat", payload);
  } 
  
  else if (strcmp(command, "Automatic.Get") == 0) {
    AutomaticToJson(doc.to<JsonObject>());
    serializeJson(doc, response, 4000);
  } 
  
  else if (strcmp(command, "Automatic.Thermostat.Set") == 0) {
    deserializeJson(doc, payload);
    Automatic.thermostat.water =     doc["W"].as<uint8_t>();
    Automatic.thermostat.fan =       doc["F"].as<uint8_t>();
    Automatic.thermostat.speed =     doc["S"].as<uint8_t>();
    Automatic.thermostat.threshold = doc["T"].as<uint8_t>();
    writeAutomatic();    
  } 
  
  else if (strcmp(command, "Automatic.Period.Set") == 0) {
    deserializeJson(doc, payload);
    Automatic.period.period1 =   doc["P1"].as<uint8_t>();
    Automatic.period.period2 =   doc["P2"].as<uint8_t>();
    Automatic.period.action1 =   doc["A1"].as<uint8_t>();
    Automatic.period.action2 =   doc["A2"].as<uint8_t>();
    Automatic.period.startTime = doc["ST"].as<uint16_t>();
    Automatic.period.endTime =   doc["ET"].as<uint16_t>();
    Automatic.period.counter =   Automatic.period.period1;
    writeAutomatic();
  } 
  
  else if (strcmp(command, "Automatic.Schedule.Add") == 0) {
    deserializeJson(doc, payload);
    uint8_t count = Automatic.scheduleCount;
    if (count < 21) {
      Automatic.schedules[count].startTime = doc["ST"].as<uint16_t>();
      Automatic.schedules[count].endTime   = doc["ET"].as<uint16_t>();
      Automatic.schedules[count].action    = doc["A"].as<uint8_t>();
      Automatic.schedules[count].days      = doc["D"].as<uint8_t>();
      Automatic.scheduleCount++;
    }
    
    else {
      doc.clear();
      doc["Error"] = "Maximum 21 events are allowed";
      serializeJson(doc, response, 1024);
    }
    sortAutomatic();
    writeAutomatic();    
  } 
  
  else if (strcmp(command, "Automatic.Schedule.Edit") == 0) {
    deserializeJson(doc, payload);
    uint8_t index = doc["Index"].as<uint8_t>();
    Automatic.schedules[index].startTime = doc["ST"].as<uint16_t>();
    Automatic.schedules[index].endTime   = doc["ET"].as<uint16_t>();
    Automatic.schedules[index].action    = doc["A"].as<uint8_t>();
    Automatic.schedules[index].days      = doc["D"].as<uint8_t>();
    sortAutomatic();
    writeAutomatic();
  } 
  
  else if (strcmp(command, "Automatic.Schedule.Remove") == 0) {
    deserializeJson(doc, payload);
    uint8_t index = doc["Index"].as<uint8_t>();
    for (uint8_t i=index; i<Automatic.scheduleCount - 1; i++)
      Automatic.schedules[i] = Automatic.schedules[i + 1];
    Automatic.scheduleCount--;
    writeAutomatic();
  } 
  
  else if (strcmp(command, "Automatic.TimeLeft.Set") == 0) {
    deserializeJson(doc, payload);
    Automatic.timeLeft.time = doc["T"].as<long>();
    Automatic.timeLeft.action = doc["A"].as<uint8_t>();
    tlReminder = Automatic.timeLeft.time;
    writeAutomatic();
  } 
  
  else if (strcmp(command, "LastState.Set") == 0) {
    deserializeJson(doc, payload);
    keepLastButtons = doc["State"].as<uint8_t>();
    writeConfig();    
  } 
  
  else if (strcmp(command, "LastState.Get") == 0) {  
    doc["State"] = keepLastButtons;
    serializeJson(doc, response, 100);
  } 
  
  else if (strcmp(command, "Thermometer.Set") == 0) {
    deserializeJson(doc, payload);
    internalThermo = doc["Internal"].as<bool>();
    biasA = doc["BiasA"].as<uint8_t>();
    biasB = doc["BiasB"].as<uint8_t>();
    biasC = doc["BiasC"].as<uint8_t>();
    biasD = doc["BiasD"].as<uint8_t>();
    writeConfig();    
  } 
  
  else if (strcmp(command, "Thermometer.Get") == 0) {
    doc["Internal"] = internalThermo;
    doc["BiasA"] = biasA;
    doc["BiasB"] = biasB;
    doc["BiasC"] = biasC;
    doc["BiasD"] = biasD;
    serializeJson(doc, response, 500);   
  } 
  
  else if (strcmp(command, "GetRemote") == 0) {
    doc["WaterON"]      = remote.waterON      != 0;
    doc["WaterOFF"]     = remote.waterOFF     != 0;
    doc["FanON"]        = remote.fanON        != 0;
    doc["FanOFF"]       = remote.fanOFF       != 0;
    doc["SpeedON"]      = remote.speedON      != 0;
    doc["SpeedOFF"]     = remote.speedOFF     != 0;
    doc["AutomaticON"]  = remote.automaticON  != 0;
    doc["AutomaticOFF"] = remote.automaticOFF != 0;
    char buffer[512];
    serializeJson(doc, response, 512);
  } 
  
  else if (strcmp(command, "PairRemote") == 0) {
    deserializeJson(doc, payload);
    uint8_t button = doc["Button"].as<uint8_t>();
    bool which = doc["Which"].as<bool>();
    if (doc["Type"].as<uint8_t>() == 1) {//====================Pair===========================
      pair_remote = true;
      rf_index = ((button - 1) * 2) + (which ? 1 : 2);
      rfTicker.once(60, []{ toThis->doRFTicker = true; });
      ledRF.on();
    }
    
    else if (doc["Type"].as<uint8_t>() == 2) {//============Same as=========================
      rfTicker.detach();
      if (button == 1) {
        if (which) remote.waterON  = remote.waterOFF;
        else       remote.waterOFF = remote.waterON;
      }
      else if (button == 2) {
        if (which) remote.fanON  = remote.fanOFF;
        else       remote.fanOFF = remote.fanON;
      }
      else if (button == 3) {
        if (which) remote.speedON  = remote.speedOFF;
        else       remote.speedOFF = remote.speedON;
      }
      else if (button == 4) {
        if (which) remote.automaticON  = remote.automaticOFF;
        else       remote.automaticOFF = remote.automaticON;
      }
      writeRemote();
    }
    
    else if (doc["Type"].as<uint8_t>() == 3) {//==========Remove============================
      rfTicker.detach();
      if (button == 1) {
        if (which) remote.waterON = 0; else remote.waterOFF = 0;
      }
      else if (button == 2) {
        if (which) remote.fanON = 0; else remote.fanOFF = 0;
      }
      else if (button == 3) {
        if (which) remote.speedON = 0; else remote.speedOFF = 0;
      }
      else if (button == 4) {
        if (which) remote.automaticON = 0; else remote.automaticOFF = 0;
      }
      else if (button == 5) {
        remote.waterON      = 0;
        remote.waterOFF     = 0;
        remote.fanON        = 0;
        remote.fanOFF       = 0;
        remote.speedON      = 0;
        remote.speedOFF     = 0;
        remote.automaticON  = 0;
        remote.automaticOFF = 0;
      }
      writeRemote();
    }
  } 
  
  else if (strcmp(command, "SetSettings") == 0) {
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      if (doc.containsKey("APSSID"))
        strcpy(setting.APSSID, doc["APSSID"].as<const char*>());
      if (doc.containsKey("APPWD"))
        strcpy(setting.APPWD, doc["APPWD"].as<const char*>());
      if (doc.containsKey("Gmail"))
        strcpy(setting.Gmail, doc["Gmail"].as<const char*>());
      if (doc.containsKey("Password"))
        strcpy(setting.Password, doc["Password"].as<const char*>());
      if (doc.containsKey("Mode"))
        setting.Mode = doc["Mode"].as<uint8_t>();
      if (doc.containsKey("MQTTServer"))
        strcpy(setting.MQTT_Server, doc["MQTTServer"].as<const char*>());
      if (doc.containsKey("MQTTPort"))
        setting.MQTT_Port = doc["MQTTPort"].as<uint16_t>();
      if (doc.containsKey("OTAServer"))
        strcpy(setting.OTA_Server, doc["OTAServer"].as<const char*>());
      if (doc.containsKey("OTAPort"))
        setting.OTA_Port = doc["OTAPort"].as<uint16_t>();
      writeSettings();
    }
    rfTicker.once(3, []{ ESP.restart(); });
  } 
  
  else if (strcmp(command, "GetInfo") == 0) {
    JsonDocument doc;
    doc["APSSID"]      = setting.APSSID;
    doc["APPWD"]       = setting.APPWD;
    doc["Mode"]        = setting.Mode;
    doc["Gmail"]       = setting.Gmail;
    doc["Password"]    = setting.Password;
    doc["MAC"]         = MAC;
    doc["Volume"]      = soundVolume;
    doc["Revert"]      = revert;
    doc["Brightness"]  = brightness;
    doc["VersionCode"] = VERSION_CODE;
    doc["Version"]     = VERSION;
    doc["WifiRestart"] = wifiReboot;
    doc["AutoRestart"] = autoRestart;
    char reason[100];
    resetReason(reason);
    doc["ResetReason"] = reason;
    serializeJson(doc, response, 2048);
  } 
  
  else if (strcmp(command, "Temperature") == 0) {
    doc["Bias"] = getRealTemperature();
    serializeJson(doc, response, 100);
    dataTransform.sendData("Temperature", response);
    result = false;
  } 
  
  else if (strcmp(command, "SetVolume") == 0) {
    deserializeJson(doc, payload);
    soundVolume = doc["Value"].as<uint16_t>();
    writeConfig();
    beep.setVolume(soundVolume);
  } 
  
  else if (strcmp(command, "SetBrightness") == 0) {
    deserializeJson(doc, payload);
    brightness = doc["Value"].as<uint16_t>();
    writeConfig();
    ledWater.setBrightness(brightness);
    ledFan.setBrightness(brightness);
    ledSpeed.setBrightness(brightness);
    ledAutomatic.setBrightness(brightness);
    ledNetwork.setBrightness(brightness);
    ledRF.setBrightness(brightness);
  } 
  
  else if (strcmp(command, "SetRevert") == 0) {
    deserializeJson(doc, payload);
    revert = doc["Value"].as<bool>();
    ledWater.setRevert(revert);
    ledFan.setRevert(revert);
    ledSpeed.setRevert(revert);
    ledAutomatic.setRevert(revert);
    writeConfig();
  } 
  
  else if (strcmp(command, "Time") == 0) {
    dataTransform.sendData("Internal.Time.Set", payload);
    result = false;
  } 
  
  else if (strcmp(command, "GetServerTime") == 0) {
    dataTransform.sendData("Internal.Time.Set", payload);
    result = false;
  } 

  else if (strcmp(command, "ResetFactory") == 0) {
    SPIFFS.format();
    ESP.restart();
  }

  else if (strcmp(command, "Restart") == 0) {
    deserializeJson(doc, payload);
    wifiReboot  = doc["Wifi"].as<uint8_t>();
    autoRestart = doc["Auto"].as<uint16_t>();
    autoRestartCounter = autoRestart;
    writeConfig();
    dataTransform.sendData("Internal.WIFI.Reboot", response);
  }
  
  else if (strcmp(command, "WhoIs") == 0) {
    doc["MAC"] = MAC;
    doc["Type"] = "NP-H31";
    serializeJson(doc, response, 200);    
  } 
  
  else if (strcmp(command, "TempTime") == 0) {
    doc["Bias"] = getRealTemperature();
    serializeJson(doc, response, 100);
    dataTransform.sendData("TempTime", response);
    result = false;
  } 
  
  else {
    Serial.printf("Unknown command: %s\n", command);
    strcpy(response, "Uknown command");
  }
  return result;
}
