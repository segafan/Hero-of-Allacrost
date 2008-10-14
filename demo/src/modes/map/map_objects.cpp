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

// Allacrost utilities
#include "utils.h"

// Allacrost engines
#include "audio.h"
#include "mode_manager.h"
#include "system.h"
#include "video.h"

// Allacrost globals
#include "global.h"

// Other game mode headers
#include "battle.h"

// Local map mode headers
#include "map.h"
#include "map_actions.h"
#include "map_dialogue.h"
#include "map_objects.h"
#include "map_sprites.h"



using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_video;
using namespace hoa_global;
using namespace hoa_battle;

namespace hoa_map {

namespace private_map {


// *****************************************************************************
// ********** MapObject Class Functions
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
	sky_object(false),
	draw_on_second_pass(false)
{}



bool MapObject::ShouldDraw() {
	if (visible == false)
		return false;

	if (context != MapMode::_current_map->_current_context)
		return false;

	// ---------- Determine if the sprite is off-screen and if so, don't draw it.
	MapRectangle img_rect;
	GetImageRectangle(img_rect);
	if (MapRectangle::CheckIntersection(img_rect, MapMode::_current_map->_draw_info.screen_edges) == false)
		return false;

	// ---------- (1) Determine the center position coordinates for the camera
	float x_pos, y_pos; // Holds the final X, Y coordinates of the camera
	float x_pixel_length, y_pixel_length; // The X and Y length values that coorespond to a single pixel in the current coodinate system
	float rounded_x_offset, rounded_y_offset; // The X and Y position offsets of the object, rounded to perfectly align on a pixel boundary


	// TODO: the call to GetPixelSize() will return the same result every time so long as the coordinate system did not change. If we never
	// change the coordinate system in map mode, then this should be done only once and the calculated values should be saved for re-use.
	// However, we've discussed the possiblity of adding a zoom feature to maps, in which case we need to continually re-calculate the pixel size
	VideoManager->GetPixelSize(x_pixel_length, y_pixel_length);
	rounded_x_offset = FloorToFloatMultiple(x_offset, x_pixel_length);
	rounded_y_offset = FloorToFloatMultiple(y_offset, y_pixel_length);
	x_pos = static_cast<float>(x_position) + rounded_x_offset;
	y_pos = static_cast<float>(y_position) + rounded_y_offset;

	// ---------- Move the drawing cursor to the appropriate coordinates for this sprite
	VideoManager->Move(x_pos - MapMode::_current_map->_draw_info.screen_edges.left, y_pos - MapMode::_current_map->_draw_info.screen_edges.top);
	return true;
} // bool MapObject::DrawHelper()



void MapObject::GetCollisionRectangle(MapRectangle& rect) const {
	float x_pos = static_cast<float>(x_position) + x_offset;
	float y_pos = static_cast<float>(y_position) + y_offset;

	rect.left = x_pos - coll_half_width;
	rect.right = x_pos + coll_half_width;
	rect.top = y_pos - coll_height;
	rect.bottom = y_pos;
}



void MapObject::GetImageRectangle(MapRectangle& rect) const {
	float x_pos = static_cast<float>(x_position) + x_offset;
	float y_pos = static_cast<float>(y_position) + y_offset;

	rect.left = x_pos - img_half_width;
	rect.right = x_pos + img_half_width;
	rect.top = y_pos - img_height;
	rect.bottom = y_pos;
}

// ****************************************************************************
// ********** PhysicalObject Class Functions
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
	if (MapObject::ShouldDraw() == true)
		animations[current_animation].Draw();
}

// *****************************************************************************
// ********** ObjectManager Class Functions
// *****************************************************************************

ObjectManager::ObjectManager() :
	_num_grid_rows(0),
	_num_grid_cols(0),
	_last_id(1000)
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



MapObject* ObjectManager::GetObject(uint32 object_id) {
	map<uint16, MapObject*>::iterator i = _all_objects.find(object_id);

	if (i == _all_objects.end())
		return NULL;
	else
		return i->second;
}



void ObjectManager::SortObjects() {
	std::sort(_ground_objects.begin(), _ground_objects.end(), MapObject_Ptr_Less());
	std::sort(_pass_objects.begin(), _pass_objects.end(), MapObject_Ptr_Less());
	std::sort(_sky_objects.begin(), _sky_objects.end(), MapObject_Ptr_Less());
}



void ObjectManager::Load(ReadScriptDescriptor& map_file) {
	// ---------- Construct the collision grid
	map_file.OpenTable("map_grid");
	_num_grid_rows = map_file.GetTableSize();
	for (uint16 r = 0; r < _num_grid_rows; r++) {
		_collision_grid.push_back(vector<uint32>());
		map_file.ReadUIntVector(r, _collision_grid.back());
	}
	map_file.CloseTable();
	_num_grid_cols = _collision_grid[0].size();
}



void ObjectManager::Update() {
	for (uint32 i = 0; i < _zones.size(); i++) {
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



void ObjectManager::DrawGroundObjects(const MapFrame* const frame, const bool second_pass) {
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i]->draw_on_second_pass == second_pass) {
			_ground_objects[i]->Draw();
		}
	}

}



