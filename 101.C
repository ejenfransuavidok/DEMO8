#include "MEK101.H"
#include"101.h"
#include"NODE.H"
#define WATCHDOGCOM1 100
#define WATCHDOGCOM2 101

static Data101 *data101[100];
static TU101 *dataTU101[100];
static unsigned char cnt101 = 0;
static unsigned char cntTU101 = 0;
static unsigned char OIKTRIGGER = 0;
static unsigned char DCONTable[256];

unsigned char IsAnyDataINVALID() {
  int i=0;
  for( i=0; i < cnt101; i++ ) {
    if( IsData101VALID(i) == INVALID )
      return 1;
  }
  return 0;
}

unsigned char IsData101VALID( unsigned char id ) {
  return data101[ id ]->validate;
}

unsigned char *GetCommandOffDCON( unsigned char id ) {
  return dataTU101[id]->DCON_OffCommand;
}

void CreateCommandOffDCON( unsigned char id ) {
  unsigned char dconhi, dconlo, crchi, crclo;
  unsigned char *command = GetCommandOffDCON( id );
  DigitToSymbols(Get_MODBUS_TU101(id), &dconhi, &dconlo);
  command[0] = 0x40;
  command[1] = dconhi;
  command[2] = dconlo;
  command[3] = 0x30;
  command[4] = 0x30;
  DigitToSymbols(command[0]+command[1]+command[2]+command[3]+command[4], &crchi, &crclo);
  command[5] = crchi;
  command[6] = crclo;
  command[7] = 0x0D;
}

unsigned char DecrementTic( unsigned char id ) {
  if( (GetProto(id) == PROTO_DCON)&&(dataTU101[ id ]->var_tic > 0) ) {
    if( --dataTU101[ id ]->var_tic == 0 ) {
       Print( "%d: set TU, tic: %d\n\r", id, dataTU101[ id ]->var_tic );
       SetTUFlag( id, GetCommandOffDCON(id) );
    }
  }
}

unsigned char SetTic( unsigned char id ) {
  unsigned char *buffer = GetTUBuffer( id );
  if( GetProto(id) == PROTO_DCON ) {
    //if before have been on command
    if( (buffer[ 3 ] != 0x30)||(buffer[ 4 ] != 0x30) ) {
      //Print( "set tic: 3->%x 4->%x\n\r", buffer[3], buffer[4] );
      return dataTU101[ id ]->var_tic = dataTU101[ id ]->tic;
    }
  }
  return 0;
}

unsigned char TU_Valid( unsigned char id ) {
  if( GetProto(id) == PROTO_MBUS )
    return TU_MBUSValid( id );
  else
    return TU_DCONValid( id );
}

unsigned char TU_MBUSValid( unsigned char id ) {
  unsigned char array[256];
  ReadBuffer101IntoArray( id, array );
  return array[1] != 0x85;
}

unsigned char TU_DCONValid( unsigned char id ) {
  unsigned char array[256];
  ReadBuffer101IntoArray( id, array );
  return ((array[0] == 0x3E)&&
	  (array[1] == 0x33)&&
	  (array[2] == 0x45)&&
	  (array[3] == 0x0D));
}

unsigned char GetParentTS(int n)
{
  return dataTU101[ n ]->parentTS;
}

unsigned char GetTUCommandLen(int n)
{
  return dataTU101[ n ]->commandlen;
}

unsigned char GetCommandLen(int n)
{
  return data101[ n ]->commandlen;
}

unsigned char *GetCommand(int n)
{
  return data101[ n ]->command;
}

unsigned char GetProto(int n)
{
  return data101[ n ]->Proto;
}

int getCnt101()
{
  return cnt101;
}

int getCntTU101()
{
  return cntTU101;
}

unsigned char *GetTUBuffer( unsigned char id )
{
  return data101[ id ]->TUBuffer;
}

int Get101IDByCom( unsigned char com )
{
  unsigned char i=0;
  for( i=0; i<cnt101; i++ ){
    if( data101[ i ]->com_port == com )
      return i;
  }
  return -1;
}

