/*****************************************************************************/
/* htd.c                                                                    */
/*****************************************************************************/
/* History:                                                                  */
/* 2011-03-27  tl  initial version copied from Lantronix's Demo 12 project   */
/*****************************************************************************/

/* Included header files */
#include "bldFlags.h"
#include <style.h>
#include <globals.h>
#include <kernel.h>
#include <io.h>
#include <ip.h>     /* defines variables and constants of IP-protocol */
#include <tcp.h>   /* defines variables and constants of TCP-protocol */
#include <udp.h>
#include <string.h>
#include <web.h>
#include <mbuf.h>
#include <filesys.h>
#include <random.h>
#include <SerFl\ECtypes.h>
#include <SerFl\serflash.h>
#include <flash.h>
#include <mbuf.h>
#include <macros.h>
#include <hwstruc.h>
#include "htd_defs.h"

extern HWST HW;
extern signed long tEcSppSerFlashRead (WORD,WORD,DWORD,BYTE far *);

int raw_gread(int web_page, WORD off, BYTE far *buf, WORD len);
int raw_gwrite(int web_page, WORD off, BYTE far *buf, WORD len, WORD erase);
void raw_gflash_clr(int web_page, WORD off);


/*** External variables ***/
extern void sendblk( BYTE * sb, WORD len );
extern void print_proto_v4();

/*** variables ***/
static BYTE lock = 0;

/*** Prototypes ***/
int web_request_callback(WCT *w, char *file, char *hdr);
//int zone_control_new_screen(WCT *w, char *file, char *hdr);
//int options_screen(WCT *w, char *file, char *hdr);
//int zone_control_new_screen2(WCT *w, char *file, char *hdr);

static int serialSendRcv(int chan, BYTE *buf, int len, int to);
static int serialSend(int chan, BYTE *buf, int len);
static void http_reply(int code);

void process_form_tags();
void process_form_tags_new_screen();
void process_form_tags_options();
//void process_form_tags_new_screen2();
void process_form_tags_keypad();
void process_form_tags_keypad2();

void getControllerData();
int getZoneNum(BYTE* buf, int len);
int getNewZoneName(int znum, BYTE* buf, int len, BYTE* newName);
int getNewSourceName(int srcNum, BYTE* buf, int len, BYTE* newName);
void sendSourceInputToHW(BYTE zoneNum, BYTE inputNum);
void sendVolUpToHW(BYTE zoneNum);
void sendVolDownToHW(BYTE zoneNum);
void sendPowerOnToHW(BYTE zoneNum);
void sendPowerOffToHW(BYTE zoneNum);
void sendMuteToggleToHW(BYTE zoneNum);
void sendCMD_0x04(BYTE zoneNum, BYTE data);
void sendCMD_0x06(BYTE zoneNum);
void initializeFlashDefaults();
void writeSettingsToFlash();
void readSettingsFromFlash();
int urldecode(char *src, char *last, char *dest);
int getFirstSelectedZone();
int getNumberOfZonesSelected();
void sendGroupSourceInputToHW(BYTE inputNum);
void sendGroupPowerOnToHW();
void sendGroupPowerOffToHW();
void sendGroupMuteToggleToHW();
void sendGroupVolUpToHW();
void sendGroupVolDownToHW();
int getZoneNumberFromHiddenInput(BYTE* buf, int len);
int getSrcNumberFromDropDown(BYTE* buf, int len);
int getNumberFromPostData(BYTE* fieldStr, BYTE* buf, int len);
void sendVolToHW(BYTE zoneNum, int volVal);
void sendGroupVolToHW(int newVolVal);
int getGroupSourceFromHiddenInput(BYTE* buf, int len);
int getCurrentSourceFromHiddenInput(BYTE* buf, int len);
void updateGroupVolumeValue();
BYTE convertVolValForDisplay(BYTE hwVolVal);
BYTE convertDispVolToHwVolVal(BYTE dispVolVal);
void refreshData();

static BYTE fbuffer[256];
static BYTE rbuffer[256];
static BYTE sbuffer[256];

BYTE *g_formTags=0;

//ZoneInfo_t g_zoneInfo[MAX_ZONES];	
SerialRcv_Cmd_0x06_Total_t g_zoneInfo;

BYTE g_selectedZone=0;
BYTE g_selectedSource=0;
BYTE g_currentSource=0;
BYTE g_groupSource=0;

BYTE g_zoneNames[MAX_ZONES][MAX_NAME_LEN];
BYTE g_srcNames[MAX_SOURCES][MAX_NAME_LEN];

BOOL g_bConnected;

static BYTE g_htdKey[] = {'H','T', 'D', '!', 0};	// this value won't be in flash on a new device so can be used
								//	to initialize a new device
BOOL g_bNewDevice;

int g_zoneButtonStates = 0x01;
BYTE g_groupVolValue=MIN_HW_VOL_VAL;
BOOL g_isLive=TRUE;

BYTE* g_cmdQueue[MAX_QUEUE];
int g_queuePtr=0;

const BYTE* BTNSET="BtnSet";
const BYTE* BTNAPPLY="BtnApply";
const BYTE* BTNLIVE="BtnLive";
const BYTE* BTNSRCL="BtnSrcL";
const BYTE* BTNSRCR="BtnSrcR";
const BYTE* BTNPWR="BtnPwr";
const BYTE* BTNPWRON="BtnPwrOn";
const BYTE* BTNPWROFF="BtnPwrOff";
const BYTE* BTNMUTE="BtnMute";
const BYTE* BTNVOLUP="BtnVolUp";
const BYTE* BTNVOLDOWN="BtnVolDown";
const BYTE* BTNZ1="BtnZ1";
const BYTE* BTNZ2="BtnZ2";
const BYTE* BTNZ3="BtnZ3";
const BYTE* BTNZ4="BtnZ4";
const BYTE* BTNZ5="BtnZ5";
const BYTE* BTNZ6="BtnZ6";
const BYTE* BTNGRP="BtnGrp";
const BYTE* BTNRFSH="BtnFrsh";
const BYTE* BTNOPTIONS="BtnOptions";
const BYTE* DDNVOL="VolDropDown";
const BYTE* DDNSRC="SrcDropDown";
const BYTE* BTNALLON="BtnAllOn";
const BYTE* BTNALLOFF="BtnAllOff";
const BYTE* BTNSTATUS="BtnStatus";


BOOL g_applyPressed=FALSE;
BOOL g_isGroup=FALSE;

static BYTE *fnZONES = "zones.html";
//static BYTE *fnNEW_SCREEN2 = "new_screen2.html";
static BYTE *fnKEYPAD = "keypad.html";
static BYTE *fnKEYPAD2 = "keypad2.html";
static BYTE *fnSETUP = "setup.html";


BYTE *g_borderColor = "2px solid #00ff00";
BYTE *g_borderColorNone = "none";
BYTE *g_versioncopyright = "<p>v2.0 &#169; 2011 HTD, Inc. </p>";

BYTE g_debugStr[80];

