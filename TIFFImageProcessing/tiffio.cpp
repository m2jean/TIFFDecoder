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

	//read IFDs
	IFD* ifd_p;
	//first IFD
	ifs.seekg(tfile->ifd_off);
	ifd_p = new IFD;
	tfile->ifd_p = ifd_p;
	while(true) {
		ifd_p->entry_c = readshort(ifs, bytord);
		cout << " entry count:" << ifd_p->entry_c;
		//read IFD entries
		ifd_p->ifd_entry_p = new IFD_ENTRY[ifd_p->entry_c];
		for (int i = 0; i < ifd_p->entry_c;i++) {
			IFD_ENTRY *ent = ifd_p->ifd_entry_p + i;
			ent->tag = readshort(ifs, bytord);
			ent->type = readshort(ifs, bytord);
			ent->value_c = readint(ifs, bytord);
			ifs.read((char *)ent->value_off, 4);
			cout << endl << "tag:" << ent->tag << " type:" << ent->type << " value count:" << ent->value_c;
		}
		ifd_p->nx_ifd_off = readint(ifs, bytord);
		if (ifd_p->nx_ifd_off != 0) {
			ifs.seekg(ifd_p->nx_ifd_off);
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
			if (!is_type_valid(ent->type)) 
				continue;//ignore unknown type
			unsigned char tsize = get_type_size(ent->type);
			unsigned int vsize = tsize*ent->value_c;
			unsigned char *values = new unsigned char[vsize];
			ent->values = values;
			if (vsize <= 4) {//TO-DO
				switch (tsize) {
				case 1: {
					for (unsigned char j = 0; j < vsize;j++)
						ent->values[j] = ent->value_off[j];
					break;
				}
				case 2: {
					*((unsigned short*)values) = readshort(ent->value_off, bytord);
					if(ent->value_c == 2)
						*((unsigned short*)(values + 2)) = readshort(ent->value_off+2, bytord);
					break;
				}
				case 4: {
					*((unsigned int*)values) = readint(ent->value_off, bytord);
					break;
				}
				}
			}
			else {
				ifs.seekg(readint(ent->value_off, bytord));
				switch (tsize) {
				case 1: {
					for (unsigned int j = 0;j < vsize;j += 1) {
						values[j] = ifs.get();
					}
					break;
				}
				case 2: {
					for (unsigned int j = 0;j < vsize;j += 2) {
						unsigned short sh = readshort(ifs, bytord);
						*((unsigned short*)(values + j)) = sh;					
					}
					break;
				}
				case 4: {
					for (unsigned int j = 0;j < vsize;j += 4) {
						unsigned int in = readint(ifs, bytord);
						*((unsigned int*)(values + j)) = in;
					}
					break;
				}
				case 8: {
					for (unsigned int j = 0;j < vsize;j += 8) {
						unsigned long long lo = readlong(ifs, bytord);
						*((unsigned long long*)(values + j)) = lo;
					}
					break;
				}
				}
			}		

			unsigned short test[] = { 1,256 };
			cout << endl << "tag:" << ent->tag << " type:" << ent->type << " value count:" << ent->value_c;
			for (unsigned int j = 0; j < ent->value_c;j++) {
				cout << " value:";
				switch(tsize) {
				case 1:
					cout << (unsigned int)(ent->values[j]);
					break;
				case 2:
					cout << *(((unsigned short*)(ent->values))+j);
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

		if (ifd_p->nx_ifd_p != 0) {
			ifd_p = ifd_p->nx_ifd_p;
		}
		else break;
	}

	ifs.close();
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

