#ifndef __MERQURY_H__
#define __MERQURY_H__

#define DIAP_LO_MERQURY 50
#define DIAP_HI_MERQURY 99

typedef unsigned char byte;
typedef enum MERQURY_STATUS_ { ON, OFF } MERQURY_STATUS;

typedef struct _MERQURY_{
	byte address;
	byte timer1;
	byte timer2;
	byte com_port;
	int cycl_oprosa;
	int cycl_oprosa_const;
	int cycl_timer2;
	byte* password;
	MERQURY_STATUS status;
	int cycle_counter;
	unsigned char maska_param[32];
} MERQURY;

int OpenMerqury(byte address, byte* password, int size, byte timer1, byte timer2);
int IsOpenedMerqury(byte address);
int IndexMerqury(byte i);
byte GetTimer1_Merqury(int n);
byte BusyMerqury(int n);
void ToMerquryData(int n);
int DataTransmittedMerqury(int n);
byte GetTimer2_Merqury(int n);
int DataReceivedMerqury(int n);
byte ReadBufferMerqury(int n);
void ClearComMerqury(int n);
void ClearBusyMerqury(int n);
int InstallMerqury(
	byte address,
	byte* password,
	byte com_port,
	byte timer1,
	byte timer2,
	int size,
	int cycl_oprosa,
	int cycl_timer2,
	unsigned char *maska);
int getCntMerqury();
int GetCyclTimer2_Merqury(int n);
byte GetComPortMerqury(int id);
void MQ_Init();
void MQ_GetI();
void MQ_GetU();
void MQ_GetP();
#endif