int findStr(BYTE *src, int srcLen, BYTE* strToFind);
void sendSourceInputToHW(BYTE zoneNum, BYTE inputNum);
int getSourceNum(BYTE* buf, int len);
void sendSourceInputToHW(BYTE zoneNum, BYTE inputNum);
void sendGroupSourceInputToHW(BYTE inputNum);
void sendVolUpToHW(BYTE zoneNum);
void sendVolDownToHW(BYTE zoneNum);
void groupVolUpHelper(int znumber, int prevVolValue);
void sendGroupVolUpToHW();
void groupVolDownHelper(int znumber, int prevVolValue);
void sendGroupVolDownToHW();
void sendPowerOnToHW(BYTE zoneNum);
void sendGroupPowerOnToHW();
void sendPowerOffToHW(BYTE zoneNum);
void sendGroupPowerOffToHW();
void sendMuteToggleToHW(BYTE zoneNum);
void sendGroupMuteToggleToHW();
void sendCMD_0x04(BYTE zoneNum, BYTE data);
void sendCMD_0x06(BYTE zoneNum);
int raw_gread(int web_page, WORD off, BYTE far *buf, WORD len);
int raw_gwrite(int web_page, WORD off, BYTE far *buf, WORD len, WORD erase);
void raw_gflash_clr(int web_page, WORD off);
void sendGroupVolHelper(int zoneNum, int newVolVal);


/**********************************************************************/
/* Procedure htd. Called from main.c                                 */
/**********************************************************************/
htd() {
	extern IP_T ip;
    
	printf("*** HTD ***\r\n");
    printf("\r\n");

	g_bConnected = FALSE;
	g_bNewDevice = FALSE;

	readSettingsFromFlash();
	if (g_bNewDevice == TRUE)
	{
		// initialize flash with defaults
		initializeFlashDefaults();
	}

	getControllerData();


	WebMethRegister("POST", web_request_callback, fnZONES);
	WebMethRegister("GET", web_request_callback, fnZONES);
	WebMethRegister("POST", web_request_callback, fnKEYPAD);
	WebMethRegister("GET", web_request_callback, fnKEYPAD);
	WebMethRegister("POST", web_request_callback, fnSETUP);
	WebMethRegister("GET", web_request_callback, fnSETUP);

	while (1) {
		nice();
	}
}

void initializeFlashDefaults()
{
	int i;

	for (i=0;i < MAX_ZONES;i++)
	{
		sprintf(g_zoneNames[i], "Zone %d", i+1);
	}

	for (i=0;i < MAX_SOURCES;i++)
	{
		sprintf(g_srcNames[i], "Src %d", i+1);
	}

	writeSettingsToFlash();
}

void writeSettingsToFlash()
{
	int i, offset, webnum;

	offset = 0x0000;	//0x0000 - 0x0400:  1k bytes for zone names
	webnum = WEB4;
	raw_gflash_clr(webnum, offset); // need to clear the flash page (64K) before writing new data 
									// clearing resets all bits to 1's
	nice();

	// zone names
	for (i=0;i < MAX_ZONES;i++)
	{
		raw_gwrite(webnum, offset, (BYTE far *)g_zoneNames[i], MAX_NAME_LEN, 0);
		nice();
		offset += FLASH_BLK_SIZE;
	}

	offset = 0x0400;	//0x0400 - 0x0800:  1k bytes for input source names
	for (i=0;i < MAX_SOURCES;i++)
	{
		
		raw_gwrite(webnum, offset, (BYTE far *)g_srcNames[i], MAX_NAME_LEN, 0);
		nice();
		offset += FLASH_BLK_SIZE;
	}

	offset = 0x1000;	//0x1000 - 0x1004
	raw_gwrite(webnum, offset, g_htdKey, sizeof(g_htdKey), 0);
	nice();
	// todo more settings...
}

void readSettingsFromFlash()
{
	int i, offset, webnum;
	BYTE key[10];

	offset = 0x0000;	//0x0000 - 0x0400:  1k bytes for zone names
	webnum = WEB4;

	// zone names
	for (i=0;i < MAX_ZONES;i++)
	{
		raw_gread(webnum, offset, (BYTE far *)g_zoneNames[i], MAX_NAME_LEN);
		nice();
		offset += FLASH_BLK_SIZE;
	}

	offset = 0x0400;	//0x0400 - 0x0800:  1k bytes for input source names
	for (i=0;i < MAX_SOURCES;i++)
	{
		
		raw_gread(webnum, offset, (BYTE far *)g_srcNames[i], MAX_NAME_LEN);
		nice();
		offset += FLASH_BLK_SIZE;
	}

	offset = 0x1000;	//0x1000 - 0x1004
	raw_gread(webnum, offset, key, sizeof(g_htdKey));
	nice();

	g_bNewDevice = FALSE;
	for (i=0;i < sizeof(g_htdKey);i++)
	{
		if (g_htdKey[i] != key[i])
		{
			g_bNewDevice = TRUE;
			break;
		}
	}

	// todo more settings...
}


int web_request_callback(WCT *w, char *file, char *hdr)
{
	int i, got, len, toget, to, fd, foundit;
	char *a;
	BYTE *p1, *p2;
	WORD start;
	DWORD offset;
	BYTE *pStr;

	/* Since I use a common buffer, prevent entry by additional web processes. */
	while (lock)
		nice();

	lock = 1;

	if (!(a=MBufGet())) {
		lock = 0;
		return(500);
	}

	toget = 0;
	for (i = 0; i < (strlen(hdr) - 15); i++) {
		if (!(strncmp(hdr+i, "Content-Length:", 15))) {
			p1 = hdr + i + 15;
			while (*p1 <= 0x20)
				p1++;
			p2 = p1;
			while (*p1 >= 0x20)
				p1++;
			memcpy(a, p2, (p1 - p2));
			a[(p1 - p2)] = 0;
			toget = atoi(a);
		}
	}

	MBufFree(a);
	start = WTIME;
	p1 = sbuffer;
	memset(sbuffer, 0, sizeof(sbuffer));
	memset(rbuffer, 0, sizeof(rbuffer));
	memset(fbuffer, 0, sizeof(fbuffer));
	
	// Read the form tags 
	while((p1 - sbuffer) < toget) {
		if ((w->t->State == ESTABLISHED) && ((len = IOCall(0, (BYTE *)&w->t->RcvFifo)) > 0)) {
			for (i = 0; i < len; i++) {
				*p1++ = IOCall(1, (BYTE *)&w->t->RcvFifo);
			}
		}
		nice();
		if ((WTIME - start) > 15) {
			lock = 0;
			return(0);
		}
	}
	g_formTags = sbuffer;

	http_reply(200);
	if (!(strcmp(file, fnKEYPAD))) 
	{
		process_form_tags_keypad(sbuffer, toget);
		print_keypad();
	}
	else if (!(strcmp(file, fnSETUP))) 
	{
		// setup.html
		process_form_tags_options(sbuffer, toget);
		print_setup();
	}


	// Wait for empty xmit fifo
	while ((w->t->State == ESTABLISHED) && ((IOCall(0, (BYTE *)&w->t->XmitFifo)) > 0))
		nice();
	lock = 0;
	
	return(0);
}

int getZoneNum(BYTE* buf, int len)
{
	// assumes that page is generated with "_znumx" format for the <Option> tag
	BYTE tagValue[20];
	int i;

	for (i=0;i < MAX_ZONES;i++)
	{
		sprintf(tagValue, "_znum%d", i);
		if (findStr(buf, len, tagValue))
			return i;
	}
	return -1;
}

int getNewNameHelper(BYTE* strToFind, BYTE* buf, int len, BYTE* newName, int maxUserInput)
{
	//returns the length of the new name
	BYTE* newNameStart;
	int i, j;
	for (i = 0; i < len; i++) 
	{
		if (!(strncmp(buf+i, strToFind, strlen(strToFind)))) 
		{
			// found it...the new name starts from the next char
			//	to an & or end of the form tags data
			i += strlen(strToFind);
			newNameStart = buf+i;
			for (j=0;j < (len-i);j++)
			{
				if (!(strncmp(buf+i+j, "&", 1))) 
				{
					//found it
					if (j == 0)
					{
						// empty string...user didn't type any input
						newName[0] = 0;
						return j;
					}
					
					break;
				}
			}

			if (j >= maxUserInput)
				j = maxUserInput  - 1;
			
			memcpy(newName, newNameStart, j);
			newName[j] = 0; //terminate it so its a string
			return j;
		}
	}
	// odd that edit field not found...html must have changed!
	return -1;
}

