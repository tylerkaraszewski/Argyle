#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <string>

class Utils
{
  public:

  static std::string llToString(long long i);
  static std::string UriEscape(const std::string& in);
  static time_t parseHttpDate(const std::string& dateStr);
  static std::string getHttpDate(time_t secondsSinceEpoch);
};

#endif
