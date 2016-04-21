#include "tiffio.h"
#include "ioutils.h"

#include <iostream>
#include <fstream>
using namespace std;

const char IFD_TYPE_SIZE[] = { 1,1,2,4,8,1,1,2,4,8,4,8 };//in byte
bool is_type_valid(unsigned short type) {
	if (type <= 0 || type >= IFD_TYPE_BOUND)
		return false;
	return true;
}
unsigned int get_type_size(unsigned short type) {
	return IFD_TYPE_SIZE[type];
}

TIFF_FILE* tiff_read(const char* path) {
	ifstream ifs(path);
	if (!ifs.good()) {
		cout << "open failed";
		cin.get();
		return nullptr;
	}
	filebuf *fb = ifs.rdbuf();
	
	char byte_order[3] = { 0,0,0 };
	fb->sgetn(byte_order, 2);
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
	
	unsigned short signature = readshort(fb, bytord);
	cout << signature;
	if (signature != 42) {
		cout << "tiff signature not match";
		cin.get();
		return nullptr;
	}

	TIFF_FILE *tfile = new TIFF_FILE;
	strcpy_s(tfile->byte_order, 3, byte_order);
	tfile->tiff_sign = signature;
	tfile->ifd_off = readint(fb, bytord);
	cout << endl << tfile->ifd_off;

	//read IFDs
	IFD* ifd_p;
	//first IFD
	fb->pubseekpos(tfile->ifd_off, ios_base::in);
	ifd_p = new IFD;
	tfile->ifd_p = ifd_p;
	while(true) {
		ifd_p->entry_c = readshort(fb, bytord);
		cout << " entry count:" << ifd_p->entry_c;
		//read IFD entries
		ifd_p->ifd_entry_p = new IFD_ENTRY[ifd_p->entry_c];
		for (int i = 0; i < ifd_p->entry_c;i++) {
			IFD_ENTRY *ent = ifd_p->ifd_entry_p + i;
			ent->tag = readshort(fb, bytord);
			ent->type = readshort(fb, bytord);
			ent->value_c = readint(fb, bytord);
			ent->value_off = readint(fb, bytord);
			//cout << endl << "tag:" << ent->tag << " type:" << ent->type << " value count:" << ent->value_c << " value offset:" << ent->value_off;
		}
		ifd_p->nx_ifd_off = readint(fb, bytord);
		if (ifd_p->nx_ifd_off != 0) {
			fb->pubseekpos(ifd_p->nx_ifd_off, ios_base::in);
			ifd_p->nx_ifd_p = new IFD;
			ifd_p = ifd_p->nx_ifd_p;
		}
		else break;
	}

	//read IFD entry values
	ifd_p = tfile->ifd_p;
	while (true) {
		for (unsigned int i = 0; i < ifd_p->entry_c;i++) {
			IFD_ENTRY *ent = ifd_p->ifd_entry_p + i;
			if (!is_type_valid(ent->type)) continue;//ignore unknown type
			unsigned char tsize = get_type_size(ent->type);
			unsigned int vsize = tsize*ent->value_c;
			unsigned char *values = new unsigned char[vsize];
			ent->values = values;
			if (vsize <= 4) {
				for (unsigned char j = 0;j < vsize;j++) {
					values[j] = ent->value_off >> (3 - j) * 8;
				}
			}
			else {
				fb->pubseekpos(ent->value_off);
				for (unsigned char j = 0;j < vsize;j+=tsize) {
					//TO-DO
					for (unsigned char k = 0;k < tsize;k++) {
						unsigned char value = ent->value_off >> (3 - (j + k)) * 8;
						if (bytord == LITTLE_ENDIAN) {
							values[j + tsize - k - 1] = value;
						}
						else if (bytord == BIG_ENDIAN) {
							values[j + k] = value;
						}
					}
				}
			}
		}

		if (ifd_p->nx_ifd_p != 0) {
			ifd_p = ifd_p->nx_ifd_p;
		}
		else break;
	}

	cin.get();
	return tfile;
}

void tiff_delete(TIFF_FILE *tiff) {
	if (tiff == nullptr)
		return;

	IFD *ifdp = tiff->ifd_p;	
	do {
		IFD *nxp = ifdp->nx_ifd_p;
		delete[] ifdp->ifd_entry_p;
		delete ifdp;
		ifdp = nxp;
	} while (ifdp != 0);
	
	delete tiff;
}

