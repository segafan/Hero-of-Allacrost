///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode interface.
*** ***************************************************************************/

#include "utils.h"
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "audio.h"
#include "video.h"
#include "global.h"
#include "script.h"
#include "input.h"
#include "system.h"
#include "battle.h"
#include "menu.h"

using namespace std;
using namespace hoa_map::private_map;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_input;
using namespace hoa_global;
using namespace hoa_script;
using namespace hoa_battle;
using namespace hoa_menu;

namespace hoa_map {

bool MAP_DEBUG = false;
// Initialize static class variable
MapMode *MapMode::_current_map = NULL;

// ****************************************************************************
// ************************** MapMode Class Functions *************************
// ****************************************************************************
// ***************************** GENERAL FUNCTIONS ****************************
// ****************************************************************************

MapMode::MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode constructor invoked" << endl;

	mode_type = MODE_MANAGER_MAP_MODE;

	_map_state = EXPLORE;

	_virtual_focus = new VirtualSprite();
	_virtual_focus->SetXPosition(0, 0.0f);
	_virtual_focus->SetYPosition(0, 0.0f);
	_virtual_focus->movement_speed = NORMAL_SPEED;
	_virtual_focus->SetNoCollision(true);
	_virtual_focus->SetVisible(false);

	// TODO: Load the map data in a seperate thread
	Load();
}



MapMode::~MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode destructor invoked" << endl;

	for (uint32 i = 0; i < _music.size(); i++) {
		_music[i].FreeMusic();
	}

	for (uint32 i = 0; i < _sounds.size(); i++) {
		_sounds[i].FreeSound();
	}

	// Delete all of the tile images
	for (uint32 i = 0; i < _tile_images.size(); i++) {
		delete(_tile_images[i]);
	}

	// Delete all of the map objects
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		delete(_ground_objects[i]);
	}
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		cout << "before delete pass objects" << endl;
		delete(_pass_objects[i]);
	}
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		cout << "before delete sky objects" << _sky_objects.size() << endl;
		delete(_sky_objects[i]);
	}
	delete(_virtual_focus);

	// Free up the dialogue window
	VideoManager->DeleteImage(_dialogue_box);
	_dialogue_window.Destroy();
}


// Resets appropriate class members.
void MapMode::Reset() {
	// Reset active video engine properties
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);

	if (!VideoManager->SetFont("default"))
    	cerr << "MAP ERROR: Failed to set the map font" << endl;

	// Let all map objects know that this is the current map
	MapMode::_current_map = this;

	// TEMP: This will need to be scripted later
	if (_music.size() > 0 && _music[0].GetMusicState() != AUDIO_STATE_PLAYING) {
		_music[0].PlayMusic();
	}
}



void MapMode::BindToLua() {
	using namespace luabind;

	module(ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapMode>("MapMode")
			.def(constructor<>())
			.def("Load", &MapMode::Load)
			.def("_AddGroundObject", &MapMode::_AddGroundObject)
			.def("_AddPassObject", &MapMode::_AddPassObject)
			.def("_AddSkyObject", &MapMode::_AddSkyObject)
	];

	module(ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapObject>("MapObject")
			.def("SetObjectID", &MapObject::SetObjectID)
			.def("SetContext", &MapObject::SetContext)
			.def("SetXPosition", &MapObject::SetXPosition)
			.def("SetYPosition", &MapObject::SetYPosition)
			.def("SetImgHalfWidth", &MapObject::SetImgHalfWidth)
			.def("SetImgHeight", &MapObject::SetImgHeight)
			.def("SetCollHalfWidth", &MapObject::SetCollHalfWidth)
			.def("SetCollHeight", &MapObject::SetCollHeight)
			.def("SetUpdatable", &MapObject::SetUpdatable)
			.def("SetVisible", &MapObject::SetVisible)
			.def("SetNoCollision", &MapObject::SetNoCollision)
			.def("SetDrawOnSecondPass", &MapObject::SetDrawOnSecondPass)
			.def("GetObjectID", &MapObject::GetObjectID)
			.def("GetContext", &MapObject::GetContext)
			.def("GetXPosition", &MapObject::GetXPosition)
			.def("GetYPosition", &MapObject::GetYPosition)
			.def("GetImgHalfWidth", &MapObject::GetImgHalfWidth)
			.def("GetImgHeight", &MapObject::GetImgHeight)
			.def("GetCollHalfWidth", &MapObject::GetCollHalfWidth)
			.def("GetCollHeight", &MapObject::GetCollHeight)
			.def("IsUpdatable", &MapObject::IsUpdatable)
			.def("IsVisible", &MapObject::IsVisible)
			.def("IsNoCollision", &MapObject::IsNoCollision)
			.def("IsDrawOnSecondPass", &MapObject::IsDrawOnSecondPass)
	];

	module(ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<PhysicalObject, MapObject>("PhysicalObject")
			.def(constructor<>())
			.def("AddAnimation", &PhysicalObject::AddAnimation)
			.def("SetCurrentAnimation", &PhysicalObject::SetCurrentAnimation)
			.def("SetAnimationProgress", &PhysicalObject::SetAnimationProgress)
			.def("GetCurrentAnimation", &PhysicalObject::GetCurrentAnimation)
	];

	module(ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<VirtualSprite, MapObject>("VirtualSprite")
			.def(constructor<>())
			.def("SetDirection", &VirtualSprite::SetDirection)
			.def("SetMovementSpeed", &VirtualSprite::SetMovementSpeed)
			.def("GetDirection", &VirtualSprite::GetDirection)
			.def("GetMovementSpeed", &VirtualSprite::GetMovementSpeed)
	];

	module(ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapSprite, VirtualSprite>("MapSprite")
			.def(constructor<>())
			.def("SetName", &MapSprite::SetName)
			.def("SetWalkSound", &MapSprite::SetWalkSound)
			.def("SetCurrentAnimation", &MapSprite::SetCurrentAnimation)
			.def("SetFacePortrait", &MapSprite::SetFacePortrait)
			.def("GetWalkSound", &MapSprite::GetWalkSound)
			.def("GetCurrentAnimation", &MapSprite::GetCurrentAnimation)
	];
} // void MapMode::BindToLua()


