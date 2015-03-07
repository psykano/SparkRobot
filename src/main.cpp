#include "application.h"
#include "NetworkPinHandler.h"
#include "SparkRobotSettings.h"

// Modify these to your liking
const int RECONNECT_DELAY = 3000;
const int MAX_MESSAGE_LENGTH = 100;
const char DELIMITER = '\n';
const IPAddress SERVER_IP(192,168,1,198);
const int SERVER_PORT = 11999;

TCPClient client;
int read;
uint8_t receiveMsg[MAX_MESSAGE_LENGTH];
size_t receiveMsgLength;
uint8_t sendMsg[MAX_MESSAGE_LENGTH];

NetworkPinHandler pinHandler;

#ifdef DEBUG
// Less than 60 Hz
const float SLOW_DT = 0.0167f;
unsigned long prevMillis;
unsigned long curMillis;
float dt;
#endif

void disconnected();
void connected();
void sendMessage(const uint8_t* message, size_t messageLength);

void setup() {
#ifdef DEBUG
  Serial.begin(9600);

  prevMillis = 0;
  curMillis = 0;
  dt = 0;
#endif

  pinHandler.setDigitalPin(7, OUTPUT_PIN);

  // Ready to receive first message
  receiveMsgLength = 0;

#ifdef DEBUG
  // Now open your Serial Terminal, and hit any key to continue!
  while(!Serial.available()) SPARK_WLAN_Loop();
#endif
}

void loop() {
#ifdef DEBUG
  curMillis = millis();
  dt = (curMillis - prevMillis) / 1000.0f;
  prevMillis = curMillis;
  
  if (dt >= SLOW_DT) {
    Serial.print("Warning: slow update rate of ");
    Serial.print(dt);
    Serial.println("s.");
  }
#endif

  if (client.connected()) {
    connected();
  } else {
    disconnected();
  }
}

void disconnected() {
#ifdef DEBUG
  Serial.println("Connecting...");
#endif
  if (client.connect(SERVER_IP, SERVER_PORT)) {
#ifdef DEBUG
    Serial.println("Connected.");
#endif
  } else {
#ifdef DEBUG
    Serial.println("Connection failed.");
#endif
    delay(RECONNECT_DELAY);
  }
}

void connected() {
  // Read message from server into buffer
  read = client.read();
  while (read > 0 && read != DELIMITER && receiveMsgLength != MAX_MESSAGE_LENGTH) {
    receiveMsg[receiveMsgLength++] = read;
    read = client.read();
  }
  // Entire message is in buffer
  if (read == DELIMITER) {
#if defined(DEBUG) && defined(VERBOSE)
    Serial.print("Received: ");
    for (unsigned int i = 0; i < receiveMsgLength; ++i) {
      Serial.print((char)receiveMsg[i]);
    }
    Serial.println();
#endif

    // Parse message
    pinHandler.readMessage(receiveMsg, receiveMsgLength);

    // Ready to receive next message
    receiveMsgLength = 0;
  } else if (receiveMsgLength == MAX_MESSAGE_LENGTH) {
    // Disregard broken message and get ready to receive next message
    read = client.read();
    while (read != -1 && read != DELIMITER) {
      read = client.read();
    }
    receiveMsgLength = 0;
  }
}

void sendMessage(const uint8_t* message, size_t messageLength) {
  if (client.connected()) {
    if (MAX_MESSAGE_LENGTH <= messageLength) {
#ifdef DEBUG
      Serial.println("Error: message to send is too large.");
#endif
    } else {
      memcpy(sendMsg, message, messageLength);
      sendMsg[messageLength] = DELIMITER;
      client.write(sendMsg, messageLength + 1);
#if defined(DEBUG) && defined(VERBOSE)
      Serial.print("Sent: ");
      for (unsigned int i = 0; i < messageLength; ++i) {
        Serial.print((char)sendMsg[i]);
      }
      Serial.println();
#endif
    }
  }
}
