#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <vector>
#include <map>

class Config
{

  public:

  static std::string S_KEY_ACCESS_FILE_PATH;
  static std::string S_KEY_ERROR_FILE_PATH;
  static std::string S_KEY_RUN_AS_USERNAME;
  static std::string S_KEY_BASE_PATH;
  static std::string S_KEY_GONE_PATH;

  Config(const std::string& filename);

  std::string getString(const std::string& key) const;

  private:
  void import();
  void parseLine(std::string line);

  static size_t S_READ_SIZE;
  static std::string S_WHITESPACE;

  std::string m_configFile;
  std::multimap<std::string, std::string> m_configValues;
};
#endif
