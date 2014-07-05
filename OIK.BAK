#include<dos.h>
#include"COM3COM4.H"
#include"OIK.H"
#include"MEK101.H"
#include"101.H"
#include"NODE.H"

struct date _date;
struct time _time;

static unsigned long int TSTimer=0;
static unsigned char TSbuffer[256];
static unsigned char pTScurrent=0;
static unsigned char pTSlast=0;

static unsigned char TSbufferSporadic[512];
static unsigned char pTScurrentSporadic=0;
static unsigned char pTSlastSporadic=0;

static unsigned long int TITTimer=0;
static unsigned char TITbuffer[256];
static unsigned char pTITcurrent=0;
static unsigned char pTITlast=0;

static unsigned char LastTUBuffer[15];

unsigned char ask[5] = {0x10, 0x00, KPID, 0x00 + KPID, 0x16};

unsigned char OIKsum(unsigned char *OIKdata, unsigned char OIKsize)
{
  unsigned char i, sum=0;
  switch(OIKdata[0]){
    case 0x10:
      return (OIKdata[1] + OIKdata[2]);
    case 0x68:
      for(i = 0; i < OIKsize; i++) {
	sum += OIKdata[i + 4];
      }
      return sum;
    default:
      return 0;
  }
}

int OIKvalid(unsigned char *OIKdata, unsigned char OIKsize)
{
  if(OIKsize == 5){
    if( (OIKdata[0] == 0x10)&&(OIKdata[4] == 0x16) ){
      return ( (OIKsum( OIKdata, OIKsize ) == OIKdata[3])&&(OIKdata[2] == KPID) );
    }
  } else if(OIKdata[0] == 0x68){
    if(OIKsize < 9)
      return 0;
    //Print( "SUMM=%x %x %d\n\r", OIKsum(OIKdata, OIKsize - 2), OIKdata[OIKsize - 2], OIKsize );
    return ( (OIKdata[3] == 0x68)&&(OIKdata[1] == OIKdata[2])&&
	     (OIKdata[5] == KPID)&&(OIKdata[9] == KPID)&&
	     (OIKsum(OIKdata, OIKsize - 6) == OIKdata[OIKsize - 2]) );
  }
  return 0;
}

void OIKparse(unsigned char *OIKdata, unsigned char OIKsize)
{
  switch(OIKdata[0]){
    case 0x10:
      OIKx10(OIKdata, OIKsize);
      break;
    case 0x68:
      OIKx68(OIKdata, OIKsize);
      break;
    default:
      return;
  }
}

int OIKx10(unsigned char *OIKdata, unsigned char OIKsize)
{
  unsigned char sporadicData[512];
  unsigned char OIKoutput[5];
  int sporadicSize = 0, i;
  OIKoutput[0] = 0x10;
  OIKoutput[1] = 0x0B;
  OIKoutput[2] = KPID;
  OIKoutput[4] = 0x16;
  switch(OIKdata[1]){
    case 0x49:
      OIKoutput[1] = 0x0B;
      break;
    case 0x40:
      OIKoutput[1] = 0x00;
      break;
    case 0x5B: case 0x5A: case 0x7B: case 0x7A:
      if( sporadicSize = GetSporadicData(sporadicData, 512) ) {
	ToCom3Str(sporadicData, sporadicSize);
	return;
      }
      if( needSendTsToOIK() ){
	return SendTStoOIK();
      } else if( needSendTitToOIK() ){
	return SendTITtoOIK();
      } else {
	OIKoutput[1] = 0x09;
      }
      break;
    default:
      return;
  }
  OIKoutput[3] = OIKsum(OIKoutput, 5);
  ToCom3Str(OIKoutput, 5);
}

int needSendTsToOIK()
{
  if( TSTimer < getGlobalTimer() ) {
    TSTimer = getGlobalTimer() + CICL_TS;
    CreateTSbuffer();
    return 1;
  } else if(pTScurrent != pTSlast) {
    return 1;
  } else if( GetOIKTrigger() ) {
    UnsetOIKTrigger();
    TSTimer = getGlobalTimer() + CICL_TS;
    CreateTSbuffer();
    return 1;
  }
  return 0;
}

int needSendTitToOIK()
{
  if( TITTimer < getGlobalTimer() ){
    TITTimer = getGlobalTimer() + CICL_TIT;
    CreateTITbuffer();
    return 1;
  } else if(pTITcurrent != pTITlast){
    return 1;
  }
  return 0;
}

