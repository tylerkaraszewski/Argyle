#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#include <string>
#include <map>

class HttpRequest
{
  public:

  typedef std::multimap<std::string, std::string> HeaderMap;

  HttpRequest();
  static void toLower(std::string& s);

  size_t parse(const std::string& buffer);
  bool isKeepAlive() const;
  const std::string& getPath() const;
  int getHttpError() const;
  void setHttpError(int httpError);
  const std::string& getUserAgent() const;
  std::string getHeader(const std::string& headerName) const;
  time_t getIfModifiedSince() const;

  private:

  static const std::string SPACES;
  static const std::string NEWLINES;
  static const std::string SEPARATOR;

  unsigned long long m_contentLength;
  bool m_keepAlive;
  std::string m_method;
  std::string m_path;
  std::string m_httpVersion;
  HeaderMap m_headers;
  std::string m_body;
  std::string m_queryString;
  int m_httpError;
  std::string m_userAgent;
  time_t m_ifModifiedSince;
};

#endif