// Loads the map from a Lua file.
bool MapMode::Load(string filename) {
	// TEMP: All of this is temporary, and will be replaced later
	_map_filename = "dat/maps/nofile.lua";

	// ---------- (1) Open map script file and begin loading data
	_num_tile_rows = 50;
	_num_tile_cols = 60;

	// ---------- (2) Load in the map tileset images
	vector<string> tileset_filenames;
	vector<StillImage> tile_images;

	tileset_filenames.push_back("img/tiles/ll_floor1.png");
	tileset_filenames.push_back("img/tiles/ll_floor2.png");
	tileset_filenames.push_back("img/tiles/ll_floor_horizontal_sand_left.png");
	tileset_filenames.push_back("img/tiles/ll_floor_horizontal_sand_right.png");
	tileset_filenames.push_back("img/tiles/ll_floor3.png");
	tileset_filenames.push_back("img/tiles/ol_rock_02.png");
	tileset_filenames.push_back("img/tiles/ol_rock_03.png");

	for (uint32 i = 0; i < tileset_filenames.size(); i++) {
		StillImage *new_img = new StillImage();
		new_img->SetFilename(tileset_filenames[i]);
		new_img->SetDimensions(2.0f, 2.0f);
		if (new_img->Load() == false)
			cerr << "MAP ERROR: tile image " << new_img->GetFilename() << " failed to load" << endl;
		_tile_images.push_back(new_img);
	}

	// ---------- (3) Setup the tile grid and map grid
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		_tile_grid.push_back(vector<MapTile>());
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			_tile_grid[r].push_back(MapTile());
			_tile_grid[r][c].lower_layer = (r + c) % 5;
			if ((r + c) % 35 == 0)
				_tile_grid[r][c].middle_layer = 5;
			else if ((r + c) % 47 == 0)
				_tile_grid[r][c].middle_layer = 6;
			else
				_tile_grid[r][c].middle_layer = -1;
			_tile_grid[r][c].upper_layer = -1;
		}
	}

	for (uint16 r = 0; r < _num_tile_rows * 2; r++) {
		_map_grid.push_back(vector<bool>(_num_tile_cols * 2, false));
	}

	// Uncomment this loop to test out tile-collision detection
 	//for (uint16 r = 0; r < _num_tile_rows * 2; r++) {
 	//	for (uint16 c = 0; c < _num_tile_cols * 2; c++) {
 	//		if ((r + c) % 70 == 0) {
 	//			_map_grid[r][c] = true;
 	//		}
 	//	}
 	//}

	_current_dialogue = 0;

	MapSprite *sp;

	// Load player sprite and rest of map objects
	sp = new MapSprite();
	sp->name = MakeUnicodeString("Claudius");
	sp->SetObjectID(0);
	sp->SetContext(1);
	sp->SetXPosition(55, 0.5f);
	sp->SetYPosition(55, 0.5f);
	sp->SetCollHalfWidth(1.0f);
	sp->SetCollHeight(2.0f);
	sp->img_half_width = 1.0f;
	sp->img_height = 4.0f;
	sp->movement_speed = NORMAL_SPEED;
	sp->direction = SOUTH;
	sp->SetPortrait( "img/portraits/map/claudius.png" );
	if (sp->Load() == false)
		return false;
	_all_objects[ 0 ] = sp;
	_camera = sp;
	_ground_objects.push_back(sp);

	MapSprite *DialogueSprite;
	MapDialogue *Dialogue = new MapDialogue();
	

	// Load player sprite and rest of map objects
	DialogueSprite = new MapSprite();
	DialogueSprite->name = MakeUnicodeString("NPC");
	DialogueSprite->SetObjectID(1);
	DialogueSprite->SetContext(1);
	DialogueSprite->SetXPosition(45, 0.5f);
	DialogueSprite->SetYPosition(45, 0.5f);
	DialogueSprite->SetCollHalfWidth(1.0f);
	DialogueSprite->SetCollHeight(2.0f);
	DialogueSprite->img_half_width = 1.0f;
	DialogueSprite->img_height = 4.0f;
	DialogueSprite->movement_speed = NORMAL_SPEED;
	DialogueSprite->SetDirection(EAST);

	ActionPathMove *new_act = new ActionPathMove( DialogueSprite, 1, true );
	new_act->destination.row = 35;
	new_act->destination.col = 45;
	Dialogue->AddText( 1, MakeUnicodeString( "This is a test" ) );
	Dialogue->AddText( 0, MakeUnicodeString( "Oh really?!" ), new_act );

	//ActionPathMove *new_act;
	/*new_act = new ActionPathMove(DialogueSprite);
	new_act->destination.row = 45;
	new_act->destination.col = 50;
	DialogueSprite->actions.push_back(new_act);
	new_act = new ActionPathMove(DialogueSprite);
	new_act->destination.row = 50;
	new_act->destination.col = 45;
	DialogueSprite->actions.push_back(new_act);

	new_act = new ActionPathMove(DialogueSprite);
	new_act->destination.row = 47;
	new_act->destination.col = 22;
	DialogueSprite->actions.push_back(new_act);
	new_act = new ActionPathMove(DialogueSprite);
	new_act->destination.row = 8;
	new_act->destination.col = 8;
	DialogueSprite->actions.push_back(new_act);*/
	//DialogueSprite->current_action = 0;

	DialogueSprite->AddDialogue( Dialogue );
	DialogueSprite->SetDialogue( 0 );
	DialogueSprite->SetPortrait( "img/portraits/map/laila.png" );
	if (DialogueSprite->Load() == false)
		return false;
	_ground_objects.push_back(DialogueSprite);
	_all_objects[ 1 ] = DialogueSprite;



	// ---------- (1) Setup GUI items (in a 1024x768 coordinate system)
	VideoManager->PushState();
	VideoManager->SetCoordSys(0, 1024, 768, 0);
	_dialogue_window.Create(1024.0f, 256.0f);
	_dialogue_window.SetPosition(0.0f, 512.0f);
	_dialogue_window.SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);

	_dialogue_box.SetFilename("img/menus/dialogue_box.png");
	if (_dialogue_box.Load() == false)
		cerr << "MAP ERROR: failed to load image: " << _dialogue_box.GetFilename() << endl;

	_dialogue_nameplate.SetFilename("img/menus/dialogue_nameplate.png");
	if (_dialogue_nameplate.Load() == false)
		cerr << "MAP ERROR: failed to load image: " << _dialogue_nameplate.GetFilename() << endl;

	_dialogue_textbox.SetDisplaySpeed(30);
	_dialogue_textbox.SetPosition(300.0f, 768.0f - 180.0f);
	_dialogue_textbox.SetDimensions(1024.0f - 300.0f - 60.0f, 180.0f - 70.0f);
	_dialogue_textbox.SetFont("default");
	_dialogue_textbox.SetDisplayMode(VIDEO_TEXT_FADECHAR);
	_dialogue_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	VideoManager->PopState();
	return true;
} // bool MapMode::Load(string filename)

// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update() {
	_time_elapsed = SystemManager->GetUpdateTime();

	// ---------- (1) Process user input

	switch (_map_state) {
		case EXPLORE:
			_HandleInputExplore();
			break;
		case DIALOGUE:
			_HandleInputDialogue();
			break;
		default:
			_HandleInputExplore();
			break;
	}

	// ---------- (2) Update all animated tile images

	// TODO

	// ---------- (3) Update all objects on the map

	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		_ground_objects[i]->Update();
	}
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		_pass_objects[i]->Update();
	}
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		_sky_objects[i]->Update();
	}

	// ---------- (4) Sort the objects so they are in the correct draw order ********
	std::sort( _ground_objects.begin(), _ground_objects.end(), MapObject::MapObject_Ptr_Less() );

} // void MapMode::Update()


// Updates the game status when MapMode is in the 'explore' state
void MapMode::_HandleInputExplore() {
	// Do the fade to battle mode
	// Doing this first should prevent user input
// 	if (_fade_to_battle_mode) {
// 		// Only start battle mode once the fade is done.
// 		if (!VideoManager->IsFading()) {
// 			// Clear fade instantly
// 			VideoManager->FadeScreen(Color::clear, 0.0f);
// 			_fade_to_battle_mode = false;
// 			BattleMode *BM = new BattleMode();
// 			ModeManager->Push(BM);
// 		}
// 		return;
// 	}

	// Go to menu mode if the user requested it
	if (InputManager->MenuPress()) {
		MenuMode *MM = new MenuMode();
		ModeManager->Push(MM);
		return;
	}

	// TEMPORARY: disable random encounters
// 	if (InputManager->SwapPress()) {
// 		_random_encounters = !_random_encounters;
// 	}

	// Toggle running versus walking
// 	if (InputManager->CancelPress()) {
// 		if (speed_double) {
// 			_focused_object->step_speed /= 2;
// 			speed_double = false;
// 		}
// 		else {
// 			_focused_object->step_speed *= 2;
// 			speed_double = true;
// 		}
// 	}

	if (InputManager->ConfirmPress()) {
		MapObject * obj = _FindNearestObject( _camera );
		if( obj && ( obj->GetType() == VIRTUAL_TYPE || obj->GetType() == SPRITE_TYPE ) )
		{
			VirtualSprite * sp = reinterpret_cast< VirtualSprite * >( obj );

			if( sp->HasDialogue() )
			{
				_camera->SaveState();
				sp->SaveState();
				_camera->moving = false;
				
				sp->moving = false;
				sp->SetDirection( VirtualSprite::CalculateOppositeDirection( _camera->GetDirection() ) );
				_current_dialogue = sp->GetCurrentDialogue();
				_dialogue_textbox.SetDisplayText( _current_dialogue->GetLine() );					
				_map_state = DIALOGUE;
				return;
			}
		}
	}

	// Detect and handle movement input from the user
	if (InputManager->UpState() || InputManager->DownState() || InputManager->LeftState() || InputManager->RightState()) {
		_camera->moving = true;
	}
	else {
		_camera->moving = false;
	}

	// Determine the direction of movement. Priority of movement is given to: up, down, left, right.
	// In the case of diagonal movement, the direction that the sprite should face also needs to be
	// deduced.
	if (_camera->moving == true) {
		if (InputManager->UpState()) {
			if (InputManager->LeftState()) {
				_camera->SetDirection(NORTHWEST);
			}
			else if (InputManager->RightState()) {
				_camera->SetDirection(NORTHEAST);
			}
			else {
				_camera->SetDirection(NORTH);
			}
		}
		else if (InputManager->DownState()) {
			if (InputManager->LeftState()) {
				_camera->SetDirection(SOUTHWEST);
			}
			else if (InputManager->RightState()) {
				_camera->SetDirection(SOUTHEAST);
			}
			else {
				_camera->SetDirection(SOUTH);
			}
		}
		else if (InputManager->LeftState()) {
			_camera->SetDirection(WEST);
		}
		else if (InputManager->RightState()) {
			_camera->SetDirection(EAST);
		}
	} // if (_camera->moving == true)
} // void MapMode::_HandleInputExplore()