int getNewZoneName(int znum, BYTE* buf, int len, BYTE* newName)
{
	int retVal;
	BYTE userInputName[MAX_NAME_LEN];
	//returns the length of the new name
	// the form's tag contains the name of the field "NewZoneName"
	// e.g. ZoneDropDown=_znum0&NewZoneName=hello&RenZone=Rename&SrcDropDown=_zi0&NewSrcName=
	// the end of the new name can be an & or the end of the form data
	//	e.g. ZoneDropDown=_znum0&NewZoneName=&RenZone=Rename&SrcDropDown=_zi0&NewSrcName=hello
	// so look for the new string in between "NewZoneName=" and an "&" or end of form tag data
	retVal = getNewNameHelper("NewZoneName=", buf, len, userInputName, MAX_ZONE_NAME_LEN_USERINPUT);
	if (retVal == -1)
		return retVal;
	
	// prefix with "Z#"
	//sprintf(newName, "Z%d-%s", znum+1, userInputName);
	sprintf(newName, "%s", userInputName);
	return strlen(newName);
}

int getNumberFromPostData(BYTE* fieldStr, BYTE* buf, int len)
{
	// return -1 if no zone number found in the hidden input field
	BYTE strbuf[MAX_NAME_LEN];	//should not have more than 2 digits for the zone number but 
								//	since we are reusing the new name helper function,
								//  better make sure it's the right buffer
	int retval;

	//leverage the function that reads fromt the new zone name input box
	retval = getNewNameHelper(fieldStr, buf, len, strbuf, 10);	// shouldn't ever be more than 10 digit number
	if (retval > 0)
		return atoi(strbuf);
	else
		return -1;	
}

int getSrcNumberFromDropDown(BYTE* buf, int len)
{
	int retval;
	retval = getNumberFromPostData("SrcDropDown=", buf, len);
	return retval;
}

int getVolNumberFromDropDown(BYTE* buf, int len)
{
	int retval;
	retval = getNumberFromPostData("VolDropDown=", buf, len);
	return retval;
}

int getZoneNumberFromHiddenInput(BYTE* buf, int len)
{
	int retval;
	retval = getNumberFromPostData("_cznum=", buf, len);
	if (retval < 0)
		return 0; //default to zone 1
	else
		return retval;
}

int getGroupSourceFromHiddenInput(BYTE* buf, int len)
{
	int retval;
	retval = getNumberFromPostData("_grpsrc=", buf, len);
	if (retval < 0)
		return 0; //default to zone 1
	else
		return retval;
}

int getCurrentSourceFromHiddenInput(BYTE* buf, int len)
{
	int retval;
	retval = getNumberFromPostData("_zsrc=", buf, len);
	if (retval < 0)
		return 0; //default to src 1
	else
		return retval;
}

int getIsGroupValueFromHiddenInput(BYTE* buf, int len)
{
	int retval;
	retval = getNumberFromPostData("_isgrp=", buf, len);
	if (retval < 0)
		return 0; //default to no grouping
	else
		return retval;
}

int getGroupMaskValueFromHiddenInput(BYTE* buf, int len)
{
	int retval;
	retval = getNumberFromPostData("_grpmsk=", buf, len);
	if (retval < 0)
		return 0; //default to no masks
	else
		return retval;
}

int getNewSourceName(int srcNum, BYTE* buf, int len, BYTE* newName)
{
	int retVal;
	BYTE userInputName[MAX_NAME_LEN];
	//returns the length of the new name
	// the form's tag contains the name of the field "NewZoneName"
	// e.g. ZoneDropDown=_znum0&NewZoneName=hello&RenZone=Rename&SrcDropDown=_zi0&NewSrcName=
	// the end of the new name can be an & or the end of the form data
	//	e.g. ZoneDropDown=_znum0&NewZoneName=&RenZone=Rename&SrcDropDown=_zi0&NewSrcName=hello
	// so look for the new string in between "NewZoneName=" and an "&" or end of form tag data
	retVal = getNewNameHelper("NewSrcName=", buf, len, userInputName, MAX_SRC_NAME_LEN_USERINPUT);
	if (retVal == -1)
		return retVal;
	
	// prefix with "S#"
	//sprintf(newName, "S%d-%s", srcNum+1, userInputName);
	sprintf(newName, "%s", userInputName);
	return strlen(newName);
}

int getSourceNum(BYTE* buf, int len)
{
	// assumes that page is generated with "_zix" format for the <Option> tag
	BYTE tagValue[20];
	int i;

	for (i=0;i < MAX_SOURCES;i++)
	{
		sprintf(tagValue, "_zi%d", i);
		if (findStr(buf, len, tagValue))
			return i;
	}
	return -1;
}

void sendSourceInputToHW(BYTE zoneNum, BYTE inputNum)
{
	BYTE data = inputNum + 0x03;	// hardware expects input num to start at 0x03.
	sendCMD_0x04(zoneNum, data);
}

void sendGroupSourceInputToHW(BYTE inputNum)
{
	// loop through each selected zone and set the source
	// keep it simple and don't loop for now
	if (g_zoneButtonStates & ZONE1)
	{
		g_zoneInfo.zones[0].inputPort = inputNum;
		sendSourceInputToHW(0, inputNum);
	}
	if (g_zoneButtonStates & ZONE2)
	{
		g_zoneInfo.zones[1].inputPort = inputNum;
		sendSourceInputToHW(1, inputNum);
	}
	if (g_zoneButtonStates & ZONE3)
	{
		g_zoneInfo.zones[2].inputPort = inputNum;
		sendSourceInputToHW(2, inputNum);
	}
	if (g_zoneButtonStates & ZONE4)
	{
		g_zoneInfo.zones[3].inputPort = inputNum;
		sendSourceInputToHW(3, inputNum);
	}
	if (g_zoneButtonStates & ZONE5)
	{
		g_zoneInfo.zones[4].inputPort = inputNum;
		sendSourceInputToHW(4, inputNum);
	}
	if (g_zoneButtonStates & ZONE6)
	{
		g_zoneInfo.zones[5].inputPort = inputNum;
		sendSourceInputToHW(5, inputNum);
	}
}

void sendVolUpToHW(BYTE zoneNum)
{
	sendCMD_0x04(zoneNum, 0x09);
}

void sendVolDownToHW(BYTE zoneNum)
{
	sendCMD_0x04(zoneNum, 0x0A);
}

BYTE convertVolValForDisplay(BYTE hwVolVal)
{
	BYTE retVolVal;

	//hw spec says 0 is max
	if (hwVolVal == 0)
		retVolVal = MAX_VOL_VALUE;
	else if (hwVolVal <= MIN_HW_VOL_VAL)	
		retVolVal = 0;	// min vol val
	else if (hwVolVal == (MIN_HW_VOL_VAL + 1))
		retVolVal = 1;	// hardware bug where VolVal 1 = 0xC4 and 0xC5!! take care of the 0xc4 case here and 
						//	the 0xc5 case is taken care of in the else below
	else
		retVolVal = hwVolVal - (MIN_HW_VOL_VAL + 1);

	return retVolVal;
}

