///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
	
	if(!_uses_lights && (_light_color != Color::white))
		return _DrawStillImage(id, _light_color);
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

	size_t num_elements = id._elements.size();
	
	float modulation = _fader.GetFadeModulation();
	Color fade_color(modulation, modulation, modulation, 1.0f);

	float x_shake = _x_shake * (_coord_sys.GetRight() - _coord_sys.GetLeft()) / 1024.0f;
	float y_shake = _y_shake * (_coord_sys.GetTop()   - _coord_sys.GetBottom()) / 768.0f;

	float x_align_offset = ((_x_align+1) * id._width)  * 0.5f * -_coord_sys.GetHorizontalDirection();
	float y_align_offset = ((_y_align+1) * id._height) * 0.5f * -_coord_sys.GetVerticalDirection();

	glPushMatrix();
	MoveRelative(x_align_offset, y_align_offset);	

	bool skip_modulation = (color == Color::white && modulation == 1.0f);

	// If we're modulating, calculate modulation color now
	if (!skip_modulation)
		fade_color = color * fade_color;
	
	Color color_modulated[4];
	
	for(uint32 iElement = 0; iElement < num_elements; ++iElement)
	{		
		glPushMatrix();
		
		float x_off = (float)id._elements[iElement].x_offset;
		float y_off = (float)id._elements[iElement].y_offset;

		if(_x_flip)
		{
			x_off = id._width - x_off - id._elements[iElement].width;
		}
		
		if(_y_flip)
		{
			y_off = id._height - y_off - id._elements[iElement].height;
		}

		x_off += x_shake;
		y_off += y_shake;

//		float x__ = 0.5f * abs(_coord_sys.GetRight()-_coord_sys.GetLeft()) / (float)_width;
//		float y__ = 0.5f * abs(_coord_sys.GetTop()-_coord_sys.GetBottom()) / (float)_height;

//		MoveRelative(x__ + x_off * _coord_sys.GetHorizontalDirection(), y__ + y_off * _coord_sys.GetVerticalDirection());

		MoveRelative(x_off * _coord_sys.GetHorizontalDirection(), y_off * _coord_sys.GetVerticalDirection());
		
		float x_scale = id._elements[iElement].width;
		float y_scale = id._elements[iElement].height;
		
		if(_coord_sys.GetHorizontalDirection() < 0.0f)
			x_scale = -x_scale;
		if(_coord_sys.GetVerticalDirection() < 0.0f)
			y_scale = -y_scale;
		
		glScalef(x_scale, y_scale, 1.0f);

		bool success;
		
		if(skip_modulation)
			success = _DrawElement(id._elements[iElement], id._elements[iElement].color);
		else
		{
			color_modulated[0] = id._elements[iElement].color[0] * fade_color;
			color_modulated[1] = id._elements[iElement].color[1] * fade_color;
			color_modulated[2] = id._elements[iElement].color[2] * fade_color;
			color_modulated[3] = id._elements[iElement].color[3] * fade_color;
			success = _DrawElement(id._elements[iElement], color_modulated);
		}
		
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

bool GameVideo::_DrawElement(const ImageElement &element, const Color *color_array) {
	Image *img = element.image;
	
	// Set vertex coordinates
	static const float xlo = 0.0f;
	static const float xhi = 1.0f;
	static const float ylo = 0.0f;
	static const float yhi = 1.0f;

	// Array of the four vertexes defined on
	// the 2D plane for glDrawArrays().
	static const float vert_coords[] = 
	{ 
		xlo, ylo,
		xhi, ylo,
		xhi, yhi,
		xlo, yhi,
	};

	// Set blending parameters
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

	// Constant function local values for array rendering
	static const int num_vertexes      = 4;
	static const int coords_per_vertex = 2;

		
	// If we have an image, setup texture coordinates
	// and the texture coordinate array for glDrawArrays().
	if (img)
	{
		// set texture coordinates	
		float s0,s1,t0,t1;

		s0 = img->u1 + element.u1 * (img->u2 - img->u1);
		s1 = img->u1 + element.u2 * (img->u2 - img->u1);
		t0 = img->v1 + element.v1 * (img->v2 - img->v1);
		t1 = img->v1 + element.v2 * (img->v2 - img->v1);


		// swap x texture coordinates for x flipping
		if (_x_flip) 
		{ 
			float temp = s0;
			s0 = s1;
			s1 = temp;
		} 

		// swap y texture coordinates for y flipping
		if (_y_flip) 
		{ 
			float temp = t0;
			t0 = t1;
			t1 = temp;
		}

		// Place texture coordinates in a 4x2 array
		// mirroring the structure of the vertex
		// array for use in glDrawArrays().
		float tex_coords[] = { 
			s0, t1,
			s1, t1,
			s1, t0,
			s0, t0,
		};

		// Enable texturing and bind texture
		glEnable(GL_TEXTURE_2D);
		_BindTexture(img->texture_sheet->tex_ID);

		// Enable texture coordinate array
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// Setup texture coordinate array
		glTexCoordPointer(coords_per_vertex, GL_FLOAT, 0, tex_coords);
		if (element.one_color)
		{
			glColor4fv((GLfloat *)&color_array[0]);
		}
		else
		{
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, 0, (GLfloat *)color_array);
		}
		// Always use a vertex array
		glEnableClientState(GL_VERTEX_ARRAY);

		// Setup the vertex array pointer
		glVertexPointer(coords_per_vertex, GL_FLOAT, 0, vert_coords);

		// Draw the object using the array pointers we've just setup
		glDrawArrays(GL_QUADS, 0, num_vertexes);
	}
	else
	{
		// Use a single call to glColor for one_color
		// images, or a setup a gl color array for multiple.
		if (element.one_color)
		{
			glColor4fv((GLfloat *)&color_array[0]);
			glDisableClientState(GL_COLOR_ARRAY);
		}
		else
		{
			glEnableClientState(GL_COLOR_ARRAY);
			//glColorPointer(4, GL_FLOAT, 0, (GLfloat *)element.color);
			glColorPointer(4, GL_FLOAT, 0, (GLfloat *)color_array);
		}
		// Always use a vertex array
		glEnableClientState(GL_VERTEX_ARRAY);

		// Setup the vertex array pointer
		glVertexPointer(coords_per_vertex, GL_FLOAT, 0, vert_coords);

		// Disable the texture array as we're not texturing
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		// Draw the object using the array pointers we've just setup
		glDrawArrays(GL_QUADS, 0, num_vertexes);
	}

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
	if(!_uses_lights)
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

void GameVideo::DrawFPS(uint32 frame_time)
{
	_PushContext();
	GUIManager->DrawFPS(frame_time);
	_PopContext();

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
