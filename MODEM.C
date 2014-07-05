#include<ctype.h>
#include<string.h>
#include<stdio.h>
#include"..\lib\7188.h"
#include"uart7.h"

/*
enum
{
  MDM_CMD_DELAY,	// must be the first one in this blk
  MDM_DIAL_TIME,
  MDM_REDIAL_PAUSE,
  MDM_AUTO_BAUD,
  MDM_DROP_DTR,
  MDM_MISC_NUMS
};

enum _mdm_rsp_
{
  MDM_CONNECT,		// must be the first one in this blk
  MDM_NO_CARRIER,
  MDM_BUSY,
  MDM_VOICE,
  MDM_NO_DIALTONE,
  MDM_ERROR,
  MDM_OK,
  MDM_RSP_NUMS,
  MDM_BAD_RESP = -1
};

enum
{
  MDM_INIT,		// must be the first one in this blk
  MDM_DIAL_PRE,
  MDM_DIAL_SUF,
  MDM_DIAL_CANCLE,
  MDM_HANG_UP,
  MDM_ANSWER,
  MDM_AUTO_ANS,
  MDM_CMD_NUMS
};
*/
/*
char MdmRspStr[MDM_RSP_NUMS][MDM_RSP_LEN+1] =
{
  "CONNECT",
  "NO CARRIER",
  "BUSY",
  "VOICE",
  "NO DIALTONE",
  "ERROR",
  "OK",
};
*/
int fDebugMode=0;
unsigned long tmptime;
int SendToModem(char *cmd)
{
  while (*cmd){
    switch (*cmd){
      case '~' :
	DelayMs(500);
	break;
      case '^' :
	_ToCom1(toupper(*(++cmd)) - 0x40);
	break;
      default:
	_ToCom1(*cmd);
	break;
    }
    cmd++;
  }
  return 0;
}

unsigned long WaitTime=10000L;
void SetWaitTime(unsigned long wt)
{
  WaitTime=wt;
}

int GetModemResponse(char *buf)
{ int i=0;
  int data;

  StopWatchStart(7);
  do{
    if(_IsCom1()){
       buf[i]=_ReadCom1();
       if(fDebugMode) putchar(buf[i]);
       if(buf[i]<' ') {
	 if(i==0) continue;
	 if(buf[i]=='\n'){
	    buf[i]=0;
	    break;
	 }
	 else if(buf[i]=='\r') continue;
       }
       i++;
    }
    else {
       StopWatchReadValue(7,&tmptime);
       if(tmptime > WaitTime) {  /* 10 secs */
	  return 1; /* timeout */
       }
    }
  } while(1);
  return 0;
}

int InitModem(void)
{  char buf[80];
/*   SendToModem("ATZ^M~~~AT S7=45 S0=0 V1 X4^M~"); */
   SendToModem("ATZ^M~~~AT S7=15 S0=0 V1 X4^M~");
   if(GetModemResponse(buf)) return 1; /* here echo "ATZ" */
   if(GetModemResponse(buf)) return 1; /* here echo "OK" */
   if(strcmp(buf,"OK")) return 2;
   if(GetModemResponse(buf)) return 1; /* here echo "AT S7=45 S0=0 V1 X4"  */
   if(GetModemResponse(buf)) return 1; /* here echo "OK" */
   if(strcmp(buf,"OK")) return 2;
   return 0;
}

int HangUp(void)
{ char buf[80];

  SendToModem("~~~+++~~~ATH0^M");
  GetModemResponse(buf);
  GetModemResponse(buf);
  return 0;
}

int AnswerPhone(void)
{ char buf[80];

  SendToModem("ATA^M");
  GetModemResponse(buf);
  GetModemResponse(buf);
  return 0;
}

int AutoAnswer(void)
{ char buf[80];

  SendToModem("ATS0=1^M");
  if(GetModemResponse(buf)) return 1;
  if(GetModemResponse(buf)) return 1;
  if(strcmp(buf,"OK")) return 2;
  return 0;
}

int SendTel(char *Phno)
{ char buf[80];
  int error=0;

/*  SendToModem("ATH1DT"); */
  SendToModem("ATDT");
  SendToModem(Phno);
  SendToModem("^M");
  GetModemResponse(buf);
  SetWaitTime(50000);
  GetModemResponse(buf);
  SetWaitTime(10000);
  printf("[test]:%s\n",buf);
  if(!strcmp("NO CARRIER",buf)){
     error=3;
  }
  else if(!strcmp("BUSY",buf)){
     error=4;
  }
  else if(!strcmp("NO DIALTONE",buf)){
     error=5;
  }
  else if(!strncmp("CONNECT",buf,7)){
	  error=0;
  }
  else error=6;
  return error;
}

int DetectConnect(void)
{
  return 0;
}

int OpenModem(void)
{
   _ClearCom1();
   printf("\ninit modem\n");
   if(0==InitModem()){
      AutoAnswer();
      printf("(Open Modem success)\n");
      return 0;
   }
   else {
      printf("(Open Modem fail)\n");
      return 1;
   }
}
