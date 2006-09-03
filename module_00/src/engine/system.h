////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   system.h
*** \author Tyler Olsen, roots@allacrost.org
*** \brief  Header file for system code management
*** 
*** The system code handles a wide variety of tasks, including timing, threads
*** and timed functions.
*** ***************************************************************************/

#ifndef __SYSTEM_HEADER__
#define __SYSTEM_HEADER__

#include "utils.h"
#include "defs.h"

//! All calls to the system engine are wrapped in this namespace.
namespace hoa_system {

//! The singleton pointer responsible for managing the system during game operation.
extern GameSystem *SystemManager;

//! Determines whether the code in the hoa_system namespace should print debug statements or not.
extern bool SYSTEM_DEBUG;


//! An internal namespace to be used only within the system code.
namespace private_system {

/** \brief An array containing two-character language identifier strings that the game supports
***
**/
// const std::string[] SUPPORTED_LANGUAGUES = {
// 	"en", // American English
// 	"de", // German
// 	"sp"  // Spanish
// }; // SUPPORTED_LANGUAGES

} // namespace private_system


/** ****************************************************************************
*** \brief Manages system information and functions
***
*** The GameModeManager class keeps a stack of GameMode objects, where the object
*** on the top of the stack is the active GameMode (there can only be one active
*** game mode at any time). The virtual Update() and Draw() functions are invoked
*** on the game mode that is on the top of the stack.
***
*** \note 1) This class is a singleton.
*** ***************************************************************************/
class GameSystem {
public:
	SINGLETON_METHODS(GameSystem);

	/** \brief Initializes the timers used in the game.
	*** 
	*** This function should only be called \b once in main.cpp, just before the main game loop begins.
	**/
	void InitializeTimers();
	//! Updates the game time variables.
	void UpdateTimers();
	
	/** \brief Retrieves the amount of time that the game should be updated by for time-based movement.
	*** This function should \b only be called in the main game loop, located in main.cpp.
	*** \return The number of milliseconds that have transpired since the last update.
	*** 
	*** \note There's a chance we could get errors in other parts of the program code if the
	*** value returned by this function is zero. We can prevent this if we always make sure the
	*** function returns at least one, but I'm not sure there exists a computer fast enough
	*** that we have to worry about it.
	**/

	const uint32 GetUpdateTime() const
		{ return _update_time; }

	/** \brief Sets the play-time of a game instance
	*** \param h The amount of hours to set.
	*** \param m The amount of minutes to set.
	*** \param s The amount of seconds to set.
	*** 
	*** This function is meant to be called whenever the user loads a saved game.
	**/
	void SetPlayTime(const uint8 h, const uint8 m, const uint8 s)
		{ _hours_played = h; _minutes_played = m; _seconds_played = s; _milliseconds_played = 0; }

	/** \brief Functions for retrieving the play time.
	*** \return The number of hours, minutes, or seconds of play time.
	**/
	//@{
	const uint8 GetPlayHours() const
		{ return _hours_played; }
	const uint8 GetPlayMinutes() const
		{ return _minutes_played; }
	const uint8 GetPlaySeconds() const
		{ return _seconds_played; }
	//@}
	
	/** \brief  Used to determine what language the game is running in.
	*** \return The language that the game is running in.
	**/
	const std::string GetLanguage() const
		{ return _language; }
		
	/** \brief Sets the language that the game should use.
	*** \param lang A two-character string representing the language to execute the game in
	**/
	void SetLanguage(std::string lang);
		
	/** \brief  Determines whether the user is done with the game.
	*** \return False if the user would like to exit the game.
	**/
	const bool NotDone() const
		{ return _not_done; }
		
	/** \brief The function to call to initialize the exit process of the game.
	*** \note  The game will exit the main loop once it reaches the end of its current iteration
	**/
	void ExitGame() 
		{ _not_done = false; }

private:
	SINGLETON_DECLARE(GameSystem);
	//! The last time that the UpdateTimers function was called, in milliseconds.
	uint32 _last_update;
	//! The number of milliseconds that have transpired on the last timer update.
	uint32 _update_time;

	/** \name  Play time variables
	*** \brief Timers that retain the total amount of time that the user has been playing
	*** When the player starts a new game or loads an existing game, these timers are reset.
	**/
	//@{
	uint8 _hours_played;
	uint8 _minutes_played;
	uint8 _seconds_played;
	//! \note Milliseconds are not retained when saving or loading a saved game file.
	uint16 _milliseconds_played;
	//@}

	//! \brief When this member is set to false, the program will exit.
	bool _not_done;
	/** \brief The language in which to render text.
	*** \note  This string will always be two characters wide.
	**/
	std::string _language;

}; // class GameSettings

} // namepsace hoa_settings

#endif
