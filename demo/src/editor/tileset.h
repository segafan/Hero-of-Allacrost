///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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
#include "data.h"
#include "tile.h"

#include <qmessagebox.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qtable.h>
#include <qdir.h>

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
class TilesetTable : public QTable
{
	public:
		TilesetTable(QWidget* parent, const QString& name, TileDatabase* db);    // constructor
		~TilesetTable();                                       // destructor

		// TODO: implement some sort of dynamic table resizing on window resize
}; // class Tileset

class DbTile
{
public:
	DbTile();
	DbTile(const std::string& filename, int walkable);

	std::string file_name;
	int walkability;
};

class TileSet
{
public:
	TileSet(TileDatabase* db);
	TileSet(TileDatabase* db, const std::string& name);

	void AddTile(const std::string& tile_name);
	void RemoveTile(const std::string& tile_name);
	DbTile& GetTile(const std::string& tile_name);
	std::list<DbTile> GetTiles();

	void Save();

	void SetName(const std::string& name);
	const std::string& GetName() const;
private:
	TileDatabase* _db;
	std::list<std::string> _tile_names;
	std::string _name;
};

class TileDatabase
{
public:
	TileDatabase();
	TileDatabase(const std::string& db_file_name);

	//! Synchronizes the TileDatabase with the specified directory. This will both add new
	//! files and also remove files that do not exist anymore.
	void Update(const std::string& tile_dir_name);

	void Save(const std::string& file_name);

	DbTile& GetTile(const std::string& tile_name);
	bool TileExists(const std::string& tile_name);

	TileSet& GetGlobalSet();

private:
	std::map<std::string, DbTile> _tiles;
	TileSet _global_set;
};

} // namespace hoa_editor

#endif
// __TILESET_HEADER__
