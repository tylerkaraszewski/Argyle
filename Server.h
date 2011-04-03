#ifndef SERVER_H
#define SERVER_H
#include <netinet/in.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include "Connection.h"
#include "Logger.h"
#include "Config.h"

class Server
{
  public:

  bool bindPort();

  // Blocks while the server runs. Returns true on graceful shutdown or false on failure.
  bool start();

  // Causes the blocking loop in 'start' to attempt to exit.
  void stop();

  // Constructor that takes a port number on which to listen.
  Server(short port, Logger& logger, const Config& config);
  ~Server();

  private:

  // Sets up the initial socket list that we'll poll on.
  void initSocketList();
  void watchSocket(int socket);
  
  // Takes a vector of file descriptors representing sockets we no longer want to keep track of.
  // These *must* appear in the same order in the vector in which they appear in m_sockets.
  void unwatchSockets(const std::vector<int>& socketsToUnwatch);

  int m_mainSocket;
  struct pollfd* m_sockets;
  int m_socketCount;
  const unsigned short m_port;
  bool m_shouldQuit;
  std::map<int, Connection*> m_connections;
  struct sockaddr_in m_address;
  Logger& m_logger;
  const Config& m_config;
};

#endif
