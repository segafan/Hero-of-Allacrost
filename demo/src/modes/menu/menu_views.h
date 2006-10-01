///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_views.h
*** \author  Daniel Steuernol steu@allacrost.org, Andy Gardner chopperdave@allacrost.org
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
	enum EQUIP_CATEGORIES { EQUIP_WEAPON = 0, EQUIP_HEADGEAR = 1, EQUIP_BODYARMOR = 2,
		EQUIP_OFFHAND = 3, EQUIP_LEGGINGS = 4, EQUIP_CATEGORY_SIZE = 5 };


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

	//! Set the character for this window
	void SetCharacter(hoa_global::GlobalCharacter *character);

	/*!
	 * \brief render this window to the screen.
	 */
	bool Draw();
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
	bool Draw();
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

	/** \brief Toggles the inventory window being in the active context for the player
	*** \param new_status Activates the inventory window when true, de-activates it when false
	**/
	void Activate(bool new_status);
	/** \brief Indicates whether the inventory window is in the active context
	*** \return True if the inventory window is in the active context
	**/
	inline bool IsActive()
		{ return (_inventory_active || _char_select_active || _item_categories_active); }

	//! If the inventory window is ready to cancel out, or cancel out a sub-window
	//bool CanCancel();
	
	void Update();

	//! \brief Draw the inventory window
	//! Takes care of drawing the inventory window to the screen.
	bool Draw();

private:
	
	//! Flag to specify if the inventory window is active
	bool _inventory_active;

	//! Flag to specify if the character select window is active
	bool _char_select_active;

	//! Flag to specify if the item category select window is active
	bool _item_categories_active;

	//! OptionBox to display all of the items
	hoa_video::OptionBox _inventory_items;

	//! OptionBox to choose character
	hoa_video::OptionBox _char_select;

	//! OptionBox to choose item category
	hoa_video::OptionBox _item_categories;

	//! \brief the sounds for MenuMode
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;

	//! The MiniCharacterWindow to be shown when needed to select a character
	//MiniCharacterSelectWindow _char_window;

	//! Updates the item text in the inventory items
	void UpdateItemText();

	//! Initializes inventory items option box
	void InitInventoryItems();

	//! Initializes char select option box
	void InitCharSelect();

	//! Initializes item category select option box
	void InitCategory();

	//! FIX ME
	void ApplyItem();

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
	//! \brief char portraits
	std::vector<hoa_video::StillImage> _full_portraits;

	//! \brief the sounds for MenuMode
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;

	//! \brief the current character for this screen.
	hoa_global::GlobalCharacter *_current_char;

	//! \brief if the window is active or not
	bool _char_select_active;
	
	//! \brief character selection option box
	hoa_video::OptionBox _char_select;

	//! \brief initialize character selection option box
	void InitCharSelect();
public:

	StatusWindow();
	~StatusWindow();
	//! \brief render this window to the screen.
	bool Draw();

	//! \brief update function handles input to the window
	void Update();

	//! \brief Check if status window is active
	inline bool IsActive() { return _char_select_active; }
	//! \brief Active this window
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

	//! \brief Updates key presses and window states
	void Update();

	//! \brief Draws the windows and option boxes
	bool Draw();

	//! \brief Activates the window
	void Activate(bool new_status);

	//! \brief Checks to see if the skills window is active
	inline bool IsActive() { return _skills_categories_active || 
							_skills_list_active ||
							_char_select_active ||
							_char_select_apply_active;}

private:

	//! \brief If the character select option box is active
	bool _char_select_active;

	//! \brief If we're choosing the character to apply the skill to
	bool _char_select_apply_active;

	//! \brief If the skills categories option box is active
	bool _skills_categories_active;

	//! \brief If the skills list option box is active
	bool _skills_list_active;

	//! brief The character select option box
	hoa_video::OptionBox _char_select;

	//! brief The skills categories option box
	hoa_video::OptionBox _skills_categories;

	//! \brief The skills list option box
	hoa_video::OptionBox _skills_list;

	//! Menu sounds
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;

	//! \brief Track whihc character's skillset was chosen
	int32 _char_skillset;

	//! \brief Initializes the skills category chooser
	void InitSkillsCategories();

	//! \brief Initializes the skills chooser
	void InitSkillsList();

	//! \brief Initializes the character selector
	void InitCharSelect();

	//! \brief Sets up the skills that comprise the different categories
	void UpdateSkillList();

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

	//! \brief Draws window
	bool Draw();

	//! \brief Performs updates
	void Update();

	//! Is window active?
	inline bool IsActive() { return _char_select_active 
							|| _equip_select_active
							|| _equip_list_active; }

	//! Activate/deactivate window
	void Activate(bool new_status);

private:
	//! \brief Character selector
	hoa_video::OptionBox _char_select;
	//! \brief Equipment selector
	hoa_video::OptionBox _equip_select;
	//! \brief Replacement selector
	hoa_video::OptionBox _equip_list;

	//! \brief Char selector active
	bool _char_select_active;
	//! \brief Equipment selector active
	bool _equip_select_active;
	//! \brief Replacement selector active
	bool _equip_list_active;

	//! \brief the sounds for MenuMode
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;
	//! \brief equipment images
	std::vector<hoa_video::StillImage> _equip_images;

	//! \brief Set up char selector
	void InitCharSelect();
	//! \brief Set up equipment selector
	void InitEquipmentSelect();
	//! \brief Set up replacement selector
	void InitEquipmentList();

	//! \brief Updates the equipment list
	void UpdateEquipList();

}; // class EquipWindow : public hoa_video::MenuWindow

class FormationWindow : public hoa_video::MenuWindow {
public:
	FormationWindow();
	~FormationWindow();
	void Update();
	bool Draw();

private:

}; // class FormationWindow : public hoa_video::MenuWindow


} // namespace private_menu

} // namespace hoa_menu

#endif
