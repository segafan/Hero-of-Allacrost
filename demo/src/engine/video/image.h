///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    image.h
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Header file for image classes
***
*** This file contains several classes that represent images loaded into the
*** engine. The public classes used outside of the video engine are:
***
*** - <b>ImageDescriptor</b> is abstract base class for other images. The only
*** time the API user should ever need to use this class is if they want a
*** container filled with both still and animated images.
***
*** - <b>StillImage</b> is a single non-animated image. This is what the user 
*** will utilize most of the time.
***
*** - <b>AnimatedImage</b> is an animated image that contains multiple frames
*** (which are AnimationFrame objects) and timing information for each frame.
***
*** The internal classes used only by the video engine are:
***
*** - <b>Image</b> represents a sub-rectangle within a texture sheet. In other 
*** words, it is a physical image in memory. It is used internally by the
*** ImageElement class.
***
*** - <b>ImageElement</b> contains a pointer to an Image, plus properties about
*** the image such as width, height, and color. This is used internally by the
*** StillImage class.
***
*** - <b>MultiImage</b> represents a single image file that contains multiple
*** embedded images in it that are adjacent to one another. It is used to
*** cut up the conglomerate image into the respective component images.
***
*** - <b>AnimationFrame</b> is a single frame of animation. It consists of a
*** StillImage, and how long the frame should be displayed in the animation.
*** ***************************************************************************/


#ifndef __IMAGE_HEADER__
#define __IMAGE_HEADER__

#include "utils.h"
#include "color.h"
#include "tex_mgmt.h"

namespace hoa_video {

// Forward declarations
class StillImage;
class AnimatedImage;

namespace private_video {

class ParticleSystem;

/** ****************************************************************************
*** \brief A container to store information about an image being loaded.
*** This class is used to pass information between image loader code and
*** OpenGL texture creation.
*** ***************************************************************************/
class ImageLoadInfo {
public:
	//! \brief The width of the image (in pixels?)
	int32 width;
	//! \brief The height of the image (in pixels?)
	int32 height;
	//! \todo Needs a comment
	void* pixels;
}; // class ImageLoadInfo


/** ****************************************************************************
*** \brief Represents a single image that is loaded and stored in a texture sheet.
*** ***************************************************************************/
class Image {
public:
	//! \brief A pointer to the texture sheet where the image is contained.
	TexSheet* texture_sheet;
	/** \brief The filename for the image.
	*** This is stored for every image in case it needs to be reloaded.
	**/
	std::string filename;
	//! \brief The coordiates of where the image is located in the texture sheet
	int32 x, y;
	/** \brief The actual uv coordinates.
	*** This is a little redundant, but saves effort on floating point calcuations.
	*** u1 and v1 are the upper-left UV coordinates, while u2 and v2 correspond to
	*** the lower-right.
	**/
	float u1, v1, u2, v2;

	//! \brief The image's width and height, in pixels
	int32 width, height;
	//! \brief Determines whether this image is in grayscale mode or not
	bool grayscale;

	/** \brief The number of times that this image is refereced by ImageDescriptors
	*** This is used to determine when the image may be deleted.
	**/
	int32 ref_count;

	/** \brief Constructor defaults image as the first one in a texture sheet.
	*** \note The actual sheet where the image is located will be determined later.
	**/
	Image(const std::string &fname, int32 w, int32 h, bool grayscale_);

	///! \brief Constructor where image coordinates are specified, along with texture coords and the texture sheet.
	Image(TexSheet *sheet, const std::string &fname, int32 x_, int32 y_, float u1_, float v1_,
		float u2_, float v2_, int32 w, int32 h, bool grayscale_);

	Image & operator=(Image &rhs)
		{ return *this; }
}; // class Image


/** ****************************************************************************
*** \brief Represents a single image within an ImageDescriptor object.
*** Compound images are formed of multiple ImageElements.
*** ***************************************************************************/
class ImageElement {
	friend class AnimatedImage;
public:
	//! \brief The image that is being referenced by this object.
	Image* image;

