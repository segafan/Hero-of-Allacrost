#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tgaread.h"

inline bool
read(FILE *fp, void *p, unsigned size)
{
	return fread(p, size, 1, fp) == 1;
}

TGAFile *
tgaread(FILE *fp)
{
	TGAFile *tga;
	bool rle = false; // run length encoded
	bool color; // true color
	unsigned char buf[512];
	unsigned char length;
	unsigned char colormaptype;
	unsigned char imagetype;
	unsigned int bytedepth;

	if (!read(fp, &length, 1) || !read(fp, &colormaptype, 1) ||
		colormaptype == 1) {
		printf("length=%d colormaptype=%d\n", length, colormaptype);
		return 0;
	}

	read(fp, &imagetype, 1);

	if (imagetype == 2) {
		color = true;
	} else if (imagetype == 3) {
		color = false;
	} else if (imagetype == 10) {
		rle = true;
		color = true;
	} else if (imagetype == 11) {
		rle = true;
		color = false;
	} else {
		printf("imagetype=%d\n", imagetype);
		return 0;
	}

	tga = new TGAFile;

	fseek(fp, 9, SEEK_CUR);

	if (!read(fp, &tga->width, 2) ||
		!read(fp, &tga->height, 2) ||
		!read(fp, &tga->depth, 1) ||
		!(tga->depth== 8 || tga->depth==16 ||
		  tga->depth==24 || tga->depth==32) ||
		!read(fp, &tga->alpha, 1)) {
		delete tga;
		return 0;
	}

	tga->alpha &= 0xf;
	if (!(tga->alpha == 0 || tga->alpha == 8)) {
		printf("alpha=%d\n", tga->alpha);
		delete tga;
		return 0;
	}

	if (color)
		tga->type = (tga->depth == 32) ? TGAFile::RGBA : TGAFile::RGB;
	else
		tga->type = TGAFile::GRAYSCALE;

	fseek(fp, length, SEEK_CUR);

	bytedepth = tga->depth / 8;
	tga->pixels = new unsigned char[tga->width * tga->height * bytedepth];
	if (!rle) {
		if (!read(fp, tga->pixels, tga->width * tga->height * bytedepth)) {
			delete tga->pixels;
			delete tga;
			return 0;
		}
	} else {
		unsigned char type;
		unsigned runlen;
		unsigned i, pixel = 0, lastpixel = tga->width * tga->height;
		for (;;) {
			read(fp, &type, 1);
			runlen = (type & 127) + 1; // # pixels in packet
			if (type & 128) {	// rle packet
				read(fp, buf, bytedepth);
				for (i = pixel; i<pixel+runlen; i++)
					memcpy(tga->pixels + i * bytedepth, buf, bytedepth);
			} else {			// raw packet
				read(fp, tga->pixels + pixel * bytedepth, bytedepth * runlen);
			}
			// should be == lastpixel, but in case file is corrupt we see if
			// it's gone past as well
			if ((pixel += runlen) >= lastpixel)
				break;
		}
	}

	return tga;
}
