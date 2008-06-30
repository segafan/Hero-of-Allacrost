///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_objects.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode objects.
*** ***************************************************************************/

#include <algorithm> //For std::replace in the MapTreasure saving code

#include "utils.h"

#include "audio.h"
#include "mode_manager.h"
#include "system.h"
#include "video.h"

#include "global.h"

#include "map.h"
#include "map_actions.h"
#include "map_dialogue.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "battle.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_mode_manager;
using namespace hoa_video;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_battle;

namespace hoa_map {

namespace private_map {


// *****************************************************************************
// ************************ MapObject Class Functions **************************
// *****************************************************************************

MapObject::MapObject() :
	object_id(-1),
	context(MAP_CONTEXT_01),
	x_position(-1),
	y_position(-1),
	x_offset(0.0f),
	y_offset(0.0f),
	img_half_width(0.0f),
	img_height(0.0f),
	coll_half_width(0.0f),
	coll_height(0.0f),
	updatable(true),
	visible(true),
	no_collision(false),
	draw_on_second_pass(false)
{}



bool MapObject::DrawHelper() {
	if (visible == false)
		return false;

	// Store the full x and y position coordinates of the sprite in a single pair of variables
	float x_pos = static_cast<float>(x_position) + x_offset;
	float y_pos = static_cast<float>(y_position) + y_offset;

	// ---------- (1) Determine if the sprite is off-screen and if so, don't draw it.

	if (x_pos + img_half_width < MapMode::_current_map->_draw_info.left_edge  ||
		x_pos - img_half_width > MapMode::_current_map->_draw_info.right_edge ||
		y_pos - img_height > MapMode::_current_map->_draw_info.bottom_edge    ||
		y_pos < MapMode::_current_map->_draw_info.top_edge) {
		return false;
	}

	// ---------- (2) Calculate the drawing coordinates and move the drawing cursor

	VideoManager->Move(x_pos - MapMode::_current_map->_draw_info.left_edge, y_pos - MapMode::_current_map->_draw_info.top_edge);
	return true;
} // bool MapObject::DrawHelper()

// ****************************************************************************
// ********************* PhysicalObject Class Functions ***********************
// ****************************************************************************

PhysicalObject::PhysicalObject() :
	current_animation(0)
{
	MapObject::_object_type = PHYSICAL_TYPE;
}



PhysicalObject::~PhysicalObject() {
	animations.clear();
}



void PhysicalObject::Update() {
	if (updatable)
		animations[current_animation].Update();
}



void PhysicalObject::Draw() {
	if (MapObject::DrawHelper() == true)
		animations[current_animation].Draw();
}

// *****************************************************************************
// *********************** MapTreasure Class Functions *************************
// *****************************************************************************

MapTreasure::MapTreasure(string image_file, uint8 num_total_frames, uint8 num_closed_frames, uint8 num_open_frames) :
	_empty(false),
	_drunes(0)
{
	MapObject::_object_type = TREASURE_TYPE;
	const uint32 DEFAULT_FRAME_TIME = 10; // The default number of milliseconds for frame animations
	std::vector<StillImage> frames;

	// (1) Load a the single row, multi column multi image containing all the treasure frames
	if (ImageDescriptor::LoadMultiImageFromElementGrid(frames, image_file, 1, num_total_frames) == false ) {
		cerr << "File: " << filename << " could not be loaded correctly." << endl;
		// TODO: throw exception
		return;
	}
	// Update the frame image sizes to work in the MapMode coordinate system
	for (uint32 i = 0; i < frames.size(); i++) {
		frames[i].SetWidth(frames[i].GetWidth() / HALF_TILE_COLS);
		frames[i].SetHeight(frames[i].GetHeight() / HALF_TILE_COLS);
	}

	// (2) Now that we know the total number of frames in the image, make sure the frame count arguments make sense
	if (num_closed_frames == 0 || num_open_frames == 0 || num_closed_frames >= num_total_frames || num_open_frames >= num_total_frames) {
		// TODO: throw exception
		return;
	}

	// (3) Dissect the frames and create the closed, opening, and open animations Create the close and opening animations for the chest
	hoa_video::AnimatedImage closed_anim;
	for (uint8 i = 0; i < num_closed_frames; i++) {
		closed_anim.AddFrame(frames[i], DEFAULT_FRAME_TIME);
	}

	hoa_video::AnimatedImage open_anim;
	for (uint8 i = num_total_frames - num_open_frames; i < num_total_frames; i++) {
		open_anim.AddFrame(frames[i], DEFAULT_FRAME_TIME);
	}

	hoa_video::AnimatedImage opening_anim;
	opening_anim.SetNumberLoops(0); // Only loop the opening animation once, not infinitely
	if (num_total_frames - num_closed_frames - num_open_frames <= 0) {
		opening_anim = open_anim;
	}
	else {
		for (uint8 i = 0 + num_closed_frames; i < num_total_frames - num_open_frames; i++) {
			opening_anim.AddFrame(frames[i], DEFAULT_FRAME_TIME);
		}
	}

	AddAnimation(closed_anim);
	AddAnimation(opening_anim);
	AddAnimation(open_anim);

	// (4) Set the collision rectangle according to the dimensions of the first frame
	SetCollHalfWidth(frames[0].GetWidth() / 2.0f);
	SetCollHeight(frames[0].GetHeight());
} // MapTreasure::MapTreasure(string image_file, uint8 num_closed_frames = 1, uint8 num_open_frames = 1)



MapTreasure::~MapTreasure() {
	for (uint32 i = 0; i < _objects_list.size(); i++) {
		delete _objects_list[i];
	}
}



void MapTreasure::LoadSaved() {
	//Add an event in the group having the ObjectID of the chest as name
	string event_name = "chest_" + NumberToString(GetObjectID());
	if (MapMode::_loading_map->_map_event_group->DoesEventExist(event_name)) {
		if (MapMode::_loading_map->_map_event_group->GetEvent(event_name) == TREASURE_EMPTY) {
			SetCurrentAnimation(OPEN_ANIM);
			_drunes = 0;
			for (uint32 i = 0; i < _objects_list.size(); i++)
				delete _objects_list[i];
			_objects_list.clear();
			_empty = true;
		}
	}
}



bool MapTreasure::AddObject(uint32 id, uint32 number) {
	hoa_global::GlobalObject* obj = GlobalCreateNewObject(id, number);

	if (obj == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "invalid object id argument passed to function" << endl;
		return false;
	}

	_objects_list.push_back(obj);
	return true;
}



void MapTreasure::Update() {
	PhysicalObject::Update();
	if (current_animation == OPENING_ANIM && animations[OPENING_ANIM].IsLoopsFinished() == true) {
		SetCurrentAnimation(OPEN_ANIM);
	}
}



void MapTreasure::Open() {
	if (_empty == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to open an empty map treasure" << endl;
		return;
	}

	SetCurrentAnimation(OPENING_ANIM);

	// Add an event to the map group indicating that the chest has now been opened
	string event_name = "chest_" + NumberToString(GetObjectID());

	if (MapMode::_current_map->_map_event_group->DoesEventExist(event_name) == true) {
		MapMode::_current_map->_map_event_group->SetEvent(event_name, TREASURE_EMPTY);
	}
	else {
		MapMode::_current_map->_map_event_group->AddNewEvent(event_name, TREASURE_EMPTY);
	}

	// Initialize the treasure menu to display the contents of the open treasure
	MapMode::_current_map->_treasure_menu->Initialize(this);
}

// *****************************************************************************
// ********************** ObjectManager Class Functions ************************
// *****************************************************************************

ObjectManager::ObjectManager() :
	_num_grid_rows(0),
	_num_grid_cols(0),
	_lastID(1000)
{
	_virtual_focus = new VirtualSprite();
	_virtual_focus->SetXPosition(0, 0.0f);
	_virtual_focus->SetYPosition(0, 0.0f);
	_virtual_focus->movement_speed = NORMAL_SPEED;
	_virtual_focus->SetNoCollision(true);
	_virtual_focus->SetVisible(false);
}



ObjectManager::~ObjectManager() {
	// Delete all of the map objects
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		delete(_ground_objects[i]);
	}
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		delete(_pass_objects[i]);
	}
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		delete(_sky_objects[i]);
	}
	delete(_virtual_focus);
}



