#include "nopf.hpp"

bool Nopf::bindLocalport()
{
  const int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&enable, sizeof(int)) < 0)
  {
    perror("setsockopt(SO_REUSEADDR) failed");
    closesocket(sockfd);
    return false;
  }

  linger optval;

  optval.l_onoff = 1;
  optval.l_linger = 0;

  if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *)&optval, sizeof(optval)) < 0)
  {
    wsaerror("setsockopt(SO_LINGER) failed");
    closesocket(sockfd);
    return false;
  }

  sockaddr_in localAddr = {};

  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = INADDR_ANY;
  localAddr.sin_port = localRawPort;

  if (bind(sockfd, (sockaddr *)&localAddr, sizeof(localAddr)) == -1)
  {
    wsaerror("bind() failed");
    closesocket(sockfd);
    return false;
  }

  return true;
}
bool Nopf::loadPublicAddr(Addr &publicAddr)
{
  if (!packetFilter.isRunning())
    packetFilter.make();
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1)
  {
    wsaerror("socket() failed");
    return false;
  }

  sockaddr_in serverAddr = {};

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = nopfServerAddr.getRawAddr();
  serverAddr.sin_port = nopfServerAddr.getRawPort();

  if (connect(sockfd, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
  {
    wsaerror("connect() failed");
    closesocket(sockfd);
    return false;
  }

  // get public address
  if (recv(sockfd, (char *)&publicAddr, sizeof(publicAddr), 0) < 0)
  {
    wsaerror("recv() failed");
    closesocket(sockfd);
    return false;
  }

  sockaddr_in localAddr;
  int len = sizeof(localAddr);

  // get localport
  if (getsockname(sockfd, (struct sockaddr *)&localAddr, &len) == -1)
  {
    wsaerror("getsockname() failed");
    closesocket(sockfd);
    return false;
  }
  localRawPort = localAddr.sin_port;

  closesocket(sockfd);
  return true;
}
PacketFilter &Nopf::getFilter()
{
  return packetFilter;
}
int Nopf::getSockfd()
{
  return sockfd;
}
unsigned short Nopf::getRawLocalPort()
{
  return localRawPort;
}
void Nopf::closePacketFilter()
{
  packetFilter.close();
}
bool NopfServer::open(Addr targetAddr)
{
  if (localRawPort == 0)
  {
    cerr << "localPort is zero" << endl;
    return false;
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1)
  {
    wsaerror("socket() failed");
    return false;
  }

  // set non-blocking mode
  u_long mode = 1;
  if (ioctlsocket(sockfd, FIONBIO, &mode) != 0)
  {
    perror("Failed to set non-blocking mode");
    closesocket(sockfd);
    return false;
  }

  if (!bindLocalport())
    return false;

  sockaddr_in serverAddr = {};

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = targetAddr.getRawAddr();
  serverAddr.sin_port = targetAddr.getRawPort();

  // send SYN packet and ignore
  connect(sockfd, (sockaddr *)&serverAddr, sizeof(serverAddr));

  closesocket(sockfd);
  sockfd = -1;
  return true;
}
bool NopfClient::join(Addr targetAddr)
{
  if (localRawPort == 0)
  {
    cerr << "localPort is zero" << endl;
    return false;
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1)
  {
    wsaerror("socket() failed");
    return false;
  }

  if (!bindLocalport())
    return false;

  sockaddr_in serverAddr = {};

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = targetAddr.getRawAddr();
  serverAddr.sin_port = targetAddr.getRawPort();

  if (connect(sockfd, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
  {
    wsaerror("connect() failed");
    closesocket(sockfd);
    return false;
  }

  return true;
}