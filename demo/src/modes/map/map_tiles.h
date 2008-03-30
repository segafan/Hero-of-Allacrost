///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_tiles.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode tile management.
***
*** This code encapsulates everything related to tiles and tile management in
*** map mode.
*** ***************************************************************************/

#ifndef __MAP_TILES_HEADER__
#define __MAP_TILES_HEADER__

#include "defs.h"
#include "utils.h"

namespace hoa_map {

namespace private_map {

//! \brief The number of tiles that are found in a tileset image (512x512 pixel image containing 32x32 pixel tiles)
const uint32 TILES_PER_TILESET = 256;

/** ****************************************************************************
*** \brief Represents a single tile on the map.
***
*** The images that a tile uses are not stored within this class. They are
*** stored in the MapMode#_tile_images vector, and this class contains three
*** indices to images in that vector. This class also does not contain any
*** information about walkability. That information is kept in a seperate vector
*** in the MapMode class.
***
*** \note The reason that tiles do not contain walkability information is that
*** each tile is 32x32 pixels, but walkability is defined on a 16x16 granularity,
*** meaning that there are four "walkable" sections to each tile. Code such as
*** pathfinding is more simple if all walkability information is kept in a seperate
*** container.
***
*** \note The coordinate system in MapMode is in terms of tiles. Specifically,
*** the screen is defined to be 32 tile columns wide and 24 tile rows high. Using
*** 32x32 tile images, this corresponds to a screen resolution of 1024x768, which
*** is the default screen resolution of Allacrost. The origin [0.0f, 0.0f] is the
*** top-left corner of the screen and the bottom-right corner coordinates are
*** [32.0f, 24.0f]. Both map tiles and map objects in Allacrost are drawn on the
*** screen using the bottom middle of the image as its reference point.
*** ***************************************************************************/
class MapTile {
public:
	/** \name Tile Layer Indeces
	*** \brief Indeces to MapMode#_tile_images, mapping to the three tile layers.
	*** \note A value less than zero means that no image is registered to that tile layer.
	**/
	//@{
	int16 lower_layer, middle_layer, upper_layer;
	//@}

	MapTile()
		{ lower_layer = -1; middle_layer = -1; upper_layer = -1; }

	MapTile(int16 lower, int16 middle, int16 upper)
		{ lower_layer = lower; middle_layer = middle; upper_layer = upper; }
}; // class MapTile


/** ****************************************************************************
*** \brief A helper class to MapMode responsible for all tile data and operations
***
*** This class is responsible for loading, updating, and drawing all tile images
*** and managing the tile grid. TileManager does <b>not</b> manage the map
*** collision grid, which is used by map objects and sprites.
*** ***************************************************************************/
class TileManager {
	friend class hoa_map::MapMode;

public:
	TileManager();
	~TileManager();

	/** \brief Handles all operations on loading tilesets and tile images for the map's Lua file
	*** \param map_file A reference to the Lua file containing the map data. The file should be open before passing the reference
	*** \param map_instance A pointer to the MapMode object which invoked this function
	**/
	void Load(hoa_script::ReadScriptDescriptor& map_file, const MapMode* map_instance);

	//! \brief Updates all animated tile images
	void Update();

	/** \brief Draws the various tile layers to the screen
	*** \param frame A pointer to the already computed information required to draw this frame
	*** \note These functions do not reset the coordinate system and hence depend that the proper coordinate system
	*** is already set prior to these function calls (0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f). These functions do make
	*** modifications to the blending draw flag and the draw cursor position which are not restored by the function
	*** upon its return, so take measures to retain this information before calling these functions if required.
	**/
	//@{
	void DrawLowerLayer(const MapFrame* const frame);
	void DrawMiddleLayer(const MapFrame* const frame);
	void DrawUpperLayer(const MapFrame* const frame);
	//@}

private:
	/** \brief The number of tile rows in the map.
	*** This number must be greater than or equal to 24 for the map to be valid.
	**/
	uint16 _num_tile_rows;

	/** \brief The number of tile rows in the map.
	*** This number must be greater than or equal to 32 for the map to be valid.
	**/
	uint16 _num_tile_cols;

	/** \brief The current map context in index format
	*** This member should only ever be equal to 0-31, which corresponds to contexts #01-#32. Note th
	**/
	uint8 _current_context;

	/** \brief A 3D vector that contains all of the map's tile objects.
	*** The outer-most vector corresponds to each map context, hence its size is at least 1 and at most 32.
	*** The middle vector is the rows of tiles in the map, while the inner-most vector is the columns of tiles.
	*** The 0th element of the outer vector corresponds to map context #1, 1st element to context #2, and so on.
	**/
	std::vector<std::vector<std::vector<MapTile> > > _tile_grid;

	//! \brief Contains the images for all map tiles, both still and animate.
	std::vector<hoa_video::ImageDescriptor*> _tile_images;

	/** \brief Contains all of the animated tile images used on the map.
	*** The purpose of this vector is to easily update all tile animations without stepping through the
	*** _tile_images vector, which contains both still and animated images.
	***
	*** \note The elements in this vector point to the same AnimatedImages that are pointed to by _tile_images. Therefore,
	*** this vector should <b>not</b> have delete invoked on its elements, since delete is already invoked on _tile_images.
	**/
	std::vector<hoa_video::AnimatedImage*> _animated_tile_images;
}; // class TileManager

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_TILES_HEADER_
