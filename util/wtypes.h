#ifndef WTYPES_H
#define WTYPES_H
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>

#define MAX_PATH 1024

typedef float FLOAT;
typedef char CHAR;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#define VOID  void
#define WINAPI  __attribute__((stdcall))
#define CALLBACK  __attribute__((stdcall))


typedef struct tagBITMAPINFOHEADER {
		unsigned int biSize;
		int biWidth;
		int biHeight;
		unsigned short biPlanes;
		unsigned short biBitCount;
		unsigned int biCompression;
		unsigned int biSizeImage;
		unsigned int biXPelsPerMeter;
		unsigned int biYPelsPerMeter;
		unsigned int biClrUsed;
		unsigned int biClrImportant;//wind unsigned long
	} BITMAPINFOHEADER;

	typedef struct tagWRIEBITMAPFILEHEADER { 
		WORD    bfType; 
		unsigned int   bfSize;
		WORD    bfReserved1; 
		WORD    bfReserved2; 
		unsigned int   bfOffBits; 
	} BITMAPFILEHEADER_WRITE;

	typedef struct tagBITMAPFILEHEADER { 
		//WORD    bfType; 
		unsigned int   bfSize;
		WORD    bfReserved1; 
		WORD    bfReserved2; 
		unsigned int   bfOffBits; 
	} BITMAPFILEHEADER;

#endif