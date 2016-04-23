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
	cout << byte_order;
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
	cout << signature;
	if (signature != 42) {
		cout << "tiff signature not match";
		cin.get();
		return nullptr;
	}

	TIFF_FILE *tfile = new TIFF_FILE;
	strcpy_s(tfile->byte_order, 3, byte_order);
	tfile->tiff_sign = signature;
	tfile->ifd_off = readint(ifs, bytord);
	cout << endl << tfile->ifd_off;

	//read IFDs, at least one IFD
	ifs.seekg(tfile->ifd_off);
	TIFF_IMAGE *img_p = new TIFF_IMAGE;
	tfile->image_p = img_p;
	while(true) {
		IFD* ifd_p = new IFD;
		img_p->ifd_p = ifd_p;
		ifd_p->entry_c = readshort(ifs, bytord);
		cout << " entry count:" << ifd_p->entry_c;
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
			print_entry(ent);

			//intepret IFD entry values
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
				img_p->bits_per_sample = ((unsigned short*)(ent->values))[0];
				break;
			}
			case PLANAR_CONFIGURATION: {
				img_p->plannar_conf= ((unsigned short*)(ent->values))[0];
				break;
			}
			}
		}

		//read next IFD if necessary
		ifd_p->nx_ifd_off = readint(ifs, bytord);
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
	cin.get();
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

		TIFF_IMAGE *nxp = img_p->nx_image;
		delete img_p;
		img_p = nxp;
	} while (img_p != 0);
	
	delete tiff;
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

