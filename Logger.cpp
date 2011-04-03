#include "Logger.h"
#include "Utils.h"
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

using std::string;

Logger::Logger(const Config& config) :
m_config(config),
m_accessFile(NULL),
m_errorFile(NULL)
{
}

bool Logger::open()
{
  string accessFilePath = m_config.getString(Config::S_KEY_ACCESS_FILE_PATH);
  string errorFilePath = m_config.getString(Config::S_KEY_ERROR_FILE_PATH);

  close();
  FILE* access = fopen(accessFilePath.c_str(), "a");
  if (access == NULL)
  {
    return false;
  }
  FILE* error = fopen(errorFilePath.c_str(), "a");
  if (error == NULL)
  {
    return false;
  }
  m_accessFile = access;
  m_errorFile = error;
  return true;
}

void Logger::close()
{
  if (m_accessFile != NULL)
  {
    fclose(m_accessFile);
    m_accessFile = NULL;
  }
  if (m_errorFile != NULL)
  {
    fclose(m_errorFile);
    m_errorFile = NULL;
  }
}

void Logger::logAccess(const string& msg)
{
  log(m_accessFile, msg);
}

void Logger::logError(const string& msg)
{
  log(m_errorFile, msg);
}

void Logger::log(FILE* file, const string& msg)
{
  if (file != NULL)
  {
    string logline = Utils::llToString(time(NULL)) + "\t" + msg + "\n";
    size_t bytesWritten = fwrite(logline.c_str(), 1, logline.size(), file);
    if (bytesWritten != logline.size())
    {
      // This wont actually go anywhere.
      printf("Log writing error %d (%s).\n", errno, strerror(errno));
    }
    else
    {
      fflush(file); // Can take this out for performance reasons if required.
    }
  }
}
