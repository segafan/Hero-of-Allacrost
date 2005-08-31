#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "gui.h"

using namespace std;
using namespace hoa_video::private_video;

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
	if(!_usesLights)
		return DrawImage(id, _lightColor);
	else
		return DrawImage(id, Color(1.0f, 1.0f, 1.0f, 1.0f));
}

	
//-----------------------------------------------------------------------------
// DrawImage: draws an image given the image descriptor, colored using the
//            color passed in.
//-----------------------------------------------------------------------------

bool GameVideo::DrawImage(const ImageDescriptor &id, const Color &color)
{
	// don't do anything if this image is completely transparent (invisible)
	if(color[3] == 0.0f)
		return true;
		
	size_t numElements = id._elements.size();
	
	float modulation = _fader.GetFadeModulation();
	Color fadeColor(modulation, modulation, modulation, 1.0f);

	float oldxoff = 0.0f, oldyoff = 0.0f;

	float shakeX = _shakeX * (_coordSys._right - _coordSys._left) / 1024.0f;
	float shakeY = _shakeY * (_coordSys._top   - _coordSys._bottom) / 768.0f;

	if(color == Color::white && modulation == 1.0f)
	{
		// call draw element with no color parameter so we don't
		// waste time doing modulation
		
		for(uint32 iElement = 0; iElement < numElements; ++iElement)
		{		
			float xoff = (float)id._elements[iElement].xOffset + shakeX;
			float yoff = (float)id._elements[iElement].yOffset + shakeY;

			MoveRelative(xoff - oldxoff, yoff - oldyoff);
			
			if(!_DrawElement(id._elements[iElement]))
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: _DrawElement() failed in DrawImage()!" << endl;
				
				MoveRelative(-xoff, -yoff);
				return false;
			}
			
			oldxoff = xoff;
			oldyoff = yoff;
		}
	}
	else
	{
		// call draw element function that takes a color parameter
		for(uint32 iElement = 0; iElement < numElements; ++iElement)
		{		
			float xoff = (float)id._elements[iElement].xOffset + shakeX;
			float yoff = (float)id._elements[iElement].yOffset + shakeY;

			MoveRelative(xoff - oldxoff, yoff - oldyoff);
			
			if(!_DrawElement(id._elements[iElement], color * fadeColor))
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: _DrawElement() failed in DrawImage()!" << endl;
				
				MoveRelative(-xoff, -yoff);
				return false;
			}
			
			oldxoff = xoff;
			oldyoff = yoff;
		}
	}

	MoveRelative(-oldxoff, -oldyoff);

	return true;
}


//-----------------------------------------------------------------------------
// _DrawElement: draws an image element. This is only used privately.
//-----------------------------------------------------------------------------

bool GameVideo::_DrawElement
(
	const ImageElement &element
)
{	
	Image *img = element.image;
	float h    = element.height;
	float w    = element.width;
	
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
		xlo = (float) w;
		xhi = 0.0f;
	} 
	else
	{
		xlo = 0.0f;
		xhi = (float) w;
	}

	if (_yflip) 
	{ 
		t0=1-t0;
		t1=1-t1;
		ylo=(float) h;
		yhi=0.0f;
	} 
	else
	{
		ylo=0.0f;
		yhi=(float) h;
	}

	
	if (cs._left > cs._right) 
	{ 
		xlo = (float) -xlo; 
		xhi = (float) -xhi; 
	}
	if (cs._bottom > cs._top) 
	{ 
		ylo=(float) -ylo; 
		yhi=(float) -yhi; 
	}

	xoff = ((_xalign+1) * w) * .5f * -cs._rightDir;
	yoff = ((_yalign+1) * h) * .5f * -cs._upDir;

	if(img)
	{
		glEnable(GL_TEXTURE_2D);
		_BindTexture(img->texSheet->texID);
	}	
	
	if(_blend)
	{
		glEnable(GL_BLEND);
		if (_blend == 1)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive
	}
	else
	{
		// if blending isn't in the draw flags, don't use blending UNLESS
		// the given image element has translucent vertex colors
		if(!element.blend)
			glDisable(GL_BLEND);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glPushMatrix();
	
	glTranslatef(xoff, yoff, 0);
	glBegin(GL_QUADS);
	
	if(img)
	{
		if(element.oneColor)
		{
			glColor4fv((GLfloat *)&element.color[0]);
			glTexCoord2f(s0, t1);
			glVertex2f(xlo, ylo); //bl
			glTexCoord2f(s1, t1);
			glVertex2f(xhi, ylo); //br
			glTexCoord2f(s1, t0);
			glVertex2f(xhi, yhi); //tr
			glTexCoord2f(s0, t0);
			glVertex2f(xlo, yhi); //tl
		}
		else
		{
			glColor4fv((GLfloat *)&element.color[0]);
			glTexCoord2f(s0, t1);
			glVertex2f(xlo, ylo); //bl
			glColor4fv((GLfloat *)&element.color[1]);
			glTexCoord2f(s1, t1);
			glVertex2f(xhi, ylo); //br
			glColor4fv((GLfloat *)&element.color[2]);
			glTexCoord2f(s1, t0);
			glVertex2f(xhi, yhi); //tr
			glColor4fv((GLfloat *)&element.color[3]);
			glTexCoord2f(s0, t0);
			glVertex2f(xlo, yhi); //tl			
		}		
	}
	else
	{
		if(element.oneColor)
		{
			glColor4fv((GLfloat *)&element.color[0]);
			glVertex2f(xlo, ylo); //bl
			glVertex2f(xhi, ylo); //br
			glVertex2f(xhi, yhi); //tr
			glVertex2f(xlo, yhi); //tl			
		}
		else
		{
			glColor4fv((GLfloat *)&element.color[0]);
			glVertex2f(xlo, ylo); //bl
			glColor4fv((GLfloat *)&element.color[1]);
			glVertex2f(xhi, ylo); //br
			glColor4fv((GLfloat *)&element.color[2]);
			glVertex2f(xhi, yhi); //tr
			glColor4fv((GLfloat *)&element.color[3]);
			glVertex2f(xlo, yhi); //tl			
		}
	}

	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	if (_blend)
		glDisable(GL_BLEND);
		
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() returned true in _DrawElement()!" << endl;
		return false;
	}		
		
	return true;
}