BYTE convertDispVolToHwVolVal(BYTE dispVolVal)
{
	BYTE retVolVal;

	//hw spec says 0 is max
	if (dispVolVal >= MAX_VOL_VALUE)
		retVolVal = 0;
	else if (dispVolVal == 0)
		retVolVal = MIN_HW_VOL_VAL;
	else
		retVolVal = dispVolVal + (MIN_HW_VOL_VAL + 1);

	return retVolVal;
}

void sendVolToHW(BYTE zoneNum, int newVolVal)
{
	int i, volDelta;
	BYTE curVolVal, curHwVolVal;
	BOOL isVolUp=FALSE;

	curHwVolVal = g_zoneInfo.zones[zoneNum].volValue;

	curVolVal = convertVolValForDisplay(curHwVolVal);
	if (curVolVal == newVolVal)
		return;
	
	// need to loop and send the volume one at a time and check 
	if (newVolVal > curVolVal)
	{
		volDelta = newVolVal - curVolVal;
		if ((curVolVal == 0) && ( newVolVal >= 1))
			volDelta += 1;// compensate...hardware bug where VolVal 1 = 0xC4 and 0xC5!!  need to increment one more than usual
		if ((curVolVal == 1) && (curHwVolVal == (MIN_HW_VOL_VAL + 1)))
			volDelta += 1;// compensate...hardware bug where VolVal 1 = 0xC4 and 0xC5!!  need to increment one more than usual
		isVolUp = TRUE;
	}
	else
	{
		volDelta = curVolVal - newVolVal;
		if ((newVolVal == 0) && (curHwVolVal == (MIN_HW_VOL_VAL + 2)))
			volDelta += 1;// compensate...hardware bug where VolVal 1 = 0xC4 and 0xC5!!  need to increment one more than usual
		if ((newVolVal == 0) && ( curVolVal >= 2))
			volDelta += 1;// compensate...hardware bug where VolVal 1 = 0xC4 and 0xC5!!  need to increment one more than usual
		isVolUp = FALSE;
	}
//	sprintf(g_debugStr, "zn=%d,cvb=%d,cva=%d, nv=%d,vD=%d,", zoneNum,g_zoneInfo.zones[zoneNum].volValue,curVolVal,newVolVal,volDelta);
	for (i = 0;i < volDelta;i++)
	{
		if (isVolUp)
			sendCMD_0x04(zoneNum, 0x09);
		else
			sendCMD_0x04(zoneNum, 0x0A);
		//sdelay(60);
	}

	//sdelay(120);
	sendCMD_0x06(0);
//	if (!isVolUp)
		// hardware compensation...volVal is off by 2 values from what is displayed on the physical keypad when volumn down is used
//		g_zoneInfo.zones[g_selectedZone].volValue -= 2;

}

void groupVolUpHelper(int znumber, int prevVolValue)
{
	int delta;

	delta = ((int)g_zoneInfo.zones[znumber].volValue - prevVolValue);
	if (delta == 1)
	{
		if (g_groupVolValue < g_zoneInfo.zones[znumber].volValue)
			g_groupVolValue = g_zoneInfo.zones[znumber].volValue;
	}
}

void updateGroupVolumeValue()
{
	int i, zmask;

	sendCMD_0x06(0);

	zmask = 0x01;
	for (i=0;i < MAX_ZONES;i++)
	{
		if (g_zoneButtonStates & zmask)
		{
			groupVolUpHelper(i, g_zoneInfo.zones[i].volValue);
		}
		zmask = zmask << 1;
	}
}

void sendGroupVolUpToHW()
{
	int i, zmask;

	// loop through each selected zone and set the source
	zmask = 0x01;
	for (i=0;i < MAX_ZONES;i++)
	{
		if (g_zoneButtonStates & zmask)
		{
			sendVolUpToHW(i);
		}
		zmask = zmask << 1;
	}

	updateGroupVolumeValue();
}

void sendGroupVolHelper(int zoneNum, int newVolVal)
{
	BYTE curVolVal, curHwVolVal;

	curHwVolVal = g_zoneInfo.zones[zoneNum].volValue;
	curVolVal = convertVolValForDisplay(curHwVolVal);

	if (curVolVal >= 0 && curVolVal <= MAX_VOL_VALUE)
	{
		if (curVolVal > (newVolVal - 2) && curVolVal < (newVolVal + 2))
		{
			// convert back to hw terms
			curHwVolVal = convertDispVolToHwVolVal(curVolVal);
			g_groupVolValue = curHwVolVal;
		}
	}
}

void sendGroupVolToHW(int newVolVal)
{
	int i,j, volDelta, zmask;
	BYTE curVolVal, curHwVolVal;
	int volDeltaArr[MAX_ZONES];
	BOOL isVolUpArr[MAX_ZONES];

	for (i=0;i < MAX_ZONES;i++)
	{
		curHwVolVal = g_zoneInfo.zones[i].volValue;
		curVolVal = convertVolValForDisplay(curHwVolVal);

		if (newVolVal > curVolVal)
		{
			volDeltaArr[i] = newVolVal - curVolVal;

			if ((curVolVal == 0) && ( newVolVal >= 1))
				volDeltaArr[i] += 1;// compensate...hardware bug where VolVal 1 = 0xC4 and 0xC5!!  need to increment one more than usual
			if ((curVolVal == 1) && (curHwVolVal == (MIN_HW_VOL_VAL + 1)))
				volDeltaArr[i] += 1;// compensate...hardware bug where VolVal 1 = 0xC4 and 0xC5!!  need to increment one more than usual

			isVolUpArr[i] = TRUE;
		}
		else
		{
			volDeltaArr[i] = curVolVal - newVolVal;
			if ((newVolVal == 0) && (curHwVolVal == (MIN_HW_VOL_VAL + 2)))
				volDeltaArr[i] += 1;// compensate...hardware bug where VolVal 1 = 0xC4 and 0xC5!!  need to increment one more than usual
			if ((newVolVal == 0) && ( curVolVal >= 2))
				volDeltaArr[i] += 1;// compensate...hardware bug where VolVal 1 = 0xC4 and 0xC5!!  need to increment one more than usual

			isVolUpArr[i] = FALSE;
		}
	}
	
	//sprintf(g_debugStr, "zn=%d,cv=%d,nv=%d,vD=%d,", zoneNum,curVolVal,newVolVal,volDelta);
	for (i = 0;i <= MAX_VOL_VALUE;i++)
	{
		zmask = 0x01;
		for (j=0;j < MAX_ZONES;j++)
		{
			if (g_zoneButtonStates & zmask)
			{
				if (volDeltaArr[j] > i)
				{
					if (isVolUpArr[j])
						sendCMD_0x04(j, 0x09);
					else
						sendCMD_0x04(j, 0x0A);
					//sdelay(60);
				}
			}
			zmask = zmask << 1;
		}
	}

	//sdelay(120);
	sendCMD_0x06(0);
	zmask = 0x01;
	for (j=0;j < MAX_ZONES;j++)
	{
		if (g_zoneButtonStates & zmask)
		{
			sendGroupVolHelper(j, newVolVal);
		}
		zmask = zmask << 1;
	}
}