// Updates the game status when MapMode is in the 'dialogue' state
void MapMode::_HandleInputDialogue() {
 	_dialogue_window.Update(_time_elapsed);
 	_dialogue_textbox.Update(_time_elapsed);

	if( _current_dialogue )
	{
		if( _current_dialogue->GetAction() ) {
			if( !_current_dialogue->GetAction()->IsFinished() )
			{
				_current_dialogue->GetAction()->Execute();
				if( _current_dialogue->GetAction()->_forced )
					return;
			}
		}
 		if (InputManager->ConfirmPress()) {
 			if (!_dialogue_textbox.IsFinished()) {
 				_dialogue_textbox.ForceFinish();
 			}
 			else {
 				bool not_finished = _current_dialogue->ReadNextLine();

 				if (!not_finished) {
 					_dialogue_window.Hide();
 					_map_state = EXPLORE;
 					// Restore the status of the map sprites
 					for (uint32 i = 0; i < _current_dialogue->GetNumLines(); i++) {
						static_cast< VirtualSprite* >( _all_objects[ _current_dialogue->GetSpeaker( i ) ] )->LoadState();
 					}
 					//_sprites[1]->UpdateConversationCounter();
 					_current_dialogue = NULL;
 				}
 				else { // Otherwise, the dialogue is automatically updated to the next line
 					_dialogue_textbox.SetDisplayText(_current_dialogue->GetLine());
					//If an action was set, execute it.
 				}
 			}
 		}
	}
} // void MapMode::_HandleInputDialogue()



MapObject* MapMode::_FindNearestObject(const VirtualSprite* sprite) {
	// The edges of the collision rectangle to check
	float top, bottom, left, right;

	// ---------- (1): Using the sprite's direction, determine the area to check for other objects
	if (sprite->direction & FACING_NORTH) {
		bottom = sprite->ComputeYLocation() - sprite->coll_height;
		top = bottom - 3.0f;
		left = sprite->ComputeXLocation() - sprite->coll_half_width;
		right = sprite->ComputeXLocation() + sprite->coll_half_width;
	}
	else if (sprite->direction & FACING_SOUTH) {
		top = sprite->ComputeYLocation();
		bottom = top + 3.0f;
		left = sprite->ComputeXLocation() - sprite->coll_half_width;
		right = sprite->ComputeXLocation() + sprite->coll_half_width;
	}
	else if (sprite->direction & FACING_WEST) {
		right = sprite->ComputeXLocation() - sprite->coll_half_width;
		left = right - 3.0f;
		bottom = sprite->ComputeYLocation();
		top = bottom - sprite->coll_height;
	}
	else if (sprite->direction & FACING_EAST) {
		left = sprite->ComputeXLocation() + sprite->coll_half_width;
		right = left + 3.0f;
		bottom = sprite->ComputeYLocation();
		top = bottom - sprite->coll_height;
	}
	else {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: sprite was set to invalid direction in MapMode::_FindNearestObject()" << endl;
		return NULL;
	}

	// A vector to contain objects which are valid for the sprite to interact with
	vector<MapObject*> valid_objects;
	// A pointer to the object which has been found to be the closest to the sprite within valid_objs
	MapObject* closest = NULL;

	// ---------- (2): Go through all objects and determine which (if any) are valid
	for (map<uint16, MapObject*>::iterator i = _all_objects.begin(); i != _all_objects.end(); i++) {
		MapObject* obj = i->second;
		if( obj == sprite ) //A sprite can't target itself
			continue;
		if (obj->context != sprite->context) // Objects in different contexts can not interact with one another
			continue;

		// Compute the full position coordinates for the object under study
		float other_x_location = obj->ComputeXLocation();
		float other_y_location = obj->ComputeYLocation();

		// Verify that the bounding boxes overlap on the horizontal axis
		if (!(other_x_location - obj->coll_half_width > right
			|| other_x_location + obj->coll_half_width < left)) {
			// Verify that the bounding boxes overlap on the vertical axis
			if (!(other_y_location - obj->coll_height > bottom
				|| other_y_location < top )) {
				// Boxes overlap on both axes, it is a valid interaction
				valid_objects.push_back(obj);
			}
		}
	} // for (map<MapObject*>::iterator i = _all_objects.begin(); i != _all_objects.end(); i++)

	// If there are one or less objects that are valid, then we are done here.
	if (valid_objects.empty()) {
		return NULL;
	}
	else if (valid_objects.size() == 1) {
		return valid_objects[0];
	}

	// ---------- (3): Figure out which of the valid objects is the closest to the sprite

	// NOTE: For simplicity, we simply find which object has the location coordinates which are
	// closest to the sprite's coordinates using the Manhattan distance.

	// Used to hold the full position coordinates of the sprite
	float source_x = sprite->ComputeXLocation();
	float source_y = sprite->ComputeYLocation();

	closest = valid_objects[0];
	float min_distance = fabs(source_x - closest->ComputeXLocation())
		+ fabs(source_y - closest->ComputeYLocation());

	// Determine which object's position is closest to the sprite
	for (uint32 i = 1; i < valid_objects.size(); i++) {
		float dist = fabs(source_x - valid_objects[i]->ComputeXLocation())
			+ fabs(source_y - valid_objects[i]->ComputeYLocation());
		if (dist < min_distance) {
			closest = valid_objects[i];
			min_distance = dist;
		}
	}
	return closest;
} // MapObject* MapMode::_FindNearestObject(VirtualSprite* sprite)



