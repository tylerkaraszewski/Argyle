#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include "HttpRequest.h"
#include "DataReader.h"
#include "Config.h"

class HttpResponse
{
  public:
  static std::string S_CGI_EXT;
  static std::string S_STANDARD_HEADERS;

  HttpResponse(const HttpRequest& request, Logger& logger, const Config& config);
  ~HttpResponse();

  static std::string unescapeUri(const std::string& escapedUri);

  bool isKeepAlive() const;
  size_t read(std::string& buffer, const size_t maxBytesToRead);
  bool done();
  const std::string& getPath();
  int getHttpResponseCode();
  size_t getBytesRead();

  private:

  // Sets us up with a DataReader, headers and a content-length, or, if no Content-Length is availabe, sets us to not be
  // a keep-alive response.
  void prepareData();
  static bool isGone(const std::string& path);

  static bool pathLooksSafe(const std::string& path);
  static bool pathIsCgiScript(const std::string& path);

  const HttpRequest& m_request;
  bool m_keepAlive;
  DataReader* m_reader;
  std::string m_path;
  std::string m_headerBuffer;
  const Config& m_config;
};

#endif