void groupVolDownHelper(int znumber, int prevVolValue)
{
	int delta;
	// hardware compensation...volVal is off by 2 values from what is displayed on the physical keypad when volumn down is used
	g_zoneInfo.zones[znumber].volValue -= 2;

	delta = ((int)g_zoneInfo.zones[znumber].volValue - prevVolValue);
	if (delta == -1)
	{
		if (g_groupVolValue > g_zoneInfo.zones[znumber].volValue)
			g_groupVolValue = g_zoneInfo.zones[znumber].volValue;
	}
}
void sendGroupVolDownToHW()
{
	int prevVolValue0, prevVolValue1, prevVolValue2, prevVolValue3, prevVolValue4, prevVolValue5;

	// loop through each selected zone and set the source
	// keep it simple and don't loop for now
	if (g_zoneButtonStates & ZONE1)
	{
		sendVolDownToHW(0);
	}
	if (g_zoneButtonStates & ZONE2)
	{
		sendVolDownToHW(1);
	}
	if (g_zoneButtonStates & ZONE3)
	{
		sendVolDownToHW(2);
	}
	if (g_zoneButtonStates & ZONE4)
	{
		sendVolDownToHW(3);
	}
	if (g_zoneButtonStates & ZONE5)
	{
		sendVolDownToHW(4);
	}
	if (g_zoneButtonStates & ZONE6)
	{
		sendVolDownToHW(5);
	}

	prevVolValue0 = g_zoneInfo.zones[0].volValue;
	prevVolValue1 = g_zoneInfo.zones[1].volValue;
	prevVolValue2 = g_zoneInfo.zones[2].volValue;
	prevVolValue3 = g_zoneInfo.zones[3].volValue;
	prevVolValue4 = g_zoneInfo.zones[4].volValue;
	prevVolValue5 = g_zoneInfo.zones[5].volValue;
	sendCMD_0x06(0);
//	printf("[%d %d %d %d %d %d] ", delta0, delta1, delta2, delta3, delta4, delta5);
	if (g_zoneButtonStates & ZONE1)
	{
		groupVolDownHelper(0, prevVolValue0);
	}
	if (g_zoneButtonStates & ZONE2)
	{
		groupVolDownHelper(1, prevVolValue1);
	}
	if (g_zoneButtonStates & ZONE3)
	{
		groupVolDownHelper(2, prevVolValue2);
	}
	if (g_zoneButtonStates & ZONE4)
	{
		groupVolDownHelper(3, prevVolValue3);
	}
	if (g_zoneButtonStates & ZONE5)
	{
		groupVolDownHelper(4, prevVolValue4);
	}
	if (g_zoneButtonStates & ZONE6)
	{
		groupVolDownHelper(5, prevVolValue5);
	}
}

void sendPowerOnToHW(BYTE zoneNum)
{
	sendCMD_0x04(zoneNum, 0x20);
}

void sendGroupPowerOnToHW()
{
	// loop through each selected zone 
	// keep it simple and don't loop for now
	if (g_zoneButtonStates & ZONE1)
	{
		sendPowerOnToHW(0);
	}
	if (g_zoneButtonStates & ZONE2)
	{
		sendPowerOnToHW(1);
	}
	if (g_zoneButtonStates & ZONE3)
	{
		sendPowerOnToHW(2);
	}
	if (g_zoneButtonStates & ZONE4)
	{
		sendPowerOnToHW(3);
	}
	if (g_zoneButtonStates & ZONE5)
	{
		sendPowerOnToHW(4);
	}
	if (g_zoneButtonStates & ZONE6)
	{
		sendPowerOnToHW(5);
	}
}

void sendPowerOffToHW(BYTE zoneNum)
{
	sendCMD_0x04(zoneNum, 0x21);
}

void sendGroupPowerOffToHW()
{
	// loop through each selected zone 
	// keep it simple and don't loop for now
	if (g_zoneButtonStates & ZONE1)
	{
		sendPowerOffToHW(0);
	}
	if (g_zoneButtonStates & ZONE2)
	{
		sendPowerOffToHW(1);
	}
	if (g_zoneButtonStates & ZONE3)
	{
		sendPowerOffToHW(2);
	}
	if (g_zoneButtonStates & ZONE4)
	{
		sendPowerOffToHW(3);
	}
	if (g_zoneButtonStates & ZONE5)
	{
		sendPowerOffToHW(4);
	}
	if (g_zoneButtonStates & ZONE6)
	{
		sendPowerOffToHW(5);
	}
}

void sendMuteToggleToHW(BYTE zoneNum)
{
	sendCMD_0x04(zoneNum, 0x22);
}

void sendGroupMuteToggleToHW()
{
	// loop through each selected zone 
	// keep it simple and don't loop for now
	if (g_zoneButtonStates & ZONE1)
	{
		sendMuteToggleToHW(0);
	}
	if (g_zoneButtonStates & ZONE2)
	{
		sendMuteToggleToHW(1);
	}
	if (g_zoneButtonStates & ZONE3)
	{
		sendMuteToggleToHW(2);
	}
	if (g_zoneButtonStates & ZONE4)
	{
		sendMuteToggleToHW(3);
	}
	if (g_zoneButtonStates & ZONE5)
	{
		sendMuteToggleToHW(4);
	}
	if (g_zoneButtonStates & ZONE6)
	{
		sendMuteToggleToHW(5);
	}
}

void sendCMD_0x04(BYTE zoneNum, BYTE data)
{
	int len, expectedLen;
	BYTE* buf;
	SerialRcv_Cmd_0x05_t rcvData[2];
	BYTE cmd[6];
	if ((zoneNum >= 0) && (zoneNum < MAX_ZONES))
	{
		cmd[0] = 0x02;
		cmd[1] = 0x00;
		cmd[2] = zoneNum + 1;	// hardware is one-based index
		cmd[3] = 0x04;
		cmd[4] = data;
		cmd[5] = cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4];

		serialSend(0, cmd, 6);
		nice();

		// expect 28 bytes back (2 x 14 cmd 5 messages (SerialRcv_Cmd_0x05_t))
		buf = (BYTE*)&rcvData;
		expectedLen = 2*sizeof(SerialRcv_Cmd_0x05_t);
		len = serialRcv(0, buf, expectedLen, 500);
		//just eat this data...don't do anything with it.
	}
}

void sendCMD_0x06(BYTE zoneNum)
{
	BYTE cmd[6];
	BYTE* buf;
	int len;
	SerialRcv_Cmd_0x06_Total_t* pCmd6RcvData;

	if ((zoneNum >= 0) && (zoneNum < MAX_ZONES))
	{
		cmd[0] = 0x02;
		cmd[1] = 0x00;
		cmd[2] = zoneNum + 1;	// hardware is one-based index
		cmd[3] = 0x06;
		cmd[4] = 0x00;
		cmd[5] = cmd[0]+cmd[1]+cmd[2]+cmd[3]+cmd[4];

		serialSend(0, cmd, 6);
		nice();

		//expect return data of 98 bytes for this specific command
		//buf = rbuffer;
		buf = (BYTE*)&g_zoneInfo;
		len = serialRcv(0, buf, 98, 500);
		if (len == 98)
		{
			g_bConnected = TRUE;
	//		printf("len = %d, zone=%d volValue=0x%x, InputPort=%d, LEDStatus=0x%x", len, g_selectedZone, g_zoneInfo.zones[g_selectedZone].volValue, g_zoneInfo.zones[g_selectedZone].inputPort, g_zoneInfo.zones[g_selectedZone].LEDStatus);	
//			sprintf(g_debugStr, "v0=0x%x, v1=0x%x, v2=0x%x, v3=0x%x, v4=0x%x, v5=0x%x", g_zoneInfo.zones[0].data1, g_zoneInfo.zones[1].data1,g_zoneInfo.zones[2].data1,g_zoneInfo.zones[3].data1,g_zoneInfo.zones[4].data1,g_zoneInfo.zones[5].data1);	
		}
		else
		{
			// didn't get the expected number of bytes from the serial port
			// if it's 0 then maybe there is no communications
			// if it's non-zero then maybe the protocol changed or the timeout isn't long enough?
			// TODO
			if (len == 0)
			{
				g_bConnected = FALSE;
			}
		}
	}
}

