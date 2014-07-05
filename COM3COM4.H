#ifndef _COM3COM4_H_
#define _COM3COM4_H_
void InstallIsr(void);
void UnInstallIsr(void);
int ReadCom3Str(char *str);
int ReadCom4Str(char *str);
void ToCom3Str(unsigned char *str, int size);
void ToCom4Str(char *str);
void ToCom3(int data);
void ToCom4(int data);
int IsCom3(void);
int IsCom4(void);
int ClearCom3(void);
int ClearCom4(void);
#endif