int GetTUFlag( unsigned char id )
{
  return data101[ id ]->TUFlag;
}

int ClearTUFlag( unsigned char id )
{
  data101[ id ]->TUFlag = 0;
  return 1;
}

int SetTUFlag( int tuid, unsigned char *buffer )
{
  //after received tuid gettig comport and looking for
  //object into TS where comport is same as com tuobject
  //else you can't send tu command
  unsigned char i=0, id=GetParentTS( tuid );
  //if( tuid == -1)
  //  return -1;
  //while( (id<cnt101)&&(dataTU101[tuid]->com_port!=data101[id]->com_port) ){
  //  id++;
  //}
  //if( id == cnt101 ) {
  //  Print( "You can't send tu: is no TS com%d\n\r", dataTU101[tuid]->com_port );
  //  exit(0);
  //}
  for( i=0; i<8; i++ ) {
    data101[id]->TUBuffer[i] = buffer[i]; //Print("%d:[%d]=%x\n\r", id, i, buffer[i]);
  }
  data101[id]->TUFlag = 1;
  return 1;
}

int IncTUFlag( int id ) {
  return data101[id]->TUFlag++;
}

int GetTU101_is_valid_id( unsigned int id )
{
  return ( id < cntTU101 );
}

unsigned short GetTU101_begin_addr( unsigned int id )
{
  if( GetTU101_is_valid_id( id ) ){
    return dataTU101[ id ]->begin;
  }
  return -1;
}

unsigned short GetTU101_end_addr( unsigned int id )
{
  if( GetTU101_is_valid_id( id ) ){
    return dataTU101[ id ]->end;
  }
  return -1;
}

int GetID_TU101_by_addr( unsigned short addr )
{
  unsigned char i;
  for(i=0; i<cntTU101; i++){
    //Print( "i = %d, begin = %d, end = %d\n",
    //  i, GetTU101_begin_addr( i ), GetTU101_end_addr( i )); exit(0);
    if(( GetTU101_begin_addr( i ) <= addr )&&( GetTU101_end_addr( i ) >= addr )){
      return i;
    }
  }
  return -1;
}

unsigned char Get_MODBUS_TU101( unsigned char id )
{
  if( GetTU101_is_valid_id( id ) ){
    return dataTU101[ id ]->addrmodbus;
  }
  return -1;
}

unsigned char Get_COM_TU101( id )
{
  if( GetTU101_is_valid_id( id ) ){
    return dataTU101[ id ]->com_port;
  }
  return -1;
}

unsigned char GetTUProto(int n)
{
  return dataTU101[ n ]->proto;
}

int installTU101(unsigned char addrmodbus,
		 unsigned char com_port,
		 unsigned short begin,
		 unsigned short end,
		 unsigned char proto,
		 unsigned char commandlen,
		 unsigned char parentTS,
		 unsigned char tic)
{
  dataTU101[cntTU101] = malloc(sizeof(TU101));
  if(!dataTU101[cntTU101]){
    Print("error memory allocation\n\r");
    exit(0);
  }
  dataTU101[cntTU101]->addrmodbus = addrmodbus;
  dataTU101[cntTU101]->com_port = com_port;
  dataTU101[cntTU101]->begin = begin;
  dataTU101[cntTU101]->end = end;
  dataTU101[cntTU101]->proto = proto;
  dataTU101[cntTU101]->commandlen = commandlen;
  dataTU101[cntTU101]->parentTS = parentTS;
  dataTU101[cntTU101]->tic = tic;
  dataTU101[cntTU101]->var_tic = 1;
  cntTU101++;
  CreateCommandOffDCON( cntTU101 - 1 );
  return (cntTU101 - 1);
}

