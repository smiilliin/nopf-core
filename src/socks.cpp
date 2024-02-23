#include "socks.hpp"

#include <sstream>
#include <vector>
#include <windivert.h>

void wsaerror(const char *msg)
{
  char buffer[256] = {};

  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer), NULL);
  cerr << msg << ": " << buffer << endl;
}

bool pipeSocket(int sock1, int sock2)
{
  char buffer[0xffff] = {};
  while (true)
  {
    u_long count = 0;
    ioctlsocket(sock1, FIONREAD, &count);
    int len = 0;

    if (count < 0)
      break;
    if (count > 0)
    {
      if ((len = recv(sock1, buffer, sizeof(buffer), 0)) < 0)
      {
        break;
      }
      if (send(sock2, buffer, len, 0) < 0)
      {
        break;
      }
    }

    ioctlsocket(sock2, FIONREAD, &count);

    if (count < 0)
      break;
    if (count > 0)
    {
      if ((len = recv(sock2, buffer, sizeof(buffer), 0)) < 0)
      {
        break;
      }
      if (send(sock1, buffer, len, 0) < 0)
      {
        break;
      }
    }
  }
  return true;
}

void PacketFilter::packetFilter()
{
  WINDIVERT_ADDRESS divertAddress;
  uint8_t packet[0xFFFF];
  UINT packetLength;

  while (true)
  {
    lock.lock();
    if (!running)
    {
      break;
    }
    lock.unlock();

    if (!WinDivertRecv(handle, packet, sizeof(packet), &packetLength, &divertAddress))
    {
      continue;
    }

    if (packetLength >= sizeof(WINDIVERT_TCPHDR))
    {
      WINDIVERT_IPHDR *ipHeader = (WINDIVERT_IPHDR *)packet;
      WINDIVERT_TCPHDR *tcpHeader = (WINDIVERT_TCPHDR *)(packet + ipHeader->HdrLength * 4);

      if (htons(tcpHeader->DstPort) == port)
      {
        if (tcpHeader->Rst)
        {
          // cout << "ignored rst" << endl;
          continue;
        }
        if (tcpHeader->Fin & tcpHeader->Ack)
        {
          // cout << "ignored fin+ack" << endl;
          continue;
        }
      }
    }

    if (!WinDivertSend(handle, packet, packetLength, nullptr, &divertAddress))
    {
      continue;
    }
  }

  running = false;

  if (handle)
  {
    WinDivertClose(handle);
  }
  lock.unlock();
}
PacketFilter::~PacketFilter()
{
  close();
}
bool PacketFilter::isRunning()
{
  return running;
}
void PacketFilter::close()
{
  lock.lock();
  if (running)
  {
    WinDivertClose(handle);
    handle = nullptr;
    running = false;
  }
  lock.unlock();
}
bool PacketFilter::make()
{
  if (running)
  {
    cerr << "This filter is already on running" << endl;
    return false;
  }
  handle = WinDivertOpen("true", WINDIVERT_LAYER_NETWORK, 0, 0);
  if (handle == INVALID_HANDLE_VALUE)
  {
    wsaerror("WinDivertOpen() failed");
    return false;
  }
  running = true;
  packetFilterThread = thread(bind(&PacketFilter::packetFilter, this));
  packetFilterThread.detach();

  return true;
}

bool ReceptiveSocket::listen(unsigned short localRawPort)
{
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1)
  {
    wsaerror("socket() failed");
    return false;
  }

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = localRawPort;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (::bind(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
  {
    wsaerror("bind() failed");
    closesocket(sockfd);
    sockfd = -1;
    return false;
  }
  if (::listen(sockfd, 1) == -1)
  {
    wsaerror("listen() failed");
    closesocket(sockfd);
    sockfd = -1;
    return false;
  }

  return true;
}

bool ReceptiveSocket::accept(int &clientfd)
{
  clientfd = ::accept(sockfd, nullptr, nullptr);
  if (clientfd == INVALID_SOCKET)
  {
    wsaerror("accept() failed");
    closesocket(sockfd);
    sockfd = -1;
    return false;
  }
  return true;
}

void ReceptiveSocket::close()
{
  if (sockfd != -1)
    closesocket(sockfd);
}

ReceptiveSocket::~ReceptiveSocket()
{
  close();
}

bool ConnectingSocket::connect(Addr target)
{
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1)
  {
    wsaerror("socket() failed");
    return false;
  }

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = target.getRawPort();
  addr.sin_addr.s_addr = target.getRawAddr();

  if (::connect(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
  {
    wsaerror("connect() failed");
    return false;
  }

  return true;
}

int ConnectingSocket::getSockfd()
{
  return sockfd;
}

Addr::Addr(string addr, unsigned short port)
{
  this->addr = inet_addr(addr.c_str());
  this->port = htons(port);
}

Addr::Addr(string str)

{
  istringstream iss(str);
  string buffer;
  vector<string> splitted;

  while (getline(iss, buffer, ':'))
  {
    splitted.push_back(buffer);
  }

  if (splitted.size() != 2)
  {
    cerr << "Wrong Addr string" << endl;
    return;
  }

  addr = inet_addr(splitted[0].c_str());

  stringstream portss(splitted[1]);
  portss >> port;
  port = htons(port);
}

uint32_t Addr::getRawAddr()
{
  return addr;
}

string Addr::getAddr()
{
  in_addr paddr;
  paddr.s_addr = addr;
  return string(inet_ntoa(paddr));
}

uint16_t Addr::getRawPort()
{
  return port;
}

uint16_t Addr::getPort()
{
  return htons(port);
}

string Addr::toString()
{
  return getAddr() + ":" + to_string(getPort());
}