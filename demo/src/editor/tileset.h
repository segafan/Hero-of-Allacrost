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
#include "tile.h"

#include <QMessageBox>
#include <Q3Table>
#include <QDir>
#include <QImageReader>
#include <QRect>

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

class TileDatabase;

/*!****************************************************************************
 * \brief Manages individual tiles into a QTable with drag and drop
 * capability.
 * 
 * \note Inherits QTable.
 *****************************************************************************/
class TilesetTable : public Q3Table
{
	public:
		//! TilesetTable constructor. Name needs to be the name, not the filename.
		TilesetTable(QWidget* parent, const QString& name);    // constructor
		~TilesetTable();                                       // destructor

		//! The name of the tileset this table is representing.
		QString tileset_name;
		//! Contains the StillImage tiles of the tileset, used in grid.cpp
		std::vector<hoa_video::StillImage> tiles;
		// TODO: implement some sort of dynamic table resizing on window resize
}; // class Tileset

class DbTile
{
	public:
		DbTile();   // constructor
		//! Constructs a database tile with file_name filename and walkability property of walkable.
		DbTile(const QString& filename, int walkable);
		~DbTile();  // destructor

		//! The file name of this tile.
		QString file_name;
		//! An integer from 0-255 specifying how many layers of this tile are walkable.
		int walkability;
};

class TileSet
{
	public:
		//! Creates a new tileset attached to a database
		TileSet(TileDatabase* db);
		//! Loads a tileset using the specified database. name is just the name of the tileset,
		//! not the filename!
		TileSet(TileDatabase* db, const QString& name);
		~TileSet();  // destructor

		//! Adds a tile from the database to the tileset.
		void AddTile(const QString& tile_name);
		//! Removes a tile from the tileset.
		void RemoveTile(const QString& tile_name);
		//! Returns a reference to the specified tile.
		DbTile& GetTile(const QString& tile_name);
		//! Returns a list with all tiles in this tileset.
		std::list<DbTile> GetTiles() const;

		//! Writes the set to the tileset-file. On new tilesets, this will only work after
		//! a name has been set.
		void Save();

		//! Sets the name of the tileset.
		void SetName(const QString& name);
		//! Returns the name of the tileset.
		const QString& GetName() const;
		
	private:
		//! The tile database this tileset is associated with.
		TileDatabase* _db;
		//! A list of tile filenames associated with this tileset.
		std::list<QString> _tile_names;
		//! The name of the tileset. This is not the filename associated with the tileset.
		QString _name;
};

class TileDatabase
{
	public:
		//! Create a new TileDatabase
		TileDatabase();
		//! Load database from the specified path
		TileDatabase(const QString& db_file_name);
		~TileDatabase();  // destructor

		//! Synchronizes the TileDatabase with the specified directory. This will both add new
		//! files and also remove files that do not exist anymore.
		void Update(const QString& tile_dir_name);

		//! Writes database to disk.
		void Save(const QString& file_name);

		//! Returns a reference to the tile with the specified name.
		DbTile& GetTile(const QString& tile_name);
		//! Returns true if the tile exists in the db.
		bool TileExists(const QString& tile_name);

		//! Return a reference to the special global tileset which contains all tiles in the db.
		TileSet& GetGlobalSet();

	private:
		//! A map with tile name as a key to a DbTile. This map contains all tiles in the database.
		std::map<QString, DbTile> _tiles;
		//! The special global tileset, always present, contains all existing tiles.
		TileSet _global_set;
};

} // namespace hoa_editor

#endif
// __TILESET_HEADER__
