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
*** \brief   Source file for battle indicator displays.
*** ***************************************************************************/

#include "system.h"
#include "video.h"

#include "global_utils.h"

#include "battle_actors.h"
#include "battle_indicators.h"
#include "battle_utils.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_system;
using namespace hoa_video;

using namespace hoa_global;

namespace hoa_battle {

namespace private_battle {

////////////////////////////////////////////////////////////////////////////////
// IndicatorElement class
////////////////////////////////////////////////////////////////////////////////

IndicatorElement::IndicatorElement(BattleActor* actor) :
	_actor(actor),
	_timer(INDICTATOR_TIME),
	_alpha_color(1.0f, 1.0f, 1.0f, 0.0f)
{
	if (actor == NULL)
		IF_PRINT_WARNING(BATTLE_DEBUG) << "constructor received NULL actor argument" << endl;
}



void IndicatorElement::Start() {
	if (_timer.IsInitial() == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "timer was not in initial state when started" << endl;
	}
	_timer.Run();
}



void IndicatorElement::Update() {
	_timer.Update();
}



void IndicatorElement::CalculateDrawAlpha() {
	if ((_timer.GetState() == SYSTEM_TIMER_RUNNING) && (_timer.GetState() == SYSTEM_TIMER_PAUSED)) {
		_alpha_color.SetAlpha(0.0f);
		return;
	}

	float alpha = 0.0f;
	// Case 1: Timer is in beginning stage and indicator graphic is fading in
	if (_timer.GetTimeExpired() < INDICATOR_FADE_TIME) {
		alpha = static_cast<float>(_timer.GetTimeExpired() / INDICATOR_FADE_TIME);
	}
	// Case 2: Timer is in final stage and indicator graphic is fading out
	else if (_timer.TimeLeft() < INDICATOR_FADE_TIME) {
		alpha = static_cast<float>(_timer.TimeLeft() / INDICATOR_FADE_TIME);
	}
	// Case 3: Timer is in middle stage and indicator graphic should be drawn with no transparency
	else {
		alpha = 1.0f;
	}

	_alpha_color.SetAlpha(alpha);
}

////////////////////////////////////////////////////////////////////////////////
// IndicatorText class
////////////////////////////////////////////////////////////////////////////////

IndicatorText::IndicatorText(BattleActor* actor, string& text, TextStyle& style) :
	IndicatorElement(actor),
	_text_image(text, style)
{}



void IndicatorText::Draw() {
	VideoManager->Move(_actor->GetXLocation(), _actor->GetYLocation());

	// Draw to the left of the sprite for characters and to the right of the sprite for enemys
	if (_actor->IsEnemy() == false) {
		VideoManager->MoveRelative(-40.0f, 0.0f); // TEMP: should move to just past the edge of the sprite image, not -40.0f
		VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	}
	else {
		VideoManager->MoveRelative(40.0f, 0.0f); // TEMP: should move to just past the edge of the sprite image, not 40.0f
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	}

	float y_position = INDICATOR_POSITION_CHANGE * _timer.PercentComplete();
	VideoManager->MoveRelative(0.0f, y_position);
	CalculateDrawAlpha();
	// TEMP: alpha modulation does not appear to be working correctly. If alpha is anything other than 1.0f, image is not drawn
// 	_text_image.Draw(_alpha_color);
	_text_image.Draw();
}

////////////////////////////////////////////////////////////////////////////////
// IndicatorImage class
////////////////////////////////////////////////////////////////////////////////

IndicatorImage::IndicatorImage(BattleActor* actor, const string& filename) :
	IndicatorElement(actor)
{
	if (_image.Load(filename) == false)
		PRINT_ERROR << "failed to load indicator image: " << filename << endl;
}



void IndicatorImage::Draw() {
	VideoManager->Move(_actor->GetXLocation(), _actor->GetYLocation());

	// Draw to the left of the sprite for characters and to the right of the sprite for enemys
	if (_actor->IsEnemy() == false) {
		VideoManager->MoveRelative(-40.0f, 0.0f); // TEMP: should move to just past the edge of the sprite image, not -40.0f
		VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	}
	else {
		VideoManager->MoveRelative(40.0f, 0.0f); // TEMP: should move to just past the edge of the sprite image, not 40.0f
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	}

	float y_position = INDICATOR_POSITION_CHANGE * _timer.PercentComplete();
	VideoManager->MoveRelative(0.0f, y_position);
	CalculateDrawAlpha();
	// TEMP: alpha modulation does not appear to be working correctly. If alpha is anything other than 1.0f, image is not drawn
// 	_image.Draw(_alpha_color);
	_image.Draw();
}

////////////////////////////////////////////////////////////////////////////////
// IndicatorSupervisor class
////////////////////////////////////////////////////////////////////////////////

IndicatorSupervisor::IndicatorSupervisor(BattleActor* actor) :
	_actor(actor)
{
	if (actor == NULL)
		IF_PRINT_WARNING(BATTLE_DEBUG) << "contructor received NULL actor argument" << endl;
}



IndicatorSupervisor::~IndicatorSupervisor() {
	for (uint32 i = 0; i < _wait_queue.size(); i++)
		delete _wait_queue[i];
	_wait_queue.clear();

	for (uint32 i = 0; i < _active_queue.size(); i++)
		delete _active_queue[i];
	_active_queue.clear();
}



void IndicatorSupervisor::Update() {
	// Update all active elements
	for (uint32 i = 0; i < _active_queue.size(); i++)
		_active_queue[i]->Update();

	// Remove all expired elements from the active queue
	while (_active_queue.empty() == false) {
		if (_active_queue.front()->IsExpired() == true) {
			delete _active_queue.front();
			_active_queue.pop_front();
		}
		else {
			// If the front element is not expired, no other elements should be expired either
			break;
		}
	}

	// TODO: determine if there is enough space to insert the next element

	if (_wait_queue.empty() == false) {
		_active_queue.push_back(_wait_queue.front());
		_active_queue.back()->Start();
		_wait_queue.pop_front();
	}
}



void IndicatorSupervisor::Draw() {
	for (uint32 i = 0; i < _active_queue.size(); i++) {
		_active_queue[i]->Draw();
	}
}



void IndicatorSupervisor::AddDamageIndicator(uint32 amount) {
	if (amount == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function was given a zero value argument" << endl;
		return;
	}

	string text = NumberToString(amount);
	TextStyle style;

	float damage_percent = static_cast<float>(amount / _actor->GetMaxHitPoints());
	if (damage_percent < 0.10f) {
		style.font = "text18";
		style.color = Color(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (damage_percent < 0.20f) {
		style.font = "text20";
		style.color = Color(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (damage_percent < 0.30f) {
		style.font = "text22";
		style.color = Color(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else { // (damage_percent >= 0.30f)
		style.font = "text24";
		style.color = Color(1.0f, 0.0f, 0.0f, 1.0f);
	}

	_wait_queue.push_back(new IndicatorText(_actor, text, style));
}



void IndicatorSupervisor::AddHealingIndicator(uint32 amount) {
	if (amount == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function was given a zero value argument" << endl;
		return;
	}

	string text = NumberToString(amount);
	TextStyle style;

	float healing_percent = static_cast<float>(amount / _actor->GetMaxHitPoints());
	if (healing_percent < 0.10f) {
		style.font = "text18";
		style.color = Color(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (healing_percent < 0.20f) {
		style.font = "text20";
		style.color = Color(0.06f, 1.0f, 0.0f, 1.0f);
	}
	else if (healing_percent < 0.30f) {
		style.font = "text22";
		style.color = Color(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else { // (healing_percent >= 0.30f)
		style.font = "text24";
		style.color = Color(0.0f, 1.0f, 0.0f, 1.0f);
	}

	_wait_queue.push_back(new IndicatorText(_actor, text, style));
}



void IndicatorSupervisor::AddMissIndicator() {
	string text = Translate("Miss");
	TextStyle style("text18", Color::white);
	_wait_queue.push_back(new IndicatorText(_actor, text, style));
}



void IndicatorSupervisor::AddStatusIndicator(GLOBAL_STATUS old_status, GLOBAL_INTENSITY old_intensity,
	GLOBAL_STATUS new_status, GLOBAL_INTENSITY new_intensity)
{
	// If the status and intensity has not changed, only a single status icon needs to be used
	if ((old_status == new_status) && (old_intensity == new_intensity)) {
		// TODO
// 		_wait_queue.push_back(new IndicatorImage(_actor, image));
	}
	// Otherwise two status icons need to be used in the indicator image
	else {
		// TODO
// 		_wait_queue.push_back(new IndicatorImage(_actor, image01, image02));
	}

	// TEMP
	IF_PRINT_DEBUG(BATTLE_DEBUG) << "status change: " << new_status << ", " << new_intensity << endl;
}

} // namespace private_battle

} // namespace hoa_battle
