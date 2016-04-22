#pragma once

#include<fstream>

#define LITTLE_ENDIAN	1
#define BIG_ENDIAN		2

///unsigned short	readshort(std::filebuf *fb, char bo = BIG_ENDIAN);
//unsigned int	readint(std::filebuf *fb, char bo = BIG_ENDIAN);
//unsigned long long	readlong(std::filebuf *fb, char bo = BIG_ENDIAN);

unsigned short	readshort(const unsigned char* buf, char bo = BIG_ENDIAN);
unsigned int	readint(const unsigned char* buf, char bo = BIG_ENDIAN);

unsigned short	readshort(std::ifstream& ifs, char bo = BIG_ENDIAN);
unsigned int	readint(std::ifstream& ifs, char bo = BIG_ENDIAN);
unsigned long long	readlong(std::ifstream& ifs, char bo = BIG_ENDIAN);