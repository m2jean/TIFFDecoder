#include "ioutils.h"

#include<fstream>
#include<iostream>

//unsigned short readshort(std::filebuf *fb, char bo) {
unsigned short readshort(std::ifstream& ifs, char bo) {

	//int aav = fb->in_avail();
	//std::cout << aav << ' ';

	char ss[2];
	ifs.read(ss, 2);
	unsigned char *s;
	s = (unsigned char *)ss;
	if (bo == LITTLE_ENDIAN) {
		return s[0] + (s[1] << 8);
	}
	else if(bo == BIG_ENDIAN){
		return (s[0] << 8) + s[1];
	}
	else return 0;
}

//unsigned int readint(std::filebuf *fb, char bo) {
unsigned int readint(std::ifstream& ifs, char bo) {
	char ss[4];
	ifs.read(ss, 4);
	unsigned char *s;
	s = (unsigned char *)ss;
	if (bo == LITTLE_ENDIAN) {
		return s[0] + (s[1] << 8) + (s[2] << 16) + (s[3] << 24);
	}
	else if (bo == BIG_ENDIAN) {
		return (s[0] << 24) + (s[1] << 16) + (s[2] << 8) + s[3];
	}
	else return 0;
}

//unsigned long long readlong(std::filebuf *fb, char bo) {
unsigned long long readlong(std::ifstream& ifs, char bo) {
	char ss[8];
	ifs.read(ss, 8);
	unsigned char *s;
	s = (unsigned char *)ss;
	if (bo == LITTLE_ENDIAN) {
		return s[0] + (s[1] << 8) + (s[2] << 16) + (s[3] << 24) + (s[4] << 32) + (s[5] << 40) + (s[6] << 48) + (s[7] << 56);
	}
	else if (bo == BIG_ENDIAN) {
		return s[7] + (s[6] << 8) + (s[5] << 16) + (s[4] << 24) + (s[3] << 32) + (s[2] << 40) + (s[1] << 48) + (s[0] << 56);
	}
	else return 0;
}

unsigned short readshort(const unsigned char* buf, char bo) {

	if (bo == LITTLE_ENDIAN) {
		return buf[0] + (buf[1] << 8);
	}
	else if (bo == BIG_ENDIAN) {
		return (buf[0] << 8) + buf[1];
	}
	else return 0;
}

unsigned int readint(const unsigned char* buf, char bo) {
	if (bo == LITTLE_ENDIAN) {
		return buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
	}
	else if (bo == BIG_ENDIAN) {
		return (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
	}
	else return 0;
}