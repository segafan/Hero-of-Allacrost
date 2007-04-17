///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    tileset.h
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Header file for editor's tileset, used for maintaining a visible
 *          "list" of tiles to select from for painting on a map.
 *****************************************************************************/
			   
#ifndef __TILESET_HEADER__
#define __TILESET_HEADER__

#include "utils.h"
#include "defs.h"
#include "script.h"
#include "video.h"

#include <QMessageBox>
#include <Q3Table>
#include <QDir>
#include <QImageReader>
#include <QRect>

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

//! Width and height of a tile in pixels.
const int TILE_WIDTH  = 32;
const int TILE_HEIGHT = 32;

/** ****************************************************************************
*** \brief Manages individual tiles in a table with drag and drop capability.
*** \note Inherits from the QTable class.
*** ***************************************************************************/
class TilesetTable : public Q3Table
{
public:
	//! TilesetTable constructor. Name needs to be the name, not the filename.
	TilesetTable(QWidget* parent, const QString& name);

	~TilesetTable();

	//! The name of the tileset this table is representing.
	QString tileset_name;

	//! Contains the StillImage tiles of the tileset, used in grid.cpp
	std::vector<hoa_video::StillImage> tiles;
	// TODO: implement some sort of dynamic table resizing on window resize
}; // class Tileset

} // namespace hoa_editor

#endif
// __TILESET_HEADER__
