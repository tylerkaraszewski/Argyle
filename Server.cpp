#include "Server.h"
#include "Utils.h"
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

using std::vector;
using std::map;

Server::Server(short port, Logger& logger, const Config& config) :
m_mainSocket(0),
m_sockets(0),
m_socketCount(0),
m_port(port),
m_shouldQuit(false),
m_logger(logger),
m_config(config)
{
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(m_port);
  m_address.sin_addr.s_addr = htonl(INADDR_ANY);
}

bool Server::bindPort()
{
  m_mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (m_mainSocket == -1)
  {
    m_logger.logError("Socket error(" + Utils::llToString(errno) + "): " + strerror(errno));
    return false;
  }

  int reuseAddr = 1;
  if (setsockopt(m_mainSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseAddr, sizeof(reuseAddr)))
  {
    m_logger.logError("Setsockopt error(" + Utils::llToString(errno) + "): " + strerror(errno));
    close(m_mainSocket);
    return false;
  }
  
  if (bind(m_mainSocket, (struct sockaddr*)&m_address, sizeof(m_address)))
  {
    m_logger.logError("Bind error(" + Utils::llToString(errno) + "): " + strerror(errno));
    close(m_mainSocket);
    return false;
  }
  return true;
}

bool Server::start()
{
  if (listen(m_mainSocket, 10))
  {
    m_logger.logError("Listen error(" + Utils::llToString(errno) + "): " + strerror(errno));
    close(m_mainSocket);
    return false;
  }

  initSocketList();

  while (m_shouldQuit == false)
  {
    int n = poll(m_sockets, m_socketCount, 1000);
    if (n == -1)
    {
      if (errno == EINTR)
      {
        // This happens when we try and exit. For a legit exit, m_shouldQuit should be set. For some other signal,
        // we'll just try and continue.
        continue;
      }
      m_logger.logError("Poll error(" + Utils::llToString(errno) + "): " + strerror(errno));
      close(m_mainSocket);
      return false;
    }
    // No socket activity, just start the next loop.
    if (n == 0)
    {
      continue;
    }

    int socketToWatch = -1;
    vector<int> socketsToUnwatch;
    for (int i = 0; i < m_socketCount; ++i)
    {
      // General error check.
      if (m_sockets[i].revents & POLLERR)
      {
        socketsToUnwatch.push_back(m_sockets[i].fd);
        continue;
      }
      if (m_sockets[i].revents & POLLHUP)
      {
        socketsToUnwatch.push_back(m_sockets[i].fd);
        continue;
      }
      if (m_sockets[i].revents & POLLNVAL)
      {
        socketsToUnwatch.push_back(m_sockets[i].fd);
        continue;
      }

      // Special socket.
      if (m_sockets[i].fd == m_mainSocket)
      {
        if (m_sockets[i].revents & POLLIN)
        {
          struct sockaddr_in newAddress;
          socklen_t newAddressSize = sizeof(newAddress);
          int newSocket = accept(m_mainSocket, (struct sockaddr*)&m_address, &newAddressSize);
          if (newSocket != -1)
          {
            socketToWatch = newSocket;
          }
          else
          {
            m_logger.logError("Accept error(" + Utils::llToString(errno) + "): " + strerror(errno));
          }
        }
      }
      else
      {
        bool isFinished = m_connections[m_sockets[i].fd]->process(m_sockets[i].events, m_sockets[i].revents);
        if (isFinished)
        {
          socketsToUnwatch.push_back(m_sockets[i].fd);
        }
      }
    }

    // @TODO: Timeout expiration goes here (or possibly after the 'unwatchSockets' call below).

    // Rebuild our socket list if anything has changed.
    unwatchSockets(socketsToUnwatch);
    watchSocket(socketToWatch);
  }
  return true;
}

void Server::stop()
{
  m_shouldQuit = true;
}

void Server::initSocketList()
{
  m_socketCount = 1;
  free(m_sockets);
  m_sockets = (struct pollfd*)malloc(sizeof(struct pollfd) * m_socketCount);
  if (m_sockets == 0)
  {
    m_logger.logError("InitSocketList malloc error: Out of Memory");
    exit(1);
  }
  m_sockets[0].fd = m_mainSocket;
  m_sockets[0].events = POLLIN;
  m_sockets[0].revents = 0;
}

void Server::watchSocket(int socket)
{
  if (socket < 0)
  {
    // not a real socket, do nothing.
    return;
  }


  m_socketCount++;
  m_sockets = (struct pollfd*)realloc(m_sockets, sizeof(struct pollfd) * m_socketCount);
  if (m_sockets == 0)
  {
    // Try and log the error and close the socket. Maybe someone will free some RAM.
    // In all likelihood, we're hosed if we even get here, though.
    m_logger.logError("InitSocketList realloc error: Out of Memory");
    shutdown(socket, SHUT_RDWR);
    close(socket);
    return;
  }
  m_sockets[m_socketCount - 1].fd = socket;
  m_sockets[m_socketCount - 1].events = POLLIN;
  m_sockets[m_socketCount - 1].revents = 0;
  m_connections[socket] = new Connection(socket, m_logger, m_config);
}

void Server::unwatchSockets(const vector<int>& socketsToUnwatch)
{
  if (socketsToUnwatch.size() == 0)
  {
    return;
  }

  int shiftBy = 0;
  vector<int>::const_iterator it = socketsToUnwatch.begin();
  for (int i = 0; i < m_socketCount; ++i)
  {
    // If this is one of the sockets we're closing, we'll do that.
    if (m_sockets[i].fd == *it)
    {
      shutdown(*it, SHUT_RDWR);
      close(*it);
      delete m_connections[*it];
      m_connections.erase(*it);
      ++shiftBy;
      ++it;
    }

    // Otherwise, we shift our array entries down.
    else if(shiftBy != 0)
    {
      memcpy(&m_sockets[i - shiftBy], &m_sockets[i], sizeof(struct pollfd));
    }
  }
  m_socketCount -= shiftBy;
  m_sockets = (struct pollfd*)realloc(m_sockets, sizeof(struct pollfd) * m_socketCount);
  if (m_sockets == 0)
  {
    // Seems pretty much impossible since we're *shrinking* it, but whatever.
    m_logger.logError("Unwatchsockets realloc error.");
  }
}

Server::~Server()
{
  vector<int> socketsToUnwatch;
  for (int i = 0; i < m_socketCount; ++i)
  {
    socketsToUnwatch.push_back(m_sockets[i].fd);
  }
  unwatchSockets(socketsToUnwatch);
  free(m_sockets);
}