static int serialRcv(int chan, BYTE *buf, int len, int to)
{
	TCB save;
	CCB  *saveCCB;
	BYTE *ptr;
	int i;
	WORD x;

	/* Save the current associated stream */
	memcpy(&save, ActPro, sizeof(TCB));
	saveCCB = ActCCB;
	
	/* Set to new channel and send data */
	ActPro->CCB_Ptr			= &AllCCB[chan];
	ActPro->Chan_Nr			= chan;
	ActPro->IO_Ptr			= AllCCB[chan].IOPtrs;
 	ActCCB					= &AllCCB[chan];

	x=sticks;
	i=0;
	do {
		if( kbhit( ) )
			buf[i++]=getch( );
		else
		nice( );
	} while ( (i<len) && ((sticks - x) < to) );
	
	/* Restore old stream */
	memcpy(ActPro, &save, sizeof(TCB));
	ActCCB = saveCCB;

	return(i);
}

void process_form_tags(BYTE* buf, int len)
{
	int zoneNum, sourceNum;
	//*** assumes sbuffer still has the form tags from the http connection
	
	// get zone number
	zoneNum = getZoneNum(buf, len);
	if (zoneNum == -1)	// was not in the dropdown
		return;

	// get zone number
	sourceNum = getSourceNum(buf, len);
	if (sourceNum == -1)	// was not in the dropdown
		return;

	// did the user press the Set Zone button?
	if (findStr(buf, len, "SelZone"))
	{
		g_selectedZone = zoneNum;
		//sendSourceInputToHW(zoneNum, g_zoneInfo[zoneNum].inputCh);
		sendSourceInputToHW(zoneNum, g_zoneInfo.zones[zoneNum].inputPort);	
	}
	// did the user press the Set Input button?
	else if (findStr(buf, len, "ZSetInput"))
	{
		//g_zoneInfo[zoneNum].inputCh = sourceNum;
		g_zoneInfo.zones[g_selectedZone].inputPort = sourceNum;	
		sendSourceInputToHW(g_selectedZone, sourceNum);
	}
	else if (findStr(buf, len, "ZPwrOn"))
	{
		sendPowerOnToHW(g_selectedZone);
	}
	else if (findStr(buf, len, "ZPwrOff"))
	{
		sendPowerOffToHW(g_selectedZone);
	}
	else if (findStr(buf, len, "ZMute"))
	{
		sendMuteToggleToHW(g_selectedZone);
	}
	else if (findStr(buf, len, "ZVolUp"))
	{
		sendVolUpToHW(g_selectedZone);
		sendCMD_0x06(g_selectedZone);
	}
	else if (findStr(buf, len, "ZVolDown"))
	{
		sendVolDownToHW(g_selectedZone);
		sendCMD_0x06(g_selectedZone);
	}
	else
	{
		// no buttons pressed...
	}
	
}


