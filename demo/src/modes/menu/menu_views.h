///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_views.h
*** \author  Daniel Steuernol steu@allacrost.org
*** \author  Andy Gardner chopperdave@allacrost.org
*** \brief   Header file for various menu views.
***
*** This code handles the different views that the user will see while the
*** is in menu mode, (the main in-game menu). This mode's primary objectives
*** are to allow the user to view stastics about their party and manage inventory
*** and equipment.
***
*** ***************************************************************************/
 
#ifndef __MENU_VIEWS__
#define __MENU_VIEWS__

#include <string>
#include <vector>

#include "utils.h"
#include "defs.h"

#include "video.h"
#include "gui.h"
#include "global.h"

#include "menu_views.h"

//! All menu mode calls are in this namespace
namespace hoa_menu {

//! This namespace is for private menu stuff only
namespace private_menu {

	//! \brief The different item categories
	enum ITEM_CATEGORY { ITEM_ALL = 0, ITEM_FIELD = 1, ITEM_BATTLE = 2, ITEM_EQUIPMENT = 3,
		ITEM_KEY = 4, ITEM_CATEGORY_SIZE = 5 };

	//! \brief The different skill types
	enum SKILL_CATEGORY { SKILL_ALL = 0, SKILL_FIELD = 1, SKILL_BATTLE = 2, 
		SKILL_CATEGORY_SIZE = 3 };

	//! \brief The different equipment categories
	enum EQUIP_CATEGORY { EQUIP_WEAPON = 0, EQUIP_HEADGEAR = 1, EQUIP_BODYARMOR = 2,
		EQUIP_OFFHAND = 3, EQUIP_LEGGINGS = 4, EQUIP_CATEGORY_SIZE = 5 };

	//! \brief The different option boxes that can be active for items
	enum ITEM_ACTIVE_OPTION { ITEM_ACTIVE_NONE = 0, ITEM_ACTIVE_CATEGORY = 1, 
		ITEM_ACTIVE_LIST = 2, ITEM_ACTIVE_CHAR = 3, ITEM_ACTIVE_SIZE = 4};

	//! \brief The different option boxes that can be active for skills
	enum SKILL_ACTIVE_OPTION { SKILL_ACTIVE_NONE = 0, SKILL_ACTIVE_CHAR = 1, 
		SKILL_ACTIVE_CATEGORY = 2, SKILL_ACTIVE_LIST = 3, SKILL_ACTIVE_CHAR_APPLY = 4,
		SKILL_ACTIVE_SIZE = 5};

	//! \brief The different option boxes that can be active for equipment
	enum EQUIP_ACTIVE_OPTION { EQUIP_ACTIVE_NONE = 0, EQUIP_ACTIVE_CHAR = 1, 
		EQUIP_ACTIVE_SELECT = 2, EQUIP_ACTIVE_LIST = 3, EQUIP_ACTIVE_SIZE = 4};


/** ****************************************************************************
*** \brief Represents an individual character window for the in-game menu.
***
*** There should be one of these windows for each character in the game.
*** It will contain all the information to be drawn for that character, and
*** also handles the placement of this.
*** ***************************************************************************/
class CharacterWindow : public hoa_video::MenuWindow
{
private:
	//! The name of the character that this window corresponds(sp?) to
	uint32 _char_id;
	//! The image of the character
	hoa_video::StillImage _portrait;
public:
	/*!
	 * \brief CharacterWindow Default constructor
	 */
	CharacterWindow();
	/*!
	* \brief CharacterWindow Destructor
	 */
	~CharacterWindow();

	/*!
	* \brief Set the character for this window
	* \param character the character to associate with this window
	*/
	void SetCharacter(hoa_global::GlobalCharacter *character);

	/*!
	 * \brief render this window to the screen
	 * \return success/failure
	 */
	void Draw();
}; // class CharacterWindow : public hoa_video::MenuWindow

/** ****************************************************************************
*** \brief Represents a window used to select a character to perform an action on.
***
*** There should only ever be one of these active, usually used after
*** selecting an item in the inventory, that needs a character to work
*** on.
*** ***************************************************************************/
class MiniCharacterSelectWindow : public hoa_video::MenuWindow
{
public:
	//! \brief CharacterWindow main constructor
	MiniCharacterSelectWindow();
	//! \brief CharacterWindow Destructor
	~MiniCharacterSelectWindow();
	//! \brief render this window to the screen.
	void Draw();
	//! \brief change the active status of the window
	void Activate(bool new_status);
	bool IsActive()
		{ return _char_window_active; }

