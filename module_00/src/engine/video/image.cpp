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
using namespace hoa_video;
using namespace hoa_utils;

namespace hoa_video 
{

//-----------------------------------------------------------------------------
// AddImage: this is the function that gives us the ability to form
//           "compound images". Call AddImage() on an existing image
//           descriptor to place a new image at the desired offsets.
//
// NOTE: it is an error to pass in negative offsets
//
// NOTE: when you create a compound image descriptor with AddImage(),
//       remember to call DeleteImage() on it when you're done. Even though
//       it's not loading any new image from disk, it increases the ref counts.
//-----------------------------------------------------------------------------
bool StillImage::AddImage(const StillImage &id, float xOffset, float yOffset, float u1, float v1, float u2, float v2)
{
	if(xOffset < 0.0f || yOffset < 0.0f)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: passed negative offsets to AddImage()!" << endl;
		}
		
		return false;
	}
	
	size_t numElements = id._elements.size();
	if(numElements == 0)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: passed in an uninitialized image descriptor to AddImage()!" << endl;
		}
		
		return false;
	}
	
	for(uint32 iElement = 0; iElement < numElements; ++iElement)
	{
		// add the new image element to our descriptor
		ImageElement elem = id._elements[iElement];
		elem.xOffset += xOffset;
		elem.yOffset += yOffset;
		elem.u1 = u1;
		elem.v1 = v1;
		elem.u2 = u2;
		elem.v2 = v2;
		
		elem.width *= (elem.u2-elem.u1);
		elem.height *= (elem.v2-elem.v1);
		
		if(elem.image)
		{
			++(elem.image->refCount);
		}
		
		_elements.push_back(elem);

		// recalculate width and height of the descriptor as a whole
		// This assumes that there are no negative offsets
		float maxX = elem.xOffset + elem.width;
		if(maxX > _width)
			_width = maxX;
			
		float maxY = elem.yOffset + elem.height;
		if(maxY > _height)
			_height = maxY;		
	}
	
	return true;	
}

//-----------------------------------------------------------------------------
// Clear: clears animated image
//-----------------------------------------------------------------------------
void AnimatedImage::Clear()
{
	_frame_index = 0;
	_frame_counter = 0;
	_frames.clear();

	SetColor(Color::white);
	
	_isStatic = false;
	_width = _height = 0.0f;
}

//-----------------------------------------------------------------------------
// Update: updates the internal frame counter for an animation
//-----------------------------------------------------------------------------
void AnimatedImage::Update()
{
	int32 numFrames = static_cast<int32>(_frames.size());
	if(numFrames <= 1)
		return;
		
	GameVideo *video = GameVideo::SingletonGetReference();
	int32 frameChange = video->GetFrameChange();
	
	_frame_counter += frameChange;
	
	while(_frame_counter >= _frames[_frame_index]._frame_time)
	{
		frameChange = _frame_counter - _frames[_frame_index]._frame_time;
		_frame_index = (_frame_index + 1) % numFrames;
		_frame_counter = frameChange;
	}
}
	
//-----------------------------------------------------------------------------
// SetFrameIndex: sets the current frame of the animation to frame_index
//-----------------------------------------------------------------------------
void AnimatedImage::SetFrameIndex(int32 frame_index)
{
	_frame_index    = frame_index;
	_frame_counter  = 0;
}
	

//-----------------------------------------------------------------------------
// AddFrame: add a new frame to the animation, passing in only the filename
//           and frame count (how long the frame should last)
//-----------------------------------------------------------------------------
bool AnimatedImage::AddFrame(const std::string &frame, int frame_time)
{
	StillImage img;
	img.SetFilename(frame);	
	img.SetDimensions(_width, _height);
	img.SetVertexColors(_color[0], _color[1], _color[2], _color[3]);
	img.SetStatic(_isStatic);
	
	AnimationFrame animFrame;
	animFrame._frame_time = frame_time;
	animFrame._image = img;	
	_frames.push_back(animFrame);

	return true;
}

//-----------------------------------------------------------------------------
// AddFrame: add a new frame to the animation, passing in a static image and
//           a frame count (how long the frame should last)
//-----------------------------------------------------------------------------
bool AnimatedImage::AddFrame(const StillImage &frame, int frame_time)
{
	AnimationFrame animFrame;
	animFrame._frame_time = frame_time;
	animFrame._image = frame;
	
	// check if the static image which was passed actually has been loaded yet
	// if it has been loaded, then we have to increment the reference count
		
	int32 numElements = static_cast<int32>(animFrame._image._elements.size());
	if(numElements)
	{
		for(int j = 0; j < numElements; ++j)
		{
			++(animFrame._image._elements[j].image->refCount);
		}		
	}
	
	_frames.push_back(animFrame);
	return true;
}
	
