#ifndef FILEREADER_H
#define FILEREADER_H

#include <string>
#include <stdio.h>
#include "DataReader.h"

class FileReader : public DataReader
{
  public:

  // File to read off disk. Handles resolving directory names and such.
  FileReader(const std::string& path, Logger& logger);

  virtual ~FileReader();

  // Reads from the data source and appends the data to 'buffer'. Returns number of bytes read. Can be called multiple
  // times. Also will 'read' headers and such, which do not come fro mthe data source directly.
  virtual size_t read(std::string& buffer, const size_t maxBytesToRead);

  private:

  // Returns true on successful FD open (and also sets m_fileDes). Returns false on failure and sets up the reader
  // to return an error message.
  bool openFile(const std::string& fullPath);
  size_t readFromInternalBufferString(std::string& internalBuffer, std::string& externalBuffer, const size_t maxBytes);

  FILE* m_file;
  size_t m_fileBytesRead;

  std::string m_contentType;
  std::string m_headerBuffer;
  Logger& m_logger;
};

#endif