int CreateTSbuffer()
{
  unsigned char i, j, k;
  unsigned short w, maska, isvalid;
  pTSlast=0;
  pTScurrent = 0;
  for(i = 0; i < getCnt101(); i++){
    isvalid = IsData101VALID( i );
    if(getFunc101(i) == TS){
      for(j = 0; j < getRegnum101(i); j++){
	w = getDataByte101(i, 2*j + 1) + ((unsigned short)getDataByte101(i, 2*j)<<8);
	maska = 0x0001;
	for(k = 0; k < 16; k++){
	  TSbuffer[pTSlast] = ((w&maska) != 0);
	  TSbuffer[pTSlast] |= isvalid;
	  pTSlast++;
	  maska = maska << 1;
	}
      }
    }
  }
  return pTSlast;
}

int TestMaska(unsigned char *maska, int nreg)
{
  unsigned char masks[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
  unsigned char nbyte = nreg/8;
  unsigned char nbit = nreg%8;
  return maska[nbyte]&masks[nbit];
}

int CreateTITbuffer()
{
  unsigned char i=0, j=0, k=0;
  pTITlast=0;
  pTITcurrent = 0;
  for(i = 0; i < getCnt101(); i++){
    if(getFunc101(i) == TIT){
      j = 0;
      while( j < getRegnum101(i) ){
	if( TestMaska( getMaska(i), j ) == 0 ){
	  j++;
	  continue;
	}
	TITbuffer[pTITlast++] =  getDataByte101(i, 2 * j);
	TITbuffer[pTITlast++] =  getDataByte101(i, 2 * j + 1);
	TITbuffer[pTITlast++] = (getValid(i) == VALID)?0x00:0x80;
	j++;
      }
    }
  }
  return pTITlast;
}

unsigned char getHowManyTS()
{
  unsigned char all = pTSlast - pTScurrent;
  if(all > MAX_TS_INTO_MAILBOX){
    return MAX_TS_INTO_MAILBOX;
  } else {
    return all;
  }
}

unsigned char getHowManyTIT()
{
  unsigned char all = pTITlast - pTITcurrent;
  if( ( all / 3 ) > MAX_TIT_INTO_MAILBOX ){
    return MAX_TIT_INTO_MAILBOX;
  } else {
    return all / 3;
  }
}

short int getStartAddrTS()
{
  return STARTNUM_TS + pTScurrent;
}

short int getStartAddrTIT()
{
  return STARTNUM_TIT + pTITcurrent/3;
}

int SendTStoOIK()
{
  unsigned char i, sum;
  unsigned char OIKdata[256];
  unsigned char howmany = getHowManyTS();
  OIKdata[0] = 0x68;
  OIKdata[2] = OIKdata[1] = howmany + 8;
  OIKdata[3] = 0x68;
  OIKdata[4] = 0x08;
  OIKdata[5] = ADDR_STATION;
  OIKdata[6] = 0x01;
  OIKdata[7] = howmany|0x80;
  OIKdata[8] = 0x14;
  OIKdata[9] = ADDR_STATION;
  OIKdata[10]= (getStartAddrTS()&0x00FF);
  OIKdata[11]= (getStartAddrTS()&0xFF00)>>8;
  for(i = 12; i < howmany + 12; i++){
    OIKdata[i] = TSbuffer[pTScurrent++];
  }
  sum = OIKsum(OIKdata, howmany + 8);
  OIKdata[howmany + 12] = sum;
  OIKdata[howmany + 13] = 0x16;
  ToCom3Str(OIKdata, howmany + 14);
  return howmany;
}

int SendTITtoOIK()
{
  unsigned char i, sum;
  unsigned char OIKdata[256];
  unsigned char howmany = getHowManyTIT();
  OIKdata[0] = 0x68;
  OIKdata[2] = OIKdata[1] = 3*howmany + 8;
  OIKdata[3] = 0x68;
  OIKdata[4] = 0x08;
  OIKdata[5] = ADDR_STATION;
  OIKdata[6] = 0x09;
  OIKdata[7] = howmany|0x80;
  OIKdata[8] = 0x14;
  OIKdata[9] = ADDR_STATION;
  OIKdata[10]= (getStartAddrTIT()&0x00FF);
  OIKdata[11]= (getStartAddrTIT()&0xFF00)>>8;
  for(i = 12; i < 3*howmany + 12; i++){
    OIKdata[i] = TITbuffer[pTITcurrent++];
  }
  sum = OIKsum(OIKdata, 3*howmany + 8);
  OIKdata[3*howmany + 12] = sum;
  OIKdata[3*howmany + 13] = 0x16;
  ToCom3Str(OIKdata, 3*howmany + 14);
  return howmany;
}

int OIKx68(unsigned char *OIKdata, unsigned char OIKsize)
{
  switch(OIKdata[6]){
    case 0x67:
      TimeSynhr(OIKdata, OIKsize);
      break;
    case 0x64:
      ToCom3Str(ask, 5);
      break;
    case 0x2D:
      SendTU(OIKdata, OIKsize);
      break;
    default:
      return 0;
  }
}

int SendTU(unsigned char *OIKdata, unsigned char OIKsize)
{
  unsigned char laddr = OIKdata[10] - 1;
  unsigned char haddr = OIKdata[11];
  unsigned short addr = (haddr << 8) + laddr;
  unsigned char id    = GetID_TU101_by_addr( addr );
  unsigned char modbus= Get_MODBUS_TU101( id );
  unsigned char com   = Get_COM_TU101( id );
  unsigned char proto = GetTUProto( id );
  unsigned char i;
  Print("id = %d\naddr = %x\nmodbus = %d\ncom = %d\nOIKSize = %d\n", id, addr, modbus, com, OIKsize);
  for( i=0; i<15; i++ ) {
    LastTUBuffer[i] = OIKdata[i];
  }
  if(( modbus == (unsigned char)-1 )||( com == (unsigned char)-1 )) {
    Print("Error address TU101!!!");
    return -1;
  }
  else {
    switch( proto ) {
      case PROTO_MBUS:
	return CreateMBUS_TU( id, modbus, haddr, laddr, OIKdata );
      case PROTO_DCON:
	return CreateDCON_TU( id, modbus, haddr, laddr, OIKdata );
    }
  }
}

int CreateDCON_TU( unsigned char id, unsigned char modbus, unsigned char haddr, unsigned char laddr, unsigned char *OIKdata) {
  unsigned char command[8];
  unsigned short crc=0;
  unsigned char addr=0;
  unsigned char crchi=0, crclo=0, dconhi=0, dconlo=0, addrhi=0, addrlo=0, objaddr=0;
  unsigned char i=0;
  DigitToSymbols(modbus, &dconhi, &dconlo);
  addr = ((unsigned short)(haddr << 8) + laddr) - GetTU101_begin_addr( id );
  objaddr = 0x01 << (addr * 2 + (OIKdata[12] != 1) );
  DigitToSymbols(objaddr, &addrhi, &addrlo);
  command[0] = 0x40;
  command[1] = dconhi;
  command[2] = dconlo;
  command[3] = addrhi;
  command[4] = addrlo;
  DigitToSymbols(command[0]+command[1]+command[2]+command[3]+command[4], &crchi, &crclo);
  command[5] = crchi;
  command[6] = crclo;
  command[7] = 0x0D;
  return SetTUFlag( id, command );
}

int CreateMBUS_TU( unsigned char id, unsigned char modbus, unsigned char haddr, unsigned char laddr, unsigned char *OIKdata) {
  unsigned char command[8];
  unsigned short crc=0;
  command[0] = modbus;
  command[1] = 0x05;
  command[2] = haddr;
  command[3] = laddr;
  command[4] = OIKdata[12] * 0xFF;
  command[5] = 0x00;
  crc	     = CRC16asm(command, 6);
  command[6] = (crc&0x00FF);
  command[7] = (crc&0xFF00)>>8;
  return SetTUFlag( id, command );
}

int compare( unsigned char *one, unsigned char *two, unsigned char size ) {
    unsigned char i;
    int r=0;
    for( i=0; i<size; i++ ) {
      r += (one[i] - two[i]);
    }
    return (r == 0);
}

unsigned char SendRequestOnTU( unsigned char id, unsigned char *request ) {
    unsigned char *tubuf = GetTUBuffer(id);
    unsigned char i;
    int s=0;
    LastTUBuffer[4] = 0x08;
    LastTUBuffer[8] = 0x07;
    for(i=4; i<13; i++) {
      s += LastTUBuffer[i];
    }
    LastTUBuffer[13] = s;
    if( compare(tubuf, request, 8) == 0 ) {
      Print("Valid TU\n\t");
      ToCom3Str( LastTUBuffer, 15 );
      return 1;
    } else {
      Print( "invalid TU\n\t" );
      return 0;
    }
}

int OIKx5B(unsigned char *OIKdata, unsigned char OIKsize)
{

}

int TimeSynhr(unsigned char *OIKdata, unsigned int OIKsize)
{
  unsigned int year = OIKdata[18];
  unsigned int mon = OIKdata[17];
  unsigned int day = OIKdata[16];
  unsigned int hour = OIKdata[15];
  unsigned int min = OIKdata[14];
  unsigned int sec = ( (unsigned int)(OIKdata[13]<<8) + OIKdata[12] )/1000;
  Print("year = %d, mon = %d, day = %d, hour = %d, min = %d, sec = %d\n\r",
    year, mon, day, hour, min, sec);
  return setDateTime( year, mon, day, hour, min, sec );
}

int setDateTime(unsigned int year, unsigned int mon, unsigned int day, unsigned int hour, unsigned int min, unsigned int sec)
{
  SetTime(hour, min, sec);
  SetDate(2000+year, mon, day);
  return 1;
}

int getHowManyTSSporadic(unsigned char *sporadicData)
{
  int i, counter=0;
  for(i = 0; i < 256; i++)
    counter += (sporadicData[i] != 0);
  return counter;
}

int CreateTSbufferSporadic(unsigned char n, unsigned char *sporadicData)
{
  unsigned char i, nbyte, nbit;
  pTSlastSporadic=0;
  pTScurrentSporadic = 0;
  for(i = 0; i < 256; i++){
    if(sporadicData[i] != 0){
      nbyte = i/8;
      nbit  = i%8;
      TSbufferSporadic[pTSlastSporadic++] = STARTNUM_TS + i;
      TSbufferSporadic[pTSlastSporadic++] = getDataByte101(n, nbyte)&&nbit;
    }
  }
  return pTSlastSporadic;
}

int SendTStoOIKSporadic(unsigned char *sporadicData)
{
  unsigned char i, sum;
  unsigned char OIKdata[256];
  unsigned char howmany = getHowManyTSSporadic(sporadicData);
  OIKdata[0] = 0x68;
  OIKdata[2] = OIKdata[1] = howmany + 6 + howmany * 2;
  OIKdata[3] = 0x68;
  OIKdata[4] = 0x08;
  OIKdata[5] = ADDR_STATION;
  OIKdata[6] = 0x01;
  OIKdata[7] = howmany|0x80;
  OIKdata[8] = 0x14;
  OIKdata[9] = ADDR_STATION;
  for(i = 10; i < 3*howmany + 10; i++){
    OIKdata[i] = TSbufferSporadic[pTScurrentSporadic++];
  }
  InsertDateTime( OIKdata, 10 + 3*howmany );
  sum = OIKsum(OIKdata, 3*howmany + 6 + 7);
  OIKdata[10 + 3*howmany + 7] = sum;
  OIKdata[10 + 3*howmany + 7 + 1] = 0x16;
  ToCom3Str(OIKdata, 10 + 3*howmany + 7 + 1);
  return howmany;
}

int OIKSendSporadic(unsigned char n, unsigned char *sporadicData)
{
  int y, m, d, h, min, sec;
  unsigned char OIKtime[7];
  GetDate(&y, &m, &d);
  GetTime(&h, &min, &sec);
  OIKtime[6-0] = (unsigned char)(y - 2000);
  OIKtime[6-1] = (unsigned char)(m);
  OIKtime[6-2] = (unsigned char)(d);
  OIKtime[6-3] = (unsigned char)(h);
  OIKtime[6-4] = (unsigned char)(min);
  OIKtime[6-6] = (unsigned char)((sec*1000)&0x00FF);
  OIKtime[6-5] = (unsigned char)(((sec*1000)&0xFF00)>>8);
  Print("sec = %d\n\r", sec);
  return SetSporadicData(sporadicData, 32, OIKtime, n);
}

int InsertDateTime(unsigned char *OIKdata, unsigned char offset)
{
  int year, mon, day, hour, min, sec;
  unsigned char mhi, mlo;
  GetDate(&year, &mon, &day);
  GetTime(&hour, &min, &sec);
  mlo = (sec * 1000) >> 8;
  mhi = ( (sec * 1000)&0xFF00 )>>8;
  OIKdata[offset++] = mlo;
  OIKdata[offset++] = mhi;
  OIKdata[offset++] = min;
  OIKdata[offset++] = hour;
  OIKdata[offset++] = day;
  OIKdata[offset++] = mon;
  OIKdata[offset++] = year;
  return offset;
}