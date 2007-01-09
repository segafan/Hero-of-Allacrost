///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

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
// _DrawStillImage: draws an image given the image descriptor, using the scene light
//                   color. Helper function to DrawImage()
//-----------------------------------------------------------------------------

bool GameVideo::_DrawStillImage(const StillImage &id)
{
	// if real lighting is enabled, draw images normally since the light overlay
	// will take care of the modulation. If not, (i.e. no overlay is being used)
	// then pass the light color so the vertex colors can do the modulation
	
	if(!_usesLights && (_lightColor != Color::white))
		return _DrawStillImage(id, _lightColor);
	else
		return _DrawStillImage(id, Color::white);
}

	
//-----------------------------------------------------------------------------
// _DrawStillImage: draws an image given the image descriptor, colored using the
//                   color passed in.
//-----------------------------------------------------------------------------

bool GameVideo::_DrawStillImage(const StillImage &id, const Color &color)
{
	// Don't do anything if this image is completely transparent (invisible)
	if (color[3] == 0.0f) {
		return true;
	}

	size_t numElements = id._elements.size();
	
	float modulation = _fader.GetFadeModulation();
	Color fadeColor(modulation, modulation, modulation, 1.0f);

	float shakeX = _shakeX * (_coord_sys.GetRight() - _coord_sys.GetLeft()) / 1024.0f;
	float shakeY = _shakeY * (_coord_sys.GetTop()   - _coord_sys.GetBottom()) / 768.0f;

	float xAlignOffset = ((_xalign+1) * id._width)  * 0.5f * -_coord_sys.GetHorizontalDirection();
	float yAlignOffset = ((_yalign+1) * id._height) * 0.5f * -_coord_sys.GetVerticalDirection();

	glPushMatrix();
	MoveRelative(xAlignOffset, yAlignOffset);	

	bool skipModulation = (color == Color::white && modulation == 1.0f);
	
	
	for(uint32 iElement = 0; iElement < numElements; ++iElement)
	{		
		glPushMatrix();
		
		float xoff = (float)id._elements[iElement].x_offset;
		float yoff = (float)id._elements[iElement].y_offset;

		if(_xflip)
		{
			xoff = id._width - xoff - id._elements[iElement].width;
		}
		
		if(_yflip)
		{
			yoff = id._height - yoff - id._elements[iElement].height;
		}

		xoff += shakeX;
		yoff += shakeY;

		MoveRelative(xoff * _coord_sys.GetHorizontalDirection(), yoff * _coord_sys.GetVerticalDirection());
		
		float xscale = id._elements[iElement].width;
		float yscale = id._elements[iElement].height;
		
		if(_coord_sys.GetHorizontalDirection() < 0.0f)
			xscale = -xscale;
		if(_coord_sys.GetVerticalDirection() < 0.0f)
			yscale = -yscale;
		
		glScalef(xscale, yscale, 1.0f);

		bool success;
		
		if(skipModulation)
			success = _DrawElement(id._elements[iElement]);
		else
			success = _DrawElement(id._elements[iElement], color * fadeColor);
		
		if(!success)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: _DrawElement() failed in DrawImage()!" << endl;
			
			glPopMatrix();
			return false;
		}			
		glPopMatrix();
	}

	glPopMatrix();
	return true;
}


//-----------------------------------------------------------------------------
// _DrawElement: draws an image element. This is only used privately.
//-----------------------------------------------------------------------------

