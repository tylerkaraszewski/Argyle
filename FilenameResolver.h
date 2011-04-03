#ifndef FILENAME_RESOLVER_H
#define FILENAME_RESOLVER_H

#include "Config.h"
#include <string>
#include <time.h>

class FilenameResolver
{
  public:

  // Based on the path given, attempts to resolve that to a file on disk. If possible, returns a string represnting the
  // actual file to read. If not possible, returns an empty string.
  static std::string resolve(const std::string& webPath, const Config& config,
                             std::string& redirectPath, time_t* mtime);

  // Gets a content-type from a filename. Doesn't do any fancy inspection of the actual file, just looks at filename
  // extension.
  static std::string getContentType(const std::string& filename);

  private:
  static std::string resolveDir(const std::string& dirPath, const Config& config, time_t* mtime);
  static std::string findNewestInDir(const std::string& webPath, const Config& config);

  std::string m_basePath;
};

#endif
