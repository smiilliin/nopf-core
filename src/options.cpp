#include <iostream>
#include <getopt.h>
#include <vector>
#include <sstream>
#include <algorithm>

#include "options.hpp"

void eraseString(vector<string> &target, string name)
{
  target.erase(remove_if(target.begin(), target.end(),
                         [&](const string &option)
                         { return option == name; }),
               target.end());
}

bool loadOptions(int argc, char **argv, Options &options)
{
  const option longOptions[] = {
      {"help", no_argument, 0, 'h'},
      {"localport", required_argument, 0, 'p'},
      {"serveraddr", required_argument, 0, 'a'},
      {"type", required_argument, 0, 't'},
      {0, 0, 0, 0}};
  int option;
  int optionIndex = 0;
  vector<string> requiredOptions = {"localport", "serveraddr", "type"};

  while ((option = getopt_long(argc, argv, "hp:a:t:", longOptions, &optionIndex)) != -1)
  {
    switch (option)
    {
    case 'h':
    {
      cout << "Usage: [-h --help] [-p --localport] [-a --serveraddr] [-t --type(server, s or not)]" << endl;
      return false;
    }
    case 'p':
    {
      stringstream ss(optarg);
      ss >> options.localport;
      eraseString(requiredOptions, "localport");

      break;
    }
    case 'a':
    {
      options.serveraddr = string(optarg);
      eraseString(requiredOptions, "serveraddr");

      break;
    }
    case 't':
    {
      string type = string(optarg);
      if (type == "server" || type == "s")
      {
        options.isServer = true;
      }
      else
      {
        options.isServer = false;
      }
      eraseString(requiredOptions, "type");

      break;
    }
    }
  }

  if (requiredOptions.size() != 0)
  {
    cout << "Option " << requiredOptions[0] << " must be set" << endl;
    return false;
  }
  return true;
}