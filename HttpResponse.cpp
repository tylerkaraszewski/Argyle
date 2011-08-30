#include "HttpResponse.h"
#include "FilenameResolver.h"
#include "FileReader.h"
#include "ErrorReader.h"
#include "Utils.h"
#include <stdlib.h>
#include <time.h>

using std::string;
string HttpResponse::S_CGI_EXT = ".cgi";
string HttpResponse::S_STANDARD_HEADERS = "Server: argyle\r\nCache-Control: max-age=600\r\n";

HttpResponse::HttpResponse(const HttpRequest& request, Logger& logger, const Config& config) :
m_request(request),
m_keepAlive(true),
m_reader(NULL),
m_config(config)
{
  if (request.getHttpError())
  {
    m_reader = new ErrorReader(request.getHttpError());
    m_keepAlive = false; // If we had an error while parsing this request, we reset the whole connection.
    return;
  }

  string redirectPath;
  time_t mtime = 0;
  m_path = FilenameResolver::resolve(unescapeUri(m_request.getPath()), m_config, redirectPath, &mtime);
  if (!redirectPath.empty())
  {
    m_reader = new ErrorReader(301, "", "Location: " + redirectPath + "\r\n");
    m_path = redirectPath;
  }
  else if (m_path.empty())
  {
    m_path = m_request.getPath();
    if (isGone(m_path))
    {
      m_reader = new ErrorReader(410);
    }
    else
    {
      m_reader = new ErrorReader(404);
    }
  }
  else if (!pathLooksSafe(m_path))
  {
    m_reader = new ErrorReader(400);
  }
  else if (pathIsCgiScript(m_path))
  {
    // @TODO: Add 'CgiReader' class.
    m_reader = new ErrorReader(501, "CGI Not Yet Implemented.");
  }
  else
  {
    m_reader = new FileReader(m_path, logger);
    int code = m_reader->getHttpResponseCode();
    if (code != 200)
    {
      delete m_reader;
      m_reader = new ErrorReader(code);
    }
  }
  int code = m_reader->getHttpResponseCode();
  m_keepAlive = m_reader->lengthKnown();
  m_headerBuffer = "HTTP/1.1 " + Utils::llToString(code) + " " + ErrorReader::getErrorMessage(code) + "\r\n";
  m_headerBuffer += S_STANDARD_HEADERS;
  if (mtime != 0)
  {
    m_headerBuffer += "Last-Modified: " + Utils::getHttpDate(mtime) + "\r\n";
  }
}

size_t HttpResponse::read(string& buffer, const size_t maxBytesToRead)
{
  size_t remaining = maxBytesToRead;
  if (!m_headerBuffer.empty())
  {
    string temp = m_headerBuffer.substr(0, maxBytesToRead);
    buffer.append(temp);
    remaining -= temp.size();
    if (remaining == 0)
    {
      return temp.size();
    }
  }
  return m_reader->read(buffer, remaining);
}

bool HttpResponse::done()
{
  return m_reader->done();
}

string HttpResponse::unescapeUri(const string& escapedUri)
{
  string unescaped;
  if (escapedUri.size() == 0)
  {
    return unescaped;
  }
  for (size_t i = 0; i < escapedUri.size(); i++)
  {
    if (escapedUri[i] == '%')
    {
      if (escapedUri.size() >= (i + 3))
      {
        string escapedChar = escapedUri.substr(i + 1, 2);
        long value = strtol(escapedChar.c_str(), NULL, 16);
        unescaped += static_cast<unsigned char>(value);
        i += 2; // skip the encoded block now.
      }
      else
      {
        return unescaped; // Error, return what we've got.
      }
    }
    else
    {
      unescaped += escapedUri[i];
    }
  }
  return unescaped;
}

bool HttpResponse::pathLooksSafe(const string& path)
{
  if (path.size() == 0)
  {
    return false;
  }
  if (path[0] != '/')
  {
    return false;
  }
  if (path.find("/../") != path.npos)
  {
    return false;
  }
  for (size_t i = 0; i < path.size(); ++i)
  {
    if ((path[i] < 32) || (path[i] > 126))
    {
      return false;
    }
  }
  return true;
}

bool HttpResponse::pathIsCgiScript(const string& path)
{
  return path.rfind(S_CGI_EXT) == (path.size() - S_CGI_EXT.size());
}

bool HttpResponse::isKeepAlive() const
{
  return m_keepAlive;
}

HttpResponse::~HttpResponse()
{
  delete m_reader;
}

const std::string& HttpResponse::getPath()
{
  return m_path;
}

int HttpResponse::getHttpResponseCode()
{
  return m_reader->getHttpResponseCode();
}

size_t HttpResponse::getBytesRead()
{
  return m_reader->getBytesRead();
}

bool HttpResponse::isGone(const string& path)
{
  if (
    (path == "/boat/rss.xml") ||
    (path == "/Sparrow.html") ||
    (path == "/atom.xml"))
  {
    return true;
  }
  return false;
}