bool GameVideo::_DrawElement(const ImageElement &element) {
	Image *img = element.image;
	
	// set vertex coordinates
	float xlo,xhi,ylo,yhi;
	xlo = 0.0f;
	xhi = 1.0f;
	ylo = 0.0f;
	yhi = 1.0f;


	// set blending parameters
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

	// make calls to OpenGL to render this image
		
	if(img)
	{
		// set texture coordinates	
		float s0,s1,t0,t1;

		s0 = img->u1 + element.u1 * (img->u2 - img->u1);
		s1 = img->u1 + element.u2 * (img->u2 - img->u1);
		t0 = img->v1 + element.v1 * (img->v2 - img->v1);
		t1 = img->v1 + element.v2 * (img->v2 - img->v1);

		// swap x texture coordinates for x flipping
		if (_xflip) 
		{ 
			float temp = s0;
			s0 = s1;
			s1 = temp;
		} 

		// swap y texture coordinates for y flipping
		if (_yflip) 
		{ 
			float temp = t0;
			t0 = t1;
			t1 = temp;
		}

		// set up blending parameters
		glEnable(GL_TEXTURE_2D);
		_BindTexture(img->texture_sheet->texID);

		glBegin(GL_QUADS);

		if (element.one_color)
		{
			glColor4fv((GLfloat *)&element.color[0]);

			//glColor3f(s0, t1, 0);
			glTexCoord2f(s0, t1);
			glVertex2f(xlo, ylo); //bl

			//glColor3f(s1, t1, 0);
			glTexCoord2f(s1, t1);
			glVertex2f(xhi, ylo); //br

			//glColor3f(s1, t0, 0);
			glTexCoord2f(s1, t0);
			glVertex2f(xhi, yhi); //tr

			//glColor3f(s0, t0, 0);
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
		glBegin(GL_QUADS);
		if (element.one_color)
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


	// clean up
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

	// set vertex coordinates
	float xlo,xhi,ylo,yhi;
	xlo = 0.0f;
	xhi = 1.0f;
	ylo = 0.0f;
	yhi = 1.0f;
	
	// set blending
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

	// make calls to OpenGL to render this image
	
	if(img)
	{
		// set texture coordinates	
		float s0,s1,t0,t1;
		s0 = img->u1 + element.u1 * (img->u2 - img->u1);
		s1 = img->u1 + element.u2 * (img->u2 - img->u1);
		t0 = img->v1 + element.v1 * (img->v2 - img->v1);
		t1 = img->v1 + element.v2 * (img->v2 - img->v1);

		// swap x texture coordinates for x flipping
		if (_xflip) 
		{ 
			float temp = s0;
			s0 = s1;
			s1 = temp;
		} 

		// swap y texture coordinates for y flipping
		if (_yflip) 
		{ 
			float temp = t0;
			t0 = t1;
			t1 = temp;
		} 

		// set up texture parameters
		glEnable(GL_TEXTURE_2D);
		_BindTexture(img->texture_sheet->texID);

		glBegin(GL_QUADS);
		if(element.one_color)
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
		glBegin(GL_QUADS);
		if(element.one_color)
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

bool GameVideo::DrawHalo(const StillImage &id, float x, float y, const Color &color)
{
	PushMatrix();
	Move(x, y);

	char oldBlendMode = _blend;
	_blend = VIDEO_BLEND_ADD;
	DrawImage(id, color);
	_blend = oldBlendMode;
	PopMatrix();
	
	return true;
}


//-----------------------------------------------------------------------------
// DrawLight: draws a light at (x,y) given the light mask
//           This is draws a light at the given position and color.
//           If you want to use center alignment, call SetDrawFlags yourself 
//           with VIDEO_X_CENTER and VIDEO_Y_CENTER
//-----------------------------------------------------------------------------
bool GameVideo::DrawLight(const StillImage &id, float x, float y, const Color &color)
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
	_gui->DrawFPS(frameTime);
	_PopContext();
	
	return true;
}


//-----------------------------------------------------------------------------
// DrawImage: draws an image descriptor (can either be a static image or
//            animated image), modulating the colors by the scene lighting
//-----------------------------------------------------------------------------

bool GameVideo::DrawImage(const ImageDescriptor &id) {
	if (id._animated) {
		const AnimatedImage &anim = dynamic_cast<const AnimatedImage &>(id);
		return _DrawStillImage(*anim.GetFrame(anim.GetCurrentFrameIndex()));
	}
	else {
		return _DrawStillImage(dynamic_cast<const StillImage &>(id));
	}
}


//-----------------------------------------------------------------------------
// DrawImage: draws an image descriptor (can either be a static image or
//            animated image), modulating the colors by a custom color
//-----------------------------------------------------------------------------

bool GameVideo::DrawImage(const ImageDescriptor &id, const Color &color)
{
	if(id._animated)
	{
		const AnimatedImage &anim = dynamic_cast<const AnimatedImage &>(id);		
		return _DrawStillImage(*anim.GetFrame(anim.GetCurrentFrameIndex()), color);
	}
	else
	{
		return _DrawStillImage(dynamic_cast<const StillImage &>(id), color);
	}
}


}  // namespace hoa_video
