#ifndef _TGAREAD_H_
#define _TGAREAD_H_

#include <stdio.h>

struct TGAFile {
	enum Type {
		RGB,
		RGBA,
		GRAYSCALE
	};

	unsigned char  *pixels;
	unsigned char	depth;
	unsigned		alpha;
	unsigned short	width;
	unsigned short	height;
	Type			type;

	inline ~TGAFile() { delete pixels; }
};

TGAFile *tgaread(FILE *fp);

#endif // !_TGAREAD_HXX_
