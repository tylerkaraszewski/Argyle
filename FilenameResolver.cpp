#include "FilenameResolver.h"
#include "HttpRequest.h"
#include "Utils.h"
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using std::string;

#ifdef __linux__
const string FilenameResolver::S_BASE_PATH = "/home/tyler/website/cooked";
#else
const string FilenameResolver::S_BASE_PATH = "/Users/tyler/Sites/newsite/cooked";
#endif

string FilenameResolver::resolve(const string& webPath, string& redirectPath, time_t* mtime)
{
  if (mtime != NULL)
  {
    *mtime = 0; // We'll set this to something useful later if we can. Setting it to 0 implies "N/A".
  }

  bool needToRedirectIfDir = false;
  // Trim trailing slashes.
  string processedWebPath = webPath.substr(0, webPath.find_last_not_of('/') + 1);
  if (processedWebPath.size() == webPath.size())
  {
    // Nothing was stripped, indicating if a directory, it was requested like a file.
    needToRedirectIfDir = true;
  }
  string fullPath = S_BASE_PATH + processedWebPath;

  // Look our path to see if it exists, and if so, what we should do with it.
  struct stat fileStat;
  if(stat(fullPath.c_str(), &fileStat) == -1)
  {
    fullPath = "";
  }

  // Is it a directory?
  else if (fileStat.st_mode & S_IFDIR)
  {
    if (needToRedirectIfDir)
    {
      redirectPath = Utils::UriEscape(webPath + "/");
      fullPath = "";
    }
    else
    {
      fullPath = resolveDir(processedWebPath, mtime);
    }
  }

  // Is it something besides a normal file?
  else if (!(fileStat.st_mode & S_IFREG))
  {
    fullPath = "";
  }

  // Report our modified time if the caller gave us a place to put it (and if we're not operating on a directory)
  // If we are looking at a directory, resolveDir should have already set this if it resolved to a particular file.
  // If it's a 404, we want to leave it blank anyway.
  if ((mtime != NULL) && !(fileStat.st_mode & S_IFDIR))
  {
    *mtime = fileStat.st_mtime;
  }

  // Normal files need no special handling.
  return fullPath;
}


string FilenameResolver::resolveDir(const string& dirPath, time_t* mtime)
{
  string temp;
  string rPath = dirPath;
  rPath = resolve(dirPath + "/main.html", temp, mtime);
  if (!rPath.empty())
  {
    return rPath;
  }

  return findNewestInDir(dirPath);
}

string FilenameResolver::findNewestInDir(const std::string& webPath)
{
  string fullPath = S_BASE_PATH + webPath;
  char * name = 0;
  DIR * cwd = opendir(fullPath.c_str());
  struct dirent * entry;
  time_t timestamp = 0;

  struct tm time;
  time.tm_sec = 0;
  time.tm_min = 0;
  time.tm_hour = 12;
  time.tm_wday = 0;
  time.tm_yday = 0;
  time.tm_isdst = 0;

  while((entry = readdir(cwd)))
  {
    // Skip files that are too short to count.
    if(strlen(entry->d_name) < 10)
    {
      continue;
    }

    int day = atoi(entry->d_name + 8);
    int mon = atoi(entry->d_name + 5);
    int year = atoi(entry->d_name);

    // Sanity check.
    if ((year < 1900) || (year > 2500) || (mon < 1) || (mon > 12))
    {
      continue;
    }

    // Because of how these are stored in a struct tm.
    mon--;
    year -= 1900;

    // Parse our date.
    time.tm_mday = day;
    time.tm_mon = mon;
    time.tm_year = year;

    time_t filestamp = mktime(&time);
    if(filestamp > timestamp)
    {
        timestamp = filestamp;
        name = entry->d_name;
    }
  }

  closedir(cwd);
  if (name != 0)
  {
    return fullPath + "/" + name;
  }
  return "";
}

string FilenameResolver::getContentType(const string& filename)
{
  string contentType = "text/plain";
  size_t offset = filename.find_last_of('.');
  string ext = filename.substr(offset + 1);
  HttpRequest::toLower(ext);
  if (ext == "html")
    contentType = "text/html";
  else if (ext == "gz")
    contentType = "application/x-compressed";
  else if (ext == "avi")
    contentType = "video/avi";
  else if (ext == "mp4")
    contentType = "video/mp4";
  else if (ext == "zip")
    contentType = "application/zip";
  else if (ext == "js")
    contentType = "application/x-javascript";
  else if (ext == "css")
    contentType = "text/css";
  else if (ext == "png")
    contentType = "image/png";
  else if (ext == "jpg")
    contentType = "image/jpeg";
  else if (ext == "xml")
    contentType = "application/xml";
  else if (ext == "pdf")
    contentType = "application/pdf";
  else if (ext == "ico")
    contentType = "image/png"; // hack for favicons.

  return contentType;
}

