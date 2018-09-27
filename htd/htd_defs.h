/*** Definitions ***/
#define WTIME (WORD) time
#define WEB1 1
#define WEB2 2
#define WEB3 3
#define WEB4 4

#define V6
#define MAX_ZONES	6
#define MAX_SOURCES	6
#define MAX_NAME_LEN 31	// 30 char limit + 1 for end of str...must be less than 32 due to flash programming
#define MAX_ZONE_NAME_LEN_USERINPUT 11
#define MAX_SRC_NAME_LEN_USERINPUT 11
#define MAX_VOL_VALUE 60
#define MIN_HW_VOL_VAL 0xC3

#define FLASH_BLK_SIZE	32

#define NUM_OF(x) (sizeof (x) / sizeof *(x))

#define ZONE1	0x01
#define ZONE2	0x02
#define ZONE3	0x04
#define ZONE4	0x08
#define ZONE5	0x10
#define ZONE6	0x20

#define MAX_QUEUE	32

typedef struct 
{
	BYTE zoneNum;
	BYTE inputCh;
	BYTE vol;
	BYTE treb;
	BYTE bal;
} ZoneInfo_t;

typedef struct 
{
	BYTE hdr;
	BYTE reserved1;
	BYTE zone;
	BYTE cmd;
	BYTE data1;
	BYTE volTrebBal;
	BYTE LEDStatus;
	BYTE reserved2;
	BYTE inputPort;
	BYTE volValue;
	BYTE trebValue;
	BYTE bassValue;
	BYTE balValue;
	BYTE checksum;
} SerialRcv_Cmd_0x05_t;

typedef struct 
{
	BYTE hdr;
	BYTE reserved1;
	BYTE zone;
	BYTE cmd;	//0x06
	BYTE data1;
	BYTE zones;
	BYTE keypads;
	BYTE data4;
	BYTE data5;
	BYTE data6;
	BYTE data7;
	BYTE data8;
	BYTE data9;
	BYTE checksum;
} SerialRcv_Cmd_0x06_t;

typedef struct 
{
	SerialRcv_Cmd_0x06_t cmd6Rcv;
	SerialRcv_Cmd_0x05_t zones[MAX_ZONES];
} SerialRcv_Cmd_0x06_Total_t;
