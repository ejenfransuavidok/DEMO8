/*
  UART7.C  demo program dor write user own ISR for COM1/COM2
*/
#include<conio.h>
#include<stdio.h>
#include<values.h>
#include"uart7.h"
#include"c:\7188OS7\DEMO\lib\I7188.h"

#ifndef __TURBOC__
 /* for MSC */
 #define enable()  _enable()
 #define disable() _disable()
#else
 #include<dos.h>
 #define outpw   outport
 #define inpw    inport
#endif

#define INT_MASK	      0xff28	/* Interrupt mask register	*/
#define INT_EOI 	      0xff22	/* End-of-interrupt register	*/
#define EOITYPE_INT0	      0x0c      /* for COM1  */
#define EOITYPE_INT1          0x0d      /* for COM2  */
#define InBufSize  1024
#define OutBufSize  128

int iBase1=0x200;		/* com1 base address */
int iBase2=0x100;	     	/* com2 base address */
int _err1=0;
int _err2=0;
static unsigned long OldIntVect1=0L;

static unsigned char InData1[InBufSize];
static unsigned char OutData1[OutBufSize];
int In_InIdx1,In_OutIdx1,Out_InIdx1,Out_OutIdx1;

static unsigned long OldIntVect2=0L;

static unsigned char InData2[InBufSize];
static unsigned char OutData2[OutBufSize];
int In_InIdx2,In_OutIdx2,Out_InIdx2,Out_OutIdx2;

int save232[4];
static unsigned long far *IntVect=(unsigned long far *)0L;

static unsigned char flgIsDataReceivedCom1;
static unsigned char flgIsDataReceivedCom2;
static unsigned char flgIsDataReceivedCom3;

static unsigned char flgIsDataTransmittedCom1;
static unsigned char flgIsDataTransmittedCom2;
static unsigned char flgIsDataTransmittedCom3;

static unsigned long int GlobalTimer=0;
static unsigned int TimerCOM1=0;
static unsigned int TimerCOM2=0;
static unsigned int TimerCOM3=0;
static unsigned int TimerThreasholdCom1=0;
static unsigned int TimerThreasholdCom2=0;
static unsigned int TimerThreasholdCom3=0;
static int __Out_OutIdx1, __Out_OutIdx2;

typedef struct THETIMER_{
  int timer;
  unsigned char flag;
  unsigned char state;
}THETIMER;

static THETIMER TimersFlags[256];

void interrupt Isr9(void)
{
  const int TIMERS = 20;
  int _i_;
  GlobalTimer++;
  TimerCOM1++;
  TimerCOM2++;
  TimerCOM3++;
  for( _i_ = 0; _i_ < TIMERS; _i_++ ) {
    if( (TimersFlags[_i_].timer>0)&&(TimersFlags[_i_].flag==1) ) TimersFlags[_i_].timer--; else TimersFlags[_i_].flag = 0;
  }
  /*
  if( (TimersFlags[1].timer>0)&&(TimersFlags[1].flag==1) ) TimersFlags[1].timer--; else TimersFlags[1].flag = 0;
  if( (TimersFlags[2].timer>0)&&(TimersFlags[2].flag==1) ) TimersFlags[2].timer--; else TimersFlags[2].flag = 0;
  if( (TimersFlags[3].timer>0)&&(TimersFlags[3].flag==1) ) TimersFlags[3].timer--; else TimersFlags[3].flag = 0;
  if( (TimersFlags[4].timer>0)&&(TimersFlags[4].flag==1) ) TimersFlags[4].timer--; else TimersFlags[4].flag = 0;
  if( (TimersFlags[5].timer>0)&&(TimersFlags[5].flag==1) ) TimersFlags[5].timer--; else TimersFlags[5].flag = 0;
  if( (TimersFlags[6].timer>0)&&(TimersFlags[6].flag==1) ) TimersFlags[6].timer--; else TimersFlags[6].flag = 0;
  if( (TimersFlags[7].timer>0)&&(TimersFlags[7].flag==1) ) TimersFlags[7].timer--; else TimersFlags[7].flag = 0;
  */
  if( (TimersFlags[100].timer>0)&&(TimersFlags[100].flag==1) ) TimersFlags[100].timer--; else TimersFlags[100].flag = 0;
  if( (TimersFlags[101].timer>0)&&(TimersFlags[101].flag==1) ) TimersFlags[101].timer--; else TimersFlags[101].flag = 0;
  if(TimerCOM1 > TimerThreasholdCom1)
    flgIsDataReceivedCom1 = 1;
  if(TimerCOM2 > TimerThreasholdCom2)
    flgIsDataReceivedCom2 = 1;
  if(TimerCOM3 > TimerThreasholdCom3)
    flgIsDataReceivedCom3 = 1;
}

