///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    image.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for image classes
 *
 * This file contains several classes:
 *
 *   -Image: represents a sub-rectangle within a texture sheet, in other words,
 *           a physical image in memory. Used internally by ImageElement.
 *
 *   -ImageElement: contains a pointer to an Image, plus properties like
 *                  width, height, and color. Used internally by StillImage
 *
 *   -ImageDescriptor: abstract base class for images. The only time the API
 *                     user should ever need to use this class is if they want
 *                     a mixed array containing both static and animated images
 *
 *   -StillImage: an image, non-animated. This is what the user will be dealing
 *                 with most of the time.
 *
 *   -AnimatedImage: an animated image, containing multiple frames
 *
 *   -AnimationFrame: a single frame of animation. Consists of a static image,
 *                    plus a number that tells how long to play the frame for.
 *****************************************************************************/ 


#ifndef __IMAGE_HEADER__
#define __IMAGE_HEADER__

#include "utils.h"
#include "color.h"
#include "tex_mgmt.h"

namespace hoa_video
{

namespace private_video
{

class ParticleSystem;

/*!***************************************************************************
 *  \brief stores information that is passed between image loaders and 
 (		   OpenGL texture creation
 *****************************************************************************/


class ImageLoadInfo
{
public:
	int32 width;
	int32 height;

	void * pixels;
};

/*!***************************************************************************
 *  \brief represents a single image. Internally it's a reference to a
 *         sub-rect within a texture sheet.
 *****************************************************************************/

class Image
{
public:
	Image(const std::string &fname, int32 w, int32 h, bool grayscale) 
	: filename(fname)
	{ 
		x = 0;
		y = 0;
		width    = w;
		height   = h;
		refCount = 0;
		u1 = 0.0f;
		u2 = 1.0f;
		v1 = 0.0f;
		v2 = 1.0f;
		this->grayscale = grayscale;
	}

	Image(TexSheet *sheet, const std::string &fname, int32 x_, int32 y_, int32 w, int32 h, float u1_, float v1_, float u2_, float v2_) 
	: filename(fname)
	{
		texSheet = sheet;
		x = x_;
		y = y_;
		width = w;
		height = h;
		u1 = u1_;
		u2 = u2_;
		v1 = v1_;
		v2 = v2_;
		refCount = 0;
	}

	Image & operator = (Image & rhs)
	{
		int n = 0;
		return *this;
	}

	TexSheet *texSheet;     //! texture sheet using this image
	std::string filename;   //! stored for every image in case it needs to be reloaded
	
	int32 x, y;             //! location of image within the sheet
	int32 width, height;    //! width and height, in pixels

	float u1, v1;           //! also store the actual uv coords. This is a bit
	float u2, v2;           //! redundant, but saves floating point calculations
	
	int32 refCount;         //! keep track of when this image can be deleted

	bool grayscale;			// track whether this image is grayscale or not
};


/*!***************************************************************************
 *  \brief Represents a single image within an image descriptor. Compound
 *         images are formed of multiple ImageElements.
 *****************************************************************************/

class ImageElement
{
public:
	ImageElement
	(
		Image *image_, 
		float xOffset_, 
		float yOffset_, 
		float width_, 
		float height_,
		float u1_,
		float v1_,
		float u2_,
		float v2_,
		Color color_[4]
	)
	{
		image    = image_;
		xOffset  = xOffset_;
		yOffset  = yOffset_;
		width    = width_;
		height   = height_;
		u1       = u1_;
		v1       = v1_;
		u2       = u2_;
		v2       = v2_;
		
		color[0] = color_[0];
		
		if(color_[1] == color[0] &&
		   color_[2] == color[0] &&
		   color_[3] == color[0])
		{
			// if all colors are the same then mark it so we don't have to process all vertex colors
			oneColor = true;

			if(color[0] == Color::white)
			{			
				// if all vertex colors are white, just set a flag so they don't have to
				// be processed
				white = true;
				blend = false;
			}
			else
			{
				// blend if alpha < 1
				blend = (color[0][3] < 1.0f);
			}
		}
		else
		{	
			color[0] = color_[0];
			color[1] = color_[1];
			color[2] = color_[2];
			color[3] = color_[3];
			
			blend = (color[0][3] < 1.0f || color[1][3] < 1.0f || color[2][3] < 1.0f || color[3][3] < 1.0f);			
		}
	}
	
	
	ImageElement(Image *image_, float xOffset_, float yOffset_, float width_, float height_, float u1_, float v1_, float u2_, float v2_)
	{
		image    = image_;
		xOffset  = xOffset_;
		yOffset  = yOffset_;
		width    = width_;
		height   = height_;
		white    = true;
		oneColor = true;
		blend    = false;
		color[0] = Color::white;
		u1       = u1_;
		v1       = v1_;
		u2       = u2_;
		v2       = v2_;
	}
	
