#ifndef DATAREADER_H
#define DATAREADER_H

#include "Logger.h"
#include <string>

class DataReader
{
  public:

  virtual ~DataReader();

  // Reads from the data source and appends the data to 'buffer'. Returns number of bytes read. Can be called multiple
  // times. Also will 'read' headers and such, which do not come fro mthe data source directly.
  virtual size_t read(std::string& buffer, const size_t maxBytesToRead) = 0;

  // Returns true if there is no more data left to read.
  virtual bool done();

  // Returns 'true' if we know the length of the object we're returning ahead of time. Will be set by the
  // constructor.
  virtual bool lengthKnown();

  // 0 if not finished yet.
  virtual int getHttpResponseCode();

  size_t getBytesRead();

  protected:
  size_t m_contentLength;
  bool m_lengthKnown;
  bool m_done;
  int m_httpCode;
  size_t m_bytesRead; // *INCLUDES HEADER BYTES*
};

#endif