unsigned long Old9;

void install_9(void)
{
   Old9=IntVect[0x09];
   IntVect[0x09]=Isr9;
}

void restore_9(void)
{
   IntVect[0x09]=Old9;
}

void SetTimerCom1(unsigned int t)
{
  TimerThreasholdCom1 = t;
  TimerCOM1 = 0;
}

void SetTimerCom2(unsigned int t)
{
  TimerThreasholdCom2 = t;
  TimerCOM2 = 0;
}

void SetTimerCom3(unsigned int t)
{
  TimerThreasholdCom3 = t;
  TimerCOM3 = 0;
}

void InstallTimerCom1()
{
  TimerThreasholdCom1 = -1;
}

void InstallTimerCom2()
{
  TimerThreasholdCom2 = -1;
}

void InstallTimerCom3()
{
  TimerThreasholdCom3 = -1;
}

void ClearTimerCom3()
{
  TimerCOM3 = 0;
}

void ClearCom3Flags()
{
  flgIsDataReceivedCom3 = 0;
}

static void interrupt Serial1_Isr(void)
{
  do{
     switch(inp(iBase1+Iir)&0x07){
      case 0:
/*	   inp(iBase1+Msr); */
	   break;
      case 1: /* No interrupt to be service */
	   goto end;
      case 2: /* can send data  */
	   flgIsDataReceivedCom1 = 0;
	   if(Out_InIdx1!=Out_OutIdx1){
	      outp(iBase1,OutData1[Out_OutIdx1++]);
	      Out_OutIdx1&=OutBufSize-1;
	      if(Out_OutIdx1==Out_InIdx1) outp(iBase1+Ier, 0x01);
	      flgIsDataTransmittedCom1 = (Out_OutIdx1 == __Out_OutIdx1);
	   }
	   else {
	      outp(iBase1+Ier, 0x01);
	   }
	   break;
      case 4: /* receive data */
ReadAgain:
	   TimerCOM1 = 0;
	   InData1[In_InIdx1++]=inp(iBase1);   /* receive next char */
	   In_InIdx1&=(InBufSize-1);
	   if(In_InIdx1==In_OutIdx1) {
	      In_InIdx1--;
	      In_InIdx1&=(InBufSize-1);
	      _err1=1;
	   }
	   if(inp(iBase1+Lsr) & 1) goto ReadAgain;
	   break;
      case 6:
	   inp(iBase1+Lsr);
	   break;
      default:
	   /* error status */
	   goto end;
     }
   }while(1);
end:
  outp(INT_EOI,EOITYPE_INT0);	  /* send EOI to 7188's INT-0 */
/* if COM2  use outp(INT_EOI,EOITYPE_INT1);  */
}

static void interrupt Serial2_Isr(void)
{
  do{
     switch(inp(iBase2+Iir)&0x07){
      case 0:
/*	   inp(iBase1+Msr); */
	   break;
      case 1: /* No interrupt to be service */
	   goto end;
      case 2: /* can send data  */
	   flgIsDataReceivedCom2 = 0;
	   if(Out_InIdx2!=Out_OutIdx2){
	      outp(iBase2,OutData2[Out_OutIdx2++]);
	      Out_OutIdx2&=OutBufSize-1;
	      if(Out_OutIdx2==Out_InIdx2) {
		outp(iBase2+Ier, 0x01);
	      }
	      flgIsDataTransmittedCom2 = (Out_OutIdx2 == __Out_OutIdx2);
	   }
	   else {
	      outp(iBase2+Ier, 0x01);
	   }
	   break;
      case 4: /* receive data */
ReadAgain:
	   TimerCOM2 = 0;
	   InData2[In_InIdx2++]=inp(iBase2);   /* receive next char */
	   In_InIdx2&=(InBufSize-1);
	   if(In_InIdx2==In_OutIdx2) {
	      In_InIdx2--;
	      In_InIdx2&=(InBufSize-1);
	      _err2=1;
	   }
	   if(inp(iBase2+Lsr) & 1) goto ReadAgain;
	   break;
      case 6:
	   inp(iBase2+Lsr);
	   break;
      default:
	   /* error status */
	   goto end;
     }
   }while(1);
end:
  outp(INT_EOI,EOITYPE_INT1);	  /* send EOI to 7188's INT-0 */
/* if COM2  use outp(INT_EOI,EOITYPE_INT1);  */
}

