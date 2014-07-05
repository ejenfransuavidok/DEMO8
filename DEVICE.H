#ifndef __DEVICE_H__
#define __DEVICE_H__

#include"101.H"
#include"CRC16.h"
#include"MERQURY.H"

enum DEVICE { DEV101, DEVMERQURY, DEVUNDEFINED };

byte GetTimer1_Device(int n);
void ToDeviceDataById(int n);
int DataTransmittedDevice(int n);
byte GetTimer2_Device(int n);
int DataReceivedDevice(int n);
byte ReadBufferDevice(int n);
int GetAllDevices();
int AddDevice( enum DEVICE dev, int id );
int GetCyclTimer2_Device(int n);
int GetDeviceId(int index);
void calcCRC(unsigned char *d, int len);
byte BusyDeviceByCom(int com_port);
byte SetBusyDevice(int com_port, int n);
byte ClearBusyDeviceById(int id);
byte ClearBusyDeviceByCom(int com_port);
byte BusyDeviceById(int id);
void PrepareComToTransmitById(int n);
void PrepareComToReceiveById(int n);
int ClearComDeviceById(int n);
int GetTUFlagDevice( unsigned char id );
int IncTUFlagDevice( int id );
unsigned char HandleTUDevice( unsigned char id );
#endif