	//! \brief Selected Index accessor
	//@{
	void SetSelectedIndex(uint32 selected)
		{ _selected_item_index = selected; }
	uint32 GetSelectedIndex()
		{ return _selected_item_index; }
	//@}


	void Update();

private:
	//! \brief specifies if the char select window is active
	bool _char_window_active;
	//! \brief pointer to the current character that the cursor is pointing to.
	uint32 _current_char_selected;
	//! \brief the item that was selected in the inventory.
	uint32 _selected_item_index;
	//! \brief Hide copy constructor.
	MiniCharacterSelectWindow(MiniCharacterSelectWindow &other) {}
	//! \brief the sounds for MenuMode
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;
}; // class MiniCharacterSelectWindow : public hoa_video::MenuWindow

/** ****************************************************************************
*** \brief Represents the inventory window to browse the party's inventory
***
*** This handles item use.  You can also view all items by category.
*** ***************************************************************************/
class InventoryWindow : public hoa_video::MenuWindow
{
public:
	InventoryWindow();
	~InventoryWindow();

	/*!
	* \brief Toggles the inventory window being in the active context for the player
	* \param new_status Activates the inventory window when true, de-activates it when false
	*/
	void Activate(bool new_status);
	
	/*!
	* \brief Indicates whether the inventory window is in the active context
	* \return True if the inventory window is in the active context
	*/
	inline bool IsActive()
		{ return _active_box; }

	//! If the inventory window is ready to cancel out, or cancel out a sub-window
	//bool CanCancel();
	
	/*!
	* \brief Updates the inventory window.  Handles key presses, switches window context, etc.
	*/
	void Update();

	/*!
	* \brief Draw the inventory window
	* \return success/failure
	*/
	void Draw();

private:
	
	//! Used for char portraits in bottom menu
	std::vector<hoa_video::StillImage> _portraits;

	//! Used for the current dungeon
	hoa_video::StillImage _location_picture;

	//! Flag to specify the active option box
	uint32 _active_box;

	//! OptionBox to display all of the items
	hoa_video::OptionBox _inventory_items;

	//! OptionBox to choose character
	hoa_video::OptionBox _char_select;

	//! OptionBox to choose item category
	hoa_video::OptionBox _item_categories;

	//! \brief the sounds for MenuMode
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;

	/*!
	* \brief Updates the item text in the inventory items
	*/
	void _UpdateItemText();

	/*!
	* \brief Initializes inventory items option box
	*/
	void _InitInventoryItems();

	/*!
	* \brief Initializes char select option box
	*/
	void _InitCharSelect();

	/*!
	* \brief Initializes item category select option box
	*/
	void _InitCategory();

	/*!
	* \brief Draw contents of bottom menu
	*/
	void _DrawBottomMenu();

	//! FIX ME
	void _TEMP_ApplyItem();

}; // class InventoryWindow : public hoa_video::MenuWindow

/** ****************************************************************************
*** \brief Represents the Status window, displaying all the information about the character.
***
*** This window display all the attributes of the character.
*** You can scroll through them all as well, to view all the different characters.
*** ***************************************************************************/
class StatusWindow : public hoa_video::MenuWindow
{
private:
	//! char portraits
	std::vector<hoa_video::StillImage> _full_portraits;

	//! A graphic for the location (map) that the player is currently on
	hoa_video::StillImage _location_picture;

	//! the sounds for MenuMode
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;

	//! the current character for this screen.
	hoa_global::GlobalCharacter *_current_char;

	//! if the window is active or not
	bool _char_select_active;
	
	//! character selection option box
	hoa_video::OptionBox _char_select;

	/*!
	* \brief initialize character selection option box
	*/
	void _InitCharSelect();

	/*!
	* \brief Draw contents of bottom menu
	*/
	void _DrawBottomMenu();
	
public:

	StatusWindow();
	~StatusWindow();
	
	/*!
	* \brief render this window to the screen
	* \return success/failure
	*/
	void Draw();

