/* 
 * global.cpp
 *	Hero of Allacrost global game class code
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */

#include <iostream>
#include "global.h"

using namespace std;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;

namespace hoa_global {

SINGLETON_INITIALIZE(GameInstance);

// ****************************************************************************
// ******************************* GObject ************************************
// ****************************************************************************

GObject::GObject(string name, unsigned char type, unsigned int usable, int id, int count) {
	obj_name = name;
	obj_type = type;
	usable_by = usable;
	obj_id = id;
	obj_count = count;
}



GObject::GObject() {
	obj_name = "unknown";
	obj_type = GLOBAL_DUMMY_OBJ;
	usable_by = GLOBAL_NO_CHARACTERS;
	obj_id = -1;
	obj_count = 0;
}



GObject::~GObject() {
	obj_count = 0;
}

// ****************************************************************************
// ******************************** GItem *************************************
// ****************************************************************************

GItem::GItem(string name, unsigned char use, unsigned int usable, int id, int count) :
	GObject(name, GLOBAL_ITEM, usable, id, count) {
	use_case = use;
}



GItem::GItem() : 
	GObject() {
	use_case = GLOBAL_UNUSABLE_ITEM;
}



GItem::~GItem() {}

// ****************************************************************************
// ****************************** GSkillBook **********************************
// ****************************************************************************

GSkillBook::GSkillBook(string name, unsigned char use, unsigned int usable, int id, int count) :
	GObject(name, GLOBAL_SKILL_BOOK, usable, id, count) {
}



GSkillBook::GSkillBook() : 
	GObject() {
	obj_type = GLOBAL_SKILL_BOOK;
}



GSkillBook::~GSkillBook() {}

// ****************************************************************************
// ******************************* GWeapon ************************************
// ****************************************************************************

GWeapon::GWeapon(string name, unsigned int usable, int id, int count) :
	GObject(name, GLOBAL_WEAPON, usable, id, count) {
}



GWeapon::GWeapon() : 
	GObject() {
	obj_type = GLOBAL_WEAPON;
}



GWeapon::~GWeapon() {}

// ****************************************************************************
// ******************************** GArmor ************************************
// ****************************************************************************

GArmor::GArmor(string name, unsigned char type, unsigned int usable, int id, int count) :
	GObject(name, type, usable, id, count) {
}



GArmor::GArmor() : 
	GObject() {
}



GArmor::~GArmor() {}

// ****************************************************************************
// ******************************** GSkill ************************************
// ****************************************************************************

GSkill::GSkill(string name, unsigned int sp) {
	skill_name = name;
	sp_usage = sp;
}



GSkill::GSkill() {
	skill_name = "unknown";
	sp_usage = 0;
}



GSkill::~GSkill() {}

// ****************************************************************************
// ****************************** GAttackPoint ********************************
// ****************************************************************************

GAttackPoint::GAttackPoint(float x, float y, int def, int eva, unsigned char elem_weak, 
                           unsigned char elem_res, unsigned char stat_weak, unsigned char stat_res) {
	x_position = x;
	y_position = y;
	defense = def;
	evade = eva;
	elemental_weakness = elem_weak;
	elemental_resistance = elem_res;
	status_weakness = stat_weak;
	status_resistance = stat_res;
}



GAttackPoint::GAttackPoint() {
	x_position = 0;
	y_position = 0;
	defense = 0;
	evade = 0;
	elemental_weakness = GLOBAL_NO_ELEMENTAL;
	elemental_resistance = GLOBAL_NO_ELEMENTAL;
	status_weakness = GLOBAL_NO_STATUS;
	status_resistance = GLOBAL_NO_STATUS;
}



GAttackPoint::~GAttackPoint() {}

// ****************************************************************************
// ******************************** GEnemy ************************************
// ****************************************************************************



// ****************************************************************************
// ****************************** GCharacter **********************************
// ****************************************************************************


// ****************************************************************************
// ***************************** GameInstance *********************************
// ****************************************************************************

GameInstance::GameInstance() {}

GameInstance::~GameInstance() {}

 
}// namespace hoa_global
