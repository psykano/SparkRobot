#ifndef NETWORK_PIN_HANDLER_H
#define NETWORK_PIN_HANDLER_H

#include "application.h"
#include "SparkRobotSettings.h"
#include <stdlib.h>

// Spark core pinout
const int DIGITAL_PINS = 8;
const int ANALOG_PINS = 8;
const unsigned int DIGITAL_PIN[DIGITAL_PINS] = {D0, D1, D2, D3, D4, D5, D6, D7};
const unsigned int ANALOG_PIN[ANALOG_PINS] = {A0, A1, A2, A3, A4, A5, A6, A7};

const char DIGITAL_ID = 'D';
const char ANALOG_ID = 'A';
const char HIGH_ID = 'H';
const char LOW_ID = 'L';
const char SEPARATOR = '_';

enum PinStatus {
  UNUSED_PIN,
  INPUT_PIN,
  OUTPUT_PIN
};

class NetworkPinHandler {
public:
  NetworkPinHandler();
  void setDigitalPin(unsigned int pinNumber, PinStatus status);
  void setAnalogPin(unsigned int pinNumber, PinStatus status);
  void setSendMessage(void (*_sendMessage) (const uint8_t*, size_t));
  void readMessage(const uint8_t* msg, size_t size);
  void sendDigitalRead(unsigned int pinNumber);
  void sendAnalogRead(unsigned int pinNumber);
private:
  PinStatus digitalPinStatus[DIGITAL_PINS];
  PinStatus analogPinStatus[ANALOG_PINS];
  void (*sendMessage) (const uint8_t*, size_t);
  // NOTE not expecting to handle more than 99 pins...
  char pinNumberStr[3];
  uint8_t digitalReadMsg[5];
  uint8_t analogReadMsg[8];
  char digitalReadValue;
  char analogReadValue[5];
};

#endif
