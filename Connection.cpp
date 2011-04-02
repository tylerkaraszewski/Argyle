#include "Connection.h"
#include "HttpRequest.h"
#include "Utils.h"
#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>

using std::string;

const size_t Connection::S_MAX_REQUEST_SIZE = 1024 * 8;
const size_t Connection::S_MAX_WRITE_SIZE = 1024 * 256;

Connection::Connection(const int socket, Logger& logger) :
m_socket(socket),
m_totalBytesToWrite(0),
m_bytesWritten(0),
m_currentRequest(0),
m_currentResponse(0),
m_isWriting(false),
m_logger(logger)
{
}

bool Connection::process(short int& pollEvents, short int& pollRevents)
{
  // Do writing first, since we can tell whether this will finish or not, and we don't want to block our writes on
  // infinite reads.
  if ((pollRevents & POLLOUT) && m_isWriting)
  {
    m_currentResponse->read(m_writeBuffer, S_MAX_WRITE_SIZE - m_writeBuffer.size());
    int bytesSent = send(m_socket, m_writeBuffer.c_str(), m_writeBuffer.size(), 0);
    if (bytesSent == -1)
    {
      if (errno == EPIPE)
      {
        return false; // try again, PIPEs happen.
      }
      m_logger.logError("Connection send error(" + Utils::llToString(errno) + "): " + strerror(errno));
    }
    m_writeBuffer = m_writeBuffer.substr(bytesSent);

    bool retval = false; // Assume we've got more to do unless we're done.
    if (m_currentResponse->done() && m_writeBuffer.empty())
    {
      m_logger.logAccess(Utils::llToString(m_currentResponse->getBytesRead()) + "\t" + 
                         Utils::llToString(m_currentResponse->getHttpResponseCode()) + "\t" + 
                         m_currentResponse->getPath() + "\t" + m_currentRequest->getUserAgent());
      if (m_currentRequest->isKeepAlive() && m_currentResponse->isKeepAlive())
      {
        pollEvents = POLLIN; // Ready for the next request.
      }
      else
      {
        pollEvents = 0; // Done with this connection.
        retval = true;
      }
      m_writeBuffer.resize(0);
      m_isWriting = false;
      delete m_currentResponse;
      m_currentResponse = NULL;
      delete m_currentRequest;
      m_currentRequest = NULL;
    }
    return retval;
  }

  // We don't do this while we're writing, because (for one) we want to keep our request object around till we're
  // finished with it, and we don't want to keep a whole list of requests.
  if (pollRevents & POLLIN)
  {
    char buffer[S_MAX_REQUEST_SIZE];
    int bytesRead = recv(m_socket, buffer, S_MAX_REQUEST_SIZE, 0);

    if (bytesRead == -1)
    {
      if (errno == EPIPE)
      {
        return false; // try again, PIPEs happen.
      }
      m_logger.logError("Recv error(" + Utils::llToString(errno) + "): " + strerror(errno));
      pollEvents = 0;
      return true;
    }

    if (bytesRead == 0)
    {
      return true; // EOF, done.
    }

    m_buffer.append(buffer, bytesRead);
    m_currentRequest = new HttpRequest();
    size_t bytesParsed = m_currentRequest->parse(m_buffer);
    if (bytesParsed == 0)
    {
      if (m_buffer.size() >= S_MAX_REQUEST_SIZE)
      {
        m_currentRequest->setHttpError(413);
      }
      else
      {
        return false;
      }
    }
    // proto-if-modified-since support.
    // @TODO: Make this actually do something useful.
    string ims = m_currentRequest->getHeader("if-modified-since");
    if (!ims.empty())
    {
      m_logger.logError("Debug: parsed '" + ims + "' to: " + Utils::llToString(m_currentRequest->getIfModifiedSince()));
    }

    m_currentResponse = new HttpResponse(*m_currentRequest, m_logger);
    m_buffer = m_buffer.substr(bytesParsed); // Trim this off our input buffer since we're done reading it.
    pollEvents = POLLOUT; // Now we want to know if we can write.
    m_isWriting = true;
  }
  
  return false;
}

Connection::~Connection()
{
  delete m_currentRequest;
  delete m_currentResponse;
}
