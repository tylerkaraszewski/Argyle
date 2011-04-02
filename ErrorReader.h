#ifndef ERRORREADER_H
#define ERRORREADER_H

#include <string>
#include "DataReader.h"

class ErrorReader : public DataReader
{
  public:

  // If "extraMessage" is set, it will be appended to the message body, which is normally jsut the error status.
  // If "extraHeaders" is set, they'll be appended to the message headers. Note that they must be terminated properly
  // with '\r\n' at the end of each line.
  ErrorReader(int httpErrorCode, const std::string& extraMsg = "", const std::string& extraHeaders = "");

  virtual ~ErrorReader();

  virtual size_t read(std::string& buffer, const size_t maxBytesToRead);

  static std::string getErrorMessage(int httpErrorCode);

  private:

  std::string m_dataBuffer;
};

#endif
