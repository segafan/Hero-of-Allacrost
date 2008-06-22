///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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

//#include <QHeaderView>
#include <QImageReader>
#include <QMessageBox>
#include <QRect>
#include <Q3Table>
//#include <QTableWidgetItem>
#include <QVariant>

#include "defs.h"
#include "script.h"
#include "utils.h"
#include "video.h"

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

/*	struct AnimatedTileData {
		int tile_id;
		int time;
	};
*/
//! Width and height of a tile in pixels.
const int TILE_WIDTH  = 32;
const int TILE_HEIGHT = 32;

/** ****************************************************************************
*** \brief Manages individual tiles in a tileset and everything related with
***        them, such as walkability, animations, and so on.
*** ***************************************************************************/
class Tileset
{
public:
	Tileset(QWidget* parent);

	//! Tileset constructor. Name needs to be the name of the tileset, not the filename.
	Tileset(QWidget* parent, const QString& name);

	~Tileset();

	/** \brief Loads the tileset definition file and stores its data in the class containers
	*** \param name The name of the tileset (not the filename)
	*** \return True if the tileset was loaded successfully
	*** \note This function will clear the previously loaded contents when it is called
	**/
	bool Load(const QString& name);

	//void Save();

	//! The parent QWidget of this tileset, used for upward propagation of error messages
	QWidget* owner;

	//! The name of the tileset this table is representing.
	QString tileset_name;

	//! Contains the StillImage tiles of the tileset, used in grid.cpp.
	std::vector<hoa_video::StillImage> tiles;

	//! Contains walkability information for each tile.
	std::map<int, std::vector<int32> > walkability;

	//! Contains autotiling information for any autotileable tile.
	std::map<int, std::string> autotileability;

	//! Reference to the table implementation of this tileset in the bottom of the editor.
	Q3Table* table;

private:
	//std::vector<std::vector<AnimatedTileData> > _animated_tiles;
}; // class Tileset

} // namespace hoa_editor

#endif // __TILESET_HEADER__
