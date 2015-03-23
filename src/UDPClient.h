#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "application.h"
#include <cstring>

const unsigned int CONNECT_DELAY = 2000; // ms
const unsigned int MAX_ACK_MSG_LENGTH = 50;

class UDPClient {
public:
  UDPClient() : _connected(false), serverPort(0) {}
  bool setup(unsigned int port);
  bool connected();
  bool connect(IPAddress ip, unsigned int port,
              const uint8_t* connectMsg, size_t connectMsgSize,
              const uint8_t* ackMsg);
  bool buffer();
  int read();
  size_t write(const uint8_t* buffer, size_t size);

private:
  UDP udp;
  bool _connected;
  IPAddress serverIp;
  unsigned int serverPort;
};

#endif
