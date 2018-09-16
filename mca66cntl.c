#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <termios.h>

#define SERIAL_PORT "/dev/ttyUSB0"
int serialfd;

enum zone {
  nozone,
  zone1 = 2,
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

struct codes {
  const char *name;
  const char *code;
} codes[] = {
  { "ALL POWER ON",                       "\x02\x00\x01\x04\x38\x3F" },
  { "ALL POWER OFF",                      "\x02\x00\x01\x04\x39\x40" },
  { "Zone 1 - SET INPUT CHANNEL 1",       "\x02\x00\x01\x04\x03\x0A" },
  { "Zone 1 - SET INPUT CHANNEL 2",       "\x02\x00\x01\x04\x04\x0B" },
  { "Zone 1 - SET INPUT CHANNEL 3",       "\x02\x00\x01\x04\x05\x0C" },
  { "Zone 1 - SET INPUT CHANNEL 4",       "\x02\x00\x01\x04\x06\x0D" },
  { "Zone 1 - SET INPUT CHANNEL 5",       "\x02\x00\x01\x04\x07\x0E" },
  { "Zone 1 - SET INPUT CHANNEL 6",       "\x02\x00\x01\x04\x08\x0F" },
  { "Zone 1 - VOLUME UP",                 "\x02\x00\x01\x04\x09\x10" },
  { "Zone 1 - VOLUME DOWN",               "\x02\x00\x01\x04\x0A\x11" },
  { "Zone 1 - POWER ON",                  "\x02\x00\x01\x04\x20\x27" },
  { "Zone 1 - POWER OFF",                 "\x02\x00\x01\x04\x21\x28" },
  { "Zone 1 - MUTE TOGGLE",               "\x02\x00\x01\x04\x22\x29" },
  { "Zone 1 - BASS UP",                   "\x02\x00\x01\x04\x26\x2D" },
  { "Zone 1 - BASS DOWN",                 "\x02\x00\x01\x04\x27\x2E" },
  { "Zone 1 - TREBLE UP",                 "\x02\x00\x01\x04\x28\x2F" },
  { "Zone 1 - TREBLE DOWN",               "\x02\x00\x01\x04\x29\x30" },
  { "Zone 1 - BALANCE RIGHT",             "\x02\x00\x01\x04\x2A\x31" },
  { "Zone 1 - BALANCE LEFT",              "\x02\x00\x01\x04\x2B\x32" },
  { "Zone 1 - PART MODE INPUT CHANNEL 1", "\x02\x00\x01\x04\x3A\x41" },
  { "Zone 1 - PART MODE INPUT CHANNEL 2", "\x02\x00\x01\x04\x3B\x42" },
  { "Zone 1 - PART MODE INPUT CHANNEL 3", "\x02\x00\x01\x04\x3C\x43" },
  { "Zone 1 - PART MODE INPUT CHANNEL 4", "\x02\x00\x01\x04\x3D\x44" },
  { "Zone 1 - PART MODE INPUT CHANNEL 5", "\x02\x00\x01\x04\x3E\x45" },
  { "Zone 1 - PART MODE INPUT CHANNEL 6", "\x02\x00\x01\x04\x3F\x46" },
  { "Zone 1 - QUERY ZONE STATE",          "\x02\x00\x01\x06\x00\x09" },
  { "Zone 2 - SET INPUT CHANNEL 1",       "\x02\x00\x02\x04\x03\x0B" },
  { "Zone 2 - SET INPUT CHANNEL 2",       "\x02\x00\x02\x04\x04\x0C" },
  { "Zone 2 - SET INPUT CHANNEL 3",       "\x02\x00\x02\x04\x05\x0D" },
  { "Zone 2 - SET INPUT CHANNEL 4",       "\x02\x00\x02\x04\x06\x0E" },
  { "Zone 2 - SET INPUT CHANNEL 5",       "\x02\x00\x02\x04\x07\x0F" },
  { "Zone 2 - SET INPUT CHANNEL 6",       "\x02\x00\x02\x04\x08\x10" },
  { "Zone 2 - VOLUME UP",                 "\x02\x00\x02\x04\x09\x11" },
  { "Zone 2 - VOLUME DOWN",               "\x02\x00\x02\x04\x0A\x12" },
  { "Zone 2 - POWER ON",                  "\x02\x00\x02\x04\x20\x28" },
  { "Zone 2 - POWER OFF",                 "\x02\x00\x02\x04\x21\x29" },
  { "Zone 2 - MUTE TOGGLE",               "\x02\x00\x02\x04\x22\x2A" },
  { "Zone 2 - BASS UP",                   "\x02\x00\x02\x04\x26\x2E" },
  { "Zone 2 - BASS DOWN",                 "\x02\x00\x02\x04\x27\x2F" },
  { "Zone 2 - TREBLE UP",                 "\x02\x00\x02\x04\x28\x30" },
  { "Zone 2 - TREBLE DOWN",               "\x02\x00\x02\x04\x29\x31" },
  { "Zone 2 - BALANCE RIGHT",             "\x02\x00\x02\x04\x2A\x32" },
  { "Zone 2 - BALANCE LEFT",              "\x02\x00\x02\x04\x2B\x33" },
  { "Zone 2 - PART MODE INPUT CHANNEL 1", "\x02\x00\x02\x04\x3A\x42" },
  { "Zone 2 - PART MODE INPUT CHANNEL 2", "\x02\x00\x02\x04\x3B\x43" },
  { "Zone 2 - PART MODE INPUT CHANNEL 3", "\x02\x00\x02\x04\x3C\x44" },
  { "Zone 2 - PART MODE INPUT CHANNEL 4", "\x02\x00\x02\x04\x3D\x45" },
  { "Zone 2 - PART MODE INPUT CHANNEL 5", "\x02\x00\x02\x04\x3E\x46" },
  { "Zone 2 - PART MODE INPUT CHANNEL 6", "\x02\x00\x02\x04\x3F\x47" },
  { "Zone 2 - QUERY ZONE STATE",          "\x02\x00\x02\x06\x00\x0A" },
  { "Zone 3 - SET INPUT CHANNEL 1",       "\x02\x00\x03\x04\x03\x0C" },
  { "Zone 3 - SET INPUT CHANNEL 2",       "\x02\x00\x03\x04\x04\x0D" },
  { "Zone 3 - SET INPUT CHANNEL 3",       "\x02\x00\x03\x04\x05\x0E" },
  { "Zone 3 - SET INPUT CHANNEL 4",       "\x02\x00\x03\x04\x06\x0F" },
  { "Zone 3 - SET INPUT CHANNEL 5",       "\x02\x00\x03\x04\x07\x10" },
  { "Zone 3 - SET INPUT CHANNEL 6",       "\x02\x00\x03\x04\x08\x11" },
  { "Zone 3 - VOLUME UP",                 "\x02\x00\x03\x04\x09\x12" },
  { "Zone 3 - VOLUME DOWN",               "\x02\x00\x03\x04\x0A\x13" },
  { "Zone 3 - POWER ON",                  "\x02\x00\x03\x04\x20\x29" },
  { "Zone 3 - POWER OFF",                 "\x02\x00\x03\x04\x21\x2A" },
  { "Zone 3 - MUTE TOGGLE",               "\x02\x00\x03\x04\x22\x2B" },
  { "Zone 3 - BASS UP",                   "\x02\x00\x03\x04\x26\x2F" },
  { "Zone 3 - BASS DOWN",                 "\x02\x00\x03\x04\x27\x30" },
  { "Zone 3 - TREBLE UP",                 "\x02\x00\x03\x04\x28\x31" },
  { "Zone 3 - TREBLE DOWN",               "\x02\x00\x03\x04\x29\x32" },
  { "Zone 3 - BALANCE RIGHT",             "\x02\x00\x03\x04\x2A\x33" },
  { "Zone 3 - BALANCE LEFT",              "\x02\x00\x03\x04\x2B\x34" },
  { "Zone 3 - PART MODE INPUT CHANNEL 1", "\x02\x00\x03\x04\x3A\x43" },
  { "Zone 3 - PART MODE INPUT CHANNEL 2", "\x02\x00\x03\x04\x3B\x44" },
  { "Zone 3 - PART MODE INPUT CHANNEL 3", "\x02\x00\x03\x04\x3C\x45" },
  { "Zone 3 - PART MODE INPUT CHANNEL 4", "\x02\x00\x03\x04\x3D\x46" },
  { "Zone 3 - PART MODE INPUT CHANNEL 5", "\x02\x00\x03\x04\x3E\x47" },
  { "Zone 3 - PART MODE INPUT CHANNEL 6", "\x02\x00\x03\x04\x3F\x48" },
  { "Zone 3 - QUERY ZONE STATE",          "\x02\x00\x03\x06\x00\x0B" },
  { "Zone 4 - SET INPUT CHANNEL 1",       "\x02\x00\x04\x04\x03\x0D" },
  { "Zone 4 - SET INPUT CHANNEL 2",       "\x02\x00\x04\x04\x04\x0E" },
  { "Zone 4 - SET INPUT CHANNEL 3",       "\x02\x00\x04\x04\x05\x0F" },
  { "Zone 4 - SET INPUT CHANNEL 4",       "\x02\x00\x04\x04\x06\x10" },
  { "Zone 4 - SET INPUT CHANNEL 5",       "\x02\x00\x04\x04\x07\x11" },
  { "Zone 4 - SET INPUT CHANNEL 6",       "\x02\x00\x04\x04\x08\x12" },
  { "Zone 4 - VOLUME UP",                 "\x02\x00\x04\x04\x09\x13" },
  { "Zone 4 - VOLUME DOWN",               "\x02\x00\x04\x04\x0A\x14" },
  { "Zone 4 - POWER ON",                  "\x02\x00\x04\x04\x20\x2A" },
  { "Zone 4 - POWER OFF",                 "\x02\x00\x04\x04\x21\x2B" },
  { "Zone 4 - MUTE TOGGLE",               "\x02\x00\x04\x04\x22\x2C" },
  { "Zone 4 - BASS UP",                   "\x02\x00\x04\x04\x26\x30" },
  { "Zone 4 - BASS DOWN",                 "\x02\x00\x04\x04\x27\x31" },
  { "Zone 4 - TREBLE UP",                 "\x02\x00\x04\x04\x28\x32" },
  { "Zone 4 - TREBLE DOWN",               "\x02\x00\x04\x04\x29\x33" },
  { "Zone 4 - BALANCE RIGHT",             "\x02\x00\x04\x04\x2A\x34" },
  { "Zone 4 - BALANCE LEFT",              "\x02\x00\x04\x04\x2B\x35" },
  { "Zone 4 - PART MODE INPUT CHANNEL 1", "\x02\x00\x04\x04\x3A\x44" },
  { "Zone 4 - PART MODE INPUT CHANNEL 2", "\x02\x00\x04\x04\x3B\x45" },
  { "Zone 4 - PART MODE INPUT CHANNEL 3", "\x02\x00\x04\x04\x3C\x46" },
  { "Zone 4 - PART MODE INPUT CHANNEL 4", "\x02\x00\x04\x04\x3D\x47" },
  { "Zone 4 - PART MODE INPUT CHANNEL 5", "\x02\x00\x04\x04\x3E\x48" },
  { "Zone 4 - PART MODE INPUT CHANNEL 6", "\x02\x00\x04\x04\x3F\x49" },
  { "Zone 4 - QUERY ZONE STATE",          "\x02\x00\x04\x06\x00\x0C" },
  { "Zone 5 - SET INPUT CHANNEL 1",       "\x02\x00\x05\x04\x03\x0E" },
  { "Zone 5 - SET INPUT CHANNEL 2",       "\x02\x00\x05\x04\x04\x0F" },
  { "Zone 5 - SET INPUT CHANNEL 3",       "\x02\x00\x05\x04\x05\x10" },
  { "Zone 5 - SET INPUT CHANNEL 4",       "\x02\x00\x05\x04\x06\x11" },
  { "Zone 5 - SET INPUT CHANNEL 5",       "\x02\x00\x05\x04\x07\x12" },
  { "Zone 5 - SET INPUT CHANNEL 6",       "\x02\x00\x05\x04\x08\x13" },
  { "Zone 5 - VOLUME UP",                 "\x02\x00\x05\x04\x09\x14" },
  { "Zone 5 - VOLUME DOWN",               "\x02\x00\x05\x04\x0A\x15" },
  { "Zone 5 - POWER ON",                  "\x02\x00\x05\x04\x20\x2B" },
  { "Zone 5 - POWER OFF",                 "\x02\x00\x05\x04\x21\x2C" },
  { "Zone 5 - MUTE TOGGLE",               "\x02\x00\x05\x04\x22\x2D" },
  { "Zone 5 - BASS UP",                   "\x02\x00\x05\x04\x26\x31" },
  { "Zone 5 - BASS DOWN",                 "\x02\x00\x05\x04\x27\x32" },
  { "Zone 5 - TREBLE UP",                 "\x02\x00\x05\x04\x28\x33" },
  { "Zone 5 - TREBLE DOWN",               "\x02\x00\x05\x04\x29\x34" },
  { "Zone 5 - BALANCE RIGHT",             "\x02\x00\x05\x04\x2A\x35" },
  { "Zone 5 - BALANCE LEFT",              "\x02\x00\x05\x04\x2B\x36" },
  { "Zone 5 - PART MODE INPUT CHANNEL 1", "\x02\x00\x05\x04\x3A\x45" },
  { "Zone 5 - PART MODE INPUT CHANNEL 2", "\x02\x00\x05\x04\x3B\x46" },
  { "Zone 5 - PART MODE INPUT CHANNEL 3", "\x02\x00\x05\x04\x3C\x47" },
  { "Zone 5 - PART MODE INPUT CHANNEL 4", "\x02\x00\x05\x04\x3D\x48" },
  { "Zone 5 - PART MODE INPUT CHANNEL 5", "\x02\x00\x05\x04\x3E\x49" },
  { "Zone 5 - PART MODE INPUT CHANNEL 6", "\x02\x00\x05\x04\x3F\x4A" },
  { "Zone 5 - QUERY ZONE STATE",          "\x02\x00\x05\x06\x00\x0D" },
  { "Zone 6 - SET INPUT CHANNEL 1",       "\x02\x00\x06\x04\x03\x0F" },
  { "Zone 6 - SET INPUT CHANNEL 2",       "\x02\x00\x06\x04\x04\x10" },
  { "Zone 6 - SET INPUT CHANNEL 3",       "\x02\x00\x06\x04\x05\x11" },
  { "Zone 6 - SET INPUT CHANNEL 4",       "\x02\x00\x06\x04\x06\x12" },
  { "Zone 6 - SET INPUT CHANNEL 5",       "\x02\x00\x06\x04\x07\x13" },
  { "Zone 6 - SET INPUT CHANNEL 6",       "\x02\x00\x06\x04\x08\x14" },
  { "Zone 6 - VOLUME UP",                 "\x02\x00\x06\x04\x09\x15" },
  { "Zone 6 - VOLUME DOWN",               "\x02\x00\x06\x04\x0A\x16" },
  { "Zone 6 - POWER ON",                  "\x02\x00\x06\x04\x20\x2C" },
  { "Zone 6 - POWER OFF",                 "\x02\x00\x06\x04\x21\x2D" },
  { "Zone 6 - MUTE TOGGLE",               "\x02\x00\x06\x04\x22\x2E" },
  { "Zone 6 - BASS UP",                   "\x02\x00\x06\x04\x26\x32" },
  { "Zone 6 - BASS DOWN",                 "\x02\x00\x06\x04\x27\x33" },
  { "Zone 6 - TREBLE UP",                 "\x02\x00\x06\x04\x28\x34" },
  { "Zone 6 - TREBLE DOWN",               "\x02\x00\x06\x04\x29\x35" },
  { "Zone 6 - BALANCE RIGHT",             "\x02\x00\x06\x04\x2A\x36" },
  { "Zone 6 - BALANCE LEFT",              "\x02\x00\x06\x04\x2B\x37" },
  { "Zone 6 - PART MODE INPUT CHANNEL 1", "\x02\x00\x06\x04\x3A\x46" },
  { "Zone 6 - PART MODE INPUT CHANNEL 2", "\x02\x00\x06\x04\x3B\x47" },
  { "Zone 6 - PART MODE INPUT CHANNEL 3", "\x02\x00\x06\x04\x3C\x48" },
  { "Zone 6 - PART MODE INPUT CHANNEL 4", "\x02\x00\x06\x04\x3D\x49" },
  { "Zone 6 - PART MODE INPUT CHANNEL 5", "\x02\x00\x06\x04\x3E\x4A" },
  { "Zone 6 - PART MODE INPUT CHANNEL 6", "\x02\x00\x06\x04\x3F\x4B" },
  { "Zone 6 - QUERY ZONE STATE",          "\x02\x00\x06\x06\x00\x0E" },
};

int set_interface_attribs (int fd, int speed, int parity)
{
  struct termios tty;

  memset (&tty, 0, sizeof (tty));
  if (tcgetattr(fd, &tty) != 0) {
    perror("tcgetattr");
    return -1;
  }

  cfsetospeed(&tty, speed);
  cfsetispeed(&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
  // disable IGNBRK for mismatched speed tests; otherwise receive break
  // as \000 chars
  tty.c_iflag &= ~IGNBRK;         // disable break processing
  tty.c_lflag = 0;                // no signaling chars, no echo,
                                  // no canonical processing
  tty.c_oflag = 0;                // no remapping, no delays
  tty.c_cc[VMIN]  = 0;            // read doesn't block
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    perror("tcsetattr");
    return -1;
  }

  return 0;
}

void set_blocking (int fd, int should_block)
{
  struct termios tty;

  memset(&tty, 0, sizeof (tty));
  if (tcgetattr(fd, &tty) != 0) {
    perror("tcgetattr");
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  if (tcsetattr (fd, TCSANOW, &tty) != 0) {
    perror("tcsetattr");
  }
}
      
int process_command(enum zone z, enum command c)
{
  char ch;
  int i;
  
  printf("Sending %s\n", codes[z + c].name);
  write(serialfd, codes[z + c].code, 6);

  sleep(1);

  for (i = 0; read(serialfd, &ch, 1) && i < 512; i++)
    printf("%02hhx ", ch);
  
  printf("\nRead %d bytes\n", i);

  return 0;
}

int main(int argc, char *argv[])
{
  int i;

  if ((serialfd = open(SERIAL_PORT, O_RDWR|O_NOCTTY|O_SYNC)) < 0) {
    perror("Failed to open " SERIAL_PORT);

    return -1;
  }
  /* Valid baud rates:                                                    *
   * B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400 */
  /* Valid parities:                     *
   * 0 -> no parity                      *
   * PARENB|PARODD -> odd parity         *
   * PARENB -> even parity               *
   * PARENB|PARODD|CMSPAR -> mark parity *
   * PARENB|CMSPAR -> space parity       */
  if (set_interface_attribs(serialfd, B38400, 0)) {
    return -1;
  }
  set_blocking(serialfd, 0);

  //  process_command(zone1, cmd_power_on);

  process_command(nozone, cmd_all_on);
  process_command(zone1, cmd_set_input_ch1);
  process_command(zone2, cmd_set_input_ch1);
  process_command(zone3, cmd_set_input_ch1);
  process_command(zone4, cmd_set_input_ch1);

  return 0;
}
