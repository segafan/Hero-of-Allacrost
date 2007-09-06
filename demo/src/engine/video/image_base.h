///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    image_base.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for image base classes
***
*** This file contains several classes that represent various types of images
*** and image data manipulated by the video engine. All of these classes are
*** embedded in the private_video namespace, and thus are of no concern to
*** those who simply wish to use the video engine API. Below is a list and
*** short description of all the classes represented defined in this header:
***
*** - <b>ImageMemory</b> is a class used for loading and manipulating raw
*** image data stored in a system side buffer (ie not in texture memory)
***
*** - <b>BaseImageTexture</b> describes a sub-rectangle contained within a
*** texture sheet. In other words, it can essentially be viewed as a pointer
*** to an image's location in texture memory. Pointers of this type are
*** referenced in the ImageElement line of classes, and are used throughout
*** the texture management code. It is an abstract class.
***
*** - <b>ImageTexture</b> derives from BaseImageTexture and adds a filename
*** and tag information to the member data. This is used to internally
*** represent an image that has been loaded from a file and any special
*** properties of that image data.
***
*** - <b>BaseImageElement</b> is an abstract class that is used by the public
*** set of image classes to refer to ImageTexture types. In other words, it
*** is an intermediary medium between the API for images and the ImageTexture
*** set of classes. These elements are combined together in the public image
*** classes to create composite images.
***
*** - <b>ImageElement</b> contains a pointer to an ImageTexture type. It is
*** used internally by the public StillImage class.
***
*** \note There are more derived classes from this set in other areas of the
*** code. In particular, there is a TextImageTexture and TextImageElement class
*** defined in the text.h header file.
*** ***************************************************************************/

#ifndef __IMAGE_BASE_HEADER__
#define __IMAGE_BASE_HEADER__

#include "defs.h"
#include "utils.h"
#include "color.h"
#include "texture.h"

namespace hoa_video {

namespace private_video {

/** ****************************************************************************
*** \brief A wrapper around an image buffer in system memory
***
*** This class is used to pass image texture data between the CPU and GPU.
*** There are also several routines for performing manipulations on the raw
*** image data. This class may also be used as a temporary holder for pixel
*** data.
*** ***************************************************************************/
class ImageMemory {
public:
	ImageMemory();

	~ImageMemory();

	//! \brief The width of the image data (in pixels)
	int32 width;

	//! \brief The height of the image dat (in pixels)
	int32 height;

	//! \brief Buffer of data, usually of size width * height * 4 (RGBA, 8 bits per component)
	void* pixels;

	/** \brief Loads raw image data from a file and stores the data in the class members
	*** \param file_name The filename of the image to load, which should have a .png or .jpg extension
	*** \return True if the image was loaded successfully, false if it was not
	**/
	bool LoadImage(const std::string& filename);

	/** \brief Saves raw image data to a file
	*** \param file_name The full filename of the image to load
	*** \param png_image Set to true if this is a PNG image, or false if it is a JPG image
	*** \return True if the image was saved successfully, false if it was not
	**/
	bool SaveImage(const std::string& filename, bool png_image);

	/** \brief Converts the image data to grayscale format
	*** \note Calling this function when the image data is already grayscaled will create the
	*** exact same grayscaled image, but still take CPU time to do the conversion. There is
	*** no way 
	*** \note You can not convert from grayscale back to the original image. If you wish to
	*** do that, you must re-load or otherwise re-create the original.
	**/
	void ConvertToGrayscale();

	/** \brief Converts the RGBA pixel buffer to a RGB one
	*** \note Upon conversion, this function will also reduced the memory size pointed to
	*** by pixels to 3/4s of its original size, since the alpha information is no longer
	*** neeeded.
	**/
	void RGBAToRGB();

	/** \brief Set the class members by making a copy of a texture sheet
	*** \param texture A pointer to the TexSheet to be copied
	***
	*** This function effectively copies a texture (in video memory) to a system-side memory buffer
	**/
	void CopyFromTexture(TexSheet* texture);

	/** \brief Set the class members by making a copy of a texture sheet image
	*** \param img A pointer to the image to be copied
	***
	*** This function effectively copies an image (in video memory) to a system-side memory buffer
	**/
	void CopyFromImage(BaseImageTexture* img);

private:
	/** \brief Loads raw image data from a PNG file and stores the data in the class members
	*** \param file_name Filename of the PNG image to load
	*** \return True if the PNG image was loaded successfully, false if it was not
	**/
	bool _LoadPngImage(const std::string& filename);

	/** \brief Loads raw image data from a JPG file and stores the data in the class members
	*** \param file_name Filename of the JPG image to load
	*** \return True if the JPG image was loaded successfully, false if it was not
	**/
	bool _LoadJpgImage(const std::string& filename);

