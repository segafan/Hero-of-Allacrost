///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2013 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    editor_utils.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map editor utility code
*** *****************************************************************************/

#ifndef __EDITOR_UTILS_HEADER__
#define __EDITOR_UTILS_HEADER__

#include <QString>

#include "utils.h"

namespace hoa_editor {

//! Determines whether the code in the hoa_editor namespace should print debug statements or not.
extern bool EDITOR_DEBUG;

//! \brief The value used to indicate that no tile is placed at a particular location
const int32 NO_TILE = -1;

//! \brief Used to indicate a tile that has been selected as part of a multi-select
const int32 TILE_SELECTED = -2;

//! \brief Used to indicate a non-existing or invalid tile context ID
const int32 NO_CONTEXT = -1;

//! \brief The maximum number of contexts allowed on a map
const uint32 MAX_CONTEXTS = 32;

//! \brief The limits to the size dimensions of a map, in number of tiles
//@{
const int32 MINIMUM_MAP_LENGTH = 32;
const int32 MAXIMUM_MAP_LENGTH = 1000;
const int32 MINIMUM_MAP_HEIGHT = 24;
const int32 MAXIMUM_MAP_HEIGHT = 1000;
//@}

//! \brief The tile dimension sizes in number of pixels
//@{
const uint32 TILE_LENGTH = 32;
const uint32 TILE_HEIGHT = 32;
//@}

//! \brief The dimensions of a tileset image file in number of pixels
//@{
const uint32 TILESET_LENGTH = 512;
const uint32 TILESET_HEIGHT = 512;
//@}

//! \brief The dimensions of a tileset image file in number of tiles
//@{
const uint32 TILESET_NUM_COLS = 16;
const uint32 TILESET_NUM_ROWS = 16;
//@}

//! \brief The number of tiles that a tileset holds (TILESET_NUM_COLS * TILESET_NUM_ROWS)
const uint32 TILESET_NUM_TILES = 256;

//! \brief The dimensions of a tile's collision quadrant, in pixels
//@{
const uint32 TILE_QUADRANT_LENGTH = TILE_LENGTH / 2;
const uint32 TILE_QUADRANT_HEIGHT = TILE_HEIGHT / 2;
//@}

//! \brief The number of collision quadrants in a single tile
const uint32 TILE_NUM_QUADRANTS = 4;

//! \brief Various modes for tile editing
enum TILE_EDIT_MODE {
	INVALID_TILE   = -1,
	PAINT_TILE     =  0,
	MOVE_TILE      =  1,
	DELETE_TILE    =  2,
	TOTAL_TILE     =  3
};

//! \brief Represents different types of transition patterns for autotileable tiles.
enum TRANSITION_PATTERN_TYPE {
	INVALID_PATTERN     = -1,
	NW_BORDER_PATTERN   =  0,
	N_BORDER_PATTERN    =  1,
	NE_BORDER_PATTERN   =  2,
	E_BORDER_PATTERN    =  3,
	SE_BORDER_PATTERN   =  4,
	S_BORDER_PATTERN    =  5,
	SW_BORDER_PATTERN   =  6,
	W_BORDER_PATTERN    =  7,
	NW_CORNER_PATTERN   =  8,
	NE_CORNER_PATTERN   =  9,
	SE_CORNER_PATTERN   =  10,
	SW_CORNER_PATTERN   =  11,
	TOTAL_PATTERN       =  12
};

//! \brief The name of the editor application
const QString APP_NAME("Allacrost Map Editor");

} // namespace hoa_editor

#endif // __EDITOR_UTILS_HEADER__
