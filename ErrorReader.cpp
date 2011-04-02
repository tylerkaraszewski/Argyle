#include "ErrorReader.h"
#include "HttpResponse.h"
#include "Utils.h"

using std::string;

ErrorReader::ErrorReader(int httpErrorCode, const std::string& extraMsg, const std::string& extraHeaders)
{
  m_lengthKnown = true;
  m_done = false;
  m_httpCode = httpErrorCode;
  m_bytesRead = 0;
  
  string num = Utils::llToString(httpErrorCode);
  string msg = getErrorMessage(httpErrorCode);
  string extMsg = msg;
  if (!extraMsg.empty())
  {
    extMsg += "\n" + extraMsg;
  }
  extMsg += "\n";
  m_contentLength = extMsg.size();
  m_dataBuffer += "Content-Length: " + Utils::llToString(m_contentLength) + "\r\n";
  m_dataBuffer += "Content-Type: text/plain\r\n";
  m_dataBuffer += extraHeaders;
  m_dataBuffer += "\r\n";
  m_dataBuffer += extMsg;
}

size_t ErrorReader::read(std::string& buffer, const size_t maxBytesToRead)
{
  if (maxBytesToRead < m_dataBuffer.size())
  {
    buffer.append(m_dataBuffer.substr(0, maxBytesToRead));
    m_dataBuffer = m_dataBuffer.substr(maxBytesToRead);
    m_bytesRead += maxBytesToRead;
    return maxBytesToRead;
  }
  int size = m_dataBuffer.size();
  buffer.append(m_dataBuffer);
  m_dataBuffer.clear();
  m_done = true;
  m_bytesRead += size;
  return size;
}

string ErrorReader::getErrorMessage(int httpErrorCode)
{
  switch(httpErrorCode)
  {
    case(200): return "OK";
    case(300): return "Multiple Choices";
    case(301): return "Moved Permanently";
    case(302): return "Found";
    case(303): return "See Other";
    case(304): return "Not Modified";
    case(305): return "Use Proxy";
    case(306): return "";
    case(307): return "Temporary Redirect";
    case(400): return "Bad Request";
    case(401): return "Unauthorized";
    case(402): return "Payment Required";
    case(403): return "Forbidden";
    case(404): return "Not Found";
    case(405): return "Method Not Allowed";
    case(406): return "Not Acceptable";
    case(407): return "Proxy Authentication Required";
    case(408): return "Request Timeout";
    case(409): return "Conflict";
    case(410): return "Gone";
    case(411): return "Length Required";
    case(412): return "Precondition Failed";
    case(413): return "Request Entity Too Large";
    case(414): return "Request-URI Too Long";
    case(415): return "Unsupported Media Type";
    case(416): return "Requested Range Not Satisfiable";
    case(417): return "Expectation Failed";
    case(500): return "Internal Server Error";
    case(501): return "Not Implemented";
    case(502): return "Bad Gateway";
    case(503): return "Service Unavailable";
    case(504): return "Gateway Timeout";
    case(505): return "HTTP Version Not Supported";
    default:   return "Unknown Error";
  }
}

ErrorReader::~ErrorReader()
{
}
