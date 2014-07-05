#include"CONFIG.H"

/*int  install101(unsigned char addrmodbus,
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
		unsigned char commandlen
		);
int installTU101(unsigned char addrmodbus,
		 unsigned char com_port,
		 unsigned short begin,
		 unsigned short end,
		 unsigned char proto,
		 unsigned char commandlen,
		 unsigned char parentTS,
		 unsigned char tic
		 );
typedef struct _GLOBALSTRUCT_ {
	unsigned char mbus;
	unsigned char type;
	unsigned char com;
	unsigned char regs;
	unsigned char maska[32];
	unsigned short cycle;
	unsigned short begin;
	unsigned short end;
	unsigned char tic;
	unsigned char proto;
} GLOBALSTRUCT;
install101(0x01, TS, 2, 5, maska, 1000, 5, INVALID, 3, 0, 1, PROTO_MBUS, commandMBUS, 8);
installTU101(0x01, 2, 0x0000, 0x0009, PROTO_MBUS, TUCOMMAND_MBUS, 0, TIC);
*/
static FILE_DATA far *fdata;
static unsigned char *fname = "CONFIG.DAT";
unsigned short TIMERS_COUNTER = 0;
static unsigned char counter = 0;
static char parentformbus = -1;
static char parentfordcon = -1;
unsigned char commandMBUS[8] = {0x01, 0x04, 0x00, 0x00, 0x00, 23, 0x00, 0x00};
unsigned char commandDCON[6] = {0x40, 0x30, 0x30, 0x00, 0x00, 0x0D};

int PrintGS( const GLOBALSTRUCT *gs ) {
  int i = 0;
  Print( "mbus  = %x\n\r", gs->mbus );
  Print( "type  = %x\n\r", gs->type );
  Print( "com   = %x\n\r", gs->com );
  Print( "regs  = %d\n\r", gs->regs );
  Print( "cycle = %d\n\r", gs->cycle );
  Print( "begin = %x\n\r", gs->begin );
  Print( "end   = %x\n\r", gs->end );
  Print( "tic   = %d\n\r", gs->tic );
  Print( "proto = %d\n\r", gs->proto );
  for( i=0; i < 32; i++ ) {
    Print( "maska[%d] = %x\n\r", i, gs->maska[i] );
  }
}

int SetTSTITRecord( const GLOBALSTRUCT *gs ) {
  unsigned char i;
  unsigned short cycle2;
  unsigned char commandlen;
  unsigned char *command;
  unsigned short timer1 = TIMERS_COUNTER++, timer2 = TIMERS_COUNTER++;
  cycle2 = (gs->proto == PROTO_MBUS)?5:0;
  commandlen = (gs->proto == PROTO_MBUS)?8:6;
  command = (gs->proto == PROTO_MBUS)?commandMBUS:commandDCON;
  AddDevice(
    DEV101,
    install101(
	gs->mbus, gs->type, gs->com, gs->regs, gs->maska, gs->cycle, cycle2,
	INVALID, 3, timer1, timer2, gs->proto, command, commandlen
	)
  );
  Print( "mbus   = %d\n\r", gs->mbus );
  Print( "type   = %d\n\r", gs->type );
  Print( "com    = %d\n\r", gs->com );
  Print( "regs   = %d\n\r", gs->regs );
  Print( "cycle  = %d\n\r", gs->cycle );
  Print( "cycle2 = %d\n\r", cycle2 );
  Print( "timer1 = %d\n\r", timer1 );
  Print( "timer2 = %d\n\r", timer2 );
  Print( "proto  = %d\n\r", gs->proto );
  Print( "commandlen = %d\n\r", commandlen );
  for( i=0; i<32; i++ ) {
    Print( "maska[%d]  = %d\n\r", i, gs->maska[i] );
  }
  for( i=0; i<commandlen; i++ ) {
    Print( "comm[%d]  = %x\n\r", i, command[i] );
  }
  if(gs->proto == PROTO_MBUS) {
    parentformbus = counter++;
  } else {
    parentfordcon = counter++;
  }
  return 0;
}

int SetMerquryRecord( const GLOBALSTRUCT *gs ) {
  int i;
  unsigned short timer1 = TIMERS_COUNTER++, timer2 = TIMERS_COUNTER++;
  AddDevice(
    DEVMERQURY,
    InstallMerqury(
	gs->mbus,
	gs->password,
	gs->com,
	timer1,
	timer2,
	8,//password size
	gs->cycle,
	5,
	gs->maska)
  );
  Print( "START__MERQURY__\n\r" );
  Print( "mbus   = %d\n\r", gs->mbus );
  Print( "type   = %d\n\r", gs->type );
  Print( "com    = %d\n\r", gs->com );
  Print( "cycle  = %d\n\r", gs->cycle );
  Print( "timer1 = %d\n\r", timer1 );
  Print( "timer2 = %d\n\r", timer2 );
  //Print( "password = %s\n\r", gs->password );
  for( i=0; i<32; i++ ) {
    Print( "maska[%d]  = %d\n\r", i, gs->maska[i] );
  }
  Print( "END__MERQURY__\n\r" );
  return 0;
}

int SetTURecord( const GLOBALSTRUCT *gs ) {
  char parent = (gs->proto == PROTO_MBUS)?parentformbus:parentfordcon;
  if( parent == -1 ) {
    Print( "ERROR install TU\n\r" );
    exit(0);
    return 0;
  }
  installTU101(
	gs->mbus, gs->com, gs->begin, gs->end, gs->proto, 8, parent, gs->tic
	);
  return 0;
}

unsigned char ReadConfigFile() {
  byte password[8] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
  GLOBALSTRUCT far *fpointer;
  GLOBALSTRUCT GS;
  int i=0, N=0;
  fpointer=(GLOBALSTRUCT far *)GetFilePositionByName( fname );
  fdata=GetFileInfoByName( fname );
  N = fdata->size / sizeof(GLOBALSTRUCT);
  for( i=0; i<N; i++ ){
    GS = fpointer[i];
    Print( "GS=%d\n\r", GS );
    if( (GS.type == TS)||(GS.type == TIT) ) {
      SetTSTITRecord( &GS );
    } else if( GS.type == MERQURY ) {
      SetMerquryRecord( &GS );
    }
  }
  /*
  AddDevice(
    DEVMERQURY,
    InstallMerqury(
	40,
	password,
	1,
	TIMERS_COUNTER,
	TIMERS_COUNTER+1,
	8,//password size
	5000,
	5)
  );
  */
  InitDCONTable();
  //Print( "N = %d STRUCTS = %d END!!!\n\r", fdata->size / sizeof(GLOBALSTRUCT), i ); //exit(0);
  return 0;
}