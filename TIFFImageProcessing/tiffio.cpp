#include "tiffio.h"
#include "ioutils.h"

#include <iostream>
#include <fstream>
using namespace std;

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

	IFD* ifd_p;

	fb->pubseekpos(tfile->ifd_off, ios_base::in);
	ifd_p = new IFD;
	tfile->ifd_p = ifd_p;
	while(true) {
		ifd_p->entry_c = readshort(fb, bytord);
		cout << " entry count:" << ifd_p->entry_c;
	
		ifd_p->ifd_entry_p = new IFD_ENTRY[ifd_p->entry_c];
		for (int i = 0; i < ifd_p->entry_c;i++) {
			IFD_ENTRY *ent = ifd_p->ifd_entry_p + i;
			ent->id = readshort(fb, bytord);
			ent->type = readshort(fb, bytord);
			ent->value_c = readint(fb, bytord);
			ent->value_off = readint(fb, bytord);
			cout << endl << "id:" << ent->id << " type:" << ent->type << " value count:" << ent->value_c << " value offset:" << ent->value_off;
		}
		ifd_p->nx_ifd_off = readint(fb, bytord);
		if (ifd_p->nx_ifd_off != 0) {
			fb->pubseekpos(ifd_p->nx_ifd_off, ios_base::in);
			ifd_p->nx_ifd_p = new IFD;
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

