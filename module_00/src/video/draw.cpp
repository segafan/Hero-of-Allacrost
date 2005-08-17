#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "coord_sys.h"
#include "gui.h"

using namespace std;
using namespace hoa_video::local_video;

namespace hoa_video 
{

//-----------------------------------------------------------------------------
// DrawImage: draws an image given the image descriptor, using the scene light
//            color
//-----------------------------------------------------------------------------

bool GameVideo::DrawImage(const ImageDescriptor &id)
{
	// if real lighting is enabled, draw images normally since the light overlay
	// will take care of the modulation. If not, (i.e. no overlay is being used)
	// then pass the light color so the vertex colors can do the modulation
	if(_usesLights)
		return DrawImage(id, Color(1.0f, 1.0f, 1.0f, 1.0f));
	else
		return DrawImage(id, _lightColor);
}

	
//-----------------------------------------------------------------------------
// DrawImage: draws an image given the image descriptor, colored using the
//            color passed in.
//-----------------------------------------------------------------------------

bool GameVideo::DrawImage(const ImageDescriptor &id, const Color &color)
{
	size_t numElements = id._elements.size();
	
	for(uint iElement = 0; iElement < numElements; ++iElement)
	{		
		glPushMatrix();
		MoveRel((float)id._elements[iElement].xOffset, (float)id._elements[iElement].yOffset);
		
		// include screen shaking effects
		MoveRel(_shakeX * (_coordSys._right - _coordSys._left) / 1024.0f, 
		        _shakeY * (_coordSys._top   - _coordSys._bottom) / 768.0f);  
		
		if(!DrawElement(
			id._elements[iElement].image, 
			id._elements[iElement].width, 
			id._elements[iElement].height,
			id._elements[iElement].color * color  // include color passed in
		))
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: DrawElement() failed in DrawImage()!" << endl;
			return false;
		}
		glPopMatrix();
	}

	return true;
}


//-----------------------------------------------------------------------------
// DrawElement: draws an image element. This is only used privately.
//-----------------------------------------------------------------------------

bool GameVideo::DrawElement(const Image *const img, float w, float h, const Color &c)
{
	Color color = c;
	
	if(color.color[3] == 0.0f)
	{
		// do nothing, alpha is 0
		return true;
	}

	color.color[0] *= _fader.GetFadeModulation();
	color.color[1] *= _fader.GetFadeModulation();
	color.color[2] *= _fader.GetFadeModulation();
	
	float s0,s1,t0,t1;
	float xoff,yoff;
	float xlo,xhi,ylo,yhi;

	CoordSys &cs = _coordSys;
	
	if(img)
	{
		s0 = img->u1;
		s1 = img->u2;
		t0 = img->v1;
		t1 = img->v2;
	}

	if (_xflip) 
	{ 
		s0=1-s0; 
		s1=1-s1; 
	} 

	if (_yflip) 
	{ 
		t0=1-t0;
		t1=1-t1;
	} 

	if (_xflip)
	{
		xlo = (float) w;
		xhi = 0.0f;
	}
	else
	{
		xlo = 0.0f;
		xhi = (float) w;
	}
	if (cs._left > cs._right) 
	{ 
		xlo = (float) -xlo; 
		xhi = (float) -xhi; 
	}

	if (_yflip)
	{
		ylo=(float) h;
		yhi=0.0f;
	}
	else
	{
		ylo=0.0f;
		yhi=(float) h;
	}
	
	if (cs._bottom > cs._top) 
	{ 
		ylo=(float) -ylo; 
		yhi=(float) -yhi; 
	}

	xoff = ((_xalign+1) * w) * .5f * (cs._left < cs._right ? -1 : +1);
	yoff = ((_yalign+1) * h) * .5f * (cs._bottom < cs._top ? -1 : +1);

	if(img)
	{
		glEnable(GL_TEXTURE_2D);
		BindTexture(img->texSheet->texID);
	}	
	
	if (_blend || color.color[3] < 1.0f) 
	{
		glEnable(GL_BLEND);
		if (_blend == 1)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glPushMatrix();
	
	glTranslatef(xoff, yoff, 0);
	glBegin(GL_QUADS);
		glColor4fv(&(color.color[0]));
		
		if(img)
			glTexCoord2f(s0, t1);
		
		glVertex2f(xlo, ylo); //bl

		if(img)
			glTexCoord2f(s1, t1);

		glVertex2f(xhi, ylo); //br

		if(img)
			glTexCoord2f(s1, t0);

		glVertex2f(xhi, yhi);//tr

		if(img)
			glTexCoord2f(s0, t0);

		glVertex2f(xlo, yhi);//tl

	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	if (_blend)
		glDisable(GL_BLEND);	
		
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() returned true in DrawElement()!" << endl;
		return false;
	}		
		
	return true;
}


//-----------------------------------------------------------------------------
// DrawHalo: draws a halo at (x,y) given the halo image
//           This is draws a halo image at the given position and color.
//           If you want to use center alignment, call SetDrawFlags yourself 
//           with VIDEO_X_CENTER and VIDEO_Y_CENTER
//-----------------------------------------------------------------------------

bool GameVideo::DrawHalo
(
	const ImageDescriptor &id, 
	float x, 
	float y, 
	const Color &color
)
{
	Move(x, y);

	int oldBlendMode = _blend;
	_blend = VIDEO_BLEND_ADD;
	DrawImage(id, color);
	_blend = oldBlendMode;
	
	return true;
}


//-----------------------------------------------------------------------------
// DrawLight: draws a light at (x,y) given the light mask
//           This is draws a light at the given position and color.
//           If you want to use center alignment, call SetDrawFlags yourself 
//           with VIDEO_X_CENTER and VIDEO_Y_CENTER
//-----------------------------------------------------------------------------

bool GameVideo::DrawLight
(
	const ImageDescriptor &id, 
	float x, 
	float y, 
	const Color &color
)
{
	if(!_usesLights)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called DrawLight() even though real lighting was not enabled!" << endl;
		return false;
	}

	return DrawHalo(id, x, y, color);
}


//-----------------------------------------------------------------------------
// DrawFPS: draws current frames per second
//-----------------------------------------------------------------------------

bool GameVideo::DrawFPS(int frameTime)
{
	return _gui->DrawFPS(frameTime);
}


}  // namespace hoa_video