int  install101(unsigned char addrmodbus,
		unsigned char func_ts_tu_tit,
		unsigned char com_port,
		unsigned char number_reg,
		unsigned char *maska_param,
		int           cycl_oprosa,
		int           cycl_timer2,
		unsigned char validate,
		unsigned char num_request,
		unsigned char timer1,
		unsigned char timer2,
		unsigned char proto,
		unsigned char *command,
		unsigned char commandlen)
{
  int i;
  data101[cnt101] = malloc(sizeof(Data101));
  if(!data101[cnt101]){
    Print("error memory allocation\n\r");
    exit(0);
  }
  data101[cnt101]->addrmodbus = addrmodbus;
  data101[cnt101]->func_ts_tu_tit = func_ts_tu_tit;
  data101[cnt101]->com_port = com_port;
  data101[cnt101]->number_reg = number_reg;
  memcpy(data101[cnt101]->maska_param, maska_param, 32);
  data101[cnt101]->cycl_oprosa = cycl_oprosa;
  data101[cnt101]->cycl_timer2 = cycl_timer2;
  Print("data[%d]->%d = %d\n\r", cnt101, data101[cnt101]->cycl_oprosa, cycl_oprosa);
  data101[cnt101]->validate = validate;
  data101[cnt101]->num_request = num_request;
  data101[cnt101]->timer1 = timer1;
  data101[cnt101]->timer2 = timer2;
  data101[cnt101]->beginTSADDR = getFirstADDR(cnt101);
  data101[cnt101]->cntValid = 3;
  data101[cnt101]->first = -1;
  data101[cnt101]->TUFlag = 0;
  data101[cnt101]->Proto = proto;
  data101[cnt101]->command = command;
  data101[cnt101]->commandlen = commandlen;
  SetMyFlag( timer1, cycl_oprosa, 1);//on
  SetMyFlag(timer2, 0, 0);//off
  InstallSporadicData(cnt101);
  cnt101++;
  return (cnt101 - 1);
}

int getFirstADDR(int n)
{
  int i = n - 1;
  int j, summ=0;
  while(i >= 0){
    if(data101[i]->func_ts_tu_tit == data101[n]->func_ts_tu_tit){
      for(j = 0; j < data101[i]->number_reg*2*8; j++){
	if( data101[i]->maska_param[j/8]&createMaska(j%8) != 0 ){
	  summ++;
	}
      }
      return summ + data101[i]->beginTSADDR;
    }
    i--;
  }
  return STARTNUM_TS;
}

unsigned char getValid(int n)
{
  return data101[n]->validate;
}

unsigned char* getMaska(int n)
{
  return data101[n]->maska_param;
}

unsigned char* getData(int n)
{
  return data101[n]->data;
}

unsigned char getFunc101(int n)
{
  return data101[n]->func_ts_tu_tit;
}

unsigned char getRegnum101(int n)
{
  return data101[n]->number_reg;
}

unsigned char getDataByte101(int n, int indx)
{
  return data101[n]->data[indx];
}

unsigned short getBeginAddr(int n)
{
  return data101[n]->beginTSADDR;
}

unsigned char Busy101(int n)
{
  //Print("%d\n\r", (who_is_doing[data101[n]->com_port] != 255) );
  return ( BusyDeviceByCom(data101[n]->com_port) != 255 );
}

void SetBusy101(int n)
{
  SetBusyDevice(data101[n]->com_port, n);
}

void ClearBusy101(int n)
{
  ClearBusyDeviceByCom(data101[n]->com_port);
}

void calcCRCDCON(unsigned char *d, int len)
{
  unsigned char ascii[16] = { 0x30, 0x31, 0x32, 0x33, 0x34,
			      0x35, 0x36, 0x37, 0x38, 0x39,
			      0x41, 0x42, 0x43, 0x44, 0x45,
			      0x46
			    };
  unsigned char i, summ = 0, hi=0, lo=0;
  d[3] = d[4] = 0;
  for( i=0; i<len-1; i++ )
    summ += d[i];
  hi = (summ&0xF0) >> 4;
  lo = summ&0x0F;
  d[len - 3] = ascii[ hi ];
  d[len - 2] = ascii[ lo ];
}

int validCRC(unsigned char *d, int len)
{
  unsigned short crc = CRC16asm(d, len - 2);
  return(
	( d[len - 2] == (crc&0x00FF) ) &&
	( d[len - 1] == (crc&0xFF00)>>8 )
	);
}