//-----------------------------------------------------------------------------
// _DrawElement: draws an image element. This is only used privately.
//
// Note: this version includes modulation set by the image descriptor and
//       screen fader
//-----------------------------------------------------------------------------

bool GameVideo::_DrawElement
(
	const ImageElement &element,
	const Color &modulateColor
)
{	
	Image *img = element.image;
	float h    = element.height;
	float w    = element.width;
	
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
		xlo = (float) w;
		xhi = 0.0f;
	} 
	else
	{
		xlo = 0.0f;
		xhi = (float) w;
	}

	if (_yflip) 
	{ 
		t0=1-t0;
		t1=1-t1;
		ylo=(float) h;
		yhi=0.0f;
	} 
	else
	{
		ylo=0.0f;
		yhi=(float) h;
	}

	
	if (cs._left > cs._right) 
	{ 
		xlo = (float) -xlo; 
		xhi = (float) -xhi; 
	}
	if (cs._bottom > cs._top) 
	{ 
		ylo=(float) -ylo; 
		yhi=(float) -yhi; 
	}

	xoff = ((_xalign+1) * w) * .5f * -cs._rightDir;
	yoff = ((_yalign+1) * h) * .5f * -cs._upDir;

	if(img)
	{
		glEnable(GL_TEXTURE_2D);
		_BindTexture(img->texSheet->texID);
	}	
	
	if(_blend)
	{
		glEnable(GL_BLEND);
		if (_blend == 1)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive
	}
	else
	{
		// if blending isn't in the draw flags, don't use blending UNLESS
		// the given image element has translucent vertex colors
		if(!element.blend)
			glDisable(GL_BLEND);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glPushMatrix();
	
	glTranslatef(xoff, yoff, 0);
	glBegin(GL_QUADS);
	
	if(img)
	{
		if(element.oneColor)
		{
			Color color = element.color[0] * modulateColor;
			glColor4fv((GLfloat *)&color);
			glTexCoord2f(s0, t1);
			glVertex2f(xlo, ylo); //bl
			glTexCoord2f(s1, t1);
			glVertex2f(xhi, ylo); //br
			glTexCoord2f(s1, t0);
			glVertex2f(xhi, yhi); //tr
			glTexCoord2f(s0, t0);
			glVertex2f(xlo, yhi); //tl
		}
		else
		{
			Color color[4];
			color[0] = modulateColor * element.color[0];
			color[1] = modulateColor * element.color[1];
			color[2] = modulateColor * element.color[2];
			color[3] = modulateColor * element.color[3];
			
			glColor4fv((GLfloat *)&color[0]);
			glTexCoord2f(s0, t1);
			glVertex2f(xlo, ylo); //bl
			glColor4fv((GLfloat *)&color[1]);
			glTexCoord2f(s1, t1);
			glVertex2f(xhi, ylo); //br
			glColor4fv((GLfloat *)&color[2]);
			glTexCoord2f(s1, t0);
			glVertex2f(xhi, yhi); //tr
			glColor4fv((GLfloat *)&color[3]);
			glTexCoord2f(s0, t0);
			glVertex2f(xlo, yhi); //tl			
		}		
	}
	else
	{
		if(element.oneColor)
		{
			Color color = element.color[0] * modulateColor;			
			glColor4fv((GLfloat *)&color);
			glVertex2f(xlo, ylo); //bl
			glVertex2f(xhi, ylo); //br
			glVertex2f(xhi, yhi); //tr
			glVertex2f(xlo, yhi); //tl			
		}
		else
		{
			Color color[4];
			color[0] = modulateColor * element.color[0];
			color[1] = modulateColor * element.color[1];
			color[2] = modulateColor * element.color[2];
			color[3] = modulateColor * element.color[3];

			glColor4fv((GLfloat *)&color[0]);
			glVertex2f(xlo, ylo); //bl
			glColor4fv((GLfloat *)&color[1]);
			glVertex2f(xhi, ylo); //br
			glColor4fv((GLfloat *)&color[2]);
			glVertex2f(xhi, yhi); //tr
			glColor4fv((GLfloat *)&color[3]);
			glVertex2f(xlo, yhi); //tl			
		}
	}

	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	if (_blend)
		glDisable(GL_BLEND);
		
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() returned true in _DrawElement()!" << endl;
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
	_PushContext();
	Move(x, y);

	int32 oldBlendMode = _blend;
	_blend = VIDEO_BLEND_ADD;
	DrawImage(id, color);
	_blend = oldBlendMode;	
	_PopContext();
	
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

bool GameVideo::DrawFPS(int32 frameTime)
{
	_PushContext();
	bool success = _gui->DrawFPS(frameTime);
	_PopContext();
	
	return success;
}


}  // namespace hoa_video
