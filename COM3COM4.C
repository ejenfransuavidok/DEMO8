/*
  COM3COM4.C
   Demo program for I-7188 COM3/COM4 ISR
*/
#include<conio.h>
#include<dos.h>
#include"UART7.H"

#ifdef __TURBOC__
 #define Enable()  enable()
 #define Disable() disable()
 #define outpw     outport
 #define inpw      inport
#else
 #define Enable()  _enable()
 #define Disable() _disable()
#endif

#define  ASCII_CR    0x0d
#define  ASCII_LF    0x0a
#define  ASCII_BS    0x08
#define  ASCII_BELL  0x07
#define  ASCII_TAB   0x09
#define  ASCII_XON   0x11
#define  ASCII_XOFF  0x13
#define IN_BUF_SIZE   1024
#define OUT_BUF_SIZE  1024
#define SizeToXoff    100
#define SizeToXon     200

#define CTL_OFF            0xff00       /* Ctl reg offset for peripherals */

#define OFFS_SPRT1_BDV     0x18         /* serial port bauddiv reg  */
#define OFFS_SPRT1_RX      0x16         /* serial port receive reg  */
#define OFFS_SPRT1_TX      0x14         /* serial port transmit reg */
#define OFFS_SPRT1_STAT    0x12         /* serial port status reg   */
#define OFFS_SPRT1_CTL     0x10         /* serial port control reg  */

#define OFFS_SPRT0_BDV     0x88         /* serial port bauddiv reg  */
#define OFFS_SPRT0_RX      0x86         /* serial port receive reg  */
#define OFFS_SPRT0_TX      0x84         /* serial port transmit reg */
#define OFFS_SPRT0_STAT    0x82         /* serial port status reg  */
#define OFFS_SPRT0_CTL     0x80         /* serial port control reg */

#define SPRT1_BDV          (CTL_OFF+OFFS_SPRT1_BDV)
#define SPRT1_RX           (CTL_OFF+OFFS_SPRT1_RX)
#define SPRT1_TX           (CTL_OFF+OFFS_SPRT1_TX)
#define SPRT1_STAT         (CTL_OFF+OFFS_SPRT1_STAT)
#define SPRT1_CTL          (CTL_OFF+OFFS_SPRT1_CTL)
#define SPRT0_BDV          (CTL_OFF+OFFS_SPRT0_BDV)
#define SPRT0_RX           (CTL_OFF+OFFS_SPRT0_RX)
#define SPRT0_TX           (CTL_OFF+OFFS_SPRT0_TX)
#define SPRT0_STAT         (CTL_OFF+OFFS_SPRT0_STAT)
#define SPRT0_CTL          (CTL_OFF+OFFS_SPRT0_CTL)

#define SPRT_CTL_RSIE_ES      0x1000   /*  enable Rx status interrupts */
#define SPRT_CTL_BRK_ES       0x0800   /*  send break                  */
#define SPRT_CTL_TB8_ES       0x0400   /*  Transmit data bit 8         */
#define SPRT_CTL_HS_ES        0x0200   /*  hardware handshake enable   */
#define SPRT_CTL_TXIE_ES      0x0100   /*  enable transmit interrupt   */
#define SPRT_CTL_RXIE_ES      0x0080   /*  enable receive interrupt    */
#define SPRT_CTL_TMODE_ES     0x0040   /*  enable transmitter          */
#define SPRT_CTL_RMODE_ES     0x0020   /*  enable receiver             */
#define SPRT_CTL_EVN_ES       0x0010   /*  even parity                 */
#define SPRT_CTL_PE_ES        0x0008   /*  enable parity checking      */
#define SPRT_CTL_MODE1_ES     0x0001   /*  Async. mode A               */
#define SPRT_CTL_MODE2_ES     0x0002   /*  Async. address recog. mode  */
#define SPRT_CTL_MODE3_ES     0x0003   /*  Async. mode B               */
#define SPRT_CTL_MODE4_ES     0x0004   /*  Async. mode C               */
/*
// Asynchronous serial port status register (SPRT_STAT) ES only
*/
#define SPRT_STAT_BRK1_ES     0x0400  /*  Long break detected    */
#define SPRT_STAT_BRK0_ES     0x0200  /*  Short break detected   */
#define SPRT_STAT_RB8_ES      0x0100  /*  Receive data bit 9     */
#define SPRT_STAT_RDR_ES      0x0080  /*  Receive data ready     */
#define SPRT_STAT_THRE_ES     0x0040  /*  Tx holding reg. empty  */
#define SPRT_STAT_FRAME_ES    0x0020  /*  Framing error detected */
#define SPRT_STAT_OVERFLOW_ES 0x0010  /*  Overrun error detected */
#define SPRT_STAT_PARITY_ES   0x0008  /*  Parity error detected  */
#define SPRT_STAT_TEMT_ES     0x0004  /*  transmitter is empty   */
#define SPRT_STAT_HS0_ES      0x0002  /*  *CTS signal asserted   */

