#include "tiffio.h"
#include "ioutils.h"

#include <iostream>
#include <fstream>
using namespace std;

const char IFD_TYPE_SIZE[] = { 0,1,1,2,4,8,1,1,2,4,8,4,8 };//in byte
bool is_type_valid(unsigned short type) {
	if (type <= 0 || type >= IFD_TYPE_BOUND)
		return false;
	return true;
}
unsigned int get_type_size(unsigned short type) {
	return IFD_TYPE_SIZE[type];
}

void print_entry(IFD_ENTRY*);
void get_entry_value(ifstream &ifs, IFD_ENTRY *ent, char byte_order);
void interpret_tags(TIFF_IMAGE *img_p, IFD_ENTRY *ent);

TIFF_FILE* tiff_read(const char* path) {
	ifstream ifs(path);
	if (!ifs.good()) {
		cout << "open failed";
		cin.get();
		return nullptr;
	}
	//filebuf *fb = ifs.rdbuf();
	
	char byte_order[3] = { 0,0,0 };
	//fb->sgetn(byte_order, 2);
	ifs.read(byte_order, 2);
	//cout << byte_order;
	char bytord;
	if (strcmp(byte_order, LITTLE_ENDIAN_S)==0) {
		bytord = LITTLE_ENDIAN;
	}
	else if (strcmp(byte_order, BIG_ENDIAN_S)==0) {
		bytord = BIG_ENDIAN;
	}
	else{
		cout << "header byte_order unrecognized";
		cin.get();
		return nullptr;
	}
	
	unsigned short signature = readshort(ifs, bytord);
	//cout << signature;
	if (signature != 42) {
		cout << "tiff signature not match";
		cin.get();
		return nullptr;
	}

	TIFF_FILE *tfile = new TIFF_FILE;
	strcpy_s(tfile->byte_order, 3, byte_order);
	tfile->tiff_sign = signature;
	tfile->ifd_off = readint(ifs, bytord);
	//cout << endl << tfile->ifd_off;

	//read IFDs, at least one IFD
	ifs.seekg(tfile->ifd_off);
	TIFF_IMAGE *img_p = new TIFF_IMAGE;
	tfile->image_p = img_p;
	while(true) {
		IFD* ifd_p = new IFD;
		img_p->ifd_p = ifd_p;
		ifd_p->entry_c = readshort(ifs, bytord);
		//cout << " entry count:" << ifd_p->entry_c;
		//read IFD entries
		ifd_p->ifd_entries = new IFD_ENTRY[ifd_p->entry_c];
		for (int i = 0; i < ifd_p->entry_c;i++) {
			IFD_ENTRY *ent = ifd_p->ifd_entries + i;
			ent->tag = readshort(ifs, bytord);
			ent->type = readshort(ifs, bytord);
			ent->value_c = readint(ifs, bytord);
			ifs.read((char *)ent->value_off, 4);
			//cout << endl << "tag:" << ent->tag << " type:" << ent->type << " value count:" << ent->value_c;

			//read entry values
			if (!is_type_valid(ent->type))
				continue;//ignore unknown type
			get_entry_value(ifs, ent, bytord);
			//print_entry(ent);

			//intepret IFD entry values
			interpret_tags(img_p, ent);
		}

		if (img_p->sample_per_pix == 0) {
			if (img_p->phtmtrc_intprt == RGB)
				img_p->sample_per_pix = 3;
			else
				img_p->sample_per_pix = 1;
		}

		//read next IFD offset
		ifd_p->nx_ifd_off = readint(ifs, bytord);

		//read image arrays
		ifs.seekg(img_p->strip_off);
		img_p->image = new unsigned char[img_p->strip_bytes_c];
		ifs.read((char *)img_p->image, img_p->strip_bytes_c);

		//read next IFD if necessary
		if (ifd_p->nx_ifd_off != 0) {
			ifs.seekg(ifd_p->nx_ifd_off);
			img_p->nx_image = new TIFF_IMAGE;
			img_p = img_p->nx_image;
		}
		else {
			img_p->nx_image = 0;
			break;
		}
	}

	ifs.close();
	//cin.get();
	return tfile;
}

