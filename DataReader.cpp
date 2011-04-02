#include "DataReader.h"
#include "FilenameResolver.h"
using std::string;

bool DataReader::done()
{
  return m_done;
}

bool DataReader::lengthKnown()
{
  return m_lengthKnown;
}

int DataReader::getHttpResponseCode()
{
  return m_httpCode;
}

size_t DataReader::getBytesRead()
{
  return m_bytesRead;
}

DataReader::~DataReader()
{
}