/*  all Sbreaks must set this bit */
#define SPRT_STAT_BRK_ES    SPRT_STAT_BRK0_ES

#define OFFS_INT_EOI       0x22         /* End-of-interrupt register  */
#define EOITYPE_SPRT0      0x14
#define EOITYPE_SPRT1      0x11         /* Serial port 1 on ES    */
#define OFFS_INT_MASK      0x28         /* Interrupt mask register   */
#define INT_EOI            (CTL_OFF+OFFS_INT_EOI)
#define INT_MASK           (CTL_OFF+OFFS_INT_MASK)

#define ENABLE_RX_DX  SPRT_CTL_MODE1_ES | SPRT_CTL_TMODE_ES | SPRT_CTL_RMODE_ES | SPRT_CTL_RXIE_ES | SPRT_CTL_TXIE_ES
#define NoIntCtrlMask SPRT_CTL_MODE1_ES | SPRT_CTL_TMODE_ES | SPRT_CTL_RMODE_ES
#define WithIntCtrlmask ENABLE_RX_DX

static int InBuf0[IN_BUF_SIZE];
static int OutBuf0[OUT_BUF_SIZE];
static int InBeginIdx0;
static int InEndIdx0;
static int OutBeginIdx0;
static int OutEndIdx0;
static char ReceiveXoff0=0;
static char NeedXoff0=0;
static char SendXoff0=0;

static int InBuf1[IN_BUF_SIZE];
static int OutBuf1[OUT_BUF_SIZE];
static int InBeginIdx1;
static int InEndIdx1;
static int OutBeginIdx1;
static int OutEndIdx1;
static char ReceiveXoff1=0;
static char NeedXoff1=0;
static char SendXoff1=0;

int SerCtl0,SerCtl1;

void InitSerial(void)
{ int bdv;
  InBeginIdx0=InEndIdx0=OutBeginIdx0=OutEndIdx0=0;
  InBeginIdx1=InEndIdx1=OutBeginIdx1=OutEndIdx1=0;
  SerCtl0=SerCtl1=WithIntCtrlmask;
  outpw(SPRT0_CTL,SerCtl0); /* N,8,1 */
  //outpw(SPRT1_CTL,SerCtl1); /* N,8,1 */
  /* baudrate :
  // bdv=2500000L/baudrate+1
  // 130-> 19200
  // 22 -> 115200
  */
  bdv=(int)(2500000L/9600L)+1;
  outpw(SPRT0_BDV,bdv);
  //outpw(SPRT1_BDV,bdv);
  inpw(SPRT0_RX);
  //inpw(SPRT1_RX);
  outpw(SPRT0_STAT,0);
  //outpw(SPRT1_STAT,0);
}

int IsCom3(void)
{
   return InEndIdx0-InBeginIdx0;
}

int IsCom4(void)
{
   return InEndIdx1-InBeginIdx1;
}

int ClearCom3(void)
{
   ClearCom3Flags();
   return (InEndIdx0 = InBeginIdx0 = 0);
}

int ClearCom4(void)
{
   return (InEndIdx1 = InBeginIdx1 = 0);
}

void ToCom3(int data)
{ int done;
  int nextidx;

  do{
     Disable();
     done=(nextidx=(OutEndIdx0+1)&(OUT_BUF_SIZE-1)) != OutBeginIdx0;
     if(done){
	OutBuf0[OutEndIdx0]=data;
	OutEndIdx0=nextidx;
     }
     Enable();
     outpw(SPRT0_CTL,ENABLE_RX_DX);
  } while(!done);
}

