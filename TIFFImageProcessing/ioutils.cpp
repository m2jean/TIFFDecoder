#include "ioutils.h"

#include<fstream>

unsigned short readshort(std::filebuf *fb, char bo) {
	char s[2];
	fb->sgetn(s, 2);
	if (bo == LITTLE_ENDIAN) {
		return s[0] + (s[1] << 8);
	}
	else if(bo == BIG_ENDIAN){
		return (s[0] << 8) + s[1];
	}
	else return 0;
}

unsigned int readint(std::filebuf *fb, char bo) {
	char s[4];
	fb->sgetn(s, 4);
	if (bo == LITTLE_ENDIAN) {
		return s[0] + (s[1] << 8) + (s[2] << 16) + (s[3] << 24);
	}
	else if (bo == BIG_ENDIAN) {
		return (s[0] << 24) + (s[1] << 16) + (s[2] << 8) + s[3];
	}
	else return 0;
}