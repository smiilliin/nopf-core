#ifndef SOCKS_HPP
#define SOCKS_HPP

#include <iostream>
#include <mingw.thread.h>
#include <mingw.mutex.h>
#include <winsock2.h>
#include <windows.h>

using namespace std;

void wsaerror(const char *msg);

#pragma pack(push, 1)
class Addr
{
  uint32_t addr;
  uint16_t port;

public:
  Addr() : addr(0), port(0) {}
  Addr(string addr, unsigned short port);
  Addr(string str);
  uint32_t getRawAddr();
  string getAddr();
  uint16_t getRawPort();
  uint16_t getPort();
  string toString();
};
#pragma pack(pop)

class PacketFilter
{
  thread packetFilterThread;
  bool running;
  unsigned short port;
  HANDLE handle;
  mutex lock;

  void packetFilter();

public:
  PacketFilter(unsigned short port) : port(port), running(false)
  {
  }
  ~PacketFilter();
  void close();
  bool isRunning();
  bool make();
};

class ReceptiveSocket
{
  int sockfd;

public:
  ReceptiveSocket() {}
  bool listen(unsigned short localRawPort);
  bool accept(int &clientfd);
  void close();

  ~ReceptiveSocket();
};
class ConnectingSocket
{
  int sockfd;

public:
  ConnectingSocket() {}
  bool connect(Addr target);
  int getSockfd();
};
bool pipeSocket(int sock1, int sock2);

#endif