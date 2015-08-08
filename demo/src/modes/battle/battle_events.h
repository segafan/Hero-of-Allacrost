////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_events.h
*** \author  Jacob Rudolph, rujasu@allacrost.org
*** \brief   Header file for battle events.
***
*** This file contains the header for special events that occur in BattleMode.
*** ***************************************************************************/

#ifndef __BATTLE_EVENTS_HEADER__
#define __BATTLE_EVENTS_HEADER__

#include "global.h"
#include "system.h"
#include "script.h"

namespace hoa_battle {

class BattleEvent {
public:
	BattleEvent(uint32 id);

	virtual ~BattleEvent();

	//! \brief Script Function Accessors
	//@{
	ScriptObject* GetBeforeFunction()   { return _before; }
	ScriptObject* GetDuringFunction()   { return _during; }
	ScriptObject* GetAfterFunction()    { return _after; }
	//@}

private:
	//! \brief unique ID number of event
	uint32 _id;

	//! \brief The name of the event
	hoa_utils::ustring _name;

	//! \brief Script that executes at beginning of battle
	ScriptObject* _before;
	//! \brief Script that executes on calls to BattleMode Update function
	ScriptObject* _during;
	//! \brief Script that executes when battle is won or lost
	ScriptObject* _after;

	hoa_system::SystemTimer* _timer;
}; // class BattleEvent

} // namespace hoa_battle

#endif // __BATTLE_EVENTS_HEADER__
