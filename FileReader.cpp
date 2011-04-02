#include "FileReader.h"
#include "FilenameResolver.h"
#include "HttpResponse.h"
#include "Utils.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

using std::string;

FileReader::FileReader(const string& path, Logger& logger) :
m_file(NULL),
m_fileBytesRead(0),
m_logger(logger)
{
  m_lengthKnown = false;
  m_done = false;
  m_httpCode = 0;
  m_contentLength = 0;
  m_bytesRead = 0;

  if (openFile(path))
  {
    m_contentType = FilenameResolver::getContentType(path);
    m_headerBuffer += "Content-Length: " + Utils::llToString(m_contentLength) + "\r\n";
    m_headerBuffer += "Content-Type: " + m_contentType + "\r\n";
    m_headerBuffer += "\r\n";
    m_httpCode = 200;
  }

  // If openFile failed, it will have set an appropriate error code.
}

bool FileReader::openFile(const string& fullPath)
{

  // Look up this file's size.
  struct stat fileStat;
  if(stat(fullPath.c_str(), &fileStat) == -1)
  {
    m_httpCode = 404;
    return false;
  }

  // If we *can* find this file's info, then we need to open the actual file.
  m_file = fopen(fullPath.c_str(), "r");
  if (m_file == NULL)
  {
    if (errno == EACCES)
    {
      m_httpCode = 403;
      return false;
    }
    else if (errno == ENOENT)
    {
      m_httpCode = 404;
      return false;
    }
    else
    {
      m_logger.logError("FileReader fopen error(" + Utils::llToString(errno) + "): " + strerror(errno));
      m_httpCode = 500;
      return false;
    }
  }
  
  m_contentLength = fileStat.st_size;
  m_lengthKnown = true;

  // Now we're ready to start reading.
  return true;
}

size_t FileReader::read(string& buffer, const size_t maxBytesToRead)
{
  size_t bytesRead = 0;
  bytesRead += readFromInternalBufferString(m_headerBuffer, buffer, maxBytesToRead);
  if (bytesRead == maxBytesToRead)
  {
    m_bytesRead += bytesRead;
    return bytesRead;
  }

  if (m_file != NULL)
  {
    size_t remainingBytes = maxBytesToRead - bytesRead;
    char buf[remainingBytes];
    size_t fileBytes = fread(buf, 1, remainingBytes, m_file);
    if (fileBytes != remainingBytes)
    {
      // either EOF or error.
      if (!feof(m_file))
      {
        int err = ferror(m_file);
        m_logger.logError("FileReader fread error(" + Utils::llToString(err) + "): " + strerror(err));
      }
    }
    m_fileBytesRead += fileBytes;
    bytesRead += fileBytes;
    buffer.append(buf, fileBytes);

    if ((m_fileBytesRead == m_contentLength) || (fileBytes == 0))
    {
      m_done = true;
      fclose(m_file);
      m_file = NULL;
    }
  }
  else
  {
    m_done = true;
  }
  m_bytesRead += bytesRead;
  return bytesRead;
}

size_t FileReader::readFromInternalBufferString(string& internalBuffer, string& externalBuffer, const size_t maxBytes)
{
  int bytesRead = 0;
  if (internalBuffer.size() != 0)
  {
    string temp = internalBuffer.substr(0, maxBytes);
    bytesRead = temp.size();
    internalBuffer = internalBuffer.substr(bytesRead);
    externalBuffer += temp;
  }
  return bytesRead;
}

FileReader::~FileReader()
{
  if (m_file)
  {
    if (fclose(m_file))
    {
      m_logger.logError("FileReader fclose error(" + Utils::llToString(errno) + "): " + strerror(errno));
    }
    m_file = NULL;
  }
}
