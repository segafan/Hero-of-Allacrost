#include "images.h"
#include <cassert>
#include <string>
#include <GL/gl.h>
#include <IL/ilut.h>

using namespace std;

namespace hoa_video {

	//flags definition
	/**************************************************************************
	 * returns lowest integer y such that 2^y >= x, used for making textures an
	 * appropriate size, fell free to move to utils if you want but i didnt
	 * feel it was worth it
	 **************************************************************************/
	static unsigned int p2(unsigned int x) {
#if 0
		int pos=0, count=0;
		for (int p=0;p<32;p++) {
			if ((1<<p) & x) {
				pos=p;
				count++;
			}
		}
		return (count==1)?1<<pos:1<<(pos+1);
#else
		/* Least power of 2 greater than or equal to x, branch free.
		 * I won't tell you how it works though ;-) */
		assert(x != 0);
		x -= 1;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		return x + 1;
#endif
	}

	void ImagesVS::LoadTexture(const ImageDescriptor &id) {
		//load image -- check return value?
		id.texid = ilutGLLoadImage((char *)id.filename.c_str());
		glBindTexture(GL_TEXTURE_2D, id.texid);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	void ImagesVS::DrawImage(int i, bool blend, int xalign, int yalign,
							bool xflip, bool yflip) {
		if (i<0 || i>=(int)images.size())
			return;

		ImageDescriptor &img = images[i];
		int s0,s1,t0,t1;
		float xoff,yoff;
		if (xflip) { s0=1, s1=0; } else { s0=0; s1=1; }
		if (yflip) { t0=1, t1=0; } else { t0=0, t1=1; }
		xoff = -0.5f * ((xalign+1) * img.width);
		yoff = -0.5f * ((yalign+1) * img.height);

		glPushAttrib(GL_TEXTURE_BIT);
		glPushAttrib(GL_ENABLE_BIT);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, img.texid);
		(blend ? glEnable : glDisable)(GL_BLEND);

		glPushMatrix();
		glTranslatef(xoff, yoff, 0);

		glBegin(GL_QUADS);
		glTexCoord2i(s0, t1);
		glVertex2f(0, 0); //bl

		glTexCoord2i(s1, t1);
		glVertex2f(img.width, 0); //br

		glTexCoord2i(s1, t0);
		glVertex2f(img.width, img.height);//tr

		glTexCoord2i(s0, t0);
		glVertex2f(0, img.height);//tl
		glEnd();
		glPopMatrix();

		glPopAttrib();
		glPopAttrib();
	}

//static list of all ImageVS - needed to reload destroyed textures
list<ImagesVS *> ImagesVS::ivs;

void ImagesVS::Allocate(bool allocate) {
	list<ImagesVS *>::iterator i;
	for (i=ivs.begin();i!=ivs.end();i++) {
		if (!(*i)->inVideo) continue; //nothing to do
		if (!allocate) {
			(*i)->UnloadVideo();
			//naughty but only i will call this
			(*i)->inVideo=true;
		} else {
			//naughty but only i will call this
			(*i)->inVideo=false;
			(*i)->LoadVideo();
		}
	}
}

ImagesVS::ImagesVS() {
	inVideo=false;
	ivs.push_back(this);
}

ImagesVS::~ImagesVS() {
	UnloadVideo();
	ivs.remove(this);
}

int ImagesVS::LoadImage(const ImageDescriptor &id) {
	images.push_back(id);

	if (inVideo)
		LoadTexture(id);
	return images.size()-1;
}


void ImagesVS::LoadVideo() {
	if (inVideo) return;
	for (unsigned i=0;i<images.size();i++) {
		if (images[i].texid!=0)
			glDeleteTextures(1, &images[i].texid);
		LoadTexture(images[i]);
	}
	inVideo=true;
}

void ImagesVS::UnloadVideo() {
	if (!inVideo) return;
	for (unsigned i=0;i<images.size();i++) {
		if (images[i].texid != 0) {
			glDeleteTextures(1,&images[i].texid);
			images[i].texid=0;
		}
	}
}

void ImagesVS::Select() {
	if (!inVideo) {
		LoadVideo();
		inVideo=true;
	}
}

void ImagesVS::Deselect() {
}

}
