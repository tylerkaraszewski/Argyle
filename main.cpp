#include "Server.h"
#include "Utils.h"
#include "Logger.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>

using std::string;

const char* username = "tyler";
Server* server = NULL;

// Currently handles SIGINT and SIGTERM.
void sigHandler(int signum)
{
  if (server != NULL)
  {
    server->stop();
  }
}

bool setUser(const char* runAs, Logger& logger)
{
  bool retval = true;
  struct passwd pwd;
  struct passwd* pwdPtr = 0;

  size_t bufsize = 32;
  char* buffer = static_cast<char*>(malloc(bufsize));
  if (buffer == NULL)
  {
    logger.logError("Setuser malloc error: Out of Memory");
    return false;
  }
  int e;
  while ((e = getpwnam_r(runAs, &pwd, buffer, bufsize, &pwdPtr)) == ERANGE)
  {
    bufsize *= 2;
    // this needs to do a better job of freeing things if it fails.
    buffer = static_cast<char*>(realloc(static_cast<void*>(buffer), bufsize));
    if (buffer == NULL)
    {
      logger.logError("Setuser realloc error: Out of Memory");
      return false;
    }
  }
  if (e != 0)
  {
    logger.logError("Getpwdnam_r error(" + Utils::llToString(errno) + "): " + strerror(errno));
    retval = false;
  }
  else if (pwdPtr == 0)
  {
    logger.logError(string("Getpwdnam_r failed, couldn't find user ") + username);
    retval = false;
  }

  // Haven't failed yet.
  if (retval)
  {
    // Set group first or we lose the abilty to after we're not superuser.
    if (setegid(pwd.pw_gid) == -1)
    {
      logger.logError("Setegid error(" + Utils::llToString(errno) + "): " + strerror(errno));
      retval = false;
    }
    else if (seteuid(pwd.pw_uid) == -1)
    {
      logger.logError("Seteuid error(" + Utils::llToString(errno) + "): " + strerror(errno));
      retval = false;
    }
  }
  free(buffer);
  return retval;
}

int main(int argc, char *argv[])
{
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sigHandler);
  signal(SIGTERM, sigHandler);

  if (daemon(0, 0) == -1)
  {
    return 5; // No logfiles yet.
  }

  Logger logger;
  logger.open();

  server = new Server(80, logger);
  if (!server->bindPort())
  {
    return 1;
  }
  if (!logger.open())
  {
    return 2;
  }
  else if (!setUser(username, logger))
  {
    return 3;
  }
  else if (!server->start())
  {
    return 4;
  }
  //@TODO: Set user back to root so Server destructor can clean up effectively (may not be nessecary?).
  delete server;
  server = NULL;
  logger.close();
  return 0;
}
