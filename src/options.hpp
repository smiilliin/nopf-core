#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <string>

using namespace std;

struct Options
{
  unsigned short localport;
  string serveraddr;
  bool isServer;
};
bool loadOptions(int argc, char **argv, Options &options);

#endif