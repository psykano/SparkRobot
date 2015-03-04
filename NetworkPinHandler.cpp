#include "NetworkPinHandler.h"

NetworkPinHandler::NetworkPinHandler() {
  for (unsigned int i = 0; i < DIGITAL_PINS; ++i) {
    digitalPinStatus[i] = UNUSED_PIN;
  }
  for (unsigned int i = 0; i < ANALOG_PINS; ++i) {
    analogPinStatus[i] = UNUSED_PIN;
  }
}

void NetworkPinHandler::setDigitalPin(unsigned int pinNumber, PinStatus status) {
  if (pinNumber < DIGITAL_PINS) {
    digitalPinStatus[pinNumber] = status;
    switch (status) {
      case INPUT_PIN:
        pinMode(DIGITAL_PIN[pinNumber], INPUT);
        break;
      case OUTPUT_PIN:
        pinMode(DIGITAL_PIN[pinNumber], OUTPUT);
        break;
    }
  }
}

void NetworkPinHandler::setAnalogPin(unsigned int pinNumber, PinStatus status) {
  if (pinNumber < ANALOG_PINS) {
    analogPinStatus[pinNumber] = status;
    switch (status) {
      case INPUT_PIN:
        pinMode(ANALOG_PIN[pinNumber], INPUT);
        break;
      case OUTPUT_PIN:
        pinMode(ANALOG_PIN[pinNumber], OUTPUT);
        break;
    }
  }
}

void NetworkPinHandler::readMessage(const uint8_t* msg, size_t size) {
  if ((msg[0] == 'D' || msg[0] == 'A') && size > 3 && size <= 7) {
    uint8_t pinDigits = 0;
    uint8_t outputDigits = 0;

    for (unsigned int i = 2; i < size; ++i) {
      if ('_' == msg[i]) {
        pinDigits = i - 1;
        outputDigits = size - i - 1;
        break;
      }
    }

    if (0 == pinDigits || pinDigits == size-1) {
      return;
    }

    unsigned int pinNumIdx = 1;
    unsigned int pinNum = 0;
    for (unsigned int i = pinNumIdx; i < pinDigits + pinNumIdx; ++i) {
      pinNum = (10 * pinNum) + (msg[i] - '0');
    }

    if ((msg[0] == 'D' && pinNum < DIGITAL_PINS && OUTPUT_PIN == digitalPinStatus[pinNum]) ||
      (msg[0] == 'A' && pinNum < ANALOG_PINS && OUTPUT_PIN == analogPinStatus[pinNum])) {

      unsigned int outputNumIdx = 2 + pinDigits;
      if (msg[outputNumIdx] == 'H') {
        if (msg[0] == 'D') {
          digitalWrite(DIGITAL_PIN[pinNum], HIGH);
        } else {
          digitalWrite(ANALOG_PIN[pinNum], HIGH);
        }
      } else if (msg[outputNumIdx] == 'L') {
        if (msg[0] == 'D') {
          digitalWrite(DIGITAL_PIN[pinNum], LOW);
        } else {
          digitalWrite(ANALOG_PIN[pinNum], LOW);
        }
      } else {
        unsigned int outputNum = 0;
        for (unsigned int i = outputNumIdx; i < outputDigits + outputNumIdx; ++i) {
          outputNum = (10 * outputNum) + (msg[i] - '0');
        }
        if (outputNum <= 255) {
          if (msg[0] == 'D') {
            analogWrite(DIGITAL_PIN[pinNum], outputNum);
          } else {
            analogWrite(ANALOG_PIN[pinNum], outputNum);
          }
        } // end analog number validation
      } // end analog write
    } // end pin number validation
  } // end message first char and size validation
}

void NetworkPinHandler::sendDigitalRead(unsigned int pinNumber) {
  // TODO
}

void NetworkPinHandler::sendAnalogRead(unsigned int pinNumber) {
  // TODO
}
