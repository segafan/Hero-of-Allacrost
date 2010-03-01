////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_indicators.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for battle indicator displays.
***
*** This code contains the implementation of various indicators and indicator
*** supporting classes. Indicators are small images and text that appear
*** alongside battle sprites to inform the player about status changes such as
*** damage, healing, and elemental or status effects.
*** ***************************************************************************/

#ifndef __BATTLE_INDICATORS_HEADER__
#define __BATTLE_INDICATORS_HEADER__

#include "defs.h"
#include "utils.h"

#include "battle_utils.h"

namespace hoa_battle {

namespace private_battle {

//! \brief The total amount of time (in milliseconds) that the display sequence lasts for indicator elements
const uint32 INDICTATOR_TIME = 5000;

//! \brief The amount of time (in milliseconds) that indicator elements fade at the beginning and end of the display sequence
const uint32 INDICATOR_FADE_TIME = 1000;

//! \brief The total vertical distance that indictor elements travel during the display sequence
const float INDICATOR_POSITION_CHANGE = 100.0f; // TODO: need to think of a better name for this constant


/** ****************************************************************************
*** \brief An abstract class for displaying information about a change in an actor's state
***
*** Indicators are text or graphics that appear next to actor sprites in battle.
*** They typically represent changes to the actor such as numeric text representing
*** damage or healing, icons representing status effects, etc.
***
*** \note Indicators are drawn at different orientations for different actors. For
*** example, indicator elements and draw to the left of character actors and to
*** the right for enemy actors.
*** ***************************************************************************/
class IndicatorElement {
public:
	//! \param actor A valid pointer to the actor object this indicator
	IndicatorElement(BattleActor* actor);

	virtual ~IndicatorElement()
		{}

	//! \brief Begins the display of the indicator element
	void Start();

	//! \brief Returns a floating point value that represents the height of the element drawn
	virtual float ElementHeight() const = 0;

	//! \brief Draws the indicator information to the screen
	virtual void Draw() = 0;

	/** \brief Calculates the standard alpha (transparency) value for drawing the element
	***
	*** Calling this function will set the alpha value of the _alpha_color member. Indicator elements
	*** generally fade in and fade out to make their appearance more seamless on the battle field.
	*** Alpha gradually increases from 0.0f to 1.0f in the first stage, remains at 1.0f for a majority
	*** of the time, then gradually decreases back to 0.0f as the display finishes.
	**/
	void CalculateDrawAlpha();

	//! \brief Returns true when the indicator element has expired and should be removed
	bool IsExpired() const
		{ return _timer.IsFinished(); }

	//! \name Class member accessor methods
	//@{
	const BattleActor* GetActor() const
		{ return _actor; }

	const hoa_system::SystemTimer& GetTimer() const
		{ return _timer; }
	//@}

protected:
	//! \brief The actor that the indicator element
	BattleActor* _actor;

	//! \brief Used to monitor the display progress
	hoa_system::SystemTimer _timer;

	//! \brief A modulation color used to modify the alpha (transparency) of the drawn element
	hoa_video::Color _alpha_color;
}; // class IndicatorElement


/** ****************************************************************************
*** \brief Displays an item of text next to an actor
***
*** Text indicators are normally used to display numeric text representing the
*** amount of damage dealt to the actor or the amount of healing performed. Another
*** common use is to display the word "Miss" when the actor is a target for a skill
*** that did not connect successfully. The style of the rendered text can also be
*** varied and is typically used for drawing text in different colors such as red
*** for damage and green for healing. The text size may be made larger to indicate
*** more powerful or otherwise significant changes as well.
*** ***************************************************************************/
class IndicatorText : public IndicatorElement {
public:
	/** \param actor A valid pointer to the actor object this indicator
	*** \param text The text to use to render the text image
	*** \param style The style to use to render the text image
	**/
	IndicatorText(BattleActor* actor, std::string& text, hoa_video::TextStyle& style);

	~IndicatorText()
		{}

	//! \brief Returns the height of the rendered text image
	float ElementHeight() const
		{ return _text_image.GetHeight(); }

	//! \brief Draws the text image
	void Draw();

protected:
	//! \brief The rendered image of the text to display
	hoa_video::TextImage _text_image;
}; // class IndicatorText


/** ****************************************************************************
*** \brief Manages all indicator elements for an actor
***
*** Text indicators are normally used to display numeric text representing the
*** amount of damage dealt to the actor or the amount of healing performed. Another
*** common use is to display the word "Miss" when the actor is a target for a skill
*** that did not connect successfully. The style of the rendered text can also be
*** varied and is typically used for drawing text in different colors such as red
*** for damage and green for healing. The text size may be made larger to indicate
*** more powerful or otherwise significant changes as well.
*** ***************************************************************************/
class IndicatorSupervisor {
public:
	//! \param actor A valid pointer to the actor object that this class is responsible for
	IndicatorSupervisor(BattleActor* actor);

	~IndicatorSupervisor();

	//! \brief Processes the two FIFO queues
	void Update();

	//! \brief Draws all elements present in the active queue
	void Draw();

	/** \brief Creates indicator text representing a numeric amount of damage dealt
	*** \param amount The amount of damage to display, in hit points. Should be non-zero.
	***
	*** This function will not actually cause any damage to come to the actor (that is, the actor's
	*** hit points are not modified by this function). The degree of damage relative to the character's
	*** maximum hit points determines the color and size of the text rendered.
	**/
	void AddDamageIndicator(uint32 amount);

	/** \brief Creates indicator text representing a numeric amount of healing dealt
	*** \param amount The amount of healing to display, in hit points. Should be non-zero.
	***
	*** This function will not actually cause any healing to come to the actor (that is, the actor's
	*** hit points are not modified by this function). The degree of healing relative to the character's
	*** maximum hit points determines the color and size of the text rendered.
	**/
	void AddHealingIndicator(uint32 amount);

	/** \brief Creates indicator text showing a miss on the actor
	*** Miss text is always drawn with the same style in a small font with white text
	**/
	void AddMissIndicator();

private:
	//! \brief A pointer to the actor that this class supervises indicator elements for
	BattleActor* _actor;

	//! \brief A FIFO queue container of elements that are waiting to be started and added to the active elements container
	std::deque<IndicatorElement*> _wait_queue;

	//! \brief A FIFO queue container of all elements that have begun and are going through their display sequence
	std::deque<IndicatorElement*> _active_queue;
}; // class IndicatorSupervisor

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_INDICATORS_HEADER__
