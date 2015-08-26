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

#include "battle.h"
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

//! \brief The total amount of time (in milliseconds) that the display sequence lasts for indicator elements
const uint32 INDICTATOR_TIME = 5000;

//! \brief The amount of time (in milliseconds) that indicator elements fade at the beginning and end of the display sequence
const uint32 INDICATOR_FADE_TIME = 1000;

//! \brief The total vertical distance that indictor elements travel during the display sequence
const float INDICATOR_POSITION_CHANGE = 100.0f; // TODO: need to think of a better name for this constant

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



bool IndicatorElement::CalculateDrawAlpha() {
	// Case 1: Timer is not running nor paused so indicator should not be drawn
	if ((_timer.GetState() == SYSTEM_TIMER_RUNNING) && (_timer.GetState() == SYSTEM_TIMER_PAUSED)) {
		_alpha_color.SetAlpha(0.0f);
		return true;
	}
	// Case 2: Timer is in beginning stage and indicator graphic is fading in
	else if (_timer.GetTimeExpired() < INDICATOR_FADE_TIME) {
		_alpha_color.SetAlpha(static_cast<float>(_timer.GetTimeExpired()) / static_cast<float>(INDICATOR_FADE_TIME));
		return true;
	}
	// Case 3: Timer is in final stage and indicator graphic is fading out
	else if (_timer.TimeLeft() < INDICATOR_FADE_TIME) {
		_alpha_color.SetAlpha(static_cast<float>(_timer.TimeLeft()) / static_cast<float>(INDICATOR_FADE_TIME));
		return true;
	}
	// Case 4: Timer is in middle stage and indicator graphic should be drawn with no transparency
	else {
		return false;
	}
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
		VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
		VideoManager->MoveRelative(-20.0f, 0.0f); // TEMP: should move to just past the edge of the sprite image
	}
	else {
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
		VideoManager->MoveRelative(_actor->GetSpriteWidth() / 2.0f, 0.0f);
	}

	float y_position = INDICATOR_POSITION_CHANGE * _timer.PercentComplete();
	VideoManager->MoveRelative(0.0f, y_position);

	if (CalculateDrawAlpha() == true)
		_text_image.Draw(_alpha_color);
	else
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



IndicatorImage::IndicatorImage(BattleActor* actor, const StillImage& image) :
	IndicatorElement(actor),
	_image(image)
{} // TODO: check if the image argument is valid...currently seems the only possible check is that its filename is not an empty string.



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

	if (CalculateDrawAlpha() == true)
		_image.Draw(_alpha_color);
	else
		_image.Draw();
}

////////////////////////////////////////////////////////////////////////////////
// IndicatorBlendedImage class
////////////////////////////////////////////////////////////////////////////////

IndicatorBlendedImage::IndicatorBlendedImage(BattleActor* actor, const string& first_filename, const string& second_filename) :
	IndicatorElement(actor),
	_second_alpha_color(1.0f, 1.0f, 1.0f, 0.0f)
{
	if (_first_image.Load(first_filename) == false)
		PRINT_ERROR << "failed to load first indicator image: " << first_filename << endl;
	if (_second_image.Load(second_filename) == false)
		PRINT_ERROR << "failed to load second indicator image: " << second_filename << endl;
}



IndicatorBlendedImage::IndicatorBlendedImage(BattleActor* actor, const StillImage& first_image, const StillImage& second_image) :
	IndicatorElement(actor),
	_first_image(first_image),
	_second_image(second_image),
	_second_alpha_color(1.0f, 1.0f, 1.0f, 0.0f)
{} // TODO: check if the image arguments are valid...currently seems the only possible check is that its filename is not an empty string.



void IndicatorBlendedImage::Draw() {
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

	// Case 1: Initial fade in of first image
	if (_timer.GetTimeExpired() <= INDICATOR_FADE_TIME) {
		CalculateDrawAlpha();
		_first_image.Draw(_alpha_color);
	}
	// Case 2: Opaque draw of first image
	else if (_timer.GetTimeExpired() <= (INDICATOR_FADE_TIME * 2)) {
		_first_image.Draw();
	}
	// Case 3: Blended draw of first and second images
	else if (_timer.GetTimeExpired() <= (INDICATOR_FADE_TIME * 3)) {
		_alpha_color.SetAlpha(static_cast<float>((INDICATOR_FADE_TIME * 3) - _timer.GetTimeExpired())
			/ static_cast<float>(INDICATOR_FADE_TIME));
		_second_alpha_color.SetAlpha(1.0f - _alpha_color.GetAlpha());
		_first_image.Draw(_alpha_color);
		_second_image.Draw(_second_alpha_color);
	}
	// Case 4: Opaque draw of second image
	else if (_timer.GetTimeExpired() <= (INDICATOR_FADE_TIME * 4)) {
		_second_image.Draw();
	}
	// Case 5: Final fade out of second image
	else { // (_timer.GetTimeExpired() <= INDICATOR_TIME)
		CalculateDrawAlpha();
		_second_image.Draw(_alpha_color);
	}
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
		style.color = Color(1.0f, 0.275f, 0.275f, 1.0f);
	}
	else if (damage_percent < 0.20f) {
		style.font = "text20";
		style.color = Color(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (damage_percent < 0.30f) {
		style.font = "text22";
		style.color = Color(0.784f, 0.0f, 0.0f, 1.0f);
	}
	else { // (damage_percent >= 0.30f)
		style.font = "text24";
		style.color = Color(0.627f, 0.0f, 0.0f, 1.0f);
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
		StillImage* image = BattleMode::CurrentInstance()->GetStatusIcon(new_status, new_intensity);
		_wait_queue.push_back(new IndicatorImage(_actor, *image));
	}
	// Otherwise two status icons need to be used in the indicator image
	else {
		StillImage* first_image = BattleMode::CurrentInstance()->GetStatusIcon(old_status, old_intensity);
		StillImage* second_image = BattleMode::CurrentInstance()->GetStatusIcon(new_status, new_intensity);
		_wait_queue.push_back(new IndicatorBlendedImage(_actor, *first_image, *second_image));
	}
}

} // namespace private_battle

} // namespace hoa_battle
