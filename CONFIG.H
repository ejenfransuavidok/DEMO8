#ifndef __CONFIG_H__
#define __CONFIG_H__
#include"c:\7188OS7\DEMO\lib\I7188.h"
#include"DEVICE.H"

#define TU 1
#define TS 2
#define TIT 3
#define MERQURY 4

typedef struct _GLOBALSTRUCT_ {
	unsigned char mbus;
	unsigned char type;
	unsigned char com;
	unsigned char regs;
	unsigned char maska[32];
	unsigned char password[8];
	unsigned short cycle;
	unsigned short begin;
	unsigned short end;
	unsigned char tic;
	unsigned char proto;
} GLOBALSTRUCT;

#define TSPREAMBLE 0xFF
#define TUPREAMBLE 0xFE
/*
typedef struct  {
  unsigned mark;
  unsigned char fname[12];
  unsigned char year;
  unsigned char month;
  unsigned char day;
  unsigned char hour;
  unsigned char minute;
  unsigned char sec;
  unsigned long size;
  char far *addr;
  unsigned CRC;
  unsigned CRC32;
} FILE_DATA;
*/

unsigned char ReadConfigFile();
#endif