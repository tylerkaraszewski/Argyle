#ifndef LOGGER_H
#define LOGGER_H

#include "Config.h"
#include <string>

class Logger
{
  public:
  Logger(const Config& config);

  bool open();
  void close();
  void logError(const std::string& msg);
  void logAccess(const std::string& msg);

  private:
  void log(FILE* file, const std::string& msg);

  const Config& m_config;
  FILE* m_accessFile;
  FILE* m_errorFile;
};

#endif
