///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    texture_controller.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for texture management code
***
*** This code declares a single class, TextureController, which manages all
*** texture sheets in use by the game. This class is a singleton, and it is
*** essentially an extension of the GameVideo class.
*** ***************************************************************************/

#ifndef __TEXTURE_CONTROLLER_HEADER__
#define __TEXTURE_CONTROLLER_HEADER__

#include "defs.h"
#include "utils.h"
#include "texture.h"
#include "image_base.h"

// OpenGL includes
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

namespace hoa_video {

//! \brief The singleton pointer for the instance of the texture controller
extern TextureController* TextureManager;

class TextureController : public hoa_utils::Singleton<TextureController> {
	friend class hoa_utils::Singleton<TextureController>;
	friend class GameVideo;
	friend class private_video::ImageMemory;
	friend class ImageDescriptor;
	friend class StillImage;
	friend class private_video::ImageTexture;
	friend class private_video::TextImageTexture;
	friend class private_video::BaseImageElement;
	friend class TextSupervisor;
	friend class RenderedText;
	friend class private_video::TexSheet;
	friend class private_video::FixedTexMemMgr;
	friend class private_video::VariableTexMemMgr;
	friend class private_video::ParticleSystem;

public:
	TextureController();

	bool SingletonInitialize();

	/** \brief Unloads all texture sheets from memory 
	*** \return True only if all texture sheets were successfully unloaded
	***
	*** This leaves the lists of images intact so that they can be reloaded.
	*** This function is typically invoked when the GL context is changed, so
	*** that textures may be properly reloaded after the new context has been
	*** applied.
	**/
	bool UnloadTextures();

	/** \brief Reloads all texture sheets that have been unloaded
	*** \return True only if all texture sheets were successfully reloaded
	***
	*** This is typically called after a GL context change has occurred.
	**/
	bool ReloadTextures();

	//! \brief Cycles forward to show the next texture sheet
	void DEBUG_NextTexSheet();

	//! \brief Cycles backward to show the previous texture sheet
	void DEBUG_PrevTexSheet();

private:
	~TextureController();

	//! \brief The ID of the last texture that was bound. Used to eliminate redundant binding of textures
	GLuint _last_tex_id;

	//! \brief A vector containing all of the texture sheets currently being managed by this class
	std::vector<private_video::TexSheet*> _tex_sheets;

	//! \brief A STL map containing all of the images currently being managed by this class
	std::map<std::string, private_video::ImageTexture*> _images;

	//! \brief A STL set containing all of the text images currently being managed by this class
	std::set<private_video::TextImageTexture*> _text_images;

	//! \brief An index to _tex_sheets of the current texture sheet being shown in debug mode. -1 indicates no sheet
	int32 _debug_current_sheet;

	//! \brief Keeps track of the number of texture switches per frame
	uint32 _debug_num_tex_switches;

	// ---------- Private methods

	//! \name Texture Operations
	//@{
	/** \brief Creates a blank texture of the given width and height and returns integer used by OpenGL to refer to this texture. Returns 0xffffffff on failure.
	*** \param width The desired width of the texture
	*** \param height The desired height of the texture
	*** \return The OpenGL ID for this texture, or INVALID_TEXTURE_ID if the texture could not be created
	**/
	GLuint _CreateBlankGLTexture(int32 width, int32 height);

	/** \brief A wrapper to glBindTexture() that also adds checking to eliminate redundant texture binding
	*** \param tex_id The integer handle to the OpenGL texture to bind
	*** \note Redundancy checks are already implemented by most drivers, but this is a double check "just in case"
	**/
	void _BindTexture(GLuint tex_id);

	/** \brief A wrapper to glDeleteTextures() that also adds checking to eliminate redundant texture binding 
	*** \param tex_id The integer handle to the OpenGL texture to delete
	 */
	void _DeleteTexture(GLuint tex_id);

	/** \brief Saves all temporary textures (textures not loaded from a file) to disk
	*** \return True only if all temporary textures were successfully saved to a file
	***
	*** This is used when the GL context is being destroyed, perhaps because we are
	*** switching from windowed to fullscreen. We must save all textures to disk so
	*** that we can reload them after the new GL context is created.
	**/
	bool _SaveTempTextures();

	/** \brief Deletes any temporary textures that were saved in the "img/temp" directory
	*** \return True if the "img/temp" directory was successfully emptied
	**/
	bool _DeleteTempTextures()
		{ return hoa_utils::CleanDirectory("img/temp"); }
	//@}

	//! \name Texture Sheet Operations
	//@{
	/** \brief Creates a new texture sheet
	*** \param width The width of the sheet, in pixels
	*** \param height The height of the sheet, in pixels
	*** \param type Specifies what type of images this texture sheet manages (e.g. 32x32 images, 64x64 images, variable size, etc)
	*** \param is_static If true, this texture sheet is meant to manage images which are not expected to be loaded and unloaded very often
	*** \return A pointer to the newly created TexSheet, or NULL if a new should could not be created
	**/
	private_video::TexSheet* _CreateTexSheet(int32 width, int32 height, private_video::TexSheetType type, bool is_static);

	/** \brief Removes references to a texture sheet and deletes it from memory
	*** \param sheet A pointer to the sheet we wish to remove
	**/
	void _RemoveSheet(private_video::TexSheet* sheet);

	/** \brief Inserts an image into a compatible texture sheet
	*** \param image A pointer to the image to insert
	*** \param load_info The attributes of the image to be inserted
	*** \param is_static Indicates whether the image is static or not
	*** \return A new texsheet with the image contained within it, or NULL if an error occured and the image could not be added to any sheet
	***
	*** A new texture sheet will be created by this function in one of two cases. First, if there was no room for the image in any existing
	*** compatible texture sheets. Second, if the image is very large (either height or width of the image exceeds 512 pixels), it will
	*** merit having its own un-shared texture sheet.
	**/
	private_video::TexSheet* _InsertImageInTexSheet(private_video::BaseImageTexture* image, private_video::ImageMemory& load_info, bool is_static);

	/** \brief Iterate through all currently loaded images and if they belong to the specified TexSheet, reload them into it
	*** \param sheet A pointer to the TexSheet whose images we wish to reload
	*** \return True only if every single image owned by the TexSheet was successfully reloaded back into it
	**/
	bool _ReloadImagesToSheet(private_video::TexSheet* sheet);
	//@}

	//! \name Image Operations
	//@{

	//@}

	//! \name Text Image Operations
	//@{
	/** \brief Determines if a TextImageTexture is already registered and maintainted by the texture controller
	*** \param img A pointer to the TextImageTexture to check for
	*** \return True if the TextImageTexture is already registered, false if it is not
	**/
	bool _IsTextImageRegistered(private_video::TextImageTexture* img)
		{ if (_text_images.find(img) == _text_images.end()) return false; else return true; }

	/** \brief Registers a TextImageTexture to be managed by the internal set
	*** \param img A pointer to the TextImageTexture to register
	*** \note The function make sure that it does not register any TextImageTexture more than once
	**/
	void _RegisterTextImage(private_video::TextImageTexture* img);
	//@}

	/** \brief Displays the currently selected texture sheet.
	*** By using DEBUG_NextTexSheet() and DEBUG_PrevTexSheet(), you can change the current texture sheet so the sheet shown by this function
	*** cycles through all currently loaded texture sheets.
	**/
	void _DEBUG_ShowTexSheet();
}; // class TextureController : public hoa_utils::Singleton<TextureController>

} // namespace hoa_video

#endif // __TEXTURE_CONTROLLER_HEADER__
