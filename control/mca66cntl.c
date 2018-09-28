#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mca66.h>

/* Controller for mca66 */
int main(int argc, char *argv[])
{
  int opt;
  char* device;
  int debug = false;
  
  while ((opt = getopt (argc, argv, "Dht:")) != -1) {
    switch (opt)
      {
      case 't':
      	device = optarg;
        break;
      case 'D':
        debug = true;
        break;
      case 'h':
      	fprintf(stderr, "usage: %s [-D] [-t TTY device] command\n", argv[0]);
      	return -2;
      	break;
      default:
        abort();
      }
  }
  
  init_audio();

  return 0;
}
