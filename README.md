# NOPF-core

Instead of connecting via port forwarding, connect using TCP hole punching.

## Mechanism

- In order for a client to connect to the server, query each other's public IP, port from nopf-server.
- Using the obtained IP and port, the server first sends a SYN packet to the client.
- Even though the SYN packet is discarded by NAT/PAT on the client side, it is recorded on the server side.
- Because of recorded port, client can connect to server.
- Prevents ports from being cleared in NAT/PAT by blocking RST, FIN+ACK packets.

## Usage

**The process must proceed in this order: server inputs client's address -> client inputs server's address.**  
**You should not input serveraddr with the same current public IP address**

```
Usage: [-h --help] [-p --localport] [-a --serveraddr] [-t --type(server, s or not)]
```

### --localport(-p)

- Case of server: port of your server local port
- Case of client: port to connect via localhost(you should connect to this port)

### --serveraddr(-a)

nopf-server address

### --type(-t)

- Case of server: input "s" or "server"
- Case of client: input other than "s" or "server"

## Example

nopf-core server(localport:3000, serveraddr: 12.123.234.456:12345)

```
nopf-core> ./run.bat brun --localport=3000 --serveraddr=123.123.234.456:12345 --type=server
111.111.111.111:40737   (this is your public address and server's address)
222.222.222.222:64712     (input client's address first)
Started pipe between localhost:3000 - 222.222.222.222:64712 (it shows when a client is connected)
```

nopf-core client(localport:3010, serveraddr: 12.123.234.456:12345)

```
nopf-core> ./run.bat brun --localport=3010 --serveraddr=123.123.234.456:12345 --type=client
222.222.222.222:64712   (this is your public address and client's address)
111.111.111.111:40737     (input server's address last)
Started pipe between localhost:3010 - 111.111.111.111:40737 (it shows when you connect to localhost:3010)
```

See also: https://github.com/smiilliin/nopf-server

## Development

you should install cmake, g++(mingw64), mingw-std-threads, windivert in windows

### mingw-std-threads

https://github.com/meganz/mingw-std-threads

### windivert

https://github.com/basil00/Divert

### CMake

- `./run.bat debug`: run cmake with debug mode
- `./run.bat release`: run cmake with release mode

### Build or Run

- `./run.bat build`: build with g++
- `./run.bat run`: run with g++
- `./run.bat brun`: build and run