void ObjectManager::Load(hoa_script::ReadScriptDescriptor& map_file) {
	// ---------- Construct the collision grid
	map_file.OpenTable("map_grid");
	_num_grid_rows = map_file.GetTableSize();
	for (uint16 r = 0; r < _num_grid_rows; r++) {
		_collision_grid.push_back(vector<uint32>());
		map_file.ReadUIntVector(r, _collision_grid.back());
	}
	map_file.CloseTable();
	_num_grid_cols = _collision_grid[0].size();

	// ---------- (7) Load the saved states of all the objects
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		_ground_objects[i]->LoadSaved();
	}

	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		_pass_objects[i]->LoadSaved();
	}

	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		_sky_objects[i]->LoadSaved();
	}
}


void ObjectManager::Update() {
	for( uint32 i = 0; i < _zones.size(); i++ ) {
		_zones[i]->Update();
	}

	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		_ground_objects[i]->Update();
	}
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		_pass_objects[i]->Update();
	}
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		_sky_objects[i]->Update();
	}
}



void ObjectManager::SortObjects(){
	std::sort(_ground_objects.begin(), _ground_objects.end(), MapObject_Ptr_Less());
// 	std::sort(_pass_objects.begin(), _pass_objects.end(), MapObject_Ptr_Less());
// 	std::sort(_sky_objects.begin(), _sky_objects.end(), MapObject_Ptr_Less());
}



