cl /c /Gs /Odi /FPa /Fm /G2 demo8.c modem.c uart7.c 
link /MA /NOE /NOI demo8 modem uart7,,,..\lib\7188s;
del *.obj
del *.map