	//! \brief The x offset in the image stack.
	float x_offset;
	//! \brief The y offset in the image stack.
	float y_offset;
	/** \brief The texture coordinates for the image.
	*** u1, v1 describes the upper-left corner while u2, v2 describes the bottom-right.
	**/
	float u1, v1, u2, v2;

	//! \brief The width of the image in the stack.
	float width;
	//! \brief The height of the image in the stack.
	float height;

	//! \brief The colors of the four image vertices.
	Color color[4];

	//! \brief A true value indicates to perform blending with this element.
	bool blend;
	//! \brief True if all of the vertices are the same color.
	bool one_color;
	//! \brief Set to true if the vertices are all white.
	bool white;

	/** \brief Constructor specifying a specific image element.
	*** Multiple elements can be stacked to form one compound image
	**/
	ImageElement(Image *image_, float x_offset_, float y_offset_, float u1_, float v1_,
		float u2_, float v2_, float width_, float height_, Color color_[4]);
	
	//! \brief Constructor defaulting the element to have white vertices and disables blending.
	ImageElement(Image *image_, float x_offset_, float y_offset_, float u1_, float v1_,
		float u2_, float v2_, float width_, float height_);
}; // class ImageElement


/** ****************************************************************************
*** \brief Representation for multiple images contained within a single file
*** This is used for loading tileset images, sprite sheets, and other image
*** files where there are multiple images embedded within the file.
*** ***************************************************************************/
class MultiImage {
public:
	//! \brief The name of the image file
	std::string filename;

	//! \brief The number of rows of images in the file.
	uint32 rows;
	//! \brief The number of columns of images in the file.
	uint32 cols;

	//! \brief The width of the entire image in the file.
	float width;
	//! \brief The height of the entire image in the file.
	float height;
	
	//! \brief Tracks whether this multi-image should be grayscale or not.
	bool grayscale;


	//! \brief A pointer to an animated image where the cut image will be loaded into
	//! \note This structure is only used if the user called the constructor with an AnimatedImage argument.
	AnimatedImage* animated_image;
	//! \brief A pointer to a vector of still images where the cut image will be loaded into
	//! \note This structure is only used if the user called the constructor with a StillImage vector argument.
	std::vector<StillImage>* still_images;

	//! \brief This constructor is used if the multi-image contains animation frames.
	MultiImage(AnimatedImage& id, const std::string& filename, const uint32 rows, const uint32 cols,
		const float width = 0.0f, const float height = 0.0f, const bool grayscale = false);
	//! \brief This constructor is used if the multi-image contains several still images.
	MultiImage(std::vector <StillImage>& id, const std::string& filename, const uint32 rows, const uint32 cols,
		const float width = 0.0f, const float height = 0.0f, const bool grayscale = false);
}; // class MultiImage

} // namespace private_video

/** ****************************************************************************
*** \brief The abstract base class for StillImage and AnimatedImage.
*** ***************************************************************************/
class ImageDescriptor {
	friend class GameVideo;
public:
	virtual ~ImageDescriptor()
		{}

	//! \brief Clears all data retained by the object (color, width, height, etc.)
	virtual void Clear() = 0;

	//! \name Class Member Set Functions
	//@{
	/** \brief Makes the image static.
	*** \param is_static Set to true to make the image static.
	**/
	virtual void SetStatic(bool is_static) = 0;
	/** \brief Sets the image's width.
	*** \param width The desired width of the image.
	**/
	virtual void SetWidth(float width) = 0;
	/** \brief Sets the image's height.
	*** \param height The desired height of the image.
	**/
	virtual void SetHeight(float height) = 0;
	/** \brief Sets the image's dimensions.
	*** \param width desired width of the image
	*** \param height desired height of the image
	**/
	virtual void SetDimensions(float width, float height) = 0;
	/** \brief Sets the image's color.
	*** \param color The desired color of the image.
	**/
	virtual void SetColor(const Color &color) = 0;
	/** \brief Sets the image's vertex colors
	*** \param tl The top left vertex color..
	*** \param tr The top right vertex color.
	*** \param bl The bottom left vertex color.
	*** \param br The bottom right vertex color.
	**/
	virtual void SetVertexColors(const Color &tl, const Color &tr, const Color &bl, const Color &br) = 0;
	//@}

