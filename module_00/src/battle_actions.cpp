///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    battle_actions.cpp
 * \author  Corey Hoffstein, visage@allacrost.org
 * \date    Last Updated: December 20, 2005
 * \brief   Header file for battle mode interface.
 *
 * Declares skill actions that may be used to customize skills
 *****************************************************************************/

#include "battle_actions.h"
#include "global.h"

using namespace hoa_global;
using namespace std;

namespace hoa_battle {

/*
	BATTLEACTION


*/
void BattleAction::Initialize(GlobalSkill *sk, Actor *a, std::vector<Actor *> arguments) {
	_skill = sk;
	_host = a;
	_arguments = arguments;
}

/* 

	MOVEACTION

*/
int MoveAction::GetX() const{
	return _moveX;
}

int MoveAction::GetY() const{
	return _moveY;
}

/*

	MOVERELATIVETOCURRENTLOCATION

*/

void MoveRelativeToCurrentLocation::Update(uint32 dt) {
	//move relative to the current location
}

/*

	MOVERELATIVETOORIGIN

*/

void MoveRelativeToOrigin::Update(uint32 dt) {
	//move relative to our original location
}

/*

	MOVERELATIVETOPOSITION

*/

void MoveRelativeToPosition::Update(uint32 dt) {
	//move relative to another location
}

/*

	MOVEABSOLUTE

*/

void MoveAbsolute::Update(uint32 dt) {
}

/*

	PERFORMSKILL

*/

void PerformSkill::Update(uint32 dt) {
	//perform the skill, statistically
}

/* 

	PLAYCHARACTERANIMATION

*/

void PlayCharacterAnimation::Update(uint32 dt) {
	//have the host perform some sort of animation
}

/*

	PERFORMVISUALEFFECT

*/

void PerformVisualEffect::Update(uint32 dt) {
	//perform a visual effect
}

/*

	PERFORMAUDIOEFFECT

*/

void PerformAudioEffect::Update(uint32 dt) {
	//play an audio effect
}

/*

	DISPLAYSKILLEFFECTS

*/
void DisplaySkillEffects::Update(uint32 dt) {
	//display the effects of the skill
}

/*

	RETREATACTION

*/
void RetreatAction::Update(uint32) {
	//check the animation and see if the player is off the screen yet
}

/*

	FINISHSKILL

*/
void FinishSkill::Update(uint32) {
	//delete the skill, and set the actor into idlemode
	GetHost()->SetMode(new IdleMode(GetHost()));
}

}

