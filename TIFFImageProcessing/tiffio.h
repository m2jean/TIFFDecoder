#pragma once

#ifndef TIFFIO_H
#define TIFFIO_H

//field types
#define IFD_TYPE_BYTE		1
#define IFD_TYPE_ASCII		2
#define IFD_TYPE_SHORT		3
#define IFD_TYPE_LONG		4
#define IFD_TYPE_RATIONAL	5
#define IFD_TYPE_SBYTE		6
#define IFD_TYPE_UNDEFINED	7
#define IFD_TYPE_SSHORT		8
#define IFD_TYPE_SLONG		9
#define IFD_TYPE_SRATIONAL	10
#define IFD_TYPE_FLOAT		11
#define IFD_TYPE_DOUBLE		12
//boundary
#define IFD_TYPE_BOUND		13


struct IFD_ENTRY {
	unsigned short	tag;
	unsigned short	type;
	unsigned int	value_c;
	unsigned int	value_off	= 0;
	unsigned char*	values		= 0;
};

struct IFD {
	unsigned short	entry_c;
	IFD_ENTRY*		ifd_entry_p	= 0;
	unsigned int	nx_ifd_off	= 0;
	IFD*			nx_ifd_p	= 0;
};

#define LITTLE_ENDIAN_S	"II"
#define BIG_ENDIAN_S	"MM"

struct TIFF_FILE {
	char			byte_order[3];
	unsigned short	tiff_sign;
	unsigned int	ifd_off		= 0;
	IFD*			ifd_p		= 0;
};

TIFF_FILE* tiff_read(const char* path);
void tiff_delete(TIFF_FILE *tiff);

#endif // !TIFFIO_H
