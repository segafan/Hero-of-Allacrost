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


// Forward declarations
class StillImage;
class AnimatedImage;

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

	/*!***************************************************************************
	*  \brief Constructor defaulting image to first one in a texsheet, a sheet that is
	*         determined later.
	* \param fname file where texsheet is
	* \param w width of the image
	* \param h height of the image
	* \param grayscale is image grayscale
	*****************************************************************************/
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

	/*!***************************************************************************
	*  \brief Constructor where image coordinates are specified, along with tex coords
	*         and texsheet.
	* \param sheet texsheet image is found on
	* \param fname file where texsheet is
	* \param x_ x coordinate of image in texsheet
	* \param y_ y coordinate of image in texsheet
	* \param w width of the image
	* \param h height of the image
	* \param u1_ upper-left u coordinate for the image
	* \param v1_ upper-left v coordinate for the image
	* \param u2_ lower-right u coordinate for the image
	* \param v2_ lower-right v coordinate for the image
	* \param grayscale is image grayscale
	*****************************************************************************/
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
		return *this;
	}

	//! texture sheet using this image
	TexSheet *texSheet;
	//! stored for every image in case it needs to be reloaded
	std::string filename;
	
	//! location of image within the sheet
	int32 x, y;
	//! width and height, in pixels
	int32 width, height;

	//! also store the actual uv coords. This is a bit
	float u1, v1;
	//! redundant, but saves floating point calculations
	float u2, v2;
	
	//! keep track of when this image can be deleted
	int32 refCount;

	//! track whether this image is grayscale or not
	bool grayscale;
};


/*!***************************************************************************
 *  \brief Represents a single image within an image descriptor. Compound
 *         images are formed of multiple ImageElements.
 *****************************************************************************/

class ImageElement
{
public:

	/*!***************************************************************************
	*  \brief Constructor specifying a specific image element.  Multiple elements can be stacked
	*         to form one compound image
	* \param image_ pointer to the image the lements references
	* \param xOffset_ x offset of the image with regards to stacking
	* \param yOffset_ y offset of the image with regards to stacking
	* \param width_ width of the image
	* \param height_ height of the image
	* \param u1_ upper-left u coordinate for the image
	* \param v1_ upper-left v coordinate for the image
	* \param u2_ lower-right u coordinate for the image
	* \param v2_ lower-right v coordinate for the image
	* \param color_[] colors of the four vertices
	*****************************************************************************/
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
	
	
	/*!***************************************************************************
	*  \brief Constructor defaulting the element to white vertices.  Disables blending.
	* \param image_ pointer to the image the lements references
	* \param xOffset_ x offset of the image with regards to stacking
	* \param yOffset_ y offset of the image with regards to stacking
	* \param width_ width of the image
	* \param height_ height of the image
	* \param u1_ upper-left u coordinate for the image
	* \param v1_ upper-left v coordinate for the image
	* \param u2_ lower-right u coordinate for the image
	* \param v2_ lower-right v coordinate for the image
	*****************************************************************************/
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
	
	//! perform blending with this element
	bool blend;
	//! are all of the vertices one color
	bool oneColor;
	//! are the vertices white
	bool white;	
	//! the image being referenced
	Image * image;

	//! x offset in the image stack
	float xOffset;
	//! y offset in the image stack
	float yOffset;
	
	//! width of the image in the stack
	float width;
	//! height of the image in the stack
	float height;
	
	//! tex coords for the image
	float u1, v1, u2, v2;
	
	//! vertex colors
	Color color[4];
	
	//! give AnimatedImage direct access
	friend class AnimatedImage;
};


/*!***************************************************************************
 *  \brief Information for a multi image (used for load in batch mode)
 *****************************************************************************/

class MultiImage
{
public:
	MultiImage (AnimatedImage& id, const std::string& filename, const uint32 rows, const uint32 cols, const float width=0.0f, const float height=0.0f, const bool grayscale=false) :
		_filename (filename),
		_rows (rows),
		_cols (cols),
		
		_width (width),
		_height (height),
		_grayscale (grayscale),
		_animated_image (&id),
		_still_images (0)
	{
	}

