/* 
 * scene.h
 *  Header file for Hero of Allacrost scene mode
 *  (C) 2004 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */

#ifndef __SCENE_HEADER__
#define __SCENE_HEADER__ 

// Partially defined namespace to avoid recursive inclusion problems.
namespace hoa_scene {
	class SceneMode;
} // namespace hoa_scene
 
#include <string>
#include "audio.h"
#include "video.h"
#include "global.h"

namespace hoa_scene {

namespace local_scene {

// How many milliseconds must pass before the user can exit the scene
const int MIN_SCENE_UPDATES = 750;

} // namespace local_scene
 
/******************************************************************************
  SceneMode Class - Pushed onto the stack when we are displaying a full-screen art scene

	>>>members<<<
		Uint32 scene_timer: keeps track of the number of times Update() has been called
		ImageDescriptor scene: the image we are displaying in this scene
	 
	>>>functions<<<
	SceneMode(): Loads up the scene image
	~SceneMode(): Frees the scene image
	
	void Update(Uint32 time_elapsed): Processes user input, updates the game state & game mode stack appropriately
	void Draw(): Draws the scene
 
	>>>notes<<<
		1) The user can not finish viewing the scene until after it has been on the screen for a short while. This is
			done so that the user doesn't accidentally skip a scene without viewing it (and so they are forced to gaze
			at the gorgeous artwork ^_~). That time is set to 25 times Update() is called, which is about 750ms.
	
 *****************************************************************************/
class SceneMode : public hoa_global::GameMode {
private:
	int scene_timer;

	hoa_global::InputState* input;	
	
	//hoa_video::ImageDescriptor scene;
public: 
  SceneMode();
  ~SceneMode();
  
  void Update(Uint32 time_elapsed);
  void Draw();
};

} // namespace hoa_scene

#endif
