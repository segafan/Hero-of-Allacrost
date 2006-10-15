////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_objects.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for global game objects.
*** ***************************************************************************/

#include <iostream>

#include "utils.h"
#include "video.h"
#include "script.h"

#include "global.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;

namespace hoa_global {

// ****************************************************************************
// ***** GlobalObject Class
// ****************************************************************************

GlobalObject::GlobalObject(uint32 id, uint32 count)
{
	if (id == 0) {
		if (GLOBAL_DEBUG) cerr << "WARNING: GlobalObject constructor called with invalid ID member (0)" << endl;
		_id = 0;
		_type = 0;
		_usable_by = 0;
		_count = 0;
		_name = 0;
		_icon_path = "";
		return;
	}
	
	_id = id;
	_count = count;

	// The remaining data members are set by reading in the object's data contained within a Lua file
	// TODO: Waiting on support from script manager
// 	_type = ;
// 	_usable_by = ;
// 	_name = ;
// 	_icon_path = ;
} // GlobalObject::GlobalObject(uint32 id, uint32 count)

// ****************************************************************************
// ***** GlobalItem Class
// ****************************************************************************

GlobalItem::GlobalItem(uint32 id, uint32 count) : GlobalObject(id, count)
{
	// TEMP: there is only one type of item
	_type = GLOBAL_ITEM;
	_name = MakeUnicodeString("Healing Potion");
	_icon_path = "img/icons/items/healing_potion.png";
	_icon_image.SetFilename("img/icons/items/healing_potion.png");
	if (_icon_image.Load() == false) {
		cerr << "ERROR: In GlobalWeapon constructor, failed to load icon image" << endl;
	}
	_usage = 0;
	// End TEMP

	// TODO: use the id to load the item properties from a Lua file
}

// ****************************************************************************
// ***** GlobalWeapon Class
// ****************************************************************************

GlobalWeapon::GlobalWeapon(uint32 id, uint32 count) : GlobalObject(id, count)
{
	// TEMP: there is only one type of weapon
	_type = GLOBAL_WEAPON;
	_name = MakeUnicodeString("Karlate Sword");
	_icon_path = "img/icons/inventory/karlate_sword.png";
	_icon_image.SetFilename("img/icons/weapons/karlate_sword.png");
	if (_icon_image.Load() == false) {
		cerr << "ERROR: In GlobalWeapon constructor, failed to load icon image" << endl;
	}
	// TODO: use the id to load the weapon properties from a Lua file
} // GlobalWeapon::GlobalWeapon(uint32 id, uint32 count)

// ****************************************************************************
// ***** GlobalArmor Class
// ****************************************************************************

GlobalArmor::GlobalArmor(uint32 id, uint8 type, uint32 count) : GlobalObject(id, count)
{
	// TEMP: there is only one type of each class of armor
	_usable_by = GLOBAL_CHARACTER_CLAUDIUS;
	if (type == GLOBAL_HEAD_ARMOR) {
		_name = MakeUnicodeString("Karlate Helmet");
		_icon_path = "img/icons/armor/karlate_helmet.png";
		_icon_image.SetFilename("img/icons/armor/karlate_helmet.png");
		if (_icon_image.Load() == false) {
			cerr << "ERROR: In GlobalWeapon constructor, failed to load icon image" << endl;
		}
	}
	else if (type == GLOBAL_TORSO_ARMOR) {
		_name = MakeUnicodeString("Karlate Chest Guard");
		_icon_path = "img/icons/inventory/karlate_chest_guard.png";
		_icon_image.SetFilename("img/icons/armor/karlate_chest_guard.png");
		if (_icon_image.Load() == false) {
			cerr << "ERROR: In GlobalWeapon constructor, failed to load icon image" << endl;
		}
	}
	else if (type == GLOBAL_ARMS_ARMOR) {
		_name = MakeUnicodeString("Karlate Gauntlets");
		_icon_path = "img/icons/inventory/karlate_gauntlets.png";
		_icon_image.SetFilename("img/icons/armor/karlate_gauntlets.png");
		if (_icon_image.Load() == false) {
			cerr << "ERROR: In GlobalArmor constructor, failed to load icon image" << endl;
		}
	}
	else if (type == GLOBAL_LEGS_ARMOR) {
		_name = MakeUnicodeString("Karlate Greaves");
		_icon_path = "img/icons/inventory/karlate_greaves.png";
		_icon_image.SetFilename("img/icons/armor/karlate_greaves.png");
		if (_icon_image.Load() == false) {
			cerr << "ERROR: In GlobalArmor constructor, failed to load icon image" << endl;
		}
	}
	else {
		cerr << "ERROR: GlobalArmor constructor received bad type in constructor" << endl;
		return;
	}
	
	// TODO: use the id to load the armor properties from a Lua file
} // GlobalArmor::GlobalArmor(uint32 id, uint8 type, uint32 count)

} // namespace hoa_global