static unsigned char ratelo, ratehi, format;

int GetBaudData(unsigned long baud, int data, int parity, int stop)
{ int cc;

  format=0;
  switch(data) {
    case 5:  break; /* format+=0; */
    case 6: format+=1; break;
    case 7: format+=2; break;
    case 8: format+=3; break;
    default: return(DataError);
  }

  switch(parity) {
    case 0: break;  /* format+=0; No Parity*/
    case 1: format+=0x18; break; /* Even parity */
    case 2: format+=0x08; break; /* Odd parity */
    case 3: format+=0x28; break; /* Mark parity */
    case 4: format+=0x38; break; /* Space parity */
    default: return(ParityError);
  }

  switch(stop) {
      case 1: break; /* format+=0; */
      case 2: format+=4;    break;
      default: return(StopError);
  }

  switch(baud){
    case 1200L : ratehi=0; ratelo=96; break;
    case 2400L : ratehi=0; ratelo=48; break;
    case 4800L : ratehi=0; ratelo=24; break;
    case 9600L : ratehi=0; ratelo=12; break;
    case 19200L: ratehi=0; ratelo=6; break;
    case 38400L: ratehi=0; ratelo=3; break;
    case 57600L: ratehi=0; ratelo=2; break;
    case 115200L: ratehi=0; ratelo=1; break;
    default    : return BaudRateError;		/* baud rate error */
  }
  return NoError;
}

int _InstallCom1(unsigned long baud, int data, int parity, int stop)
{ int cc;

  if(OldIntVect1) _RestoreCom1();
  _err1=0;
  cc=GetBaudData(baud,data,parity,stop);
  if(cc) return cc;
  /* _asm cli */          disable();
  save232[2]=inp(iBase1+Lcr); /* format                 */
  outp(iBase1+Lcr,0x80);	 /* 1 set DLAB (baud rate)   */
  save232[0]=inp(iBase1+Dll); /* baud rate              */
  save232[1]=inp(iBase1+Dlh);
  outp(iBase1+Dll,ratelo);
  outp(iBase1+Dlh,ratehi);
  outp(iBase1+Lcr,format);	 /* 2. data format  */
  save232[3]=inp(iBase1+Ier); /* interrupt status*/

  outp(iBase1+Fcr, 0x81);	/* 3. enable & clear FIFO   */
  outp(iBase1+Ier, 0x01);	/* 4. enable COM1 interrupt */
  outp(iBase1+Mcr,0x0b);        /* set DTR line active      */

  In_InIdx1=In_OutIdx1=Out_InIdx1=Out_OutIdx1=0; /* 5. init QUEUE */
  OldIntVect1=IntVect[0x0c];	                 /* save old ISR  */
/* if use COM2 -->IntVect[0x0d] */
  IntVect[0x0c]=(unsigned long)Serial1_Isr;        /* install new ISR */
  outpw(INT_MASK,inpw(INT_MASK)&0xffef);/* 6. enable INT-0 of 7188 interrupt */
/* if use COM2 -->outpw(INT_MASK,inpw(INT_MASK)&0xffdf); */
  Set485DirToReceive(1);	/* 7. 485 initial in receive direction */
  /*_asm sti  */  enable();
  return(NoError);
}

