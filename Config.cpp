#include "Config.h"
#include "Utils.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

using std::string;
using std::vector;
using std::pair;

string Config::S_KEY_ACCESS_FILE_PATH = "AccessLogFile";
string Config::S_KEY_ERROR_FILE_PATH = "ErrorLogFile";
string Config::S_KEY_RUN_AS_USERNAME = "RunAsUserName";
string Config::S_KEY_BASE_PATH = "BasePath";
string Config::S_KEY_GONE_PATH = "GonePath";

size_t Config::S_READ_SIZE = 4096;
string Config::S_WHITESPACE = "\r\n\t ";

Config::Config(const string& filename) :
m_configFile(filename)
{
  import();
}

void Config::import()
{

  FILE* file = fopen(m_configFile.c_str(), "r");
  if (file == NULL)
  {
    return;
  }

  char buffer[S_READ_SIZE];
  bool doneReading = false;
  size_t remaining = 0;
  while (!doneReading)
  {
    size_t readSize = S_READ_SIZE - remaining;
    size_t bytesRead = fread(buffer + remaining, 1, readSize, file);
    if (bytesRead < readSize)
    {
      doneReading = true;
    }

    // parse what we've read so far.
    vector<string> lines;
    char* start = buffer;
    char* end = buffer;
    while (end < (buffer + bytesRead + remaining))
    {
      // Skip to a useful character.
      if ((*start == '\n') || (*start == '\r'))
      {
        start++;
        end++;
        continue;
      }

      end++;
      if ((*end == '\n') || (*end == '\r'))
      {
        lines.push_back(string(start, end - start));
        start = end;
      }
    }
    for (size_t i = 0; i < lines.size(); i++)
    {
      parseLine(lines[i]);
    }
    remaining = end - start;
    memmove(buffer, start, remaining);
  }
  // We still have one line in the buffer if it didn't have a terminating newline.
  parseLine(string(buffer, remaining));

  int fileError = ferror(file);
  if (fileError)
  {
    // Log?
  }
  fclose(file);
  return;
}

void Config::parseLine(string line)
{
  size_t front = line.find_first_not_of(S_WHITESPACE);
  if (front == line.npos)
  {
    return;
  }
  line = line.substr(front);
  line = line.substr(0, line.find_first_of("#"));
  line = line.substr(0, line.find_last_not_of(S_WHITESPACE) + 1);
  if (line.empty())
  {
    return;
  }

  // Now leading/trailing whitespace and comments are removed.

  string key;
  string value;
  size_t ws = line.find_first_of(S_WHITESPACE);
  if (ws == line.npos)
  {
    key = line;
  }
  else
  {
    key = line.substr(0, ws);
    value = line.substr(line.find_first_not_of(S_WHITESPACE, ws));
  }

  if ((key == S_KEY_ACCESS_FILE_PATH) ||
      (key == S_KEY_ERROR_FILE_PATH) ||
      (key == S_KEY_RUN_AS_USERNAME) ||
      (key == S_KEY_BASE_PATH)
  )
  {
    m_configValues.erase(key);
    m_configValues.insert(pair<string, string>(key, value));
  }
  else if(key == S_KEY_GONE_PATH)
  {
    m_configValues.insert(pair<string, string>(key, value));
  }
}

string Config::getString(const string& key) const
{
  std::multimap<std::string, std::string>::const_iterator it = m_configValues.find(key);
  if (it == m_configValues.end())
  {
    return "";
  }
  return it->second;
}