bool MapMode::_DetectCollision(VirtualSprite* sprite) {
	// NOTE: Whether the argument pointer is valid is not checked here, since the object pointer
	// itself presumably called this function.

	// The single X,Y floating point coordinates of the sprite
	float x_location = sprite->ComputeXLocation();
	float y_location = sprite->ComputeYLocation();

	// The coordinates corresponding to the four sides of the sprite's collision rectangle (cr)
	float cr_left = x_location - sprite->coll_half_width;
	float cr_right = x_location + sprite->coll_half_width;
	float cr_top = y_location - sprite->coll_height;
	// The bottom of the sprite's collision rectangle is its y_location

	// ---------- (1): Check if the sprite's position has gone out of bounds

	if (cr_left < 0.0f || cr_top < 0.0f || cr_right >= static_cast<float>(_num_tile_cols * 2) ||
		y_location >= static_cast<float>(_num_tile_rows * 2)) {
		return true;
	}

	// Do not do tile or object based collision detection for this sprite if it has this member set
	if (sprite->no_collision == true) {
		return false;
	}

	// A pointer to the layer of objects to do the collision detection with
	vector<MapObject*>* objects;

	if (sprite->sky_object == false) { // Do tile collision detection for ground objects only

		// ---------- (2): Determine if the sprite's collision rectangle overlaps any unwalkable tiles

		// NOTE: Because the sprite's collision rectangle was determined to be within the map bounds,
		// the map grid tile indeces referenced in this loop are all valid entries.
		for (uint32 r = static_cast<uint32>(cr_top); r <= static_cast<uint32>(y_location); r++) {
			for (uint32 c = static_cast<uint32>(cr_left); c <= static_cast<uint32>(cr_right); c++) {
				if (_map_grid[r][c] == true) { // Then this overlapping tile is unwalkable
					return true;
				}
			}
		}
		objects = &_ground_objects;
	}
	else {
		objects = &_sky_objects;
	}

	// ---------- (3): Determine if two object's collision rectangles overlap

	for (uint32 i = 0; i < objects->size(); i++) {
		// Only verify this object if it is not the same object as the sprite
		if ((*objects)[i]->object_id != sprite->object_id
			&& !(*objects)[i]->no_collision
			&& (*objects)[i]->context == sprite->context ) 
		{
			// Only verify this object if it has no_collision set to false
			if ( !(*objects)[i]->no_collision ) {
				// Compute the full position coordinates of the other object
				float other_x_location = (*objects)[i]->ComputeXLocation();
				float other_y_location = (*objects)[i]->ComputeYLocation();;

				// Verify that the bounding boxes overlap on the horizontal axis
				if (!(other_x_location - (*objects)[i]->coll_half_width > cr_right
					|| other_x_location + (*objects)[i]->coll_half_width < cr_left)) {
					// Verify that the bounding boxes overlap on the vertical axis
					if (!(other_y_location - (*objects)[i]->coll_height > y_location
						|| other_y_location < cr_top )) {
						// Boxes overlap on both axis, there is a colision
						return true;
					}
				}
			}
		}
	}

	// No collision was detected
	return false;
} // bool MapMode::_DetectCollision(VirtualSprite* sprite)



