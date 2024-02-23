#ifndef NOPF_HPP
#define NOPF_HPP

#include "socks.hpp"

#include <vector>
#include <string>
#include <sstream>
#include <windows.h>
#include <functional>

using namespace std;

class Nopf
{
protected:
  unsigned short localRawPort;
  PacketFilter packetFilter;
  Addr nopfServerAddr;
  int sockfd;

  bool bindLocalport();

public:
  Nopf(Addr nopfServerAddr) : nopfServerAddr(nopfServerAddr), localRawPort(0), sockfd(-1), packetFilter(nopfServerAddr.getPort()) {}
  bool loadPublicAddr(Addr &publicAddr);
  PacketFilter &getFilter();
  int getSockfd();
  unsigned short getRawLocalPort();
  void closePacketFilter();
};
class NopfServer : public Nopf
{
public:
  NopfServer(Addr nopfServerAddr) : Nopf(nopfServerAddr) {}
  bool open(Addr nopfClientAddr);
};
class NopfClient : public Nopf
{
public:
  NopfClient(Addr nopfServerAddr) : Nopf(nopfServerAddr) {}
  bool join(Addr nopfClientAddr);
};

#endif