	/** \brief Saves image data to a PNG file
	*** \param filename Name of the file, without the extension
	*** \return True if the process was carried out with no problem, false otherwise
	**/
	bool _SavePngImage(const std::string& filename) const;

	/** \brief Saves image data to a JPG file
	*** \param filename Name of the file, without the extension
	*** \return True if the process was carried out with no problem, false otherwise
	**/
	bool _SaveJpgImage(const std::string& filename) const;
}; // class ImageMemory


/** ****************************************************************************
*** \brief Represents the location and properties of an image in texture memory
***
*** This class can be thought of as a pointer to the image in texture (GPU)
*** memory. The TextureManager singleton has a container filled with each
*** instance of this object (and its derivative classes), which primarily
*** serves to prevent the duplication of the same image data in more than one
*** location in texture memory.
***
*** \note Objects of this class are intended to be constructed via the new
*** operator. The copy constructor and copy assignment operator are kept
*** private because we do not wish to allow duplicate pointers to the same
*** texture data, as this greatly complicates the reference counting system.
***
*** \note Although this class is not abstract, typically we do not create any
*** instances of it. Instead, we create instances of its derivative classes.
*** ***************************************************************************/
class BaseImageTexture {
	friend class GameVideo;

public:
	BaseImageTexture();

	BaseImageTexture(int32 width_, int32 height_);

	BaseImageTexture(TexSheet* texture_sheet_, int32 width_, int32 height_);

	virtual ~BaseImageTexture();

	// ---------- Public members

	//! \brief A pointer to the texture sheet where the image is contained.
	TexSheet* texture_sheet;

	//! \brief The image's width and height as stored in the texture sheet, in pixels
	int32 width, height;

	//! \brief The coordiates of where the image is located in the texture sheet (in pixels)
	int32 x, y;

	/** \brief The actual uv coordinates.
	*** This is a little redundant, but saves effort on floating point calcuations.
	*** u1 and v1 are the upper-left UV coordinates, while u2 and v2 correspond to
	*** the lower-right. They are expressed in the [0.0, 1.0] range.
	**/
	float u1, v1, u2, v2;

	//! \brief True if the image should be drawn smoothed (using GL_LINEAR)
	bool smooth;

	/** \brief The number of times that this image is refereced by ImageDescriptors
	*** This is used to determine when the image may be safely deleted.
	**/
	int32 ref_count;

	// ---------- Public methods

	/** \brief Decrements the reference count by one
	*** \return True if there are no more references to the image
	**/
	bool RemoveReference()
		{ ref_count--; if (ref_count <= 0) return true; else return false; }

	//! \brief Increments the reference count by one
	void AddReference()
		{ ref_count++; }

private:
	BaseImageTexture(const BaseImageTexture& copy);
	BaseImageTexture& operator=(const BaseImageTexture& copy);
}; // class BaseImageTexture


/** ****************************************************************************
*** \brief Represents a single image that is loaded and stored in a texture sheet.
***
*** This object is intended to reperesent a texture image that was created by
*** loading an image file. The TextureManager singleton keeps a std::map of all
*** ImageTexture objects created, using the concatenated filename and tags of
*** each object as the map key. When creating a new ImageTexture object, you
*** should generally do the following:
*** 
*** -# First make sure that the filename + tags is not already located in the
***    image map in the TextureController class
*** -# Invoke the class constructor if the map search found no matching entry
*** -# Call the TextureManager to insert the new image into a texture sheet
*** -# If insertion was successful, call AddReference()
*** -# If insertion was successful, add the new object to the TextureManager's
***    image map
***
*** \note Like its base class, this class is only intended to be created via the
*** new operator, and the copy constructor and copy assignment operator are kept
*** private to avoid complex reference management requirements.
*** ***************************************************************************/
class ImageTexture : public BaseImageTexture {
public:
	ImageTexture(const std::string& filename_, const std::string& tags_, int32 width_, int32 height_);

	ImageTexture(TexSheet* texture_sheet_, const std::string& filename_, const std::string& tags_, int32 width_, int32 height_);

	virtual ~ImageTexture();

	// ---------- Public members

	/** \brief The name of the image file where this texture data was loaded from
	*** This is stored for every image file to prevent duplicate copies of the same image file data
	*** in multiple ImageTextures, and also in case for when the image data needs to be reloaded.
	*** The reload case may happen when a context change happens, such a screen resolution change or
	*** a toggle between fullscreen and windowed modes.
	**/
	std::string filename;

