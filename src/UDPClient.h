#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "application.h"

class UDPClient {
public:
  UDPClient() : _connected(false) {}
  bool setup(unsigned int port);
  bool connected();
  bool connect(IPAddress ip, unsigned int port);
  bool buffer();
  int read();
  size_t write(const uint8_t* buffer, size_t size);

private:
  UDP udp;
  bool _connected;
};

#endif