void MapMode::_FindPath(const VirtualSprite* sprite, std::vector<PathNode>& path, const PathNode& dest) {
	// NOTE: Refer to the implementation of the A* algorithm to understand what all these lists and scores are
	std::vector<PathNode> open_list;
	std::vector<PathNode> closed_list;

	// The starting node of this path discovery
	PathNode source_node(static_cast<int16>(sprite->y_position), static_cast<int16>(sprite->x_position));

	// The current "best node"
	PathNode best_node;
	// Used to hold the eight adjacent nodes
	PathNode nodes[8];

	// Temporary delta variables used in calculation of a node's heuristic (h score)
	uint32 x_delta, y_delta;
	// The number of grid elements that the sprite's collision rectange spreads outward and upward
	int16 x_span, y_span;
	// The number to add to a node's g_score, depending on whether it is a lateral or diagonal movement
	int16 g_add;

	path.clear();
	x_span = static_cast<int16>(sprite->coll_half_width);
	y_span = static_cast<int16>(sprite->coll_height);

	// Check that the source node is not the same as the destination node
	if (source_node == dest) {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: source node is same as destination in MapMode::_FindPath()" << endl;
		return;
	}

	// Check that the destination is valid for the sprite to move to
	if ((dest.col - x_span < 0) || (dest.row - y_span < 0) ||
		(dest.col + x_span >= (_num_tile_cols * 2)) || (dest.row >= (_num_tile_rows * 2))) {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: sprite can not move to destination node on path because it exceeds map boundaries" << endl;
		return;
	}
	for (int16 r = dest.row - y_span; r < dest.row; r++) {
		for (int16 c = dest.col - x_span; c < dest.col + x_span; c++) {
			if (_map_grid[r][c] == true) {
				if (MAP_DEBUG)
					cerr << "MAP ERROR: sprite can not move to destination node on path because one or more grid tiles are unwalkable" << endl;
				return;
			}
		}
	}

	open_list.push_back(source_node);

	while (!open_list.empty()) {
		sort(open_list.begin(), open_list.end(), PathNode::NodePred());
		best_node = open_list.back();
		open_list.pop_back();
		closed_list.push_back(best_node);

		// Check if destination has been reached, and break out of the loop if so
		if (best_node == dest) {
			break;
		}

		// Setup the coordinates of the 8 adjacent nodes to the best node
		nodes[0].row = closed_list.back().row - 1; nodes[0].col = closed_list.back().col;
		nodes[1].row = closed_list.back().row + 1; nodes[1].col = closed_list.back().col;
		nodes[2].row = closed_list.back().row;     nodes[2].col = closed_list.back().col - 1;
		nodes[3].row = closed_list.back().row;     nodes[3].col = closed_list.back().col + 1;
		nodes[4].row = closed_list.back().row - 1; nodes[4].col = closed_list.back().col - 1;
		nodes[5].row = closed_list.back().row - 1; nodes[5].col = closed_list.back().col + 1;
		nodes[6].row = closed_list.back().row + 1; nodes[6].col = closed_list.back().col - 1;
		nodes[7].row = closed_list.back().row + 1; nodes[7].col = closed_list.back().col + 1;

		// Check the eight adjacent nodes
		for (uint8 i = 0; i < 8; ++i) {
			// ---------- (A): Check that the sprite's collision rectangle will be within the map boundaries
			if ((nodes[i].col - x_span < 0) || (nodes[i].row - y_span < 0) ||
				(nodes[i].col + x_span >= (_num_tile_cols * 2)) || (nodes[i].row >= (_num_tile_rows * 2))) {
				continue;
			}

			// ---------- (B): Check that all grid nodes that the sprite's collision rectangle will overlap are walkable
			bool continue_loop = true;
			for (int16 r = nodes[i].row - y_span; r < nodes[i].row && continue_loop; r++) {
				for (int16 c = nodes[i].col - x_span; c < nodes[i].col + x_span; c++) {
					if (_map_grid[r][c] == true) {
						continue_loop = false;
						break;
					}
				}
			}
			if (continue_loop == false) { // This node is invalid
				continue;
			}

			// ---------- (C): Check if the Node is already in the closed list
			if (find(closed_list.begin(), closed_list.end(), nodes[i]) != closed_list.end()) {
				continue;
			}

			// ---------- (D): If this point has been reached, the node is valid for the sprite to move to

			// If this is a lateral adjacent node, g_score is +10, otherwise diagonal adjacent node is +14
			if (i < 4)
				g_add = 10;
			else
				g_add = 14;

			// Set the node's parent and calculate its g_score
			nodes[i].parent_row = best_node.row;
			nodes[i].parent_col = best_node.col;
			nodes[i].g_score = best_node.g_score + g_add;

			// ---------- (E): Check to see if the node is already on the open list and update it if necessary
			vector<PathNode>::iterator iter = find(open_list.begin(), open_list.end(), nodes[i]);
			if (iter != open_list.end()) {
				// If its G is higher, it means that the path we are on is better, so switch the parent
				if (iter->g_score > nodes[i].g_score) {
					iter->g_score = nodes[i].g_score;
					iter->f_score = nodes[i].g_score + iter->h_score;
					iter->parent_row = nodes[i].parent_row;
					iter->parent_col = nodes[i].parent_col;
				}
			}
			// ---------- (F): Add the new node to the open list
			else {
				// Calculate the H and F score of the new node (the heuristic used is diagonal)
				x_delta = abs(dest.col - nodes[i].col);
				y_delta = abs(dest.row - nodes[i].row);
				if (x_delta > y_delta)
					nodes[i].h_score = 14 * y_delta + 10 * (x_delta - y_delta);
				else
					nodes[i].h_score = 14 * x_delta + 10 * (y_delta - x_delta);

				nodes[i].f_score = nodes[i].g_score + nodes[i].h_score;
				open_list.push_back(nodes[i]);
			}
		} // for (uint8 i = 0; i < 8; ++i)
	} // while (!open_list.empty())

	if (open_list.empty()) {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: could not find path to destination" << endl;
		path.push_back( source_node );
		return;
	}

	// Add the destination node to the vector, retain its parent, and remove it from the closed list
	path.push_back(closed_list.back());
	int16 parent_row = closed_list.back().parent_row;
	int16 parent_col = closed_list.back().parent_col;
	//PathNode* parent = closed_list.back().parent;
	closed_list.pop_back();

	// Go backwards through the closed list to construct the path
	for (vector<PathNode>::iterator iter = closed_list.end() - 1; iter != closed_list.begin(); --iter) {
		if ( iter->col == parent_col && iter->row == parent_row ) { // Find the parent of the node, add it to the path, and then set a new parent node
			path.push_back(*iter);
			parent_col = iter->parent_col;
			parent_row = iter->parent_row;
		}
	}
	std::reverse( path.begin(), path.end() );
} // void MapMode::_FindPath(const VirtualSprite* sprite, std::vector<PathNode>& path, const PathNode& dest)

