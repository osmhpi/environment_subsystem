
#include "subsystem.h"

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  // check for command line arguments, print usage string if none present
  if (argc == 1)
    {
      fprintf(stderr, "usage: %s [FILE...]\n", argv[0]);
      return 2;
    }

  int res = 0;

  // iterate over command line arguments, open and print each one
  int i;
  for (i = 1; i < argc; ++i)
    {
      int fd = open(argv[i], O_RDONLY);
      if (fd == -1)
        {
          fprintf(stderr, "open: %s: %s\n", argv[i], strerror(errno));
          res = 1;
          continue;
        }

      PrintFile(fd);
      close(fd);
    }

  // indicate failure to the OS if one file failed to open
  return res;
}