	/** \brief Retains various tags used to uniquely identify the image when certain properties are applied
	***
	*** Tags should always be presented in the same order, since the tag information is used in the key
	*** to lookup the image in the TextureManager's image map. The list of tags below is sorted from highest
	*** priority (should be at the beginning of the tag) to lowest priority (should be at the end of the tag).
	***
	*** -# \<T>: indicates that the image is temporary (created by the video engine itself)
	*** -# \<Xrow_ROWS>: used for multi image elements. "row" is the row number of this particular element
	***    while "ROWS" is the total number of rows of elements in the multi image
	*** -# \<Ycol_COLS>: used for multi image elements. "col" is the column number of this particular element
	***    while "COLS" is the total number of columns of elements in the multi image
	*** -# \<G>: used to indicate that this image texture has been converted to grayscale mode
	***
	*** \note The \<T> tag and multi image tags can not appear together
	*** \note The \<T> tag is likely temporary, as its need will later be replaced with procedural image classes
	*** \note Please remember to document new tags here when they are added
	**/
	std::string tags;

private:
	ImageTexture(const ImageTexture& copy);
	ImageTexture& operator=(const ImageTexture& copy);
}; // class ImageTexture : public BaseImageTexture


/** ****************************************************************************
*** \brief An abstract base class representing an image element
***
*** Image elements represent two things: a pointer to an ImageTexture, and
*** information about this element's position in relationship to other elements.
*** The purpose of this class is to allow the public image API classes
*** (StillImage, etc.) to be able to manage "composite" images consisting of
*** many different "elements". So really, this class is a facilitator in the
*** construction and management of composite images. Non-composite images
*** require only one image element.
***
*** \note This class is abstract because it does not contain any type of image
*** texture pointers, although there are methods defined to retrieve those
*** pointers from the derived classes.
***
*** \note The copy constructor and copy assignement operator ensure that the
*** reference to the image texure being used is updated appropriately. The
*** derived classes should not require the implementation of a copy constructor
*** or assignment operator, at least not for the purposes of proper reference
*** counting.
***
*** \note If the destructor finds that the image texture has no more references
*** after it decrements the reference counter, it will mark it as free on the
*** image texture's texture sheet. However it will <b>not</b> attempt to remove
*** it from any container in the TextureManager class, so that is up to the
*** destructors of the derived classes to implement.
*** ***************************************************************************/
class BaseImageElement {
	friend class AnimatedImage;

public:
	//! \brief Constructor that set the element to have all white vertices and disables blending
	BaseImageElement(float width_, float height_, float x_offset_, float y_offset_,
		float u1_, float v1_, float u2_, float v2_);

	BaseImageElement(float width_, float height_, float x_offset_, float y_offset_,
		float u1_, float v1_, float u2_, float v2_, Color color_[4]);

	virtual ~BaseImageElement();

	BaseImageElement(const BaseImageElement& copy);

	BaseImageElement& operator=(const BaseImageElement& copy);

	// ---------- Public members

	//! \brief The dimenions of the imag element, in coordinate system units
	float width, height;

	//! \brief The draw position offsets of the element
	float x_offset, y_offset;

	/** \brief The texture coordinates for the image element
	*** (u1, v1) represents the upper-left corner while (u2, v2) represents
	*** the bottom-right corner.
	**/
	float u1, v1, u2, v2;

	//! \brief The colors of each of the four image vertices (tl, tr, bl, br)
	Color color[4];

	//! \brief True indicates to perform blending with this element
	bool blend;

	//! \brief Set to true if all vertices are the same color
	bool unichrome_vertices;

	//! \brief Set to true if all vertices are white in color
	bool white_vertices;

	//! \brief A pointer to the element's texture image, or NULL if it does not have one
	BaseImageTexture* base_image;

	// ---------- Public methods

	/** \brief Draws the image element at the appropriate position
	*** \param color_array A pointer to a 4-element array of vertex colors to draw (default value == NULL)
	***
	*** The reason this function takes a color_array pointer is to allow support for modulation
	*** of colors. In other words, this allows us to fade images to blackness or alter the
	*** amount of transparency. When no modulation is taking place, this function should be
	*** called with no argument, which will then cause the element to use its own vertex
	*** colors to draw the element.
	**/
	void Draw(const Color* color_array = NULL) const;
}; // class BaseImageElement


/** ****************************************************************************
*** \brief Represents a single image element within an StillImage object
***
*** This class is used soley for ImageTexture data and is used to support
*** multiple image elements in the StillImage class.
*** ***************************************************************************/
class ImageElement : public BaseImageElement {
public:
	//! \brief Constructor that set the element to have all white vertices and disables blending
	ImageElement(ImageTexture* image_, float width_, float height_, float x_offset_, float y_offset_,
		float u1_, float v1_, float u2_, float v2_);

	ImageElement(ImageTexture* image_, float width_, float height_, float x_offset_, float y_offset_,
		float u1_, float v1_, float u2_, float v2_, Color color_[4]);

	// ---------- Public members

	//! \brief The texture image that is referenced by this element
	ImageTexture* image;
}; // class ImageElement : public BaseImageElement

} // namespace private_video

} // namespace hoa_video

#endif // __IMAGE_BASE_HEADER__
