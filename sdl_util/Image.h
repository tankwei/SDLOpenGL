#ifndef _IMAGE_H
#define _IMAGE_H

#include "stdio.h"
#include <SDL2/SDL_stdinc.h>

class Image
{
public:
	Image();
	virtual ~Image();
public:
	int nWidth;
	int nHeight;
	unsigned char* imgData;

	unsigned char getRAt(int x, int y){
		unsigned char* addr = getAddrAt(x, y);
		char col = addr[0];
		return col;
	}
	unsigned char getGAt(int x, int y){
		unsigned char* addr = getAddrAt(x, y);
		char col = addr[1];
		return col;
	}
	unsigned char getBAt(int x, int y){
		unsigned char* addr = getAddrAt(x, y);
		char col = addr[2];
		return col;
	}
	unsigned char getAAt(int x, int y){
		unsigned char* addr = getAddrAt(x, y);
		char col = addr[3];
		return col;
	}

	unsigned char* getAddrAt(int x, int y){
		int sizePerLine = nWidth * 4;
		long offset = sizePerLine * y + x * 4;
		return this->imgData + offset;
	}
	void setColorAt(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a){
		unsigned char* addr = getAddrAt(x, y);
		addr[0] = r;
		addr[1] = g;
		addr[2] = b;
		addr[3] = a;
	}
	bool loadImage(const char *filePath);
private:
	int check_if_png(const char *filePath, FILE **fp);
	void read_png(FILE *fp, int sig_read);
	int readPng(const char *inname);
	int readPngFile(const char *inname);
	int test_one_file(const char *inname, const char *outname);
};

#endif
