#include "NetworkPinHandler.h"

NetworkPinHandler::NetworkPinHandler() {
  for (unsigned int i = 0; i < DIGITAL_PINS; ++i) {
    digitalPinStatus[i] = UNUSED_PIN;
  }
  for (unsigned int i = 0; i < ANALOG_PINS; ++i) {
    analogPinStatus[i] = UNUSED_PIN;
  }
  digitalReadMsg[0] = DIGITAL_ID;
  analogReadMsg[0] = ANALOG_ID;
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

void NetworkPinHandler::setSendMessage(void (*_sendMessage) (const uint8_t*, size_t)) {
  sendMessage = _sendMessage;
}

void NetworkPinHandler::readMessage(const uint8_t* msg, size_t size) {
  // NOTE not expecting to handle more than 99 pins...
  if ((msg[0] == DIGITAL_ID || msg[0] == ANALOG_ID) && size > 3 && size <= 7) {
    uint8_t pinDigits = 0;
    uint8_t outputDigits = 0;

    for (unsigned int i = 2; i < size; ++i) {
      if (SEPARATOR == msg[i]) {
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

    if ((msg[0] == DIGITAL_ID && pinNum < DIGITAL_PINS && OUTPUT_PIN == digitalPinStatus[pinNum]) ||
      (msg[0] == ANALOG_ID && pinNum < ANALOG_PINS && OUTPUT_PIN == analogPinStatus[pinNum])) {

      unsigned int outputNumIdx = 2 + pinDigits;
      if (msg[outputNumIdx] == HIGH_ID) {
        if (msg[0] == DIGITAL_ID) {
          digitalWrite(DIGITAL_PIN[pinNum], HIGH);
        } else {
          digitalWrite(ANALOG_PIN[pinNum], HIGH);
        }
      } else if (msg[outputNumIdx] == LOW_ID) {
        if (msg[0] == DIGITAL_ID) {
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
          if (msg[0] == DIGITAL_ID) {
            analogWrite(DIGITAL_PIN[pinNum], outputNum);
          } else {
            analogWrite(ANALOG_PIN[pinNum], outputNum);
          }
        } // end analog value validation
#ifdef DEBUG
        else {
          Serial.print("Error: invalid value to write to pin ");
          Serial.print(pinNum);
          Serial.println(".");
        }
#endif
      } // end analog write
    } // end pin number validation
#ifdef DEBUG
    else {
      Serial.print("Error: can't write to pin ");
      Serial.print(pinNum);
      Serial.println(".");
    }
#endif
  } // end message first char and size validation
#if defined(DEBUG) && defined(VERBOSE)
  else {
    Serial.println("Error: can't read message.");
  }
#endif
}

void NetworkPinHandler::sendDigitalRead(unsigned int pinNumber) {
  if (pinNumber < DIGITAL_PINS && INPUT_PIN == digitalPinStatus[pinNumber]) {
    int pinDigits = sprintf(pinNumberStr, "%d", pinNumber);
    if (pinDigits > 0) {
      int value = digitalRead(DIGITAL_PIN[pinNumber]);
      if (HIGH == value) {
        digitalReadValue = HIGH_ID;
      } else { // LOW == val
        digitalReadValue = LOW_ID;
      }
      unsigned int j = 1;
      for (unsigned int i = 0; i < pinDigits; ++i) {
        digitalReadMsg[j++] = pinNumberStr[i];
      }
      digitalReadMsg[j++] = SEPARATOR;
      digitalReadMsg[j++] = digitalReadValue;
      sendMessage(digitalReadMsg, j);
    }
#ifdef DEBUG
    else {
      Serial.print("Error: couldn't convert digital pin ");
      Serial.print(pinNumber);
      Serial.println(" to string.");
    }
#endif
  }
#ifdef DEBUG
  else {
    Serial.print("Error: can't do a digital read of pin ");
    Serial.print(pinNumber);
    Serial.println(".");
  }
#endif
}

void NetworkPinHandler::sendAnalogRead(unsigned int pinNumber) {
  if (pinNumber < ANALOG_PINS && INPUT_PIN == analogPinStatus[pinNumber]) {
    int pinDigits = sprintf(pinNumberStr, "%d", pinNumber);
    if (pinDigits > 0) {
      int value = analogRead(DIGITAL_PIN[pinNumber]);
      // No need to check for error
      int valueDigits = sprintf(analogReadValue, "%d", value);
      unsigned int j = 1;
      for (unsigned int i = 0; i < pinDigits; ++i) {
        analogReadMsg[j++] = pinNumberStr[i];
      }
      analogReadMsg[j++] = SEPARATOR;
      for (unsigned int i = 0; i < valueDigits; ++i) {
        analogReadMsg[j++] = analogReadValue[i];
      }
      sendMessage(analogReadMsg, j);
    }
#ifdef DEBUG
    else {
      Serial.print("Error: couldn't convert analog pin ");
      Serial.print(pinNumber);
      Serial.println(" to string.");
    }
#endif
  }
  #ifdef DEBUG
  else {
    Serial.print("Error: can't do an analog read of pin ");
    Serial.print(pinNumber);
    Serial.println(".");
  }
  #endif
}
