#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
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
	if(argc < 2)
		goto usage;
	
	init_audio(device, debug);
	
	// Process commands
	char* cmd = argv[1];
	int zonen = atoi(argv[2]);
	int tozone = -1; // Target zone (using library numbers)
	
	// Convert zone from zonen
	switch(zonen){
	case 1: tozone = zone1;
	case 2: tozone = zone2;
	case 3: tozone = zone3;
	case 4: tozone = zone4;
	case 5: tozone = zone5;
	case 6: tozone = zone6;
	default: tozone = nozone;
	}
	
	/*
		Command format:

		str int str...
		CMD zone arguments
		
		- zone 0 means all zones (if possible), zone is [1-6]
	*/
	
	// Parse command and arguments -- TODO fix index out of bounds checks
	if (strcmp("STATUS", cmd) == 0){
		// Get the current state of the system -- Parse data structures and output in readable form
		// TODO
	}

	// Power
	else if(strcmp("POWER", cmd) == 0){
		if(strcmp("ON", argv[3]) == 0){
			// Power on all ones
			process_command(tozone, cmd_all_on);
		}
		else if(strcmp("OFF", argv[3]) == 0){
			// Power off all zones
			process_command(tozone, cmd_all_off);
		}
	}

	// Volume
	else if(strcmp("VOLUME", cmd) == 0){
		if(strcmp("UP", argv[3]) == 0){
			process_command(tozone, cmd_volume_up);
		}
		else if(strcmp("DOWN", argv[3]) == 0){
			process_command(tozone, cmd_volume_down);
		}
	}

	// Bass
	else if(strcmp("BASS", cmd) == 0){
		if(strcmp("UP", argv[3]) == 0){
			process_command(tozone, cmd_bass_up);
		}
		else if(strcmp("DOWN", argv[3]) == 0){
			process_command(tozone, cmd_bass_down);
		}
	}

	// Treble
	else if(strcmp("TREBLE", cmd) == 0){
		if(strcmp("UP", argv[3]) == 0){
			process_command(tozone, cmd_treble_up);
		}
		else if(strcmp("DOWN", argv[3]) == 0){
			process_command(tozone, cmd_treble_down);
		}
	}

	// Balance
	else if(strcmp("BALANCE", cmd) == 0){
		if(strcmp("LEFT", argv[3]) == 0){
			process_command(tozone, cmd_balance_left);
		}
		else if(strcmp("RIGHT", argv[3]) == 0){
			process_command(tozone, cmd_balance_right);
		}
	}

	// Input
	else if(strcmp("INPUT", cmd) == 0){
		enum command c = cmd_part_mode_input_ch1 -1;
		int ch = atoi(argv[3]);
		if(argv[3][0] == '-'){
			// Unset
			ch *= -1;
		}
		c += ch;
		process_command(tozone, c);
	}

	// Query
	else if(strcmp("VOLUME", cmd) == 0){
		process_command(tozone, cmd_query_zone_state);
	}

	// Mute
	else if(strcmp("MUTE", cmd) == 0){
		// TODO -- this is wrong, mute state needs to be checked.
		if(strcmp("ON", argv[3]) == 0){
			// Mute
			process_command(tozone, cmd_mute_toggle);
		}
		else if(strcmp("OFF", argv[3]) == 0){
			// Unmute
			process_command(tozone, cmd_mute_toggle);
		}
	}
	
	// Other, invalid command
	else{
		fprintf(stderr, "Invalid command: %s\n", cmd);
		return -1;
	}

	return 0;
}