	bool blend;
	bool oneColor;
	bool white;	
	Image * image;

	float xOffset;
	float yOffset;
	
	float width;
	float height;
	
	float u1, v1, u2, v2;
	
	Color color[4];
	
	friend class AnimatedImage;
};

}


/*!***************************************************************************
 *  \brief base class for StillImage and AnimatedImage
 *****************************************************************************/
class ImageDescriptor
{
public:

	virtual ~ImageDescriptor() {}

	virtual void Clear() = 0;
	
	virtual void SetStatic(bool isStatic) = 0;
	virtual void SetWidth(float width) = 0;
	virtual void SetHeight(float height) = 0;
	virtual void SetDimensions(float width, float height) = 0;

	virtual void SetColor(const Color &color) = 0;	
	virtual void SetVertexColors (const Color &tl, const Color &tr, const Color &bl, const Color &br) = 0;

	virtual float GetWidth() const = 0;
	virtual float GetHeight() const = 0;

	bool Load();
	bool Draw();

	bool IsGrayScale()
	{ return _grayscale; }

protected:

	Color _color[4];      //! used only as a parameter to LoadImage. Holds the color of the upper left, upper right, lower left, and lower right vertices respectively

	bool  _isStatic;      //! used only as a parameter to LoadImage. This tells
	                      //! whether the image being loaded is to be loaded
	                      //! into a non-volatile area of texture memory
	                      
	float _width, _height;  //! width and height of image, in pixels.
	                         //! If the StillImage is a compound, i.e. it
	                         //! contains multiple images, then the width and height
	                         //! refer to the entire compound           

	bool _animated; 
	bool _grayscale;
	
	friend class GameVideo;
};


/*!***************************************************************************
 *  \brief this represents an image, used internally ONLY
 *
 *  \note  StillImages may be simple images or compound images. Compound
 *         images are when you stitch together multiple small images to create
 *         a large image, e.g. with TilesToObject(). Externally though,
 *         it's fine to think of compound images as just a single image.
 *****************************************************************************/
class StillImage : public ImageDescriptor
{
public:

	StillImage(bool grayscale = false) 
	{
		Clear();
		_animated = false;
		_grayscale = grayscale;
	}
	
	//! AddImage allows you to create compound images. You start with a 
	//! newly created StillImage, then call AddImage(), passing in
	//! all the images you want to add, along with the x, y offsets they
	//! should be positioned at. The u1,v1,u2,v2 tell which portion of the
	//! image to use (usually 0,0,1,1)
	bool AddImage(const StillImage &id, float xOffset, float yOffset, float u1 = 0.0f, float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f);
	
	void Clear()
	{
		_isStatic = false;
		_width = _height = 0.0f;
		_filename.clear();
		_elements.clear();
		SetColor(Color::white);
	}

	void SetFilename(const std::string &filename) { _filename = filename; }

	void SetColor(const Color &color)
	{
		_color[0] = _color[1] = _color[2] = _color[3] = color;
	}
	
	void SetVertexColors (const Color &tl, const Color &tr, const Color &bl, const Color &br)
	{
		_color[0] = tl;
		_color[1] = tr;
		_color[2] = bl;
		_color[3] = br;
	}
	
	void SetDimensions   (float width, float height) {	_width  = width;  _height = height; }
	
	void SetWidth        (float width)    { _width = width; }	
	void SetHeight       (float height)   {	_height = height; }
	
	virtual void SetStatic(bool isStatic)  { _isStatic = isStatic; }
	
	std::string GetFilename() const { return _filename; }
	
	float GetWidth() const  { return _width; }
	float GetHeight() const { return _height; }
	void  GetVertexColor(Color &c, int colorIndex) { c = _color[colorIndex]; }

	void EnableGrayScale()
	{ 
		this->_grayscale = true; 
		this->Load();
	}
	void DisableGrayScale()
	{ 
		this->_grayscale = false; 
		this->Load();
	}
			
private:

	std::string _filename;  //! used only as a parameter to LoadImage.

	//! an image descriptor represents a compound image, which is made
	//! up of multiple elements
	std::vector <private_video::ImageElement> _elements;

	friend class GameVideo;
	friend class AnimatedImage;
	friend class private_video::ParticleSystem;
};	



/*!***************************************************************************
 *  \brief this represents an animated image.
 *****************************************************************************/

class AnimationFrame
{
public:
	
