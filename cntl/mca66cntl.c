#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mca66.h>

/* Controller for mca66 */
int main(int argc, char *argv[])
{
	int opt;
	char* device = "/dev/ttyUSB0";
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
				usage:
				fprintf(stderr, "usage: %s [-D] [-t TTY device] command\n", argv[0]);
				return -2;
				break;
			default:
				abort();
			}
	}
	
	// Only one command allowed at a time
	if(argc != 2)
		goto usage;
	
	init_audio(device, debug);
	
	// Process commands
	char* cmd = argv[1];
	
	

	return 0;
}