	/*!
	* \brief update function handles input to the window
	*/
	void Update();

	/*!
	* \brief Check if status window is active
	* \return true if the window is active, false if it's not
	*/
	inline bool IsActive() { return _char_select_active; }
	
	/*!
	* \brief Active this window
	* \param new_value true to activate window, false to deactivate window
	*/
	void Activate(bool new_value);
		
}; // class StatusWindow : public hoa_video::MenuWindow


/** ****************************************************************************
*** \brief Represents the Skills window, displaying all the skills for the character.
***
*** This window display all the skills for a particular character.
*** You can scroll through them all, filter by category, choose one, and apply it
*** to a character.
*** ***************************************************************************/

class SkillsWindow : public hoa_video::MenuWindow {
public:

	SkillsWindow();
	~SkillsWindow();

	/*!
	* \brief Updates key presses and window states
	*/
	void Update();

	/*!
	* \brief Draws the windows and option boxes
	* \return success/failure
	*/
	void Draw();

	/*!
	* \brief Activates the window
	* \param new_value true to activate window, false to deactivate window
	*/
	void Activate(bool new_status);

	/*!
	* \brief Checks to see if the skills window is active
	* \return true if the window is active, false if it's not
	*/
	inline bool IsActive() { return _active_box; }

private:

	//! Flag to specify the active option box
	uint32 _active_box;

	//! The character select option box
	hoa_video::OptionBox _char_select;

	//! The skills categories option box
	hoa_video::OptionBox _skills_categories;

	//! The skills list option box
	hoa_video::OptionBox _skills_list;

	//! Menu sounds
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;

	//! Track whihc character's skillset was chosen
	int32 _char_skillset;

	/*!
	* \brief Initializes the skills category chooser
	*/
	void _InitSkillsCategories();

	/*!
	* \brief Initializes the skills chooser
	*/
	void _InitSkillsList();

	/*!
	* \brief Initializes the character selector
	*/
	void _InitCharSelect();

	/*!
	* \brief Sets up the skills that comprise the different categories
	*/
	void _UpdateSkillList();

	/*!
	* \brief Draw contents of bottom menu
	*/
	void _DrawBottomMenu();

}; //class SkillsWindow : public hoa_video::MenuWindow


/** ****************************************************************************
*** \brief Represents the Equipment window, allowing the player to change equipment.
***
*** This window changes a character's equipment.
*** You can choose a piece of equipment and replace with an item from the given list.
*** ***************************************************************************/

class EquipWindow : public hoa_video::MenuWindow {
public:
	EquipWindow();
	~EquipWindow();

	/*!
	* \brief Draws window
	* \return success/failure
	*/
	void Draw();

	/*!
	* \brief Performs updates
	*/
	void Update();

	/*!
	* \brief Checks to see if the equipment window is active
	* \return true if the window is active, false if it's not
	*/
	inline bool IsActive() { return _active_box; }

	/*!
	* \brief Activates the window
	* \param new_value true to activate window, false to deactivate window
	*/
	void Activate(bool new_status);

private:

	//! Character selector
	hoa_video::OptionBox _char_select;
	
	//! Equipment selector
	hoa_video::OptionBox _equip_select;
	
	//! Replacement selector
	hoa_video::OptionBox _equip_list;

	//! Flag to specify the active option box
	uint32 _active_box;

	//! the sounds for MenuMode
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;
	
	//! equipment images
	std::vector<hoa_video::StillImage> _equip_images;

	/*!
	* \brief Set up char selector
	*/
	void _InitCharSelect();
	
	/*!
	* \brief Set up equipment selector
	*/
	void _InitEquipmentSelect();
	
	/*!
	* \brief Set up replacement selector
	*/
	void _InitEquipmentList();

	/*!
	* \brief Updates the equipment list
	*/
	void _UpdateEquipList();

	/*!
	* \brief Draw contents of bottom menu
	*/
	void _DrawBottomMenu();

}; // class EquipWindow : public hoa_video::MenuWindow

class FormationWindow : public hoa_video::MenuWindow {
public:
	FormationWindow();
	~FormationWindow();
	void Update();
	void Draw();

private:

}; // class FormationWindow : public hoa_video::MenuWindow


} // namespace private_menu

} // namespace hoa_menu

#endif