//-----------------------------------------------------------------------------
// GetWidth: returns the width of the 1st frame of the animation
//-----------------------------------------------------------------------------
float AnimatedImage::GetWidth() const
{
	return _frames[0]._image.GetWidth();
}


//-----------------------------------------------------------------------------
// GetHeight: returns the height of the 1st frame of the animation
//-----------------------------------------------------------------------------
float AnimatedImage::GetHeight() const
{
	return _frames[0]._image.GetHeight();
}

	
//-----------------------------------------------------------------------------
// GetFrame: returns pointer to the indexth frame of animation as a static image.
//           This function is somewhat dangerous since it lets you mess around
//           with the internals of the animation, so only use it if it's
//           necessary.
//-----------------------------------------------------------------------------
StillImage *AnimatedImage::GetFrame(int32 index) const
{
	return const_cast<StillImage *>(&(_frames[index]._image));
}


//-----------------------------------------------------------------------------
// Load: loads the image
//-----------------------------------------------------------------------------

bool ImageDescriptor::Load()
{
	return VideoManager->LoadImage(*this);
}


//-----------------------------------------------------------------------------
// Draw: draws the image
//-----------------------------------------------------------------------------

bool ImageDescriptor::Draw()
{
	return VideoManager->DrawImage(*this);
}


//-----------------------------------------------------------------------------
// SetWidth: sets all frames to be a certain width
//-----------------------------------------------------------------------------

void AnimatedImage::SetWidth(float width)
{
	_width = width;
	
	// update frames	
	int32 n_frames = GetNumFrames();	
	for(int32 j = 0; j < n_frames; ++j)
	{
		StillImage *img = GetFrame(j);
		img->SetWidth(width);
	}
}


//-----------------------------------------------------------------------------
// SetHeight: sets all frames to be a certain height
//-----------------------------------------------------------------------------

void AnimatedImage::SetHeight(float height)
{
	_height = height;

	// update frames	
	int32 n_frames = GetNumFrames();	
	for(int32 j = 0; j < n_frames; ++j)
	{
		StillImage *img = GetFrame(j);
		img->SetHeight(height);
	}
}


//-----------------------------------------------------------------------------
// SetDimensions: sets all frames to be a certain width and height
//-----------------------------------------------------------------------------

void AnimatedImage::SetDimensions(float width, float height)
{
	SetWidth(width);
	SetHeight(height);

	// update frames	
	int32 n_frames = GetNumFrames();	
	for(int32 j = 0; j < n_frames; ++j)
	{
		StillImage *img = GetFrame(j);
		img->SetDimensions(width, height);
	}
}


//-----------------------------------------------------------------------------
// SetColor: sets all frames to be a certain color
//-----------------------------------------------------------------------------
void AnimatedImage::SetColor(const Color &color)
{
	SetVertexColors(color, color, color, color);
	int32 n_frames = GetNumFrames();	

	// update frames	
	for(int32 j = 0; j < n_frames; ++j)
	{
		StillImage *img = GetFrame(j);
		img->SetColor(color);
	}
}


//-----------------------------------------------------------------------------
// SetVertexColors: sets all frames to have the vertex colors specified
//-----------------------------------------------------------------------------
void AnimatedImage::SetVertexColors (const Color &tl, const Color &tr, const Color &bl, const Color &br)
{
	_color[0] = tl;
	_color[1] = tr;
	_color[2] = bl;
	_color[3] = br;

	// update frames	
	int32 n_frames = GetNumFrames();	
	for(int32 j = 0; j < n_frames; ++j)
	{
		StillImage *img = GetFrame(j);
		img->SetVertexColors(tl, tr, bl, br);
	}
}


//-----------------------------------------------------------------------------
// SetStatic: sets all frames to be loaded statically
//            Note: if the frames are already loaded, it doesn't bother to try
//            to unload them and then load them again statically.
//-----------------------------------------------------------------------------
void AnimatedImage::SetStatic(bool isStatic)
{
	_isStatic = isStatic;
}

//-----------------------------------------------------------------------------
// EnableGrayScale: Turns the animated image gray
//-----------------------------------------------------------------------------
void AnimatedImage::EnableGrayScale()
{
	// turn all frames gray
	int32 n_frames = GetNumFrames();	
	for(int32 j = 0; j < n_frames; ++j)
	{
		StillImage *img = GetFrame(j);
		img->EnableGrayScale();
	}
}

//-----------------------------------------------------------------------------
// DisableGrayScale: Turns the animated image gray
//-----------------------------------------------------------------------------
void AnimatedImage::DisableGrayScale()
{
	// turn all frames gray
	int32 n_frames = GetNumFrames();	
	for(int32 j = 0; j < n_frames; ++j)
	{
		StillImage *img = GetFrame(j);
		img->DisableGrayScale();
	}
}


}  // namespace hoa_video
