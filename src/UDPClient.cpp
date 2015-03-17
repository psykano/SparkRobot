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

bool UDPClient::connect(IPAddress ip, unsigned int port) {
  return false; // TODO
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
  return udp.write(buffer, size);
}