void tiff_delete(TIFF_FILE *tiff) {
	if (tiff == nullptr)
		return;

	TIFF_IMAGE *img_p = tiff->image_p;
	do {
		IFD* ifdp = img_p->ifd_p;
		for (int i = 0;i < ifdp->entry_c;i++)
			delete[] ifdp->ifd_entries[i].values;
		delete[] ifdp->ifd_entries;
		delete ifdp;

		delete[] img_p->bits_per_sample;
		delete[] img_p->image;
		TIFF_IMAGE *nxp = img_p->nx_image;
		delete img_p;
		img_p = nxp;
	} while (img_p != 0);
	
	delete tiff;
}

unsigned short get_bits_per_pixel(TIFF_IMAGE *img_p) {
	unsigned short bpp = 0;
	for (int i = 0; i < img_p->sample_per_pix;i++) {
		bpp += img_p->bits_per_sample[i];
	}
	return bpp;
}

void array_lshift(unsigned char* arr, unsigned int arr_size, unsigned int bits2shift) {
	unsigned int bytes2shift = bits2shift / 8;
	bits2shift = bits2shift % 8;
	if (arr_size <= bytes2shift) {
		for (unsigned int i = 0; i < arr_size;i++)
			arr[i] = 0;
		return;
	}

	if (bytes2shift != 0) {
		unsigned int i = 0;
		for (; i < arr_size - bytes2shift;i++) {
			arr[i] = arr[i + bytes2shift];
		}
		for (; i < arr_size;i++) {
			arr[i] = 0;
		}
	}

	if (bits2shift != 0) {
		arr[0] << bits2shift;
		unsigned char mask = 0xff << (8-bits2shift);
		for (unsigned int i = 1; i < arr_size - bytes2shift;i++) {
			arr[i - 1] += arr[i] & mask;
			arr[i] << bits2shift;
		}
	}
}

unsigned int get_grayscale_pixel(TIFF_IMAGE *img_p, unsigned int x, unsigned int y) {
	if (img_p->phtmtrc_intprt > 1) throw -1;
	if (x > img_p->width - 1 || y > img_p->length - 1)
		throw -1;

	unsigned short bpp = get_bits_per_pixel(img_p);
	unsigned int pix_c = y*img_p->width + x;

	unsigned int base = bpp*pix_c / 8;
	unsigned char header = bpp*pix_c % 8;
	unsigned char tailer = bpp*(pix_c + 1) % 8;
	tailer = tailer ? (8 - tailer) : 0;
	unsigned short bits_read = bpp + header + tailer;
	unsigned char bytes_read = bits_read/8 + bits_read % 8 ? 1 : 0;

	unsigned char* buf = new unsigned char[bytes_read];
	for (unsigned int i = 0; i < bytes_read;i++)
		buf[i] = img_p->image[base + i];

	buf[bytes_read - 1] &= 0xff << tailer;
	array_lshift(buf, bytes_read, header);

	unsigned int result = 0;
	unsigned int i = 0;
	for(; i < bpp/8;i++){
		result += (unsigned int)buf[i] << (bpp - (i+1)*8);
	}
	if (bpp % 8 != 0)
		result += (unsigned int)buf[i] >> (8 - bpp % 8);

	if (img_p->phtmtrc_intprt == BLACK_IS_ZERO)
		result = ~(0xffffffff << bpp) - result;

	return result;
}


