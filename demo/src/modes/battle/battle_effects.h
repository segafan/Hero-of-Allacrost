////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2009 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_effects.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for battle actor effects.
***
*** This file contains the code that manages effects that influence an actor's
*** behavior and properties.
*** ***************************************************************************/

#ifndef __BATTLE_EFFECTS_HEADER__
#define __BATTLE_EFFECTS_HEADER__

#include "global_effects.h"
#include "script.h"
#include "system.h"

namespace hoa_battle {

class BattleStatusEffect : public hoa_global::GlobalStatusEffect {
public:
	/** \param type The status type that this class object should represent
	*** \param intensity The intensity of the status (default value == GLOBAL_INTENSITY_NEUTRAL)
	**/
	BattleStatusEffect(hoa_global::GLOBAL_STATUS type, hoa_global::GLOBAL_INTENSITY intensity = hoa_global::GLOBAL_INTENSITY_NEUTRAL);

	~BattleStatusEffect();

	//! \brief Resets and starts the status effect's timer
	void StartTimer()
		{ _timer.Reset(); _timer.Run(); }

	/** \brief Sets how long the timer should run for
	*** \param n The number of milliseconds that the timer should run for
	**/
	void SetDuration(uint32 n)
		{ _timer.Initialize(n); }

	//! \brief Class Member Access Functions
	//@{
	hoa_global::GLOBAL_INTENSITY GetIntensity() const
		{ return _intensity; }

	hoa_system::SystemTimer& GetTimer()
		{ return _timer; }

	ScriptObject* GetUpdateFunction()
		{ return _update; }
	//@}

private:
	//! \brief A timer used to determine how long the status effect lasts
	hoa_system::SystemTimer _timer;

	//! \brief A pointer to the update script function
	ScriptObject* _update;
}; // class BattleStatusEffect : public hoa_global::GlobalStatusEffect

} // namespace hoa_battle

#endif // __BATTLE_EFFECTS_HEADER__