	MultiImage (std::vector <StillImage>& id, const std::string& filename, const uint32 rows, const uint32 cols, const float width=0.0f, const float height=0.0f, const bool grayscale=false) :
		_filename (filename),
		_rows (rows),
		_cols (cols),
		_width (width),
		_height (height),
		_grayscale (grayscale),
		_animated_image (0),
		_still_images (&id)
	{
	}

	std::string _filename;	//!< Name of the image file.

	uint32 _rows;		//!< Number of rows.
	uint32 _cols;		//!< Number of columns.

	float _width;	//!< Width of the image.
	float _height;	//!< Height of the image.
	
	//! track whether this multi-image is grayscale or not
	bool _grayscale;

	// Just one of this structures will be used, depending if the image is a multi-image o an animated image
	AnimatedImage* _animated_image;			//!< Pointer to an animated image, where the cut image will be loaded
	std::vector <StillImage>* _still_images;	//!< Pointer to an array of still images, where the cut image will be loaded
};


}


/*!***************************************************************************
 *  \brief base class for StillImage and AnimatedImage
 *****************************************************************************/
class ImageDescriptor
{
public:

	/*!***************************************************************************
	*  \brief Destructor
	*****************************************************************************/
	virtual ~ImageDescriptor() {}
	
	/*!***************************************************************************
	*  \brief Clears the descriptor (color, width, height, etc.)
	*****************************************************************************/
	virtual void Clear() = 0;
	
	/*!***************************************************************************
	*  \brief Makes the image static
	* \param isStatic whether the image is static or not
	*****************************************************************************/
	virtual void SetStatic(bool isStatic) = 0;
	
	/*!***************************************************************************
	*  \brief Sets image width
	* \param width desired width of the image
	*****************************************************************************/
	virtual void SetWidth(float width) = 0;
	
	/*!***************************************************************************
	*  \brief Sets image height
	* \param height desired height of the image
	*****************************************************************************/
	virtual void SetHeight(float height) = 0;
	
	/*!***************************************************************************
	*  \brief Sets image dimensions
	* \param width desired width of the image
	* \param height desired height of the image
	*****************************************************************************/
	virtual void SetDimensions(float width, float height) = 0;

	/*!***************************************************************************
	*  \brief Sets image color
	* \param color desired color of the image
	*****************************************************************************/
	virtual void SetColor(const Color &color) = 0;
	
	/*!***************************************************************************
	*  \brief Sets image vertex colors
	* \param tl top left vertex color
	* \param tr top right vertex color
	* \param bl bottom left vertex color
	* \param br bottom right vertex color
	*****************************************************************************/
	virtual void SetVertexColors (const Color &tl, const Color &tr, const Color &bl, const Color &br) = 0;

	/*!***************************************************************************
	* \brief Returns image width
	* \return width of image
	*****************************************************************************/
	virtual float GetWidth() const = 0;
	
	/*!***************************************************************************
	* \brief Returns image height
	* \return height of image
	*****************************************************************************/
	virtual float GetHeight() const = 0;

	/*!***************************************************************************
	* \brief Loads image
	* \return Success/failure
	*****************************************************************************/
	bool Load();
	
	/*!***************************************************************************
	* \brief Draws image
	* \return Success/failure
	*****************************************************************************/
	bool Draw();

	/*!***************************************************************************
	* \brief Returns if image is grayscale
	* \return True for grayscale, false for not
	*****************************************************************************/
	bool IsGrayScale()
	{ return _grayscale; }

protected:

    //! used only as a parameter to LoadImage. Holds the color of the upper left, upper right, lower left, and lower right vertices respectively
	Color _color[4];

	//! used only as a parameter to LoadImage. This tells
	//! whether the image being loaded is to be loaded
	//! into a non-volatile area of texture memory
	bool  _isStatic;      
	
	//! width and height of image, in pixels.
	//! If the StillImage is a compound, i.e. it
	//! contains multiple images, then the width and height
	//! refer to the entire compound  	
	float _width, _height;           

	//! if image is animated
	bool _animated;
	//! if image is grayscale
	bool _grayscale;
	