int _InstallCom2(unsigned long baud, int data, int parity, int stop)
{
  int cc;

  if(OldIntVect2) _RestoreCom2();
  _err2=0;
  cc=GetBaudData(baud,data,parity,stop);
  if(cc) return cc;
  /* _asm cli */          disable();
  save232[2]=inp(iBase2+Lcr); /* format                 */
  outp(iBase2+Lcr,0x80);	 /* 1 set DLAB (baud rate)   */
  save232[0]=inp(iBase2+Dll); /* baud rate              */
  save232[1]=inp(iBase2+Dlh);
  outp(iBase2+Dll,ratelo);
  outp(iBase2+Dlh,ratehi);
  outp(iBase2+Lcr,format);	 /* 2. data format  */
  save232[3]=inp(iBase2+Ier); /* interrupt status*/

  outp(iBase2+Fcr, 0x81);	/* 3. enable & clear FIFO   */
  outp(iBase2+Ier, 0x01);	/* 4. enable COM1 interrupt */
  outp(iBase2+Mcr,0x0b);        /* set DTR line active      */

  In_InIdx2=In_OutIdx2=Out_InIdx2=Out_OutIdx2=0; /* 5. init QUEUE */
  OldIntVect2=IntVect[0x0d];	                 /* save old ISR  */
/* if use COM2 -->IntVect[0x0d] */
  IntVect[0x0d]=(unsigned long)Serial2_Isr;        /* install new ISR */
  outpw(INT_MASK,inpw(INT_MASK)&0xffdf);/* 6. enable INT-0 of 7188 interrupt */
/* if use COM2 -->outpw(INT_MASK,inpw(INT_MASK)&0xffdf); */
  Set485DirToReceive(2);	/* 7. 485 initial in receive direction */
  /*_asm sti  */  enable();
  return(NoError);
}

int _RestoreCom1(void)
{
  if(!OldIntVect1) return NoError;
  /* _asm cli */ disable();
  IntVect[0x0c]=OldIntVect1;	  /* 1. restore OLD ISR */
/* if use COM2 -->IntVect[0x0d] */
  OldIntVect1=0L;
  outp(iBase1+Lcr,0x80);	  /* 2. restore baud rate */
  outp(iBase1+Dll,save232[0]);
  outp(iBase1+Dlh,save232[1]);
  outp(iBase1+Lcr,(save232[2])&0x7f); /* 3. restore data format */

  outp(iBase1+Ier, save232[3]);/* 4. restore enable COM1's interrupt */
  outpw(INT_MASK,inpw(INT_MASK)|0x0010);/* 6. disable INT-0 of 7188 interrupt */
/* if use COM2 -->outpw(INT_MASK,inpw(INT_MASK)|0x0020); */
/*  outp(0x21,inp(0x21) | 0x10); */   /* mask IRQ 4 for COM1 */
  /* _asm sti */ enable();
  return NoError;
}

int _RestoreCom2(void)
{
  if(!OldIntVect2) return NoError;
  /* _asm cli */ disable();
  IntVect[0x0d]=OldIntVect2;	  /* 1. restore OLD ISR */
/* if use COM2 -->IntVect[0x0d] */
  OldIntVect2=0L;
  outp(iBase2+Lcr,0x80);	  /* 2. restore baud rate */
  outp(iBase2+Dll,save232[0]);
  outp(iBase2+Dlh,save232[1]);
  outp(iBase2+Lcr,(save232[2])&0x7f); /* 3. restore data format */

  outp(iBase2+Ier, save232[3]);/* 4. restore enable COM1's interrupt */
  outpw(INT_MASK,inpw(INT_MASK)|0x0020);/* 6. disable INT-0 of 7188 interrupt */
/* if use COM2 -->outpw(INT_MASK,inpw(INT_MASK)|0x0020); */
/*  outp(0x21,inp(0x21) | 0x10); */   /* mask IRQ 4 for COM1 */
  /* _asm sti */ enable();
  return NoError;
}

int _IsCom1(void)
{
    return In_InIdx1-In_OutIdx1;
}

int _IsCom2(void)
{
    return In_InIdx2-In_OutIdx2;
}

long ToComTimeOut=100000L;
int _ToCom1(char data)
{ long t=ToComTimeOut;
  int nextidx,done;

  do{
     /* _asm cli */ disable();
     done=(nextidx=(Out_InIdx1+1)&(OutBufSize-1)) != Out_OutIdx1;
     if(done){
	OutData1[Out_InIdx1]=data;
	Out_InIdx1=nextidx;
     }
     /* _asm sti */ enable();
     t--;
     if(!t) break;
  } while(!done);

  if(done){
     outp(iBase1+Lcr,inp(iBase1+Lcr)&0x7f);
     outp(iBase1+Ier, 0x03);	/*  enable COM1 interrupt */
     return NoError;
  }
  else return TimeOut;
}

