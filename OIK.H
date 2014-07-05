#define KPID 0x52

void OIKparse(unsigned char *OIKdata, unsigned char OIKsize);
int OIKSendSporadic(unsigned char n, unsigned char *sporadicData);
unsigned char SendRequestOnTU( unsigned char id, unsigned char *request );
int CreateMBUS_TU( unsigned char id, unsigned char modbus, unsigned char haddr, unsigned char laddr, unsigned char *OIKdata);
int CreateDCON_TU( unsigned char id, unsigned char modbus, unsigned char haddr, unsigned char laddr, unsigned char *OIKdata);