void ToCom3Str(unsigned char *str, int size)
{
  unsigned char i;
  OutBeginIdx0 = 0;
  OutEndIdx0 = 0;
  for(i=0; i<size; i++){
    ToCom3(*str++);
    //OutBuf0[i] = *str++;
  }
  //OutEndIdx0 = size;
}

int ReadCom3(void)
{ int done;
  int data;

  do{
     Disable();
    done=IsCom3();
    if(done){
       data=InBuf0[InBeginIdx0];
       InBeginIdx0=(InBeginIdx0+1) & (IN_BUF_SIZE-1);
    }
     Enable();
  } while(!done);
  return data;
}

int ReadCom3Str(char *str)
{
  unsigned char i;
  //while(IsCom3()){
  //  *str++ = ReadCom3();
  //}
  for(i = 0; i < IsCom3(); i++){
    str[i] = InBuf0[i];
  }
  return IsCom3();
}

void ToCom4(int data)
{ int done;
  int nextidx;

  do{
     Disable();
     done=(nextidx=(OutEndIdx1+1)&(OUT_BUF_SIZE-1)) != OutBeginIdx1;
     if(done){
	OutBuf1[OutEndIdx1]=data;
	OutEndIdx1=nextidx;
     }
     Enable();
     outpw(SPRT1_CTL,ENABLE_RX_DX);
  } while(!done);
}

void ToCom4Str(char *str)
{
  while(*str){
    ToCom4(*str);
    str++;
  }
}

int ReadCom4(void)
{ int done;
  int data;

  do{
     Disable();
    done=IsCom4();
    if(done){
       data=InBuf1[InBeginIdx1];
       InBeginIdx1=(InBeginIdx1+1) & (IN_BUF_SIZE-1);
    }
     Enable();
  } while(!done);
  return data;
}

int ReadCom4Str(char *str)
{ int count=0;
  char data;

  do {
    data=ReadCom4();
    if(data!='\n') {
       *str++=data;
       count++;
    }
    else *str++=0;
  } while(data != '\n');
  return count;
}

int count0=0;
/* Serial0_Isr can use software flow control (Xon/Xoff) */
void interrupt far Serial0_Isr(void)
{ int status;
  int data;
  int bufroom;
  int ctrl;
  int datain;

  count0++;
  status=inpw(SPRT0_STAT);
  datain=status&SPRT_STAT_RDR_ES;
  bufroom=(InBeginIdx0-InEndIdx0-1)&(IN_BUF_SIZE-1);
  if(datain){
     data=inpw(SPRT0_RX);
  }
  ctrl=inpw(SPRT0_CTL);
  outpw(SPRT0_CTL,NoIntCtrlMask);
  outpw(INT_EOI,EOITYPE_SPRT0);
  if(datain){
     switch(data){
      case ASCII_XOFF:
	   ReceiveXoff0=1;
	   break;
      case ASCII_XON:
	   ReceiveXoff0=0;
	   ctrl |= ENABLE_RX_DX; /*SPRT_CTL_TMODE_ES; */
	   break;
      default:
	 if(bufroom==0){
	    /* buffer underrun  --> set error no */
	    break;
	 }
	 InBuf0[InEndIdx0]=data;
	 InEndIdx0=(InEndIdx0+1) & (IN_BUF_SIZE-1);
	 if(bufroom<SizeToXoff){
	    if((NeedXoff0!=SendXoff0)){
		ctrl |= SPRT_CTL_TMODE_ES;
	    }
	 }
	 break;
     }
  }
  if((ctrl & SPRT_CTL_TMODE_ES) && (status & SPRT_STAT_THRE_ES)){
     if(NeedXoff0){
	outpw(SPRT0_TX,ASCII_XOFF);
	NeedXoff0=0;
	SendXoff0=1;
     }
     else if(SendXoff0 && bufroom>SizeToXon){
	     outpw(SPRT0_TX,ASCII_XON);
	     SendXoff0=0;
     }
     else if((OutBeginIdx0-OutEndIdx0) && !ReceiveXoff0){ /* data in output buffer--> send out */
	outpw(SPRT0_TX,OutBuf0[OutBeginIdx0++]);
	OutBeginIdx0 &= (OUT_BUF_SIZE-1);
	if(OutBeginIdx0==OutEndIdx0){
	   goto mask_TXIE;
	}
     }
     else /*if(status & SPRT_STAT_TEMT_ES)*/{
	/*ctrl &= ~SPRT_CTL_TMODE_ES; */
mask_TXIE:
	   ctrl &= ~SPRT_CTL_TXIE_ES;
     }
  }
  outpw(SPRT0_CTL,ctrl);
}

