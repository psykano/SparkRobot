#include "UDPClient.h"

bool UDPClient::setup(unsigned int port) {
  if (!udp.begin(port)) {
    return false;
  }
  return true;
}

bool UDPClient::connected() {
  return _connected;
}

bool UDPClient::connect(IPAddress ip, unsigned int port,
                        const uint8_t* connectMsg, size_t connectMsgSize,
                        const uint8_t* ackMsg) {
  serverIp = ip;
  serverPort = port;
  write(connectMsg, connectMsgSize);
  delay(CONNECT_DELAY);
  if (buffer()) {
    int _read = read();
    uint8_t recvAckMsg[MAX_ACK_MSG_LENGTH];
    unsigned int i = 0;
    while (_read > 0 && i != MAX_ACK_MSG_LENGTH) {
      recvAckMsg[i++] = _read;
      _read = read();
    }
    if (0 == strcmp((char*)recvAckMsg, (char*)ackMsg)) {
      _connected = true;
      return true;
    }
  }
  _connected = false;
  return false;
}

bool UDPClient::buffer() {
  if (!udp.parsePacket()) {
    return false;
  }
  return true;
}

int UDPClient::read() {
  return udp.read();
}

size_t UDPClient::write(const uint8_t* buffer, size_t size) {
  if (0 == serverPort) {
    return 0;
  }
  udp.beginPacket(serverIp, serverPort);
  int ret = udp.write(buffer, size);
  udp.endPacket();
  return ret;
}