	//! give GameVideo direct access
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

	/*!***************************************************************************
	* \brief Returns if image is grayscale
	* \param grayscale whether image is grayscale or not.  Default = false.
	*****************************************************************************/
	StillImage(bool grayscale = false) 
	{
		Clear();
		_animated = false;
		_grayscale = grayscale;
	}
	
	/*!***************************************************************************
	* \brief AddImage allows you to create compound images. You start with a 
	*	    newly created StillImage, then call AddImage(), passing in
	* 	    all the images you want to add, along with the x, y offsets they
	* 	    should be positioned at. The u1,v1,u2,v2 tell which portion of the
	* 	    image to use (usually 0,0,1,1)
	* \param id image to add to stack
	* \param xOffset x offset in the stack
	* \param yOffset y offset in the stack
	* \param u1 upper-left u coordinate for the image.  Default = 0.0f.
	* \param v1 upper-left v coordinate for the image.  Default = 0.0f.
	* \param u2 lower-right u coordinate for the image.  Default = 1.0f.
	* \param v2 lower-right v coordinate for the image.  Default = 1.0f.
	*****************************************************************************/
	bool AddImage(const StillImage &id, float xOffset, float yOffset, float u1 = 0.0f, float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f);
	
	/*!***************************************************************************
	* \brief clears the image by resetting its properties
	*****************************************************************************/
	void Clear()
	{
		_isStatic = false;
		_width = _height = 0.0f;
		_filename.clear();
		_elements.clear();
		SetColor(Color::white);
	}

	/*!***************************************************************************
	* \brief Sets the filename where the image came from
	* \param filename file where image came from
	*****************************************************************************/
	void SetFilename(const std::string &filename) { _filename = filename; }

	/*!***************************************************************************
	* \brief Sets vertex colors to the one passed in
	* \param color new vertex color
	*****************************************************************************/
	void SetColor(const Color &color)
	{
		_color[0] = _color[1] = _color[2] = _color[3] = color;
	}
	
	/*!***************************************************************************
	* \brief Sets individual vertex colors to the ones passed in
	* \param tl top left vertex color
	* \param tr top right vertex color
	* \param bl bottom left vertex color
	* \param br bottom right vertex color
	*****************************************************************************/
	void SetVertexColors (const Color &tl, const Color &tr, const Color &bl, const Color &br)
	{
		_color[0] = tl;
		_color[1] = tr;
		_color[2] = bl;
		_color[3] = br;
	}
	
	/*!***************************************************************************
	* \brief Sets dimensions of the image
	* \param width desired width of the image
	* \param height desired height of the image
	*****************************************************************************/
	void SetDimensions   (float width, float height) {	_width  = width;  _height = height; }
	
	/*!***************************************************************************
	* \brief Sets width of the image
	* \param width desired width of the image
	*****************************************************************************/
	void SetWidth        (float width)    { _width = width; }
	
	/*!***************************************************************************
	* \brief Sets height of the image
	* \param width desired height of the image
	*****************************************************************************/	
	void SetHeight       (float height)   {	_height = height; }
	
	/*!***************************************************************************
	* \brief Sets image to static/animated
	* \param isStatic true if static, false if not
	*****************************************************************************/
	virtual void SetStatic(bool isStatic)  { _isStatic = isStatic; }
	
	/*!***************************************************************************
	* \brief Gets filename for image
	* \return filename the image came from
	*****************************************************************************/
	std::string GetFilename() const { return _filename; }
	
	/*!***************************************************************************
	* \brief Gets width of the image
	* \return width of the image
	*****************************************************************************/
	float GetWidth() const  { return _width; }
	
	/*!***************************************************************************
	* \brief Gets height of the image
	* \return height of the image
	*****************************************************************************/
	float GetHeight() const { return _height; }
	
	/*!***************************************************************************
	* \brief Gets color of a particular vertex
	* \param c color to store the returned color in
	* \param colorIndex index of vertex whose color we want
	*****************************************************************************/
	void  GetVertexColor(Color &c, int colorIndex) { c = _color[colorIndex]; }

