#include "DataTransform.h"

DataTransform::DataTransform() {
  step     = 1;
  counter  = 0;
  element  = NULL;
  sender   = NULL;
  receiver = NULL;
  debug    = false;
}

void DataTransform::setReceiveQueue(QueueHandle_t queue)    { this->receiver = queue; }
void DataTransform::setSendQueue(QueueHandle_t queue)       { this->sender   = queue; }
void DataTransform::setDataListener(DataReceived interface) { this->listener = interface; }
void DataTransform::setName(const char* taskName)           { strcpy(name, taskName); }

void DataTransform::sendData(const char* cmd, const char* pld) {
  if (debug)
    Serial.printf("%s >> %s=%s\n", name, cmd, pld);
  char ch;
  for (uint16_t i=0; i<strlen(cmd); i++) {
    ch = cmd[i];
    xQueueSend(sender, &ch, portMAX_DELAY);
  }
  ch = NULL;
  xQueueSend(sender, &ch, portMAX_DELAY);

  for (uint16_t i=0; i<strlen(pld); i++) {
    ch = pld[i];
    xQueueSend(sender, &ch, portMAX_DELAY);
  }
  ch = NULL;
  xQueueSend(sender, &ch, portMAX_DELAY);
}

void DataTransform::sendData(const char* cmd, const char* pld, uint16_t size) {
  if (debug)
    Serial.printf("%s >> %s=%s\n", name, cmd, pld);
  char ch;
  for (uint16_t i=0; i<strlen(cmd); i++) {
    ch = cmd[i];
    xQueueSend(sender, &ch, portMAX_DELAY);
  }
  ch = NULL;
  xQueueSend(sender, &ch, portMAX_DELAY);

  for (uint16_t i=0; i<size; i++) {
    ch = pld[i];
    xQueueSend(sender, &ch, portMAX_DELAY);
  }
  ch = NULL;
  xQueueSend(sender, &ch, portMAX_DELAY);
}

void DataTransform::loop() {
  if (xQueueReceive(receiver, &element, 0)) {
    if (step == 1)
      command[counter++] = element;
    else if (step == 2)
      payload[counter++] = element;
    if (element == NULL) {
      if (step == 2 && listener != NULL) {
        if (debug)
          Serial.printf("%s << %s=%s\n", name, command, payload);
        listener(command, payload);
      }
      step = 3 - step;
      counter = 0;
    }
  }
}