void ObjectManager::DrawPassObjects(const MapFrame* const frame) {
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		_pass_objects[i]->Draw();
	}
}



void ObjectManager::DrawSkyObjects(const MapFrame* const frame) {
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		_sky_objects[i]->Draw();
	}
}



MapObject* ObjectManager::FindNearestObject(const VirtualSprite* sprite, float search_distance) {
	// NOTE: We don't check if the argument is NULL here for performance reasons
	MapRectangle search_area;

	// ---------- (1) Using the sprite's direction, determine the boundaries of the search area to check for objects
	sprite->GetCollisionRectangle(search_area);
	if (sprite->direction & FACING_NORTH) {
		search_area.bottom = search_area.top;
		search_area.top = search_area.top - search_distance;
	}
	else if (sprite->direction & FACING_SOUTH) {
		search_area.top = search_area.bottom;
		search_area.bottom = search_area.bottom + search_distance;
	}
	else if (sprite->direction & FACING_WEST) {
		search_area.right = search_area.left;
		search_area.left = search_area.left - search_distance;
	}
	else if (sprite->direction & FACING_EAST) {
		search_area.left = search_area.right;
		search_area.right = search_area.right + search_distance;
	}
	else {
		IF_PRINT_WARNING(MAP_DEBUG) << "sprite was set to invalid direction: " << sprite->direction << endl;
		return NULL;
	}

	// ---------- (2) Go through all objects and determine which (if any) lie within the search area
	vector<MapObject*> valid_objects; // A vector to hold objects which are inside the search area (either partially or fully)
	vector<MapObject*>* search_vector = NULL; // A pointer to the vector of objects to search

	// Only search the object layer that the sprite resides on. Note that we do not consider searching the pass layer.
	if (sprite->sky_object == true)
		search_vector = &_sky_objects;
	else
		search_vector = &_ground_objects;

	for (vector<MapObject*>::iterator i = (*search_vector).begin(); i != (*search_vector).end(); i++) {
		if (*i == sprite) // Don't allow the sprite itself to be considered in the search
			continue;

		// If the object and sprite do not exist in the same context, do not consider the object for the search
		if (((*i)->context & sprite->context) == 0)
			continue;

		MapRectangle object_rect;
		(*i)->GetCollisionRectangle(object_rect);
		if (MapRectangle::CheckIntersection(object_rect, search_area) == true)
			valid_objects.push_back(*i);
	} // for (map<MapObject*>::iterator i = _all_objects.begin(); i != _all_objects.end(); i++)

	// ---------- (3) Check for early exit conditions
	if (valid_objects.empty() == true) {
		return NULL;
	}
	else if (valid_objects.size() == 1) {
		return valid_objects[0];
	}

	// ---------- (4) Figure out which of the valid objects is the closest to the sprite
	// NOTE: For simplicity, we use the Manhattan distance to determine which object is the closest
	MapObject* closest_obj = valid_objects[0];

	// Used to hold the full position coordinates of the sprite
	float source_x = sprite->ComputeXLocation();
	float source_y = sprite->ComputeYLocation();
	// Holds the minimum distance found between the sprite and a valid object
	float min_distance = fabs(source_x - closest_obj->ComputeXLocation()) +
		fabs(source_y - closest_obj->ComputeYLocation());

	for (uint32 i = 1; i < valid_objects.size(); i++) {
		float dist = fabs(source_x - valid_objects[i]->ComputeXLocation()) +
			fabs(source_y - valid_objects[i]->ComputeYLocation());
		if (dist < min_distance) {
			closest_obj = valid_objects[i];
			min_distance = dist;
		}
	}
	return closest_obj;
} // MapObject* ObjectManager::FindNearestObject(VirtualSprite* sprite, float search_distance)



bool ObjectManager::CheckMapCollision(const private_map::MapObject* const obj) {
	// NOTE: We don't check if the argument is NULL here for performance reasons
	if (obj->no_collision == true) {
		return false;
	}

	MapRectangle coll_rect;
	obj->GetCollisionRectangle(coll_rect);

	// Check if any part of the object's collision rectangle is outside of the map boundary
	if (coll_rect.left < 0.0f || coll_rect.right >= static_cast<float>(_num_grid_cols) ||
		coll_rect.top < 0.0f || coll_rect.bottom >= static_cast<float>(_num_grid_rows)) {
		return true;
	}

	// Tile based collision is not done for objects in the sky layer
	if (obj->sky_object == true) {
		return false;
	}

	// Determine if the object's collision rectangle overlaps any unwalkable tiles
	// Note that because the sprite's collision rectangle was previously determined to be within the map bounds,
	// the map grid tile indeces referenced in this loop are all valid entries and do not need to be checked.
	for (uint32 r = static_cast<uint32>(coll_rect.top); r <= static_cast<uint32>(coll_rect.bottom); r++) {
		for (uint32 c = static_cast<uint32>(coll_rect.left); c <= static_cast<uint32>(coll_rect.right); c++) {
			// Checks the collision grid at the row-column at the object's current context
			if ((_collision_grid[r][c] & obj->context) != 0) {
				return true;
			}
		}
	}

	return false;
}



