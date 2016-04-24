#pragma once

#ifndef TIFFIO_H
#define TIFFIO_H

/*
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
*/

enum IFD_TYPE{
	IFD_TYPE_BYTE		= 1,
	IFD_TYPE_ASCII		= 2,
	IFD_TYPE_SHORT		= 3,
	IFD_TYPE_LONG		= 4,
	IFD_TYPE_RATIONAL	= 5,
	IFD_TYPE_SBYTE		= 6,
	IFD_TYPE_UNDEFINED	= 7,
	IFD_TYPE_SSHORT		= 8,
	IFD_TYPE_SLONG		= 9,
	IFD_TYPE_SRATIONAL	= 10,
	IFD_TYPE_FLOAT		= 11,
	IFD_TYPE_DOUBLE		= 12,
	//boundary
	IFD_TYPE_BOUND		= 13
};

enum IFD_ENTRY_TAG {
	NEW_SUBFILE_TYPE	= 254,
	IMAGE_WIDTH			= 256,
	IMAGE_LENGTH		= 257,
	BITS_PER_SAMPLE		= 258,
	COMPRESSION			= 259,
	PHOTOMATRIC_INTERPRETATION = 262,
	STRIP_OFFSETS		= 273,
	SAMPLES_PER_PIXEL	= 277,
	ROWS_PER_STRIP		= 278,
	STRIP_BYTES_COUNT	= 279,
	PLANAR_CONFIGURATION = 284
};

enum PHOTOMATRIC_INTERPRETATION_TYPE {
	WHITE_IS_ZERO	= 0,
	BLACK_IS_ZERO	= 1,
	RGB				= 2,
	PALETTE_COLOR	= 3,
	TRASPANRANCY_MASK = 4
};

struct IFD_ENTRY {
	unsigned short	tag;
	unsigned short	type;
	unsigned int	value_c;
	unsigned char	value_off[4] = { 0,0,0,0 };
	unsigned char*	values		= 0;
};

struct IFD {
	unsigned short	entry_c;
	IFD_ENTRY*		ifd_entries	= 0;
	unsigned int	nx_ifd_off	= 0;
};

#define LITTLE_ENDIAN_S	"II"
#define BIG_ENDIAN_S	"MM"

struct TIFF_IMAGE {
	IFD* ifd_p = 0;

	//basics
	unsigned char phtmtrc_intprt;
	unsigned int width;
	unsigned int length;
	unsigned char compression	= 1;
	//strip
	unsigned int strip_off;
	unsigned int rows_per_strip = 0xffff - 1;
	unsigned int strip_bytes_c = 0;
	//sample
	unsigned char sample_per_pix	= 0;
	unsigned char *bits_per_sample	= 0;
	//mischevious
	unsigned char subfile_type = 0;
	unsigned char plannar_conf	= 1;

	unsigned char *image;

	TIFF_IMAGE* nx_image;
};

struct TIFF_FILE {
	char			byte_order[3];
	unsigned short	tiff_sign;
	unsigned int	ifd_off		= 0;
	TIFF_IMAGE*		image_p		= 0;
};


TIFF_FILE* tiff_read(const char* path);
void tiff_delete(TIFF_FILE *tiff);
unsigned short get_bits_per_pixel(TIFF_IMAGE *img_p);
unsigned int get_grayscale_pixel(TIFF_IMAGE *img_p, unsigned int x, unsigned int y);

#endif // !TIFFIO_H