void refreshData()
{
	if (g_isGroup == FALSE)
	{
		// query the zone info
		sendCMD_0x06(0);
		// the custom screen routine should update the page (like the volume dropdown to match the current zone info

		// update the selected source also
		g_selectedSource = g_zoneInfo.zones[g_selectedZone].inputPort;
	}
	else
	{
		updateGroupVolumeValue();
		// the custom screen routine should update the page (like the volume dropdown to match the current zone info
	}
}
void process_form_tags_keypad(BYTE* buf, int len)
{
	static BOOL s_powerToggle=TRUE;
	BOOL bZoneButtonPressed=FALSE;
	static BOOL s_groupPowerToggle=TRUE;
	int srcNum, volNum;
	int i;
	BOOL bHasSrcChanged=FALSE;


	//*** assumes sbuffer still has the form tags from the http connection

	// get the current hardware state since someone may have changed using another session
	refreshData();

	g_selectedZone = getZoneNumberFromHiddenInput(buf, len);

	g_isGroup = getIsGroupValueFromHiddenInput(buf, len);

	g_zoneButtonStates = getGroupMaskValueFromHiddenInput(buf, len);

	g_groupSource = getGroupSourceFromHiddenInput(buf, len);

	g_currentSource = getCurrentSourceFromHiddenInput(buf, len);

	if (g_currentSource != g_zoneInfo.zones[g_selectedZone].inputPort)
	{
		bHasSrcChanged = TRUE;	// the hidden field on the web page and the current hardware setting
								//	indicates that the source was changed by some other user web session
		g_currentSource = g_zoneInfo.zones[g_selectedZone].inputPort;
	}

	if (findStr(buf, len, BTNALLON))
	{
		for (i=0;i < MAX_ZONES;i++)
			sendPowerOnToHW(i);

		refreshData();	// needed to get the latest zone on/off state
	}
	else if (findStr(buf, len, BTNALLOFF))
	{
		for (i=0;i < MAX_ZONES;i++)
			sendPowerOffToHW(i);

		refreshData();	// needed to get the latest zone on/off state
	}
	else if (findStr(buf, len, BTNPWRON))
	{
		if (g_isGroup == FALSE)
		{
			if ((g_selectedZone >= 0) && (g_selectedZone < MAX_ZONES))
				sendPowerOnToHW(g_selectedZone);
		}
		else
		{
			sendGroupPowerOnToHW();
		}

		refreshData();	// needed to get the latest zone on/off state
	}
	else if (findStr(buf, len, BTNPWROFF))
	{
		if (g_isGroup == FALSE)
		{
			if ((g_selectedZone >= 0) && (g_selectedZone < MAX_ZONES))
				sendPowerOffToHW(g_selectedZone);
		}
		else
		{
			sendGroupPowerOffToHW();
		}

		refreshData();	// needed to get the latest zone on/off state
	}
	else if (findStr(buf, len, BTNMUTE))
	{
		if (g_isGroup == FALSE)
		{
			sendMuteToggleToHW(g_selectedZone);
		}
		else
		{
			sendGroupMuteToggleToHW();
		}
		refreshData();	// needed to get the latest mute on/off state
	}
	else if (findStr(buf, len, BTNSTATUS))
	{
		//refreshData();  //redundant now since refreshData() is now getting called at the top of this method.
	}
	else if (findStr(buf, len, BTNZ1))
	{
		if (g_isGroup == FALSE)
		{
			g_zoneButtonStates = 0;
			g_zoneButtonStates |= ZONE1;
			g_selectedZone = 0;
			g_selectedSource = g_zoneInfo.zones[g_selectedZone].inputPort;
			sendSourceInputToHW(g_selectedZone, g_zoneInfo.zones[g_selectedZone].inputPort);
		}
		else
		{
			if (g_zoneButtonStates & ZONE1)
			{
				g_zoneButtonStates &= ~ZONE1;
				// needs to be at least one zone button selected
				if (g_zoneButtonStates == 0)
					g_zoneButtonStates |= ZONE1;
			}
			else
				g_zoneButtonStates |= ZONE1;
		}
	}
	else if (findStr(buf, len, BTNZ2))
	{
		if (g_isGroup == FALSE)
		{
			g_zoneButtonStates = 0;
			g_zoneButtonStates |= ZONE2;
			g_selectedZone = 1;
			g_selectedSource = g_zoneInfo.zones[g_selectedZone].inputPort;
			sendSourceInputToHW(g_selectedZone, g_zoneInfo.zones[g_selectedZone].inputPort);
		}
		else
		{
			if (g_zoneButtonStates & ZONE2)
			{
				g_zoneButtonStates &= ~ZONE2;
				// needs to be at least one zone button selected
				if (g_zoneButtonStates = 0)
					g_zoneButtonStates |= ZONE2;
			}
			else
				g_zoneButtonStates |= ZONE2;
		}
	}
	else if (findStr(buf, len, BTNZ3))
	{
		if (g_isGroup == FALSE)
		{
			g_zoneButtonStates = 0;
			g_zoneButtonStates |= ZONE3;
			g_selectedZone = 2;
			g_selectedSource = g_zoneInfo.zones[g_selectedZone].inputPort;
			sendSourceInputToHW(g_selectedZone, g_zoneInfo.zones[g_selectedZone].inputPort);
		}
		else
		{
			if (g_zoneButtonStates & ZONE3)
			{
				g_zoneButtonStates &= ~ZONE3;
				// needs to be at least one zone button selected
				if (g_zoneButtonStates == 0)
					g_zoneButtonStates |= ZONE3;
			}
			else
				g_zoneButtonStates |= ZONE3;
		}
	}
	else if (findStr(buf, len, BTNZ4))
	{
		if (g_isGroup == FALSE)
		{
			g_zoneButtonStates = 0;
			g_zoneButtonStates |= ZONE4;
			g_selectedZone = 3;
			g_selectedSource = g_zoneInfo.zones[g_selectedZone].inputPort;
			sendSourceInputToHW(g_selectedZone, g_zoneInfo.zones[g_selectedZone].inputPort);
		}
		else
		{
			if (g_zoneButtonStates & ZONE4)
			{
				g_zoneButtonStates &= ~ZONE4;
						// needs to be at least one zone button selected
				if (g_zoneButtonStates == 0)
					g_zoneButtonStates |= ZONE4;
			}
			else
				g_zoneButtonStates |= ZONE4;
		}
	}
	else if (findStr(buf, len, BTNZ5))
	{
		if (g_isGroup == FALSE)
		{
			g_zoneButtonStates = 0;
			g_zoneButtonStates |= ZONE5;
			g_selectedZone = 4;
			g_selectedSource = g_zoneInfo.zones[g_selectedZone].inputPort;
			sendSourceInputToHW(g_selectedZone, g_zoneInfo.zones[g_selectedZone].inputPort);
		}
		else
		{
			if (g_zoneButtonStates & ZONE5)
			{
				g_zoneButtonStates &= ~ZONE5;
						// needs to be at least one zone button selected
				if (g_zoneButtonStates == 0)
					g_zoneButtonStates |= ZONE5;
			}
			else
				g_zoneButtonStates |= ZONE5;
		}
	}
	else if (findStr(buf, len, BTNZ6))
	{
		if (g_isGroup == FALSE)
		{
			g_zoneButtonStates = 0;
			g_zoneButtonStates |= ZONE6;
			g_selectedZone = 5;
			g_selectedSource = g_zoneInfo.zones[g_selectedZone].inputPort;
			sendSourceInputToHW(g_selectedZone, g_zoneInfo.zones[g_selectedZone].inputPort);
		}
		else
		{
			if (g_zoneButtonStates & ZONE6)
			{
				g_zoneButtonStates &= ~ZONE6;
						// needs to be at least one zone button selected
				if (g_zoneButtonStates == 0)
					g_zoneButtonStates |= ZONE6;
			}
			else
				g_zoneButtonStates |= ZONE6;
		}
	}
	else if (findStr(buf, len, BTNGRP))
	{
		if (g_isGroup == FALSE)
		{
			g_isGroup = TRUE;
		}
		else
		{
			g_zoneButtonStates = 0;
			g_isGroup = FALSE;
		}
	}
	else
	{
		// no buttons pressed...

		// submit was due to the OnChange javascript action from the dropdown lists
		// get source number from drop down list
		srcNum = getSrcNumberFromDropDown(buf, len);
		// get volume number from drop down list
		volNum = getVolNumberFromDropDown(buf, len);
		if ((srcNum == -1) && (volNum == -1))
		{
			// no buttons pressed and srcNum/volNum dropdowns not found...first load of page
			// query the zone info as someone may have modified things at the physical keypad
			//refreshData(); //redundant now since there's a refreshData() at the beginning of this method
		}
		else
		{
			if (bHasSrcChanged)
			{
				// if some other user web session has changed the source, don't set to a new source so we get synched back up
				srcNum = g_currentSource;
			}

			if ((srcNum >= 0) && (srcNum < MAX_SOURCES))
			{
				g_selectedSource = srcNum;
				if (g_isGroup == FALSE)
				{
					if (g_zoneInfo.zones[g_selectedZone].inputPort != srcNum)
					{
						g_currentSource = srcNum;
						g_zoneInfo.zones[g_selectedZone].inputPort = srcNum;
						sendSourceInputToHW(g_selectedZone, g_zoneInfo.zones[g_selectedZone].inputPort);
					}
				}
				else
				{
					g_groupSource = srcNum;
					sendGroupSourceInputToHW(g_groupSource);
				}
			}

			if ((volNum >= 0) && (volNum <= MAX_VOL_VALUE))
			{
				if (g_isGroup == FALSE)
				{
					sendVolToHW(g_selectedZone, volNum);
				}
				else
				{
					sendGroupVolToHW(volNum);
				}
			}
		}
	}

	// set the zone if a zone button was pressed and there's only one zone selected
/*	if (bZoneButtonPressed == TRUE)
	{
		if (getNumberOfZonesSelected() <= 1)
		{
			g_selectedZone = getFirstSelectedZone();
			sendSourceInputToHW(g_selectedZone, g_zoneInfo.zones[g_selectedZone].inputPort);
		}
	}
*/
}



int getNumberOfZonesSelected()
{
	int n;
	unsigned numBits;

	n=g_zoneButtonStates;
	for (numBits = 0; n; numBits++) 
		n &= n - 1;

	return numBits;
}

int getFirstSelectedZone()
{
	if (g_zoneButtonStates & ZONE1)
		return 0;
	if (g_zoneButtonStates & ZONE2)
		return 1;
	if (g_zoneButtonStates & ZONE3)
		return 2;
	if (g_zoneButtonStates & ZONE4)
		return 3;
	if (g_zoneButtonStates & ZONE5)
		return 4;
	if (g_zoneButtonStates & ZONE6)
		return 5;

	return 0;
}

void process_form_tags_options(BYTE* buf, int len)
{
	int zoneNum, sourceNum;
	BYTE newName[MAX_NAME_LEN];
	BYTE newNameFinal[MAX_NAME_LEN];
	int newNameLen;

	//*** assumes sbuffer still has the form tags from the http connection
	
	// get zone number
	zoneNum = getZoneNum(buf, len);
	if (zoneNum == -1)	// was not in the dropdown
		return;

	// get zone number
	sourceNum = getSourceNum(buf, len);
	if (sourceNum == -1)	// was not in the dropdown
		return;

	// did the user press the Control button?
	if (findStr(buf, len, "ZCtrl"))
	{
		// This should not be possible 
		//	because the html should be using onclick="location.href='zones.html'"
		//	and the button type should be "button" (not "submit")
	}
	// did the user press the Rename Zone button?
	else if (findStr(buf, len, "RenZone"))
	{
		// get the new name from the edit field NewZoneName
		newNameLen = getNewZoneName(zoneNum, buf, len, newName);
		if (newNameLen != -1)
		{
			g_selectedZone = zoneNum;

			memset(g_zoneNames[zoneNum], 0,MAX_NAME_LEN); 
			urldecode(newName, newName+newNameLen, g_zoneNames[zoneNum]);

			writeSettingsToFlash();
		}
	}
	// did the user press the Rename Source button?
	else if (findStr(buf, len, "RenSrc"))
	{
		// get the new name from the edit field NewSourceName
		newNameLen = getNewSourceName(sourceNum, buf, len, newName);
		if (newNameLen != -1)
		{
			g_selectedSource = sourceNum;

			memset(g_srcNames[sourceNum], 0,MAX_NAME_LEN); 
			urldecode(newName, newName+newNameLen, g_srcNames[sourceNum]);

			writeSettingsToFlash();
		}
	}
	else
	{
		// no buttons pressed...
	}
	
}