// ****************************************************************************
// **************************** DRAW FUNCTIONS ********************************
// ****************************************************************************

// Determines things like our starting tiles
void MapMode::_CalculateDrawInfo() {
	// ---------- (1) Set the default starting draw positions for the tiles (top left tile)

	// The camera's position is in terms of the 16x16 grid, which needs to be converted into 32x32 coordinates.
	float camera_x = _camera->ComputeXLocation();
	float camera_y = _camera->ComputeYLocation();

	// Determine the draw coordinates of the top left corner using the camera's current position
	_draw_info.tile_x_start = 1.0f - _camera->x_offset;
	if (IsOddNumber(_camera->x_position))
		_draw_info.tile_x_start -= 1.0f;

	_draw_info.tile_y_start = 2.0f - _camera->y_offset;
	if (IsOddNumber(_camera->y_position))
		_draw_info.tile_y_start -= 1.0f;

	// By default the map draws 32 + 1 columns and 24 + 1 rows of tiles, the maximum that can fit on the screen.
	_draw_info.num_draw_cols = TILE_COLS + 1;
	_draw_info.num_draw_rows = TILE_ROWS + 1;

	// The default starting tile row and column is relative to the map camera's current position.
	_draw_info.starting_col = (_camera->x_position / 2) - HALF_TILE_COLS;
	_draw_info.starting_row = (_camera->y_position / 2) - HALF_TILE_ROWS;

	// ---------- (2) Determine the coordinates for the screen edges on the map grid

	_draw_info.top_edge    = camera_y - HALF_SCREEN_ROWS;
	_draw_info.bottom_edge = camera_y + HALF_SCREEN_ROWS;
	_draw_info.left_edge   = camera_x - HALF_SCREEN_COLS;
	_draw_info.right_edge  = camera_x + HALF_SCREEN_COLS;

	// ---------- (3) Check for special conditions that modify the drawing state

	// Usually the map centers on the camera's position, but when the camera becomes close to
	// the edges of the map, we need to modify the drawing properties of the frame.

	// Camera exceeds the left boundary of the map
	if (_draw_info.starting_col < 0) {
		_draw_info.starting_col = 0;
		_draw_info.tile_x_start = 1.0f;
		_draw_info.left_edge = 0.0f;
		_draw_info.right_edge = SCREEN_COLS;
	}
	// Camera exceeds the right boundary of the map
	else if (_draw_info.starting_col + TILE_COLS >= _num_tile_cols) {
		_draw_info.starting_col = static_cast<int16>(_num_tile_cols - TILE_COLS);
		_draw_info.tile_x_start = 1.0f;
		_draw_info.right_edge = static_cast<float>(_num_tile_cols * 2);
		_draw_info.left_edge = _draw_info.right_edge - SCREEN_COLS;
	}

	// Camera exceeds the top boundary of the map
	if (_draw_info.starting_row < 0) {
		_draw_info.starting_row = 0;
		_draw_info.tile_y_start = 2.0f;
		_draw_info.top_edge = 0.0f;
		_draw_info.bottom_edge = SCREEN_ROWS;
	}
	// Camera exceeds the bottom boundary of the map
	else if (_draw_info.starting_row + TILE_ROWS >= _num_tile_rows) {
		_draw_info.starting_row = static_cast<int16>(_num_tile_rows - TILE_ROWS);
		_draw_info.tile_y_start = 2.0f;
		_draw_info.bottom_edge = static_cast<float>(_num_tile_rows * 2);
		_draw_info.top_edge = _draw_info.bottom_edge - SCREEN_ROWS;
	}

	// Check for the conditions where the tile images align perfectly with the screen and one less row or column of tiles is drawn
	if (IsFloatInRange(_draw_info.tile_x_start, 0.999f, 1.001f)) { // Is the value approximately equal to 1.0f?
		_draw_info.num_draw_cols--;
	}
	if (IsFloatInRange(_draw_info.tile_y_start, 1.999f, 2.001f)) { // Is the value approximately equal to 2.0f?
		_draw_info.num_draw_rows--;
	}

// 	printf("--- MAP DRAW INFO ---\n");
// 	printf("Starting row, col: [%d, %d]\n", _draw_info.starting_row, _draw_info.starting_col);
// 	printf("# draw rows, cols: [%d, %d]\n", _draw_info.num_draw_rows, _draw_info.num_draw_cols);
// 	printf("Camera position:   [%f, %f]\n", camera_x, camera_y);
// 	printf("Tile draw start:   [%f, %f]\n", _draw_info.tile_x_start, _draw_info.tile_y_start);
// 	printf("Edges (T,D,L,R):   [%f, %f, %f, %f]\n", _draw_info.top_edge, _draw_info.bottom_edge, _draw_info.left_edge, _draw_info.right_edge);
} // void MapMode::_CalculateDrawInfo()


