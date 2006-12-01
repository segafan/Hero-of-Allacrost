////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for the global game manager
*** ***************************************************************************/

#include <iostream>

#include "global.h"
#include "utils.h"
#include "video.h"
#include "script.h"

using namespace std;
using namespace hoa_video;
using namespace hoa_utils;

namespace hoa_global {

GameGlobal *GlobalManager = NULL;
bool GLOBAL_DEBUG = false;
SINGLETON_INITIALIZE(GameGlobal);

// ****************************************************************************
// ***** GameGlobal class - Initialization and Destruction
// ****************************************************************************

GameGlobal::GameGlobal() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameGlobal constructor invoked" << endl;
}



GameGlobal::~GameGlobal() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameGlobal destructor invoked" << endl;
	for (uint32 i = 0; i < _characters.size(); i++) {
		delete _characters[i];
	}
	
	// Clean up inventory items
	for (uint32 i = 0; i < _inventory.size(); ++i) {
		delete _inventory[i];
	}
}



bool GameGlobal::SingletonInitialize() {
	return true;
}

// ****************************************************************************
// ***** GameGlobal class - Character and Party Manipulations
// ****************************************************************************

void GameGlobal::AddCharacter(GlobalCharacter *ch) {
	_characters[ch->GetID()] = ch;
// 	if (_active_party.GetPartySize() < 4) {
// 		_active_party.AddCharacter(ch);
// 	}
}



GlobalCharacter* GameGlobal::GetCharacter(uint32 id) {
	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i] != 0 && _characters[i]->GetID() == id) {
			return _characters[i];
		}
	}

	if (GLOBAL_DEBUG) cerr << "GLOBAL WARNING: No character matching id #" << id << " found in party" << endl;
	return NULL;
}

// ****************************************************************************
// ***** GameGlobal class - Item and Inventory Manipulations
// ****************************************************************************

void GameGlobal::AddItemToInventory(GlobalObject *obj) {
	// Add object to the inventory if it is not already there
	if (_inventory.count(obj->GetID()) == 0) {
		_inventory[obj->GetID()] = obj;
	}
	// Otherwise increment the count of the object instance already in the inventory
	else {
		_inventory[obj->GetID()]->IncrementCount(obj->GetCount());
	}
}



void GameGlobal::RemoveFromInventory(GlobalObject *obj) {
	if (_inventory.count(obj->GetID()) != 0) {
		cerr << "GLOBAL ERROR: requested to remove an inventory item that was not in the inventory" << endl;
	}

	// Delete the object pointer in the inventory and then remove it from the map
	delete _inventory[obj->GetID()];
	_inventory.erase(obj->GetID());
}

} // namespace hoa_global