int urldecode(char *src, char *last, char *dest)
{
	int code;
	BYTE codeStr[5];

	for (; src != last; src++, dest++)
	{
		if (*src == '+') *dest = ' ';
		else if(*src == '%') 
		{
			codeStr[0] = *(src + 1);
			codeStr[1] = *(src + 2);
			codeStr[2] = 0;
			*dest = a2toh(codeStr);
			src +=2;
		}
		else *dest = *src;
	}
	//*dest = '\n';
	*++dest = '\0';
	return 0;
}

int findStr(BYTE *src, int srcLen, BYTE* strToFind)
{
	int i;

	for (i = 0; i < srcLen; i++) 
	{
		if (!(strncmp(src+i, strToFind, strlen(strToFind)))) 
		{
			return 1;
		}
	}
	return 0;
}

void getControllerData()
{
	sendCMD_0x06(0);
}


static void http_reply(int code)
{
	printf("HTTP/1.1 %u ", code);
	if (code == 200) {
		printf("OK\r\n");
		printf("Content-type: text/html\r\n");
		//printf("Cache-Control: max-age=3600\r\n");
	}
	else if (code == 404)
		printf("Not Found");
	printf("\r\n\r\n");
}

static int serialSendRcv(int chan, BYTE *buf, int len, int to)
{
	TCB save;
	CCB  *saveCCB;
	BYTE *ptr;
	int i;
			/* Save the current associated stream */
	memcpy(&save, ActPro, sizeof(TCB));
	saveCCB = ActCCB;
			/* Set to new channel and send data */
	ActPro->CCB_Ptr			= &AllCCB[chan];
	ActPro->Chan_Nr			= chan;
	ActPro->IO_Ptr			= AllCCB[chan].IOPtrs;
 	ActCCB					= &AllCCB[chan];
	FlushIn();
	sendblk(buf, len);

	sdelay(to);
	ptr = rbuffer;
	memset(rbuffer, 0, sizeof(rbuffer));
	len = kbhit();
	for (i = 0; i < ((len > sizeof(rbuffer)-1) ? sizeof(rbuffer) - 1 : len); i++) {
		*ptr++ = getch();
	}
			/* Restore old stream */
	memcpy(ActPro, &save, sizeof(TCB));
	ActCCB = saveCCB;

	return(len);
}

static int serialSend(int chan, BYTE *buf, int len)
{
	TCB save;
	CCB  *saveCCB;
	BYTE *ptr;
	int i;
			/* Save the current associated stream */
	memcpy(&save, ActPro, sizeof(TCB));
	saveCCB = ActCCB;
			/* Set to new channel and send data */
	ActPro->CCB_Ptr			= &AllCCB[chan];
	ActPro->Chan_Nr			= chan;
	ActPro->IO_Ptr			= AllCCB[chan].IOPtrs;
 	ActCCB					= &AllCCB[chan];
	FlushIn();
	sendblk(buf, len);
			/* Restore old stream */
	memcpy(ActPro, &save, sizeof(TCB));
	ActCCB = saveCCB;

	return(len);
}

int raw_gwrite(int web_page, WORD off, BYTE far *buf, WORD len, WORD erase)
{
	DWORD addr;
#if defined(XPORT) || defined(M100)
	WORD tmp, pageNum, pageOffset;
#endif


#ifdef V6
	if ((web_page < 1) || (web_page > HW.web_pages))
		return(-1);
#else
	if (web_page > (HW.f_pages - 1))
		return(-1);
#endif
	if (len == 0)
		return(0);

#ifdef V6
	web_page = ((web_page - 1) * HW.page_size) + HW.web_start;
#else
	web_page *= (HW.f_mask + 1);
	web_page += HW.f_start;
#endif
	addr = (DWORD) MK_FP((WORD)(web_page), (WORD)(off));

#if !defined(XPORT) && !defined(M100)
	if (erase)
		flsh_clr(0, FP_SEG(addr));
#endif
	flsh_pgm(FP_OFF(addr), FP_SEG(addr), FP_OFF(buf), FP_SEG(buf), len);

	return(len);
}

int raw_gread(int web_page, WORD off, BYTE far *buf, WORD len)
{
	BYTE far *addr;
	BYTE far *p;
	int i;
#if defined(XPORT) || defined(M100)
	extern void addr2pageInfo(void far *addr,  WORD *pageNo, WORD *pageOff);
	WORD tmp, pageNum = 0, pageOffset = 0;
#endif


#ifdef V6
	if ((web_page < 1) || (web_page > HW.web_pages))
		return(-1);
#else
	if (web_page > (HW.f_pages - 1))
		return(-1);
#endif
	if (len == 0)
		return(0);

#ifdef V6
	web_page = ((web_page - 1) * HW.page_size) + HW.web_start;
#else
	web_page *= (HW.f_mask + 1);
	web_page += HW.f_start;
#endif
	addr = MK_FP((WORD)(web_page), (WORD)(off));


#if defined(XPORT) || defined(M100)
#ifdef V6
	addr2pageInfo(addr, &pageNum, &pageOffset);
	while(tEcSppSerFlashRead(pageNum, pageOffset, (DWORD)len, (BYTE far *)buf) != 0);
#else
	tmp    = ((FP_SEG(addr) - HW.f_start - (HW.f_mask+1))>>8);
	pageNum    = tmp * 248 + FP_OFF(addr)/EC_SERFL_PAGE_SIZE;
	pageOffset = tmp * 64  + FP_OFF(addr)%EC_SERFL_PAGE_SIZE;
	while(tEcSppSerFlashRead(pageNum+WEB1_START_PAGE, pageOffset, (DWORD)len, (BYTE far *)&buf[0])!= 0);
#endif

#else
	movedata(FP_SEG(addr), FP_OFF(addr), FP_SEG(buf), FP_OFF(buf), len);

#endif

	return(len);
}

void raw_gflash_clr(int web_page, WORD off)
{
	DWORD addr;
#if defined(XPORT) || defined(M100)
	WORD tmp, pageNum, pageOffset;
#endif


#ifdef V6
	if ((web_page < 1) || (web_page > HW.web_pages))
		return(-1);
#else
	if (web_page > (HW.f_pages - 1))
		return(-1);
#endif

#ifdef V6
	web_page = ((web_page - 1) * HW.page_size) + HW.web_start;
#else
	web_page *= (HW.f_mask + 1);
	web_page += HW.f_start;
#endif
	addr = (DWORD) MK_FP((WORD)(web_page), (WORD)(off));

#if !defined(XPORT) && !defined(M100)
	flsh_clr(0, FP_SEG(addr));
#endif

}
