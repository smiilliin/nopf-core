#include "nopf.hpp"
#include "options.hpp"

#include <vector>

bool isRunningAsAdministrator()
{
  bool isAdmin = false;
  HANDLE hToken = nullptr;

  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
  {
    TOKEN_ELEVATION elevation;
    DWORD cbSize = sizeof(TOKEN_ELEVATION);

    if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize))
    {
      isAdmin = elevation.TokenIsElevated != 0;
    }

    CloseHandle(hToken);
  }

  return isAdmin;
}
int main(int argc, char **argv)
{
  if (!isRunningAsAdministrator())
  {
    cerr << "Please run as Administrator" << endl;
    return 1;
  }
  Options options;
  if (!loadOptions(argc, argv, options))
  {
    return 1;
  }

  WSAData wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) == -1)
  {
    return 1;
  }
  if (options.isServer)
  {
    NopfServer server(Addr(options.serveraddr));
    Addr addr;

    if (!server.loadPublicAddr(addr))
    {
      return 1;
    }
    cout << addr.toString() << endl;

    string inputClientAddr;
    cin >> inputClientAddr;
    Addr clientAddr(inputClientAddr);

    if (!server.open(clientAddr))
    {
      return 1;
    }

    ReceptiveSocket receptiveSocket;
    if (!receptiveSocket.listen(server.getRawLocalPort()))
    {
      return 1;
    }
    int clientfd;
    if (!receptiveSocket.accept(clientfd))
    {
      return 1;
    }

    ConnectingSocket connectingSocket;

    if (!connectingSocket.connect(Addr("127.0.0.1", options.localport)))
    {
      return 1;
    }
    server.closePacketFilter();

    cout << "Started pipe between "
         << "localhost:" << options.localport << " - " << clientAddr.toString() << endl;

    if (!pipeSocket(connectingSocket.getSockfd(), clientfd))
    {
      return 1;
    }
  }
  else
  {
    NopfClient client(options.serveraddr);
    Addr addr;

    if (!client.loadPublicAddr(addr))
    {
      return 1;
    }

    cout << addr.toString() << endl;

    string inputServerAddr;
    cin >> inputServerAddr;
    Addr serverAddr(inputServerAddr);

    ReceptiveSocket receptiveSocket;
    if (!receptiveSocket.listen(htons(options.localport)))
    {
      return 1;
    }
    int clientfd;
    if (!receptiveSocket.accept(clientfd))
    {
      return 1;
    }

    if (!client.join(serverAddr))
    {
      return 1;
    }

    client.closePacketFilter();

    cout << "Started pipe between "
         << "localhost:" << options.localport << " - " << serverAddr.toString() << endl;

    if (!pipeSocket(client.getSockfd(), clientfd))
    {
      return 1;
    }
  }

  WSACleanup();

  return 0;
}