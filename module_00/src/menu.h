/* 
 * menu.h
 *	Header file for Hero of Allacrost menu mode
 *	(C) 2005 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */
 
#ifndef __MENU_HEADER__
#define __MENU_HEADER__

#include "utils.h"
#include <string>
#include <vector>
#include "defs.h"
#include "engine.h"

namespace hoa_menu {

extern bool MENU_DEBUG;

namespace local_menu {
 
}
 
 /******************************************************************************
	MenuMode Class
		
	>>>members<<<

	>>>functions<<<

	>>>notes<<<
 *****************************************************************************/
class MenuMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;
	
	std::vector<hoa_video::ImageDescriptor> menu_images;	
	std::vector<hoa_audio::MusicDescriptor> menu_music;
	std::vector<hoa_audio::SoundDescriptor> menu_sound;
public: 
	MenuMode();
	~MenuMode();
	
	void Update(Uint32 time_elapsed);
	void Draw();
};


} // namespace hoa_menu

#endif