int validCRCDCON(unsigned char *d, int len)
{
  unsigned char ascii[16] = { 0x30, 0x31, 0x32, 0x33, 0x34,
			      0x35, 0x36, 0x37, 0x38, 0x39,
			      0x41, 0x42, 0x43, 0x44, 0x45,
			      0x46
			    };
  unsigned char i=0, summ=0, hi=0, lo=0;
  for( i=0; i<5; i++ ) {
    summ += d[i];
  }
  hi = ascii[ (summ&0xF0) >> 4 ];
  lo = ascii[ summ&0x0F ];
  return(
	  (d[ len - 3 ] == hi)&&(d[ len - 2 ] == lo)
	);
}

int SetCommandForMBUS( int i, unsigned char *command, int len )
{
  command[0] = data101[i]->addrmodbus;
  command[5] = data101[i]->number_reg;

  Print("M%d -> %d\n\r", i, command[0]);

  calcCRC(command, len);
  return 1;
}

int SetCommandForDCON( int i, unsigned char *command, int len )
{
  command[2] = 0x30 + data101[i]->addrmodbus;

  Print("C%d -> %d\n\r", i, command[0]);

  calcCRCDCON(command, len);
  return 1;
}

void To101Data(int i)
{
  unsigned char j=0, len=0;
  unsigned char *command;
  if( GetTUFlag( i ) == 1 ){
    command = GetTUBuffer( i );
    len = GetTUCommandLen( i );
    Print( "len=%d\n\r", len );
    for( j=0; j<8; j++ ) Print( "[%d]:%x\n\r", j, command[j] );
  }
  else{
    command = GetCommand(i);
    len = GetCommandLen( i );
    if( GetProto( i ) == PROTO_MBUS ) {
      SetCommandForMBUS( i, command, len );
    } else {
      SetCommandForDCON( i, command, len );
    }
  }
  if(data101[i]->com_port == 1){
    ToCom1Data( command, len );
  } else if(data101[i]->com_port == 2){
    ToCom2Data( command, len );
  }
  //////////////////////////////////
  //this one is doing that now -> n
  SetBusyDevice(data101[i]->com_port, i);
  //////////////////////////////////
  SetMyFlag(data101[i]->timer1, data101[i]->cycl_oprosa, 1);
}

int ToCom1Data(unsigned char *d, int len)
{
  SetLenghtOutputBufferCom1( len );
  while(len){
    _ToCom1(*d++);
    len--;
  }
  return TRUE;
}

int ToCom2Data(unsigned char *d, int len)
{
  SetLenghtOutputBufferCom2( len );
  while(len){
    _ToCom2(*d++);
    len--;
  }
  return TRUE;
}

int DataTransmitted101(int n)
{
  if(data101[n]->com_port == 1){
    return ( _isDataTransmittedCom1()&&( BusyDeviceByCom(data101[n]->com_port) == n ) );
  } else if(data101[n]->com_port == 2){
    return ( _isDataTransmittedCom2()&&( BusyDeviceByCom(data101[n]->com_port) == n ) );
  }
}

int DataReceived101(int n)
{
  unsigned int i;
  if(data101[n]->com_port == 1){
    return ( _isDataReceivedCom1()&&( BusyDeviceByCom(1) == n ) );
  } else if(data101[n]->com_port == 2){
    return ( _isDataReceivedCom2()&&( BusyDeviceByCom(2) == n ) );
  }
}

unsigned char GetTimer1_101(int n)
{
  return data101[n]->timer1;
}

unsigned char GetTimer2_101(int n)
{
  return data101[n]->timer2;
}

void ClearCom101(int n)
{
  if(data101[n]->com_port == 1){
    InstallTimerCom1();
    _ClearCom1();
  } else if(data101[n]->com_port == 2){
    InstallTimerCom2();
    _ClearCom2();
  }
}

short int GetOutputBufCom101(int n)
{
  if(data101[n]->com_port == 1){
    return _getOutputBufCom1(8);
  } else if(data101[n]->com_port == 2){
    return _getOutputBufCom2(8);
  }
}

