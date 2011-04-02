#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger
{
  public:
  Logger();
  static const std::string S_ACCESS_FILE_PATH;
  static const std::string S_ERROR_FILE_PATH;

  bool open();
  void close();
  void logError(const std::string& msg);
  void logAccess(const std::string& msg);

  private:
  void log(FILE* file, const std::string& msg);

  FILE* m_accessFile;
  FILE* m_errorFile;
};

#endif