	//! \name Class Member Get Functions
	//@{
	//! \brief Returns the image width
	virtual float GetWidth() const = 0;
	//! \brief Returns image height
	virtual float GetHeight() const = 0;
	//! \brief Returns true if the image is grayscale.
	bool IsGrayScale() const
		{ return _grayscale; }
	/** \brief Returns true if the image is animated.
	*** \note This can also be used to determine whether an ImageDescriptor pointer is a StillImage
	*** or an AnimatedImage. This would be a better solution than trying to do a dynamic_cast on
	*** the pointer.
	**/
	bool IsAnimated() const
		{ return _animated; }
	//@}

	//! \brief Loads the image file, and returns true if it was successful.
	bool Load();
	//! \brief Draws the image to the display buffer.
	void Draw();

protected:
	/** \brief The width and height of the image, in pixels.
	*** If this represents a compound StillImage is a compound, (i.e. it contains multiple images)
	*** then the width and height refer to the entire compound
	*** \todo If these dimensions are in pixels, why are these floats? Are we sure its not in the
	*** dimensions relative to the coordinate system being used?
	**/
	float _width, _height;

    //! \brief Holds the color of the upper left, upper right, lower left, and lower right vertices, respectively.
	Color _color[4];

	//! \brief Indicates whether the image being loaded should be loaded into a non-volatile area of texture memory.
	bool  _is_static;
	//! \brief True if this image is grayscale.
	bool _grayscale;
	//! \brief True if this image is animated .
	bool _animated;
}; // class ImageDescriptor


/** ****************************************************************************
*** \brief Represents a single or compound still image
*** StillImages may be simple images or compound images. Compound images are
*** created when you stitch together multiple smaller images to create a single
*** large image (e.g. with TilesToObject() function). It's fine to think of
*** compound images as just a single image.
*** ***************************************************************************/
class StillImage : public ImageDescriptor {
	friend class GameVideo;
	friend class AnimatedImage;
	friend class private_video::ParticleSystem;
public:
	StillImage(bool grayscale = false);

	//! \brief Clears the image by resetting its properties
	void Clear();

	//!  \brief Enables grayscaling for the image then reloads it
	void EnableGrayScale();
	//!  \brief Disables grayscaling for the image then reloads it
	void DisableGrayScale();

	/** \brief AddImage allows the user to create compound images.
	*** \param id The image to add to the compound image.
	*** \param x_offset The x offset of the compound image.
	*** \param yOffset The y offset of the compound image.
	*** \param u1 The upper-left u coordinate for the image. The default is 0.0f.
	*** \param v1 The upper-left v coordinate for the image. The default is 0.0f.
	*** \param u2 The lower-right u coordinate for the image. The default is 1.0f.
	*** \param v2 The lower-right v coordinate for the image. The default is 1.0f.
	***
	*** Starting with a newly created StillImage, call AddImage(), for all of the images you wish
	*** to add, along with the x and y offsets that they should be positioned at. The u1, v1, u2, v2
	*** coordinates tell which portion of the image to use (usually 0, 0, 1, 1)
	**/
	bool AddImage(const StillImage &id, float x_offset, float y_offset, float u1 = 0.0f, float v1 = 0.0f,
		float u2 = 1.0f, float v2 = 1.0f);
	