// Public draw function called by the main game loop
void MapMode::Draw() {
	_CalculateDrawInfo();

	// ---------- (1) Call Lua to determine if any lighting, etc. needs to be done before drawing
	// TODO
	// ---------- (2) Draw the lower tile layer
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->Move(_draw_info.tile_x_start, _draw_info.tile_y_start);
	for (uint32 r = static_cast<uint32>(_draw_info.starting_row);
			r < static_cast<uint32>(_draw_info.starting_row + _draw_info.num_draw_rows); r++) {
		for (uint32 c = static_cast<uint32>(_draw_info.starting_col);
				c < static_cast<uint32>(_draw_info.starting_col + _draw_info.num_draw_cols); c++) {
			if (_tile_grid[r][c].lower_layer >= 0) { // Draw a tile image if it exists at this location
				_tile_images[_tile_grid[r][c].lower_layer]->Draw();
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.num_draw_cols * 2), 2.0f);
	}

	// ---------- (2) Draw the middle tile layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(_draw_info.tile_x_start, _draw_info.tile_y_start);
	for (uint32 r = static_cast<uint32>(_draw_info.starting_row);
			r < static_cast<uint32>(_draw_info.starting_row + _draw_info.num_draw_rows); r++) {

		for (uint32 c = static_cast<uint32>(_draw_info.starting_col);
				c < static_cast<uint32>(_draw_info.starting_col + _draw_info.num_draw_cols); c++) {
			if (_tile_grid[r][c].middle_layer >= 0) { // Draw a tile image if it exists at this location
				_tile_images[_tile_grid[r][c].middle_layer]->Draw();
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}

		VideoManager->MoveRelative(static_cast<float>(_draw_info.num_draw_cols * -2), 2.0f);
	}

	// ---------- (3) Draw the ground object layer (first pass)
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i]->draw_on_second_pass == false) {
			_ground_objects[i]->Draw();
		}
	}

	// ---------- (4) Draw the pass object layer
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		_pass_objects[i]->Draw();
	}

	// ---------- (5) Draw the ground object layer (second pass)
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i]->draw_on_second_pass == true) {
			_ground_objects[i]->Draw();
		}
	}

	// ---------- (6) Draw the upper tile layer
	VideoManager->Move(_draw_info.tile_x_start, _draw_info.tile_y_start);
	for (uint32 r = static_cast<uint32>(_draw_info.starting_row);
			r < static_cast<uint32>(_draw_info.starting_row + _draw_info.num_draw_rows); r++) {
		for (uint32 c = static_cast<uint32>(_draw_info.starting_col);
				c < static_cast<uint32>(_draw_info.starting_col + _draw_info.num_draw_cols); c++) {
			if (_tile_grid[r][c].upper_layer >= 0) { // Draw a tile image if it exists at this location
				_tile_images[_tile_grid[r][c].upper_layer]->Draw();
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.num_draw_cols * 2), 2.0f);
	}

	// ---------- (7) Draw the sky object layer
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		_sky_objects[i]->Draw();
	}

	// ---------- (8) Call Lua to determine if any lighting, etc. needs to be done after drawing
	// TODO

	// ---------- (9) Draw the dialogue menu and text if necessary
	if (_map_state == DIALOGUE) {
		VideoManager->PushState();
		VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
		VideoManager->SetDrawFlags( VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0 );
		VideoManager->Move(0.0f, 768.0f);
		_dialogue_box.Draw();
		VideoManager->MoveRelative(47.0f, -42.0f);
		_dialogue_nameplate.Draw();

		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);
		VideoManager->SetTextColor(Color(Color::black));
		VideoManager->SetFont("map");
		VideoManager->MoveRelative(120.0f, -6.0f);
		VideoManager->DrawText( reinterpret_cast< VirtualSprite* >( _all_objects[ _current_dialogue->GetSpeaker() ] )->name );
		if ( reinterpret_cast< VirtualSprite* >( _all_objects[ _current_dialogue->GetSpeaker() ] )->face_portrait != NULL) {
			VideoManager->MoveRelative(0.0f, -26.0f);
			reinterpret_cast< VirtualSprite* >( _all_objects[ _current_dialogue->GetSpeaker() ] )->face_portrait->Draw();
		}
		//_dialogue_window.Show();
		_dialogue_textbox.Draw();
		VideoManager->PopState();
	}
} // void MapMode::_Draw()

// ****************************************************************************
// ************************* LUA BINDING FUNCTIONS ****************************
// ****************************************************************************

void MapMode::_AddGroundObject(private_map::MapObject *obj) {
	_ground_objects.push_back(obj);
	_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddPassObject(private_map::MapObject *obj) {
	_pass_objects.push_back(obj);
	_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddSkyObject(private_map::MapObject *obj) {
	_sky_objects.push_back(obj);
	_all_objects.insert(make_pair(obj->object_id, obj));
}

} // namespace hoa_map