short int IsCom101(int n)
{
  if(data101[n]->com_port == 1){
    return _IsCom1();
  } else if(data101[n]->com_port == 2){
    return _IsCom2();
  }
}

void installCom(int n)
{
  if(data101[n]->com_port == 1)
    _InstallCom1(9600L,8,0,1);
  else
    _InstallCom2(9600L,8,0,1);
}

int ReadBuffer101IntoArray( int id, unsigned char *array ) {
  if(data101[id]->com_port == 1){
    return ReadDataCom1(array);
  } else if(data101[id]->com_port == 2){
    return ReadDataCom2(array);
  }
  return 0;
}

int ReadBuffer101(int n)
{
  unsigned char preData[256];
  unsigned char previousData[256];
  unsigned char resultData[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  unsigned char len = 0;
  unsigned char i;
  if(data101[n]->com_port == 1){
    len = ReadDataCom1(preData);
  } else if(data101[n]->com_port == 2){
    len = ReadDataCom2(preData);
  }

  Print("M%d <- %d\n\r", n, preData[3]);

  if(validCRC(preData, len)){
    data101[n]->validate = VALID;
    data101[n]->cntValid = 3;
    //Print("{");
    for(i = 3; i < len - 2; i++){
      previousData[i - 3] = data101[n]->data[i - 3];
      data101[n]->data[i - 3] = preData[i];
      //Print("%d-%x ", i, preData[i]);
    }
    //Print("}\n\r");
    if(data101[n]->first != -1){//for first request
      if( DataAnalisys( previousData, n, resultData ) )
	  OIKSendSporadic(n, resultData);
    }
    data101[n]->first = 0;
    return len - 2 - 3;
  } else {
    if(data101[n]->cntValid == 0){
      data101[n]->validate = INVALID;
      Print( "[%d] MBUS INVALID DATA\n\r", n );
    } else {
      data101[n]->cntValid--;
    }
    return 0;
  }
}

int ReadBufferDCON(int n)
{
  unsigned char preData[256];
  unsigned char previousData[256];
  unsigned char resultData[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  unsigned char len = 0;
  unsigned char i=0, j=0, k=0, m=0;
  unsigned short d[4];
  if(data101[n]->com_port == 1){
    len = ReadDataCom1(preData);
  } else if(data101[n]->com_port == 2){
    len = ReadDataCom2(preData);
  }

  Print("C%d <- %d\n\r", n, len);

  if( validCRCDCON(preData, len) ) {
    data101[n]->validate = VALID;
    data101[n]->cntValid = 3;
    previousData[0] = data101[n]->data[0];
    previousData[1] = data101[n]->data[1];
    for( i=1; i<len - 3; i++ ){
      d[i-1] = DCONTable[preData[i]];
      //m = 0x10;
      //for( k=0; k < 4; k++ ) {
      //	data101[n]->data[j+k] = (((preData[i] - 0x30)&(m>>=1))!=0);
      //}
      //j+=4;
      //s = (char)preData[i];
      //sprintf(&s, "%c", preData[i]);
      Print(" %x-%x ", preData[i], d[i-1]);
    }
    Print("\n\r");
    data101[n]->data[0] = ((d[0]&0x0F)<<4)|(d[1]&0x0F);
    data101[n]->data[1] = ((d[2]&0x0F)<<4)|(d[3]&0x0F);
    //for( i=0; i<j; i++ ) {
    //  w|=(data101[n]->data[i]<<(15-i));
      //Print(" %d ", data101[n]->data[i]);
    //}
    //Print("\n\r");
    //data101[n]->data[0] = ( (w&0xff00)>>8 );
    //data101[n]->data[1] = ( w&0x00ff );
    if(data101[n]->first != -1){//for first request
      if( DataAnalisys( previousData, n, resultData ) ) {
	Print("D---------\n\r"); OIKSendSporadic(n, resultData);
      }
      for( i=0; i<2; i++)
	Print("NOW = %d, PRE = %d ", data101[n]->data[i], previousData[i]);
      Print("\n\r");
    }
    data101[n]->first = 0;
    return j;
  } else {
    if(data101[n]->cntValid == 0){ Print( "DCON INVALID DATA\n\r" );
      for( i=0; i<8; i++ )
	Print( "D[%d]=%x\n\r", i, preData[i] );
      data101[n]->validate = INVALID;
    } else {
      data101[n]->cntValid--;
    }
    return 0;
  }
}

void setWatchDog(int n, int t)
{
  if(data101[n]->com_port == 1){
    SetMyFlag( WATCHDOGCOM1, t, 1);
  } else if(data101[n]->com_port == 2){
    SetMyFlag( WATCHDOGCOM2, t, 1);
  }
}

int getWatchDog(int n)
{
  if(data101[n]->com_port == 1){
    return GetMyFlag( WATCHDOGCOM1 );
  } else if(data101[n]->com_port == 2){
    return GetMyFlag( WATCHDOGCOM2 );
  }
}

/*
return result - bit array with 1 if before != after and 0 if before == after
*/
int DataAnalisys( unsigned char *before, unsigned char handle, unsigned char *result )
{
  unsigned short check = 0, lo, hi;
  int i;
  data101[handle]->first = 0;
  if(data101[handle]->func_ts_tu_tit != TS)
    return 0;
  for(i = 0; i < data101[handle]->number_reg*2*8; i++){
    result[i/8]|=(
	(data101[handle]->data[i/8]&createMaska(i%8))^
	(before[i/8]&createMaska(i%8)) );
    if((result[i/8]&createMaska(i%8)) != 0){
      check = 1;
    }
  }
  /*for(i = 0; i < data101[handle]->number_reg*2; i+=2){
    lo = result[i];
    hi = result[i + 1];
    result[i] = hi;
    result[i + 1] = lo;
  }*/
  Print("S=%d\n\r", check);
  /*Print("{\n\r");
  for( i=0; i<32; i++) {
    Print("%x b-%x a-%x ", result[i], before[i], data101[handle]->data[i] );
  }
  Print("}\n\r");*/
  return check;
}

int GetCyclTimer2_101DCON( int n )
{
  return data101[n]->cycl_timer2;
}

void DigitToSymbols(unsigned char digit, unsigned char *hi, unsigned char *lo) {
  unsigned char ascii[16] = { 0x30, 0x31, 0x32, 0x33, 0x34,
			      0x35, 0x36, 0x37, 0x38, 0x39,
			      0x41, 0x42, 0x43, 0x44, 0x45,
			      0x46
			    };
  *hi = (digit&0xF0) >> 4;
  *lo = digit&0x0F;
  *hi = ascii[ *hi ];
  *lo = ascii[ *lo ];
}

unsigned char ReadBuffer101DCON( unsigned char id ) {
  unsigned char before = IsData101VALID( id );
  if( GetProto( id ) == PROTO_MBUS ) {
    ReadBuffer101( id );
  } else {
    ReadBufferDCON( id );
  }
  if( IsData101VALID( id ) != before ) {
    SetOIKTrigger();
  }
  return 1;
}

unsigned char HandleTU( unsigned char id ) {
  if( TU_Valid( id ) ) {
    SendRequestOnTU( id, GetCommand(id) );
    SetTic( id );
  }
  else
    Print( "Fail TU\n\r" );
  ClearTUFlag( id );
  return 1;
}

unsigned char SetOIKTrigger() {
  OIKTRIGGER = 1;
  return OIKTRIGGER;
}

unsigned char UnsetOIKTrigger() {
  OIKTRIGGER = 0;
  return OIKTRIGGER;
}

unsigned char GetOIKTrigger() {
  return OIKTRIGGER;
}

unsigned char InitDCONTable() {
  int i;
  for( i=0x30; i<=0x39; i++ ) {
    DCONTable[i] = i - 0x30;
  }
  for( i=0x41; i<=0x46; i++) {
    DCONTable[i] = 0x0A + i - 0x041;
  }
  return 0;
}

unsigned char GetComPort101(int id) {
  return data101[id]->com_port;
}