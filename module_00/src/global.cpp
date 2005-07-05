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
#include "video.h"

using namespace std;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;

namespace hoa_global {

bool GLOBAL_DEBUG = false;
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

//
GCharacter::GCharacter(std::string na, std::string fn, uint id) {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GCharacter constructor invoked" << endl;
	name = na;
	filename = fn;
	char_id = id;
	
	LoadFrames();
}


// Remove all frame images upon destruction
GCharacter::~GCharacter() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GCharacter destructor invoked" << endl;
	GameVideo *VideoManager = GameVideo::_GetReference();
	for (uint i = 0; i < map_frames.size(); i++) {
		VideoManager->DeleteImage(map_frames[i]);
	}
	for (uint i = 0; i < battle_frames.size(); i++) {
		VideoManager->DeleteImage(battle_frames[i]);
	}
}


void GCharacter::LoadFrames() {
	GameVideo *VideoManager = GameVideo::_GetReference();
	ImageDescriptor imd;
	string full_name = "img/sprite/" + filename;
	
	imd.width = 1;
	imd.height = 2;
	
	imd.filename = full_name + "_d1.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_d2.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_d3.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_d4.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_d5.png";
	map_frames.push_back(imd);
	
	imd.filename = full_name + "_u1.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_u2.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_u3.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_u4.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_u5.png";
	map_frames.push_back(imd);
	
	imd.filename = full_name + "_l1.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_l2.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_l3.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_l4.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_l5.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_l6.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_l7.png";
	map_frames.push_back(imd);
	
	imd.filename = full_name + "_r1.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_r2.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_r3.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_r4.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_r5.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_r6.png";
	map_frames.push_back(imd);
	imd.filename = full_name + "_r7.png";
	map_frames.push_back(imd);
	
	for (uint i = 0; i < map_frames.size(); i++) {
		VideoManager->LoadImage(map_frames[i]);
	}
}

// ****************************************************************************
// ***************************** GameInstance *********************************
// ****************************************************************************

GameInstance::GameInstance() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameInstance constructor invoked" << endl;
	VideoManager = GameVideo::_GetReference();
}

GameInstance::~GameInstance() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameInstance destructor invoked" << endl;
	for (uint i = 0; i < characters.size(); i++) {
		delete characters[i];
	}
}

void GameInstance::AddCharacter(GCharacter *ch) { 
	if (GLOBAL_DEBUG) cout << "GLOBAL: Adding new character to party: " << ch->GetName() << endl;
	characters.push_back(ch); 
}

GCharacter* GameInstance::GetCharacter(uint id) {
	for (uint i = 0; i < characters.size(); i++) {
		if (characters[i]->GetID() == id) {
			return characters[i];
		}
	}
	if (GLOBAL_DEBUG) cerr << "GLOBAL WARNING: No character matching id #" << id << " found in party" << endl;
	return NULL;
}

 
}// namespace hoa_global
