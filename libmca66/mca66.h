#ifndef MCA66_H
#define MCA66_H

#define true 1
#define false 0

typedef enum command zone;
typedef enum command command;

/* data structures */
enum zone {
  nozone,		// 0
  zone1 = 2,	// 2
  zone2 = 26,
  zone3 = 50,
  zone4 = 74,
  zone5 = 98,
  zone6 = 122,
};

enum command {
  cmd_all_on,
  cmd_all_off,
  cmd_set_input_ch1 = 0,
  cmd_set_input_ch2,
  cmd_set_input_ch3,
  cmd_set_input_ch4,
  cmd_set_input_ch5,
  cmd_set_input_ch6,
  cmd_volume_up,
  cmd_volume_down,
  cmd_power_on,
  cmd_power_off,
  cmd_mute_toggle,
  cmd_bass_up,
  cmd_bass_down,
  cmd_treble_up,
  cmd_treble_down,
  cmd_balance_right,
  cmd_balance_left,
  cmd_part_mode_input_ch1,
  cmd_part_mode_input_ch2,
  cmd_part_mode_input_ch3,
  cmd_part_mode_input_ch4,
  cmd_part_mode_input_ch5,
  cmd_part_mode_input_ch6,
  cmd_query_zone_state
};

typedef struct Codes Codes;
struct Codes {
  const char *name;
  const char *code;
}; 

/* prototypes */
int process_command(zone z, command c);
int init_audio(char* device, int debugf);

#endif