bool ObjectManager::CheckObjectCollision(const MapRectangle& rect, const private_map::MapObject* const obj) {
	// NOTE: We don't check if the argument is NULL here for performance reasons
	MapRectangle obj_rect;
	obj->GetCollisionRectangle(obj_rect);
	return MapRectangle::CheckIntersection(rect, obj_rect);
}



bool ObjectManager::DoObjectsCollide(const MapObject* const obj1, const MapObject* const obj2) {
	// NOTE: We don't check if the arguments are NULL here for performance reasons

	// Check if either of the two objects have the no_collision property enabled
	if (obj1->no_collision == true || obj2->no_collision == true) {
		return false;
	}

	// If the two objects are not contained within the same context, they can not overlap
	if (obj1->context != obj2->context) {
		return false;
	}

	MapRectangle rect1, rect2;
	obj1->GetCollisionRectangle(rect1);
	obj2->GetCollisionRectangle(rect2);

	return MapRectangle::CheckIntersection(rect1, rect2);
}



bool ObjectManager::DetectCollision(VirtualSprite* sprite) {
	// NOTE: We don't check if the argument is NULL here for performance reasons

	// ---------- (1) Check that the sprite does not collide with the map tiles or boundaries
	if (CheckMapCollision(sprite) == true)
		return true;

	// If the sprite has this property set, no further checks are needed
	if (sprite->no_collision == true) {
		return false;
	}

	// ---------- (2) Determine which set of objects to do collision detection with
	MapObject* collide_obj = NULL;
	vector<MapObject*>* objects = NULL; // A pointer to the layer of objects to do the collision detection with

	if (sprite->sky_object == false)
		objects = &_ground_objects;
	else
		objects = &_sky_objects;

	// ---------- (3) Check collision areas for all objects matching the layer and context of the sprite

	MapRectangle sprite_rect;
	sprite->GetCollisionRectangle(sprite_rect);

	for (uint32 i = 0; i < objects->size(); i++) {
		// Check for conditions where we would not want to do collision detection between the two objects
		if ((*objects)[i]->object_id == sprite->object_id) // Object and sprite are the same
			continue;
		if ((*objects)[i]->no_collision == true) // Object has no collision detection property set
			continue;
		if (((*objects)[i]->context & sprite->context) == 0) // Sprite and object do not exist in the same context
			continue;

		if (CheckObjectCollision(sprite_rect, (*objects)[i]) == true) {
			collide_obj = (*objects)[i];
			break;
		}
	}

	if (collide_obj == NULL) {
		return false;
	}

	// ---------- (4) If a collision was detected, determine any further appropriate action
	// TODO: this code needs to be removed, relocated, or improved
	// Check if the map camera collided with an enemy sprite
	EnemySprite* enemy = NULL;
	if (sprite == MapMode::_current_map->_camera && collide_obj->GetType() == ENEMY_TYPE) {
		enemy = reinterpret_cast<EnemySprite*>(collide_obj);
	}
	else if (collide_obj == MapMode::_current_map->_camera && sprite->GetType() == ENEMY_TYPE) {
		enemy = reinterpret_cast<EnemySprite*>(sprite);
	}
	else { // No enemy collision, nothing left to do here but report the collision
		return true;
	}

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
	}

	return true;
} // bool ObjectManager::DetectCollision(VirtualSprite* sprite)



void ObjectManager::FindPath(const VirtualSprite* sprite, vector<PathNode>& path, const PathNode& dest) {
	// NOTE: Refer to the implementation of the A* algorithm to understand what all these lists and score values are for
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
		PRINT_ERROR << "source node is same as destination in MapMode::_FindPath()" << endl;
		return;
	}

	// Check that the destination is valid for the sprite to move to
	if ((dest.col - x_span < 0) || (dest.row - y_span < 0) ||
		(dest.col + x_span >= (_num_grid_cols)) || (dest.row >= (_num_grid_rows))) {
		PRINT_ERROR << "sprite can not move to destination node on path because it exceeds map boundaries" << endl;
		return;
	}
	for (int16 r = dest.row - y_span; r < dest.row; r++) {
		for (int16 c = dest.col - x_span; c < dest.col + x_span; c++) {
			if ((_collision_grid[r][c] & sprite->context) > 0) {
				PRINT_ERROR << "sprite can not move to destination node on path because one or more grid tiles are unwalkable" << endl;
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

			// ---------- (C): Check if the node is already in the closed list
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
		IF_PRINT_WARNING(MAP_DEBUG) << "could not find path to destination" << endl;
		path.push_back(source_node);
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

} // namespace private_map

} // namespace hoa_map
