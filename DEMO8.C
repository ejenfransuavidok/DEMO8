#include<stdio.h>
#include<string.h>
#include"c:\7188OS7\DEMO\lib\I7188.h"
#include"uart7.h"
#include"crc16.h"
#include"DEVICE.H"
#include"OIK.h"
#include"config.h"

#define TIC 10

int main()
{
  int i, LedMode=0, ID=-1;
  unsigned char array[256];
  unsigned char cnt;
  unsigned char OIKdata[256];
  unsigned char OIKsize=0;
  unsigned char OIKbegin=0;
  char c;
  unsigned char buffer[256];
  unsigned char commandMBUS[8] = {0x01, 0x04, 0x00, 0x00, 0x00, 23, 0x00, 0x00};
  unsigned char commandDCON[6] = {0x40, 0x30, 0x30, 0x00, 0x00, 0x0D};
  //unsigned char descriptors[256];
  unsigned char descriptorsTU[256];
  unsigned char maska[32] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  //unsigned int watchdogs[4] = { 500 * 2, 3000 * 2, 3000 * 2, 3000 * 2 };
  Print( "STARTTT!!!\n\r" );
  EnableWDT();
  _InstallCom1(9600L,8,0,1);
  _InstallCom2(9600L,8,0,1);
  install_9();
  InstallIsr();
  InitSerial();
  /*uchar addrmodbus,
    uchar func_ts_tu_tit,
    uchar com_port,
    uchar number_reg,
    uchar *maska_param,
    int cicl_oprosa,
    int cycl_timer2,
    uchar validate,
    uchar num_request,
    uchar timer1,
    uchar timer2,
    uchar proto,
    uchar *command
  */
  //descriptors[getCnt101()] = install101(0x01, TS, 2, 5, maska, 1000, 5, INVALID, 3, 0, 1, PROTO_MBUS, commandMBUS, 8);
  //descriptors[getCnt101()] = install101(0x01, TIT, 1, 23, maska, 3000, 5, INVALID, 3, 2, 3, PROTO_MBUS, commandMBUS, 8);
  //descriptors[getCnt101()] = install101(0x02, TIT, 1, 23, maska, 3333, 5, INVALID, 3, 4, 5, PROTO_MBUS, commandMBUS, 8);
  //descriptors[getCnt101()] = install101(0x01, TS, 2, 1, maska, 1000, 1, INVALID, 3, 6, 7, PROTO_DCON, commandDCON, 6);
  /*
  typedef struct _TU101_{
	unsigned char addrmodbus;
	unsigned char com_port;
	unsigned char begin;
	unsigned char end;
	unsigned char proto;
	unsigned char commandlen;
	unsigned char parentTS;
	} TU101
  */
  //descriptorsTU[getCntTU101()] = installTU101(0x01, 2, 0x0000, 0x0009, PROTO_MBUS, TUCOMMAND_MBUS, 0, TIC);
  //descriptorsTU[getCntTU101()] = installTU101(0x02, 2, 0x000A, 0x000C, PROTO_DCON, TUCOMMAND_DCON, 3, TIC);
  ReadConfigFile();
  for(i = 0; i < GetAllDevices(); i++){
    ID = GetDeviceId(i);
    ClearComDeviceById(ID);
    ClearBusyDeviceById(ID); Print("ID=%d\n\r", ID);
  } //exit(0);
  /*for(i = 0; i < getCnt101(); i++){
    setWatchDog(i, watchdogs[i], 1);
  }*/
  InstallTimerCom3();
  ClearCom3();
  for(;;){
    RefreshWDT();
    //OIK///////////////////////////////////////////////////////////////
    if( ( IsCom3() != 0 )&&( OIKbegin == 0  ) ){
      OIKbegin = 1;
      SetTimerCom3(5);
    }
    if( _isDataReceivedCom3() == 1 ){
      OIKsize = ReadCom3Str(OIKdata);
      if(OIKvalid(OIKdata, OIKsize)){
	LedMode =! LedMode;
	if( LedMode )
	  LedOn();
	else
	  LedOff();
	OIKparse(OIKdata, OIKsize);
	LedMode =! LedMode;
	if( LedMode )
	  LedOn();
	else
	  LedOff();
      }
      OIKbegin = 0;
      InstallTimerCom3();
      ClearCom3();
    }
    ////////////////////////////////////////////////////////////////////
    for(i = 0; i < GetAllDevices(); i++) {
      ID = GetDeviceId(i);
      if( ( GetMyFlag( GetTimer1_Device(ID) ) == 0 )&&( BusyDeviceById(ID) == 255 ) ) {
	PrepareComToTransmitById(ID);
	DecrementTic(ID);
	ToDeviceDataById(ID);
	//setWatchDog(i, watchdogs[ID]);
      }
      if( DataTransmittedDevice(ID) == 1 ) {
	ClearComDeviceById(ID);
	SetMyFlag( GetTimer2_Device(ID), GetCyclTimer2_Device(ID), 1 );//on
	//setWatchDog(i, watchdogs[ID]);
	if( GetTUFlagDevice(ID) )
	  IncTUFlagDevice(ID);
      }
      if( GetMyFlag( GetTimer2_Device(ID) ) == 0 ) {
	SetMyFlag( GetTimer2_Device(ID), 0, 0);//off
	PrepareComToReceiveById(ID);
      }
      if( DataReceivedDevice(ID) == 1 ) {//   Print("N=%d\n\r", ID); exit(0);
	if( GetTUFlagDevice(ID) == 2 ) {
	  HandleTUDevice(ID);
	} else {
	  ReadBufferDevice(ID);
	}
	ClearComDeviceById(ID);
	ClearBusyDeviceById(ID);
      }
      /*if( getWatchDog(ID) == 0 ){
	Print("WATCHDOG#%d\n\r", ID);
	setWatchDog(ID, watchdogs[ID]);
	PrepareComToTransmit(ID);
	To101Data(ID, GetCommand(ID), GetCommandLen(ID));
	SetBusy101(ID);
      }*/
    }
    while (Kbhit()) {
      c=Getch();
      if ((c=='q')|| (c=='Q')) goto End;
      Print("(%c)",c);
    }
  }
End:
  _RestoreCom1();
  _RestoreCom2();
  UnInstallIsr();
  restore_9();
  return 0;
}