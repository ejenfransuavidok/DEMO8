
#define Txbuf	  0x00	   /* tx buffer */
#define Rxbuf	  0x00	   /* rx buffer */
#define Dll	  0x00	   /* baud lsb */
#define Dlh	  0x01	   /* baud msb */
#define Ier	  0x01	   /* int enable reg */
#define Fcr	  0x02	   /* FIFO control register */
#define Iir       0x02     /* Interrupt Identification Register */
#define Lcr	  0x03	   /* line control reg */
#define Dfr	  0x03	   /* Data format  reg */
#define Mcr	  0x04	   /* modem control reg */
#define Lsr	  0x05	   /* line status reg */
#define Msr	  0x06	   /* modem status reg */
#define Scr	  0x07	   /* Scratch reg */

#define NoError         0
#define PortError      -1
#define DataError      -2
#define ParityError    -3
#define StopError      -4
#define TimeOut        -5
#define QueueEmpty     -6
#define QueueOverflow  -7
#define BaudRateError  -13
#define CheckSumError  -14

int _InstallCom1(unsigned long lBaudRate,int iData,int iParity,int iStop);
int _InstallCom2(unsigned long lBaudRate,int iData,int iParity,int iStop);
int _RestoreCom1(void);
int _RestoreCom2(void);
int _IsCom1(void);
int _IsCom2(void);
int _ToCom1(char data);
int _ToCom2(char data);
int _ReadCom1(void);
int _ReadCom2(void);
int _ClearCom1(void);
int _ClearCom2(void);
void _ToCom1Str(char *str);
void _ToCom2Str(char *str);
int _getOutputBufCom1(int haveBeen);
int _getOutputBufCom2(int haveBeen);
int _isDataReceivedCom1();
int _isDataReceivedCom2();
int _isDataTransmittedCom1();
int _isDataTransmittedCom2();
void interrupt Isr9(void);
void install_9(void);
void restore_9(void);
void SetTimerCom1(unsigned int t);
void SetTimerCom2(unsigned int t);
void SetTimerCom3(unsigned int t);
void ClearTimerCom3();
void ClearCom3Flags();
unsigned char ReadDataCom1(unsigned char *buffer);
unsigned char ReadDataCom2(unsigned char *buffer);
void InstallTimerCom1();
void InstallTimerCom2();
void InstallTimerCom3();
void SetMyFlag(int n, unsigned int val, unsigned char state);
unsigned int GetMyFlag();
void mySleep(unsigned int ms);
unsigned long int getGlobalTimer();
int SetLenghtOutputBufferCom1( int len );
int SetLenghtOutputBufferCom2( int len );