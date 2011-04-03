#include "Config.h"
#include "Logger.h"
#include "Utils.h"

void Config::import(const string& filename)
{

  FILE* file = fopen(filename.c_str(), "r");
  if (file == NULL)
  {
    m_logger.logError("Config file read error(" + Utils::llToString(errno) + "): " + strerror(errno));
    return;
  }

}
