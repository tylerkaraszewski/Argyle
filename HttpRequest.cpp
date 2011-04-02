#include "HttpRequest.h"
#include "Utils.h"
#include <algorithm>

using std::string;
using std::pair;

const string HttpRequest::SPACES = "\t ";
const string HttpRequest::NEWLINES = "\r\n";
const string HttpRequest::SEPARATOR = ":";

HttpRequest::HttpRequest() :
m_contentLength(0),
m_keepAlive(false),
m_httpError(0),
m_ifModifiedSince(0)
{
}

size_t HttpRequest::parse(const string& buffer)
{
  size_t headerEnd = buffer.find("\r\n\r\n");
  if (headerEnd == buffer.npos)
  {
    return 0;
  }

  // Ok, now we parse this.
  size_t startOffset = 0;
  size_t endOffset = 0;

  // Special handling for the first line.
  endOffset = buffer.find_first_of(SPACES);
  if (endOffset == buffer.npos)
  {
    m_httpError = 400;
    return headerEnd + 4;
  }
  m_method = buffer.substr(startOffset, endOffset);
  startOffset = buffer.find_first_not_of(SPACES, endOffset);
  if (startOffset == buffer.npos)
  {
    m_httpError = 400;
    return headerEnd + 4;
  }
  endOffset = buffer.find_first_of(SPACES, startOffset);
  if (endOffset == buffer.npos)
  {
    m_httpError = 400;
    return headerEnd + 4;
  }
  m_path = buffer.substr(startOffset, endOffset - startOffset);
  startOffset = buffer.find_first_not_of(SPACES, endOffset);
  if (startOffset == buffer.npos)
  {
    m_httpError = 400;
    return headerEnd + 4;
  }
  endOffset = buffer.find_first_of(NEWLINES, startOffset);
  if (endOffset == buffer.npos)
  {
    m_httpError = 400;
    return headerEnd + 4;
  }
  m_httpVersion = buffer.substr(startOffset, endOffset - startOffset);

  size_t pathOffset = m_path.find_first_of("#");
  if (pathOffset != m_path.npos)
  {
    m_path = m_path.substr(0, pathOffset);
  }
  pathOffset = m_path.find_first_of("?");
  if (pathOffset != m_path.npos)
  {
    m_queryString = m_path.substr(pathOffset);
    m_path = m_path.substr(0, pathOffset);
  }

  // Now we loop across all the headers we've got.
  startOffset = buffer.find_first_not_of(NEWLINES, endOffset);
  while (endOffset < headerEnd)
  {
    endOffset = buffer.find_first_of(SEPARATOR, startOffset);
    if (endOffset == buffer.npos)
    {
      break;
    }
    string headerName = buffer.substr(startOffset, endOffset - startOffset);
    headerName = headerName.substr(0, headerName.find_last_not_of(SPACES) + 1);
    startOffset = buffer.find_first_not_of(SPACES, endOffset + 1);
    if (startOffset == buffer.npos)
    {
      break;
    }
    endOffset = buffer.find_first_of(NEWLINES, startOffset);
    if (endOffset == buffer.npos)
    {
      break;
    }
    string value = buffer.substr(startOffset, endOffset - startOffset);
    startOffset = buffer.find_first_not_of(NEWLINES, endOffset);
    while ((buffer.find_first_of(SPACES) == startOffset) && (startOffset < headerEnd))
    {
      endOffset = buffer.find_first_of(NEWLINES, startOffset);
      if (endOffset == buffer.npos)
      {
        break;
      }
      value += buffer.substr(startOffset, endOffset - startOffset);
      startOffset = buffer.find_first_not_of(NEWLINES, endOffset);
      if (startOffset == buffer.npos)
      {
        break;
      }
    }

    toLower(headerName);
    if (headerName == "user-agent")
    {
      m_userAgent = value;
    }
    else if (headerName == "if-modified-since")
    {
      m_ifModifiedSince = Utils::parseHttpDate(value);
    }
    m_headers.insert(pair<string, string>(headerName, value));
  }

  // Set our keep-alive flag.
  HeaderMap::const_iterator it = m_headers.find("connection");
  {
    string c = it->second;
    toLower(c);
    if (c.find("keep-alive") != c.npos)
    {
      m_keepAlive = true;
    }
  }

  // Now we need to parse the request body.
  it = m_headers.find("content-length");
  if (it != m_headers.end())
  {
    const string& l = it->second;
    m_contentLength = strtoll(l.c_str(), NULL, 10);
  }

  // '4' is the \r\n\r\n following the headers.
  unsigned long long totalSize = headerEnd + 4 + m_contentLength;
  if (buffer.size() >= totalSize)
  {
    m_body = buffer.substr(headerEnd + 4, m_contentLength);
  }
  else
  {
    totalSize = 0;
  }

  return totalSize;
}

void HttpRequest::toLower(string& s)
{
  transform(s.begin(), s.end(), s.begin(), ::tolower);
}

bool HttpRequest::isKeepAlive() const
{
  return m_keepAlive;
}

const string& HttpRequest::getPath() const
{
  return m_path;
}

int HttpRequest::getHttpError() const
{
  return m_httpError;
}

void HttpRequest::setHttpError(int httpError)
{
  m_httpError = httpError;
}

const string& HttpRequest::getUserAgent() const
{
  return m_userAgent;
}

time_t HttpRequest::getIfModifiedSince() const
{
  return m_ifModifiedSince;
}

string HttpRequest::getHeader(const string& headerName) const
{
  string lcHeader = headerName;
  toLower(lcHeader);
  HeaderMap::const_iterator it = m_headers.find(lcHeader);
  if (it == m_headers.end())
  {
    return "";
  }
  else
  {
    return it->second;
  }
}
