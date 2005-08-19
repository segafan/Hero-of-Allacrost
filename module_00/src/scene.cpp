///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    scene.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Source file for scene mode interface.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "scene.h"
#include "audio.h"
#include "video.h"

using namespace std;
using namespace hoa_engine;
using namespace hoa_video;
using namespace hoa_pause;
using namespace hoa_scene::private_scene;

namespace hoa_scene {

bool SCENE_DEBUG = false;

SceneMode::SceneMode() {
	if (SCENE_DEBUG) cout << "SCENE: SceneMode constructor invoked" << endl;
	VideoManager = GameVideo::_GetReference();
	ModeManager = GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();

	mode_type = ENGINE_SCENE_MODE;
	scene_timer = 0;

	// setup the scene Image Descriptor

	// VideoManager->LoadImage(scene);
}



// The destructor frees up our scene image
SceneMode::~SceneMode() {
	if (SCENE_DEBUG) cout << "SCENE: SceneMode destructor invoked" << endl;
  // VideoManager->FreeImage(scene);
}



// Restores volume or unpauses audio, then pops itself from the game stack
void SceneMode::Update(Uint32 time_elapsed) {
	scene_timer += time_elapsed;

	// User must wait 0.75 seconds before they can exit the scene
	if ((InputManager->ConfirmPress() || InputManager->CancelPress()) && scene_timer < MIN_SCENE_UPDATES) {
		ModeManager->Pop();
	}
}



// Draws the scene
void SceneMode::Draw() {
// 	Draw the scene, maybe with a filter that lets it fade in and out....?
}

} // namespace hoa_scene