/* Serial0_Isr_0  has not software flow control (Xon/Xoff) */
void interrupt far Serial0_Isr_0(void)
{ int status;
  int data;
  int bufroom;
  int ctrl;
  int datain;

  count0++;
  status=inpw(SPRT0_STAT);
  datain=status&SPRT_STAT_RDR_ES;
  if(datain){
     data=inpw(SPRT0_RX);
  }
  ctrl=inpw(SPRT0_CTL);
  outpw(SPRT0_CTL,NoIntCtrlMask);
  outpw(INT_EOI,EOITYPE_SPRT0);
  if(datain){
     ClearTimerCom3();
     InBuf0[InEndIdx0]=data;
     InEndIdx0=(InEndIdx0+1) & (IN_BUF_SIZE-1);
  }
  if((ctrl & SPRT_CTL_TMODE_ES)&& (status & SPRT_STAT_THRE_ES)){
     if(OutBeginIdx0-OutEndIdx0){ /* data in output buffer--> send out */
	outp(SPRT0_TX,OutBuf0[OutBeginIdx0++]);
	OutBeginIdx0 &= (OUT_BUF_SIZE-1);
	if(OutBeginIdx0==OutEndIdx0){
	   goto mask_TXIE;
	}
     }
     else /*if(status & SPRT_STAT_TEMT_ES)*/{
/*           ctrl &= ~SPRT_CTL_TMODE_ES; */
mask_TXIE:
	   ctrl &= ~SPRT_CTL_TXIE_ES;
     }
  }
  outpw(SPRT0_CTL,ctrl);
}

void interrupt far Serial1_Isr(void)
{ int status;
  int data;
  int bufroom;
  int ctrl;
  int datain;

  status=inpw(SPRT1_STAT);
  datain=status&SPRT_STAT_RDR_ES;
  if(datain){
     data=inpw(SPRT1_RX);
  }
  ctrl=inpw(SPRT1_CTL);
  outpw(SPRT1_CTL,NoIntCtrlMask);
  outpw(INT_EOI,EOITYPE_SPRT1);
  if(datain){
     InBuf1[InEndIdx1]=data;
     InEndIdx1=(InEndIdx1+1) & (IN_BUF_SIZE-1);
  }
  if((ctrl & SPRT_CTL_TMODE_ES)&& (status & SPRT_STAT_THRE_ES)){
     if(OutBeginIdx1-OutEndIdx1){ /* data in output buffer--> send out */
	outp(SPRT1_TX,OutBuf1[OutBeginIdx1++]);
	OutBeginIdx1 &= (OUT_BUF_SIZE-1);
	if(OutBeginIdx1==OutEndIdx1){
	   goto mask_TXIE;
	}
     }
     else /*if(status & SPRT_STAT_TEMT_ES)*/{
/*           ctrl &= ~SPRT_CTL_TMODE_ES; */
mask_TXIE:
	   ctrl &= ~SPRT_CTL_TXIE_ES;
     }
  }
  outpw(SPRT1_CTL,ctrl);
}

unsigned long far *IntVect=(unsigned long far *)0x00000000L;
unsigned long SavedVect11,SavedVect14;

void InstallIsr(void)
{
     Disable();
  //SavedVect11=IntVect[0x11];
  //IntVect[0x11]=(unsigned long)Serial1_Isr;
  //outpw(INT_MASK, inpw(INT_MASK)&(~0x0200));  /* enable COM4 INTERRUPT */
  SavedVect14=IntVect[0x14];
  IntVect[0x14]=(unsigned long)Serial0_Isr_0;
  outpw(INT_MASK, inpw(INT_MASK)&(~0x0400));  /* enable COM3 INTERRUPT */
     Enable();
}

void UnInstallIsr(void)
{
     Disable();
  //IntVect[0x11]=SavedVect11;
  IntVect[0x14]=SavedVect14;
     Enable();
}