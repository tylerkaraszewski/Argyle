#ifndef CONNECTION_H
#define CONNECTION_H
#include <string>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Logger.h"

class Connection
{
  public:
  Connection(const int socket, Logger& logger);
  ~Connection();
  bool process(short int& pollEvents, short int& pollRevents);

  private:

  static const size_t S_MAX_REQUEST_SIZE;
  static const size_t S_MAX_WRITE_SIZE;

  const int m_socket;
  std::string m_buffer;

  size_t m_totalBytesToWrite;
  size_t m_bytesWritten;
  HttpRequest* m_currentRequest;
  HttpResponse* m_currentResponse;
  bool m_isWriting;
  std::string m_writeBuffer;
  Logger& m_logger;
};

#endif