	//! \name Class Member Set Functions
	//@{
	//! \brief Sets the filename of the image
	void SetFilename(const std::string &filename)
		{ _filename = filename; }
	//! \brief Sets width of the image
	void SetWidth(float width)
		{ _width = width; }
	//! \brief Sets height of the image
	void SetHeight(float height)
		{ _height = height; }
	//! \brief Sets the dimensions (width + height) of the image.
	void SetDimensions(float width, float height)
		{ _width  = width; _height = height; }
	//! \brief Sets image to static/animated
	void SetStatic(bool is_static)
		{ _is_static = is_static; }
	//! \brief Sets the color for the image (for all four verteces).
	void SetColor(const Color &color)
		{ _color[0] = _color[1] = _color[2] = _color[3] = color; }
	/** \brief Sets individual vertex colors in the image.
	*** \param tl top left vertex color
	*** \param tr top right vertex color
	*** \param bl bottom left vertex color
	*** \param br bottom right vertex color
	**/
	void SetVertexColors(const Color &tl, const Color &tr, const Color &bl, const Color &br)
		{ _color[0] = tl; _color[1] = tr; _color[2] = bl; _color[3] = br; }
	//@}

	//! \name Class Member Get Functions
	//@{
	//! \brief Returns the filename of the image.
	std::string GetFilename() const
		{ return _filename; }
	//! \brief Returns the width of the image.
	float GetWidth() const
		{ return _width; }
	//! \brief Returns the height of the image.
	float GetHeight() const
		{ return _height; }	
	/** \brief Returns the color of a particular vertex
	*** \param c The Color object to place the color in.
	*** \param index The vertex index of the color to fetch
	*** \note If an invalid index value is used, the function will return with no warning.
	**/
	void GetVertexColor(Color &c, uint8 index)
		{ if (index > 3) return; else c = _color[index]; }
	//@}
private:
	/** \brief The name of the image file from which this image was created
	*** \todo What happens to this member if this is a compound image???
	**/
	std::string _filename;

	/** \brief The internal images that make the still image
	*** If the image is not a compound image, then this vector will contain a single element.
	**/
	std::vector <private_video::ImageElement> _elements;
}; // class StillImage : public ImageDescriptor



namespace private_video {

/** ****************************************************************************
*** \brief Represents a single frame in an animated image.
*** ***************************************************************************/
class AnimationFrame {
public:
	//! \brief The time to display this frame image, in milliseconds.
	//! \note This is relative to VIDEO_ANIMTION_FRAME_PERIOD.
	uint32 _frame_time;
	//! \brief The StillImage used for this frame in the animation.
	StillImage _image;
}; // class AnimationFrame

} // namespace private_video



/** ****************************************************************************
*** \brief Represents an animated image with both frames and timing information
***
*** Animated images are really nothing more than a series of individual still
*** images and timing information for each frame. This class assumes that
*** all frame images are the same size, so you should not attempt to use
*** this class if your frames are of different sizes. If you wish to use different
*** sized frame images in an animation, you'll need to implement the code
*** to do so yourself.
*** ***************************************************************************/
class AnimatedImage : public ImageDescriptor {
	friend class GameVideo;
public:
	AnimatedImage(bool grayscale = false);

	//! \brief Removes the data and properties allocated to the animated image
	void Clear();
	//! \brief Enables grayscale for the image
	void EnableGrayScale();
	//! \brief Disables grayscale for the image
	void DisableGrayScale();

	/** \brief Called every frame to update the animation's current frame.
	*** This will automatically synchronize the animation to VIDEO_ANIMATION_FRAME_PERIOD,
	*** i.e. 30 frames per second. If you want to update the frames yourself using some custom
	*** method, then use SetFrame()
	**/
	void Update();

