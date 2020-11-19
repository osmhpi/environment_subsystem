
#include "subsystem.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

/* translate_path - translate the given path and interpret the windows path
 * semantics. Volumes are for simplicity hardcoded to:
 *
 *   C -> /
 *   D -> home directory of the subsystem process
 */
static const char*
translate_path (const char *pathname)
{
  static char buf[PATH_MAX] = { 0 };

  // generate a path prefix from Volume
  const char *prefix;
  switch (pathname[0])
    {
      case 'C':
        prefix = "/";
        break;
      case 'D':
        prefix = getenv("HOME");
        break;
      default:
       return NULL;
    }

  // copy prefix and path to output buffer
  strcpy(buf, prefix);
  strcpy(buf + strlen(prefix), pathname + 2);

  // clean up forward / backward slash mess
  char *p = buf;
  while (*p++)
    if (*p == '\\')
      *p = '/';

  // return output buffer
  return buf;
}

/* main - start listening to a unix socket and accept connections, trying to
 * answer path translation requests by clients
 */
int
main (void)
{
  int s;
  if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
      perror("socket");
      return -1;
    }

  struct sockaddr_un local;
  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, SUBSYSTEM_SOCKET_PATH);
  unlink(SUBSYSTEM_SOCKET_PATH);
  size_t len = strlen(SUBSYSTEM_SOCKET_PATH) + sizeof(local.sun_family);

  if (bind(s, (struct sockaddr *)&local, len) == -1)
    {
      perror("bind");
      return -1;
    }

  if (listen(s, 5) == -1)
    {
      perror("listen");
      return -1;
    }

  // loop over client connections and keep answering requests
  while (1)
    {
      struct sockaddr_un remote;
      socklen_t t = sizeof(remote);

      int s2;
      if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1)
        {
          perror("accept");
          return -1;
        }

      static char buf[PATH_MAX] = { 0 };

      ssize_t n;
      if ((n = recv(s2, buf, PATH_MAX, 0)) == -1)
        {
          perror("recv");
          return -1;
        }
      buf[n] = '\0';

      printf("recv: %s\n", buf);
      const char *translated = translate_path(buf);
      printf("send: %s\n", translated);

      if (translated && send(s2, translated, strlen(translated), 0) == -1)
        {
          perror("send");
          return -1;
        }

      close(s2);
    }

  return 0;
}
