#include "MEK101.H"

typedef struct TNODE_{
  unsigned char data[1024];//max 256 + 256 + 256 = TS + ADDRLO + ADDRHI
  unsigned char time[7];
  }TNODE;

int SetSporadicData(unsigned char *OIKChangedData, int SizeChangedData, unsigned char *OIKtime, unsigned char id);
int InstallSporadicData(unsigned char id);
int GetSporadicData(unsigned char *OIKdata, int size);
int GetSporadicFromOnce( unsigned char id, unsigned char *OIKdata, int max);
int createHeadNode(unsigned char *OIKdata, unsigned char size);
int createTailNode(unsigned char *OIKdata, unsigned char offset);