void ObjectManager::DrawGroundObjects(const MapFrame* const frame, const bool second_pass) {
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i]->draw_on_second_pass == second_pass) {
			if (_ground_objects[i]->context == MapMode::_current_map->_current_context) {
				_ground_objects[i]->Draw();
			}
		}
	}

}



void ObjectManager::DrawPassObjects(const MapFrame* const frame) {
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		if (_ground_objects[i]->context == MapMode::_current_map->_current_context) {
			_pass_objects[i]->Draw();
		}
	}
}



void ObjectManager::DrawSkyObjects(const MapFrame* const frame) {
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		_sky_objects[i]->Draw();
	}
}



MapObject* ObjectManager::FindNearestObject(const VirtualSprite* sprite) {
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
		IF_PRINT_WARNING(MAP_DEBUG) << "sprite was set to invalid direction" << endl;
		return NULL;
	}

	// A vector to contain objects which are valid for the sprite to interact with
	vector<MapObject*> valid_objects;
	// A pointer to the object which has been found to be the closest to the sprite within valid_objs
	MapObject* closest = NULL;

	// ---------- (2): Go through all objects and determine which (if any) are valid
	for (map<uint16, MapObject*>::iterator i = _all_objects.begin(); i != _all_objects.end(); i++) {
		MapObject* obj = i->second;
		if (obj == sprite) // A sprite can't target itself
			continue;

		// Objects in different contexts can not interact with one another
		if ((obj->context & sprite->context) == 0) // Since objects can span multiple context, we check that no contexts are equal
			continue;

		// Compute the full position coordinates for the object under study
		float other_x_location = obj->ComputeXLocation();
		float other_y_location = obj->ComputeYLocation();

		// Verify that the bounding boxes overlap on the horizontal axis
		if (!(other_x_location - obj->coll_half_width > right || other_x_location + obj->coll_half_width < left)) {
			// Verify that the bounding boxes overlap on the vertical axis
			if (!(other_y_location - obj->coll_height > bottom || other_y_location < top )) {
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
	float min_distance = fabs(source_x - closest->ComputeXLocation()) + fabs(source_y - closest->ComputeYLocation());

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
} // MapObject* ObjectManager::FindNearestObject(VirtualSprite* sprite)



bool ObjectManager::DetectCollision(VirtualSprite* sprite) {
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
	if (cr_left < 0.0f || cr_top < 0.0f || cr_right >= static_cast<float>(_num_grid_cols) ||
		y_location >= static_cast<float>(_num_grid_rows)) {
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
				if ((_collision_grid[r][c] & sprite->context) > 0) { // Then this overlapping tile is unwalkable
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
			&& ((*objects)[i]->context & sprite->context) > 0 )
		{
			// Compute the full position coordinates of the other object
			float other_x_location = (*objects)[i]->ComputeXLocation();
			float other_y_location = (*objects)[i]->ComputeYLocation();;

			// Verify that the bounding boxes overlap on the horizontal axis
			if (!(other_x_location - (*objects)[i]->coll_half_width > cr_right
				|| other_x_location + (*objects)[i]->coll_half_width < cr_left)) {
				// Verify that the bounding boxes overlap on the vertical axis
				if (!(other_y_location - (*objects)[i]->coll_height > y_location || other_y_location < cr_top )) {
					// Boxes overlap on both axis, there is a colision
					if (sprite->GetType() == ENEMY_TYPE && (*objects)[i] == MapMode::_current_map->_camera) {
						EnemySprite *enemy = reinterpret_cast<EnemySprite*>(sprite);
						if (enemy->IsHostile()) {
							enemy->ChangeStateDead();
							BattleMode *BM = new BattleMode();
							string enemy_battle_music = enemy->GetBattleMusicTheme();
							if (enemy_battle_music != "")
								BM->AddMusic(enemy_battle_music);
							ModeManager->Push(BM);
							const vector<uint32>& enemy_party = enemy->RetrieveRandomParty();
							for (uint32 i = 0; i < enemy_party.size(); i++) {
								BM->AddEnemy(enemy_party[i]);
							}
							return false;
						}
					}

					if ((*objects)[i]->GetType() == ENEMY_TYPE && sprite == MapMode::_current_map->_camera) {
						EnemySprite *enemy = reinterpret_cast<EnemySprite*>((*objects)[i]);
						if (enemy->IsHostile()) {
							enemy->ChangeStateDead();
							BattleMode *BM = new BattleMode();
							string enemy_battle_music = enemy->GetBattleMusicTheme();
							if (enemy_battle_music != "")
								BM->AddMusic(enemy_battle_music);
							ModeManager->Push(BM);
							const vector<uint32>& enemy_party = enemy->RetrieveRandomParty();
							for (uint32 i = 0; i < enemy_party.size(); i++) {
								BM->AddEnemy(enemy_party[i]);
							}
							return false;
						}
					}
					return true;
				}
			}
		}
	}

	// No collision was detected
	return false;
} // bool ObjectManager::DetectCollision(VirtualSprite* sprite)



void ObjectManager::FindPath(const VirtualSprite* sprite, std::vector<PathNode>& path, const PathNode& dest) {
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
		(dest.col + x_span >= (_num_grid_cols)) || (dest.row >= (_num_grid_rows))) {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: sprite can not move to destination node on path because it exceeds map boundaries" << endl;
		return;
	}
	for (int16 r = dest.row - y_span; r < dest.row; r++) {
		for (int16 c = dest.col - x_span; c < dest.col + x_span; c++) {
			if ((_collision_grid[r][c] & sprite->context) > 0) {
				if (MAP_DEBUG)
					cerr << "MAP ERROR: sprite can not move to destination node on path because one or more grid tiles are unwalkable" << endl;
				return;
			}
		}
	}

	open_list.push_back(source_node);

	while (!open_list.empty()) {
// 		sort(open_list.begin(), open_list.end(), PathNode::NodePred());
		sort(open_list.begin(), open_list.end());
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
				(nodes[i].col + x_span >= _num_grid_cols) || (nodes[i].row >= _num_grid_rows)) {
				continue;
			}

			// ---------- (B): Check that all grid nodes that the sprite's collision rectangle will overlap are walkable
			bool continue_loop = true;
			for (int16 r = nodes[i].row - y_span; r < nodes[i].row && continue_loop; r++) {
				for (int16 c = nodes[i].col - x_span; c < nodes[i].col + x_span; c++) {
					if ((_collision_grid[r][c] & sprite->context) > 0) {
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
	std::reverse(path.begin(), path.end());
} // void ObjectManager::FindPath(const VirtualSprite* sprite, std::vector<PathNode>& path, const PathNode& dest)



MapObject* ObjectManager::GetObject(uint32 object_id) {
	return _all_objects[object_id];
}

} // namespace private_map

} // namespace hoa_map
