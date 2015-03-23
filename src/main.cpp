#include "application.h"
#include "NetworkPinHandler.h"
#include "UDPClient.h"
#include "SparkRobotSettings.h"

// Modify these to your liking
const unsigned int RECONNECT_DELAY = 3000;
const unsigned int MAX_MESSAGE_LENGTH = 100;
const unsigned int READ_BUFFER_LENGTH = 100;
const char DELIMITER = '\n';

const IPAddress SERVER_IP(192,168,1,198);
const unsigned int SERVER_TCP_PORT = 11999;
const unsigned int SERVER_UDP_PORT = 11998;
const unsigned int CLIENT_UDP_PORT = 9000;
const uint8_t CONNECT_MSG[] = "connect\n";
const size_t CONNECT_MSG_SIZE = 8;
const uint8_t ACK_MSG[] = "connected\n";

TCPClient tcpClient;
UDPClient udpClient;

int read;
uint8_t readBuffer[READ_BUFFER_LENGTH];
uint8_t tcpReceiveMsg[MAX_MESSAGE_LENGTH];
size_t tcpReceiveMsgLength;
uint8_t udpReceiveMsg[MAX_MESSAGE_LENGTH];
size_t udpReceiveMsgLength;
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
void sendTcpMessage(const uint8_t* message, size_t messageLength);
void sendUdpMessage(const uint8_t* message, size_t messageLength);

void setup() {
#ifdef DEBUG
  Serial.begin(9600);

  prevMillis = 0;
  curMillis = 0;
  dt = 0;
#endif

  pinHandler.setDigitalPin(7, OUTPUT_PIN);
  pinHandler.setSendMessage(sendTcpMessage);

  // Ready to receive first message
  tcpReceiveMsgLength = 0;
  udpReceiveMsgLength = 0;

#ifdef DEBUG
  // Now open your Serial Terminal, and hit any key to continue!
  while(!Serial.available()) SPARK_WLAN_Loop();
#endif

  if (udpClient.setup(CLIENT_UDP_PORT)) { // should try 0 here TODO
#ifdef DEBUG
    Serial.print("UDP Client bound successfully on port ");
    Serial.print(CLIENT_UDP_PORT);
    Serial.println(".");
#endif
  } else {
#ifdef DEBUG
    Serial.print("Error: UDP Client bound unsuccessfully on port ");
    Serial.print(CLIENT_UDP_PORT);
    Serial.println(".");
#endif
  }
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

  if (tcpClient.connected() && udpClient.connected()) {
    connected();
  } else {
    disconnected();
  }
}

void disconnected() {
  if (!tcpClient.connected()) {
#ifdef DEBUG
    Serial.print("Connecting to TCP server ");
    IPAddress& ipaddr = const_cast<IPAddress&>(SERVER_IP);
    Serial.print(ipaddr.raw_address()[0]);
    Serial.print(".");
    Serial.print(ipaddr.raw_address()[1]);
    Serial.print(".");
    Serial.print(ipaddr.raw_address()[2]);
    Serial.print(".");
    Serial.print(ipaddr.raw_address()[3]);
    Serial.print(":");
    Serial.print(SERVER_TCP_PORT);
    Serial.println("...");
#endif
    if (tcpClient.connect(SERVER_IP, SERVER_TCP_PORT)) {
#ifdef DEBUG
      Serial.println("Connected to TCP server.");
#endif
    } else {
#ifdef DEBUG
      Serial.println("Connection to TCP server failed.");
#endif
    }
  }

  if (!udpClient.connected()) {
#ifdef DEBUG
    Serial.print("Connecting to UDP server ");
    IPAddress& ipaddr = const_cast<IPAddress&>(SERVER_IP);
    Serial.print(ipaddr.raw_address()[0]);
    Serial.print(".");
    Serial.print(ipaddr.raw_address()[1]);
    Serial.print(".");
    Serial.print(ipaddr.raw_address()[2]);
    Serial.print(".");
    Serial.print(ipaddr.raw_address()[3]);
    Serial.print(":");
    Serial.print(SERVER_UDP_PORT);
    Serial.println("...");
#endif
    if (udpClient.connect(SERVER_IP, SERVER_UDP_PORT, CONNECT_MSG, CONNECT_MSG_SIZE, ACK_MSG)) {
#ifdef DEBUG
      Serial.println("Connected to UDP server.");
#endif
    } else {
#ifdef DEBUG
      Serial.println("Connection to UDP server failed.");
#endif
    }
  }

  if (!tcpClient.connected() || !udpClient.connected()) {
    delay(RECONNECT_DELAY);
  }
}