	/** \brief Adds an animation frame by using an existing static image.
	*** \param frame The still image to use as the frame image.
	*** \param frame_time The amount of millseconds to display the frame.
	*** \return True on success, false on failure.
	**/
	bool AddFrame(const StillImage &frame, uint32 frame_time);
	/** \brief Adds an animation frame using the filename of the image to add.
	*** \param frame The filename of the frame image to add.
	*** \param frame_time The number of milliseconds that this animation should last for
	*** \return True on success, false on failure.
	***
	*** This is perhaps a more convenient way to add frames, <b>but</b> this makes it impossible
	*** to control the image properties such as vertex colors, and size. If you use this function,
	*** the width and height will be the pixel width/height of the image itself. This is not what
	*** you always will want. For example, if your coordinate system is in terms of 32x32 pixel
	*** tiles, then a tile image would have a width and height of 1, not 32.
	**/
	bool AddFrame(const std::string &frame, uint32 frame_time);

	//! \name Class Member Set Functions
	//@{
	//! \brief Sets all animation frames to be a certain width.
	void SetWidth(float width);
	//! \brief Sets all animation frames to be a certain height
	void SetHeight(float height);
	//! \brief Sets all animation frames to be a certain width and height.
	void SetDimensions(float width, float height);
	//! \brief sets All frames to be of a certain color (all vertices are set to the same color)
	void SetColor(const Color &color);
	/** \brief sets all frames to have the specified vertex colors
	*** \param tl The top left vertex color
	*** \param tr The top right vertex color
	*** \param bl The bottom left vertex color
	*** \param br The bottom right vertex color
	**/
	void SetVertexColors(const Color &tl, const Color &tr, const Color &bl, const Color &br);
	/** \brief Sets the static member for all animation frame images.
	*** \note If the frames are already loaded, it doesn't bother to try to unload them
	*** and then reload them again statically.
	**/
	void SetStatic(bool is_static)
		{ _is_static = is_static; }
	/** \brief Sets the current frame index of the animation.
	*** \param frame_index The index of the frame to access
	*** \note Passing in an invalid value for the index will not change the current frame
	**/
	void SetFrameIndex(uint32 index)
		{ if (index > _frames.size()) return; _frame_index = index; _frame_counter = 0; }
	//@}

	//! \name Class Member Get Functions
	//@{
	//! \brief Returns the width of the 1st frame of animation.
	//! \note Function will return 0.0f if there are no animation frames.
	float GetWidth() const
		{ if (_frames.size() == 0) return 0.0f; else return _frames[0]._image.GetWidth(); }
	//! \brief Returns the height of the 1st frame of animation.
	//! \note Function will return 0.0f if there are no animation frames.
	float GetHeight() const
		{ if (_frames.size() == 0) return 0.0f; else return _frames[0]._image.GetHeight(); }
	//! \brief Returns the number of frames in this animation
	uint32 GetNumFrames() const
		{ return _frames.size(); }
	//! \brief Returns the index number of the current frame in the animation.
	uint32 GetCurrentFrameIndex() const
		{ return _frame_index; }
	/** \brief Returns a pointer to the StillImage at a specified frame. 
	*** \param index index of the frame you want
	*** \return A pointer to the image at that index
	*** 
	*** Using this function is dangerous since it provides direct access to an image frame.
	*** If you find yourself in constant need of using this function, think twice about
	*** what you are doing.
	**/
	StillImage *GetFrame(uint32 index) const
		{ if (index >= _frames.size()) return NULL; else return const_cast<StillImage*>(&(_frames[index]._image)); }
	/** \brief Returns the percentage of timing complete for the current frame being shown.
	*** \return A float from 0.0f to 1.0f, indicate how much of its allotted time this frame has spent.
	**/
	float GetFrameProgress() const
		{ return (float)_frame_counter / _frames[_frame_index]._frame_time; }
	//@}

private:
	//! \brief The index of which animation frame to display.
	uint32 _frame_index;
    //! \brief Counts how long each frame has been shown for.
	uint32 _frame_counter;
	//! \brief The vector of animation frames (both frame images and frame timing).
	std::vector<private_video::AnimationFrame> _frames;
};

}  // namespace hoa_video

#endif  // __IMAGE_HEADER__
