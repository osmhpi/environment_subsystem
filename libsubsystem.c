
#include "subsystem.h"

#define _GNU_SOURCE 1 // needed on linux for RTLD_NEXT in dlfcn.h
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

/* translate_path - this is a socket client that sends a path to a unx socket
 * and receives (and returns) a translated one.
 *
 * note that this is not threadsafe due to the static char buffer.
 */
static const char*
translate_path (const char *pathname)
{
  static char buffer[PATH_MAX] = { 0 };

  int s;
  if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    return NULL;

  struct sockaddr_un remote;
  remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, SUBSYSTEM_SOCKET_PATH);
  size_t len = strlen(SUBSYSTEM_SOCKET_PATH) + sizeof(remote.sun_family);

  if (connect(s, (struct sockaddr*)&remote, len) == -1)
    return NULL;

  if (send(s, pathname, strlen(pathname), 0) == -1)
    {
      close(s);
      return NULL;
    }

  ssize_t t;
  if ((t = recv(s, buffer, PATH_MAX, 0)) == -1)
    {
      close(s);
      return NULL;
    }
  buffer[t] = '\0';

  close(s);
  return buffer;
}

/* open - this is used by the linker instead of the glibc version
 */
int
open (const char *pathname, int flags)
{
  int(*f)(const char*, int) = dlsym(RTLD_NEXT, "open");
  if (f == NULL)
    return -1;

  // translat the path
  const char *translated = translate_path(pathname);
  if (translated == NULL)
    return -1;

  // open the file and return
  return f(translated, flags);
}

/* PrintFile - map a file to memory and print it to stdout
 */
int
PrintFile (int fd)
{
  // stat the file to get its size
  struct stat finfo;
  if (fstat(fd, &finfo) == -1)
    return -1;

  // if size is 0 there is nothing to print
  if (finfo.st_size == 0)
    return 0;

  // map the file
  char *data = mmap(0, finfo.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED)
    return -1;

  // print file to stdout
  fputs(data, stdout);

  // unmap and release the memory
  munmap(data, finfo.st_size);
  return 0;
}