int _ToCom2(char data)
{ long t=ToComTimeOut;
  int nextidx,done;

  do{
     /* _asm cli */ disable();
     done=(nextidx=(Out_InIdx2+1)&(OutBufSize-1)) != Out_OutIdx2;
     if(done){
	OutData2[Out_InIdx2]=data;
	Out_InIdx2=nextidx;
     }
     /* _asm sti */ enable();
     t--;
     if(!t) break;
  } while(!done);

  if(done){
     outp(iBase2+Lcr,inp(iBase2+Lcr)&0x7f);
     outp(iBase2+Ier, 0x03);	/*  enable COM2 interrupt */
     return NoError;
  }
  else return TimeOut;
}

int _ReadCom1(void)
{ unsigned data;

  while(!_IsCom1()){ ; }
  data=InData1[In_OutIdx1++];
  In_OutIdx1&=(InBufSize-1);
  return data;
}

int _ReadCom2(void)
{ unsigned data;

  while(!_IsCom2()){ ; }
  data=InData2[In_OutIdx2++];
  In_OutIdx2&=(InBufSize-1);
  return data;
}

int _ClearCom1(void)
{
  Out_InIdx1=Out_OutIdx1=0;
  flgIsDataTransmittedCom1 = 0;
  flgIsDataReceivedCom1 = 0;
  In_OutIdx1=In_InIdx1=0; return NoError;
}

int _ClearCom2(void)
{
  Out_InIdx2=Out_OutIdx2=0;
  flgIsDataTransmittedCom2 = 0;
  flgIsDataReceivedCom2 = 0;
  In_OutIdx2=In_InIdx2=0; return NoError;
}

void _ToCom1Str(char *str)
{
  while(*str) _ToCom1(*str++);
}

void _ToCom2Str(char *str)
{
  while(*str) _ToCom2(*str++);
}

int _getOutputBufCom1(int haveBeen)
{
  return haveBeen - ( Out_InIdx1 - Out_OutIdx1 );
}

int _getOutputBufCom2(int haveBeen)
{
  return haveBeen - ( Out_InIdx2 - Out_OutIdx2 );
}

int _isDataReceivedCom1()
{
  return flgIsDataReceivedCom1;
}

int _isDataReceivedCom2()
{
  return flgIsDataReceivedCom2;
}

int _isDataReceivedCom3()
{
  //Print("%d\n\r", TimerThreasholdCom3);
  return flgIsDataReceivedCom3;
}

int _isDataTransmittedCom1()
{
  return flgIsDataTransmittedCom1;
}

int _isDataTransmittedCom2()
{
  return flgIsDataTransmittedCom2;
}

void mySleep(unsigned int ms)
{
  unsigned int cnt = GlobalTimer;
  while( ms ){
    if(cnt != GlobalTimer){
      ms--;
      cnt = GlobalTimer;
    }
  }
}

void SetMyFlag(int n, unsigned int val, unsigned char state)
{
  TimersFlags[n].flag = 1;
  TimersFlags[n].timer = val;
  TimersFlags[n].state = state;
  //Print("TimersFlags[%d].flag = %d; TimersFlags[%d].timer = %d\n\r", n, TimersFlags[n].flag, n, val);
}

unsigned int GetMyFlag(int n)
{
  //if( ( (100 <= 0) && (100 > -MAXINT) ) != 0){
  //  Print("TimersFlags[%d] = %d\n\r", n, TimersFlags[n]);
  //  Print( "r == %d\n\r", ( (TimersFlags[n] <= 0) && (TimersFlags[n] > -MAXINT) ) );
  //}
  //Print("%d->%d\n\r", n, TimersFlags[n].flag);
  if( TimersFlags[n].state == 1 )
    return TimersFlags[n].flag;
  return 1;
}

unsigned char ReadDataCom1(unsigned char *buffer)
{
  unsigned char i = 0;
  for(i = 0; i < _IsCom1(); i++)
    buffer[i] = InData1[i];
  return i;
}

unsigned char ReadDataCom2(unsigned char *buffer)
{
  unsigned char i = 0;
  for(i = 0; i < _IsCom2(); i++)
    buffer[i] = InData2[i];
  return i;
}

unsigned long int getGlobalTimer()
{
  return GlobalTimer;
}

int SetLenghtOutputBufferCom1( int len )
{
  return ( __Out_OutIdx1 = len );
}

int SetLenghtOutputBufferCom2( int len )
{
  return ( __Out_OutIdx2 = len );
}