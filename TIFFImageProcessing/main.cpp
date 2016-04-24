#include"tiffio.h"

#include <iostream>

int main() {
	TIFF_FILE *file = tiff_read("C:\\Users\\¾¸\\Desktop\\5.2.10.tiff");

	TIFF_IMAGE *img = file->image_p;
	if (img->phtmtrc_intprt == WHITE_IS_ZERO || img->phtmtrc_intprt == BLACK_IS_ZERO) {// is greyscale image
		unsigned int scale = 1U << (get_bits_per_pixel(img));
		unsigned short *hist = new unsigned short[scale];
		for (int i = 0;i < scale;i++)
			hist[i] = 0;
		for(unsigned int x = 0; x < img->width;x++)
			for (unsigned int y = 0;y < img->length;y++) {
				unsigned int s = get_grayscale_pixel(file->image_p, x, y);
				hist[s]++;
			}

		for (int i = 0;i < scale;i++)
			std::cout << i << ':' << hist[i] << std::endl;

		delete[] hist;
	}


	tiff_delete(file);
	std::cin.get();
}