void get_entry_value(ifstream &ifs, IFD_ENTRY *ent, char byte_order) {
	unsigned char tsize = get_type_size(ent->type);
	unsigned int vsize = tsize*ent->value_c;
	unsigned char *values = new unsigned char[vsize];
	ent->values = values;
	if (vsize <= 4) {
		switch (tsize) {
		case 1: {
			for (unsigned char j = 0; j < vsize;j++)
				ent->values[j] = ent->value_off[j];
			break;
		}
		case 2: {
			*((unsigned short*)values) = readshort(ent->value_off, byte_order);
			if (ent->value_c == 2)
				*((unsigned short*)(values + 2)) = readshort(ent->value_off + 2, byte_order);
			break;
		}
		case 4: {
			*((unsigned int*)values) = readint(ent->value_off, byte_order);
			break;
		}
		}
	}
	else {
		ifs.seekg(readint(ent->value_off, byte_order));
		switch (tsize) {
		case 1: {
			for (unsigned int j = 0;j < vsize;j += 1) {
				values[j] = ifs.get();
			}
			break;
		}
		case 2: {
			for (unsigned int j = 0;j < vsize;j += 2) {
				unsigned short sh = readshort(ifs, byte_order);
				*((unsigned short*)(values + j)) = sh;
			}
			break;
		}
		case 4: {
			for (unsigned int j = 0;j < vsize;j += 4) {
				unsigned int in = readint(ifs, byte_order);
				*((unsigned int*)(values + j)) = in;
			}
			break;
		}
		case 8: {
			for (unsigned int j = 0;j < vsize;j += 8) {
				unsigned long long lo = readlong(ifs, byte_order);
				*((unsigned long long*)(values + j)) = lo;
			}
			break;
		}
		}
	}
}

void print_entry(IFD_ENTRY* ent) {
	cout << endl << "tag:" << ent->tag << " type:" << ent->type << " value count:" << ent->value_c;
	for (unsigned int j = 0; j < ent->value_c;j++) {
		cout << " value:";
		switch (get_type_size(ent->type)) {
		case 1:
			cout << (unsigned int)(ent->values[j]);
			break;
		case 2:
			cout << *(((unsigned short*)(ent->values)) + j);
			break;
		case 4:
			cout << (((unsigned int*)(ent->values))[j]);
			break;
		case 8:
			cout << (((unsigned long long*)(ent->values))[j]);
			break;
		}
	}
}

void interpret_tags(TIFF_IMAGE *img_p, IFD_ENTRY *ent) {
	unsigned char tsize = get_type_size(ent->type);
	switch (ent->tag) {
	case PHOTOMATRIC_INTERPRETATION: {
		img_p->phtmtrc_intprt = ((unsigned short*)(ent->values))[0];
		break;
	}
	case NEW_SUBFILE_TYPE: {
		img_p->subfile_type = ((unsigned int*)(ent->values))[0] & 0b111;
		break;
	}
	case IMAGE_WIDTH: {
		if (tsize == 2)
			img_p->width = ((unsigned short*)(ent->values))[0];
		else if (tsize == 4)
			img_p->width = ((unsigned int*)(ent->values))[0];
		else img_p->width = 0;
		break;
	}
	case IMAGE_LENGTH: {
		if (tsize == 2)
			img_p->length = ((unsigned short*)(ent->values))[0];
		else if (tsize == 4)
			img_p->length = ((unsigned int*)(ent->values))[0];
		else img_p->length = 0;
		break;
	}
	case COMPRESSION: {
		//TO-DO
		img_p->compression = ((unsigned short*)(ent->values))[0];
		break;
	}
	case STRIP_OFFSETS: {
		if (tsize == 2)
			img_p->strip_off = ((unsigned short*)(ent->values))[0];
		else if (tsize == 4)
			img_p->strip_off = ((unsigned int*)(ent->values))[0];
		else img_p->strip_off = 0;
		break;
	}
	case ROWS_PER_STRIP: {
		if (tsize == 2)
			img_p->rows_per_strip = ((unsigned short*)(ent->values))[0];
		else if (tsize == 4)
			img_p->rows_per_strip = ((unsigned int*)(ent->values))[0];
		break;
	}
	case STRIP_BYTES_COUNT: {
		if (tsize == 2)
			img_p->strip_bytes_c = ((unsigned short*)(ent->values))[0];
		else if (tsize == 4)
			img_p->strip_bytes_c = ((unsigned int*)(ent->values))[0];
		break;
	}
	case SAMPLES_PER_PIXEL: {
		img_p->sample_per_pix = ((unsigned short*)(ent->values))[0];
		break;
	}
	case BITS_PER_SAMPLE: {
		img_p->bits_per_sample = new unsigned char[ent->value_c];
		for(int i = 0; i< ent->value_c ;i++)
			img_p->bits_per_sample[i] = ((unsigned short*)(ent->values))[i];
		break;
	}
	case PLANAR_CONFIGURATION: {
		img_p->plannar_conf = ((unsigned short*)(ent->values))[0];
		break;
	}
	}
}