void connected() {
  // Receive from TCP server
  {
    // Read message from server into buffer
    read = tcpClient.read();
    while (read > 0 && read != DELIMITER && tcpReceiveMsgLength != MAX_MESSAGE_LENGTH) {
      tcpReceiveMsg[tcpReceiveMsgLength++] = read;
      read = tcpClient.read();
    }
    // Entire message is in buffer
    if (read == DELIMITER) {
#if defined(DEBUG) && defined(VERBOSE)
      Serial.print("Received via TCP: ");
      for (unsigned int i = 0; i < tcpReceiveMsgLength; ++i) {
        Serial.print((char)tcpReceiveMsg[i]);
      }
      Serial.println();
#endif

      // Parse message
      pinHandler.readMessage(tcpReceiveMsg, tcpReceiveMsgLength);

      // Ready to receive next message
      tcpReceiveMsgLength = 0;
    } else if (tcpReceiveMsgLength == MAX_MESSAGE_LENGTH) {
      // Disregard broken message and get ready to receive next message
      read = tcpClient.read();
      while (read != -1 && read != DELIMITER) {
        read = tcpClient.read();
      }
      tcpReceiveMsgLength = 0;
    }
  }

  // Receive from UDP server
  {
    if (udpClient.buffer()) {
      // Read message from server into buffer
      read = udpClient.read();
      while (read > 0 && read != DELIMITER && udpReceiveMsgLength != MAX_MESSAGE_LENGTH) {
        udpReceiveMsg[udpReceiveMsgLength++] = read;
        read = udpClient.read();
      }
      // Entire message is in buffer
      if (read == DELIMITER) {
#if defined(DEBUG) && defined(VERBOSE)
        Serial.print("Received via UDP: ");
        for (unsigned int i = 0; i < udpReceiveMsgLength; ++i) {
          Serial.print((char)udpReceiveMsg[i]);
        }
        Serial.println();
#endif

        // Parse message
        pinHandler.readMessage(udpReceiveMsg, udpReceiveMsgLength);

        // Ready to receive next message
        udpReceiveMsgLength = 0;
      } else if (udpReceiveMsgLength == MAX_MESSAGE_LENGTH) {
        // Disregard broken message and get ready to receive next message
        read = udpClient.read();
        while (read != -1 && read != DELIMITER) {
          read = udpClient.read();
        }
        udpReceiveMsgLength = 0;
      }
    }
/*
    while (udpClient.read(readBuffer, READ_BUFFER_LENGTH)) {
      for (unsigned int i = 0; i < READ_BUFFER_LENGTH && readBuffer[i] != '\0'; ++i) {
        // Entire message is in buffer
        if (readBuffer[i] == DELIMITER) {
#if defined(DEBUG) && defined(VERBOSE)
          Serial.print("Received via UDP: ");
          for (unsigned int j = 0; j < udpReceiveMsgLength; ++j) {
            Serial.print((char)udpReceiveMsg[j]);
          }
          Serial.println();
#endif
          // Parse message
          pinHandler.readMessage(udpReceiveMsg, udpReceiveMsgLength);

          // Ready to receive next message
          udpReceiveMsgLength = 0;
        } else if (udpReceiveMsgLength == MAX_MESSAGE_LENGTH) {
          // Disregard broken message and get ready to receive next message
          udpReceiveMsgLength = 0;
        } else {
          udpReceiveMsg[udpReceiveMsgLength++] = readBuffer[i];
        }
      }
    }
*/
  }
}

void sendTcpMessage(const uint8_t* message, size_t messageLength) {
  if (tcpClient.connected()) {
    if (MAX_MESSAGE_LENGTH <= messageLength) {
#ifdef DEBUG
      Serial.println("Error: message to send to TCP server is too large.");
#endif
    } else {
      memcpy(sendMsg, message, messageLength);
      sendMsg[messageLength] = DELIMITER;
      tcpClient.write(sendMsg, messageLength + 1);
#if defined(DEBUG) && defined(VERBOSE)
      Serial.print("Sent via TCP: ");
      for (unsigned int i = 0; i < messageLength; ++i) {
        Serial.print((char)sendMsg[i]);
      }
      Serial.println();
#endif
    }
  }
}

void sendUdpMessage(const uint8_t* message, size_t messageLength) {
  if (udpClient.connected()) {
    if (MAX_MESSAGE_LENGTH <= messageLength) {
#ifdef DEBUG
      Serial.println("Error: message to send to UDP server is too large.");
#endif
    } else {
      memcpy(sendMsg, message, messageLength);
      sendMsg[messageLength] = DELIMITER;
      udpClient.write(sendMsg, messageLength + 1);
#if defined(DEBUG) && defined(VERBOSE)
      Serial.print("Sent via UDP: ");
      for (unsigned int i = 0; i < messageLength; ++i) {
        Serial.print((char)sendMsg[i]);
      }
      Serial.println();
#endif
    }
  }
}