	int32     _frame_time;  // how long to display this frame (relative to VIDEO_ANIMTION_FRAME_PERIOD)
	StillImage _image;
};

class AnimatedImage : public ImageDescriptor
{
public:
	
	AnimatedImage(bool grayscale = false)
	{ 
		Clear();
		_animated = true; 
		_grayscale = grayscale;
	}


	/*!
	 *  \brief clears the animated image
	 */
	void Clear();

	void EnableGrayScale();
	void DisableGrayScale();
	

	/*!
	 *  \brief call every frame to update the animation's current frame. This will automatically
	 *         synchronize the animation to VIDEO_ANIMATION_FRAME_PERIOD, i.e. 30 frames per second.
	 *         If you want to update the frames yourself using some custom method, then use
	 *         SetFrame()
	 */
	void Update();
	

	/*!
	 *  \brief call to set the current frame index of the animation. This is useful for character
	 *         animation for example, where the current frame depends on how far the character is
	 *         from one tile to the next tile.
	 *
	 *  \param frame_index an index from 0 to numFrames - 1
	 */	
	void SetFrameIndex(int32 frame_index);
	
	
	/*!
	 *  \brief returns the number of frames in this animation
	 */
	int32 GetNumFrames() const { return static_cast<int32>(_frames.size()); }


	/*!
	 *  \brief returns an index to the current animation
	 */
	int32 GetCurFrameIndex() const { return _frame_index; }

	
	/*!
	 *  \brief adds a frame using the filename of the image. This is the most convenient
	 *         way to add frames, BUT this makes it impossible to control the image properties,
	 *         such as vertex colors, and size (width and height). If you do it this way,
	 *         the width and height will be the pixel width/height of the image itself. This isn't
	 *         always what you want- for example if your coordinate system is in terms of tiles,
	 *         then a 32x32 tile would have a width and height of 1, not 32.
	 *
	 *  \param frame filename of the frame to add
	 *
	 *  \param frame_time  how many animation periods this frame lasts for. For example, if
	 *                     VIDEO_ANIMATION_FRAME_PERIOD is 30, and frame_count is 3, then this
	 *                     frame will last for 90 milliseconds.
	 */	
	bool AddFrame(const std::string &frame, int frame_time);


	/*!
	 *  \brief adds a frame by passing in a static image.
	 *
	 *  \param frame a static image to use as the frame
	 *
	 *  \param frame_time  how many animation periods this frame lasts for. For example, if
	 *                     VIDEO_ANIMATION_FRAME_PERIOD is 30, and frame_count is 3, then this
	 *                     frame will last for 90 milliseconds.
	 */	
	bool AddFrame(const StillImage &frame, int frame_time);


	/*!
	 *  \brief returns the width of the 1st frame of animation. You should try to make all your
	 *         frames the same size, otherwise the "width" of an animation doesn't really have any
	 *         meaning.
	 */
	float GetWidth() const;


	/*!
	 *  \brief returns the height of the 1st frame of animation. You should try to make all your
	 *         frames the same size, otherwise the "height" of an animation doesn't really have any
	 *         meaning.
	 */
	float GetHeight() const;


	/*!
	 *  \brief sets all frames to loaded statically
	 */
	void SetStatic(bool isStatic);

	/*!
	 *  \brief sets all frames to be a certain width
	 */
	void SetWidth(float width);


	/*!
	 *  \brief sets all frames to be a certain height
	 */
	void SetHeight(float height);


	/*!
	 *  \brief sets all frames to be a certain width and height
	 */
	void SetDimensions(float width, float height);


	/*!
	 *  \brief sets all frames to be a certain color
	 */
	void SetColor(const Color &color);


	/*!
	 *  \brief sets all frames to have the specified vertex colors
	 */
	void SetVertexColors (const Color &tl, const Color &tr, const Color &bl, const Color &br);

	
	/*!
	 *  \brief returns a pointer to the indexth frame. For the most part, this is a function you
	 *         should never be messing with, but this function is available in case you need it.
	 */
	StillImage *GetFrame(int32 index) const;

	float GetFrameProgress() const
	{
		return (float)_frame_counter / _frames[_frame_index]._frame_time;
	}

private:

	int32 _frame_index;      //! index of which animation frame to show
	int32 _frame_counter;    //! count how long each frame has been shown for
	
	std::vector<AnimationFrame> _frames;  //! vector of animation frames

	friend class GameVideo;
};


}  // namespace hoa_video

#endif  // !__IMAGE_HEADER__