	/*!***************************************************************************
	* \brief Enables grayscaling for the image then reloads it
	*****************************************************************************/
	void EnableGrayScale()
	{ 
		this->_grayscale = true; 
		this->Load();
	}
	
	/*!***************************************************************************
	* \brief Disables grayscaling for the image then reloads it
	*****************************************************************************/
	void DisableGrayScale()
	{ 
		this->_grayscale = false; 
		this->Load();
	}
			
private:

	//! used only as a parameter to LoadImage.
	std::string _filename;  

	//! an image descriptor represents a compound image, which is made
	//! up of multiple elements
	std::vector <private_video::ImageElement> _elements;

	//! give GameVideo, AnimatedImage, and ParticleSystem direct access
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
	
	//! how long to display this frame (relative to VIDEO_ANIMTION_FRAME_PERIOD)
	int32     _frame_time;
	
	//! The image used for this frame
	StillImage _image;
};

class AnimatedImage : public ImageDescriptor
{
public:
	
	/*!
	 *  \brief Constructs an animated image.  Grayscale defaults to false
	 * \param grayscale true for grayscale, false for not
	 */
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

	/*!
	 *  \brief Enables grayscale for the image
	 */
	void EnableGrayScale();
	
	/*!
	 *  \brief Disables grayscale for the image
	 */
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
	 * \return number of frames in the animation
	 */
	int32 GetNumFrames() const { return static_cast<int32>(_frames.size()); }


	/*!
	 *  \brief returns an index to the current animation
	 * \return the current frame index
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
	 * \return success/failure
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
	 * \return success/failure
	 */	
	bool AddFrame(const StillImage &frame, int frame_time);


	/*!
	 *  \brief returns the width of the 1st frame of animation. You should try to make all your
	 *         frames the same size, otherwise the "width" of an animation doesn't really have any
	 *         meaning.
	 * \return width of the image
	 */
	float GetWidth() const;


	/*!
	 *  \brief returns the height of the 1st frame of animation. You should try to make all your
	 *         frames the same size, otherwise the "height" of an animation doesn't really have any
	 *         meaning.
	 * \return height of the image
	 */
	float GetHeight() const;


	/*!
	 *  \brief sets all frames to loaded statically
	 * \param isStatic true for static image, false for animated
	 */
	void SetStatic(bool isStatic);

	/*!
	 *  \brief sets all frames to be a certain width
	 * \param width desired width of the images
	 */
	void SetWidth(float width);


	/*!
	 *  \brief sets all frames to be a certain height
	 * \param height desired height of the images
	 */
	void SetHeight(float height);


	/*!
	 *  \brief sets all frames to be a certain width and height
	 * \param width desired width of the images
	 * \param height desired height of the images
	 */
	void SetDimensions(float width, float height);


	/*!
	 *  \brief sets all frames to be a certain color
	 * \param color color to set all of the vertices to
	 */
	void SetColor(const Color &color);


	/*!
	 *  \brief sets all frames to have the specified vertex colors
	 * \param tl top left vertex color
	* \param tr top right vertex color
	* \param bl bottom left vertex color
	* \param br bottom right vertex color
	 */
	void SetVertexColors (const Color &tl, const Color &tr, const Color &bl, const Color &br);

	
	/*!
	 *  \brief returns a pointer to the indexth frame. For the most part, this is a function you
	 *         should never be messing with, but this function is available in case you need it.
	 * \param index index of the frame you want
	 * \return the image at that index
	 */
	StillImage *GetFrame(int32 index) const;

	/*!
	 *  \brief returns the percentage complete of this frame being shown
	 * \return float [0,1] describing how much of its allotted time this frame has spent
	 */
	float GetFrameProgress() const
	{
		return (float)_frame_counter / _frames[_frame_index]._frame_time;
	}

private:

	//! index of which animation frame to show
	int32 _frame_index;
    //! count how long each frame has been shown for
	int32 _frame_counter;
	
	//! vector of animation frames
	std::vector<AnimationFrame> _frames;

	//! give GameVideo direct access
	friend class GameVideo;
};



}  // namespace hoa_video

#endif  // !__IMAGE_HEADER__
