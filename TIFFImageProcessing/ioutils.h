#pragma once

#include<fstream>

#define LITTLE_ENDIAN	1
#define BIG_ENDIAN		2

unsigned short	readshort(std::filebuf *fb, char bo = BIG_ENDIAN);
unsigned int	readint(std::filebuf *fb, char bo = BIG_ENDIAN);