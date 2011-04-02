#include "Utils.h"
#include <stdio.h>

using std::string;

string Utils::llToString(long long i)
{
  string s;
  char str[32] = {0};
  size_t strLen = snprintf(str, 31, "%lli", i);
  s.append(str, strLen);
  return s;
}

string Utils::UriEscape(const string& in)
{
  string out;
  for (size_t i = 0; i < in.size(); i++)
  {
    char current = in[i];
    if (
         ((current >= 65) && (current <= 90)) || // A-Z
         ((current >= 97) && (current <= 122)) || // a-z
         ((current >= 48) && (current <= 57)) || // 0-9
         (current == '$') || 
         (current == '&') || 
         (current == '+') || 
         (current == ',') || 
         (current == '/') || 
         (current == ':') || 
         (current == ';') || 
         (current == '=') || 
         (current == '?') || 
         (current == '@') || 
         (current == '-') || 
         (current == '_') || 
         (current == '.') || 
         (current == '+') || 
         (current == '!') || 
         (current == '*') || 
         (current == '(') || 
         (current == ')') ||
         (current == '\'')
       )
    {
      out += current;
    }
    else
    {
      char str[4] = {0};
      snprintf(str, 4, "%%%02hhx", current);
      out += str;
    }
  }
  return out;
}

time_t Utils::parseHttpDate(const string& dateStr)
{
  // strptime onmy ubuntu box ignores the timezone parameter at the end of the format string.
  // This isn't the case on OS X, so we keep it for that platform.
  #ifdef __linux__
    #define REQ_INPUT_LEN 26
  #else
    #define REQ_INPUT_LEN 29
  #endif

  struct tm date;
  const char* in = dateStr.c_str();
  char* parsed = strptime(in, "%a, %d %b %Y %T %Z", &date);
  if (parsed != (in + REQ_INPUT_LEN))
  {
    return 0;
  }
  return timegm(&date);
}

string Utils::getHttpDate(time_t secondsSinceEpoch)
{
  char buffer[30] = {0};
  struct tm* date = gmtime(&secondsSinceEpoch);
  strftime(buffer, 30,  "%a, %d %b %Y %T %Z", date);
  return buffer;
}
