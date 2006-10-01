///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    tileset.cpp
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Source file for editor's tileset, used for maintaining a visible
 *          "list" of tiles to select from for painting on the map.
 *****************************************************************************/

#include "tileset.h"

using namespace std;
using namespace hoa_data;
using namespace hoa_editor;

TilesetTable::TilesetTable(QWidget* parent, const QString& name, TileDatabase* db)
	: QTable(parent, (const char*) name)
{
	// Set some table properties.
	setReadOnly(true);
	setShowGrid(false);
	setSelectionMode(QTable::Single);
	setTopMargin(0);
	setLeftMargin(0);

	std::list<DbTile> tiles;
	
	// Create filename from name.
	if (name == "Global")
		tiles=db->GetGlobalSet().GetTiles();
	else
	{
		TileSet Set(db,name);
		tiles=Set.GetTiles();
	}

	// Set up the table
	int num_columns = visibleWidth() / TILE_WIDTH - 1;
	setNumCols(num_columns);
	setNumRows(static_cast<int> (ceil(
			static_cast<double> (tiles.size()) / static_cast<double> (num_columns))));
	for (int i = 0; i < numRows(); i++)
		setRowHeight(i, TILE_HEIGHT);
	for (int i = 0; i < numCols(); i++)
		setColumnWidth(i, TILE_WIDTH);

	// Read in tiles and create table items
	int32 row=0;
	int32 col=0;
	for(std::list<DbTile>::const_iterator it=tiles.begin(); it!=tiles.end(); it++)
	{
		std::string tile_path="img/tiles/"+(*it).file_name;
		QPixmap tile_pixmap=QPixmap(tile_path);
		setPixmap(row, col, tile_pixmap);
		setText(row, col, tile_path);

		col++;
		if (col == numCols())
		{
			col = 0;
			row++;
		} // end of the column has been reached
	}	
} // Tileset constructor

TilesetTable::~TilesetTable()
{
} // Tileset destructor

DbTile::DbTile()
{
}

DbTile::DbTile(const std::string& filename, int walkable)
{
	file_name=filename;
	walkability=walkable;
}

TileDatabase::TileDatabase() : _global_set(this)
{
}

TileDatabase::TileDatabase(const std::string& db_file_name) : _global_set(this)
{
	DataDescriptor read_data;
	read_data.OpenFile(db_file_name.c_str(), READ);

	// Read filenames and add tiles
	read_data.OpenTable("tile_filenames");
	for(uint32 i=1; i<=read_data.GetTableSize(); i++)
	{
		read_data.OpenTable(i);
		// read properties and insert tile
		std::string file_name=read_data.ReadString(0);
		int32 walkability=read_data.ReadInt(1);
		DbTile tile(file_name, walkability);
		_tiles.insert(std::pair<std::string,DbTile>(file_name,tile));
		read_data.CloseTable();

		// add tile to global set
		_global_set.AddTile(file_name);
	}
	read_data.CloseTable();

	read_data.CloseFile();
}

void TileDatabase::Update(const std::string& tile_dir_name)
{
	QDir tile_dir(tile_dir_name);	
	QStringList files=tile_dir.entryList("*.png");

	// iterate through the tiles in the database and delete ones that do not exist
	for(std::map<std::string,DbTile>::iterator it=_tiles.begin(); it!=_tiles.end(); it++)
	{
		if(files.find((*it).first) == files.end())
		{
			// Remove from global set
			_global_set.RemoveTile((*it).first);

			_tiles.erase(it);
		}
	}

	// iterate through all files in the directory and add new ones
	for(QStringList::const_iterator it=files.begin(); it!=files.end(); it++)
	{
		std::string tile_file=*it;
		// if it does not exist in the database yet, add it
		if(_tiles.find(tile_file)==_tiles.end())
		{
			DbTile new_tile(tile_file,255);
			_tiles.insert(std::pair<std::string, DbTile>(tile_file,new_tile));

			_global_set.AddTile(tile_file);
		}
	}	
}

void TileDatabase::Save(const std::string& file_name)
{
	DataDescriptor write_data;
	write_data.OpenFile(file_name.c_str(),WRITE);

	//Write tiles
	write_data.WriteComment("Stores names and properties of all files in the database");
	write_data.BeginTable("tile_filenames");
	int index=1;
	for(std::map<std::string,DbTile>::iterator it=_tiles.begin(); it!=_tiles.end(); it++)
	{
		// Ugly work around, can't currently begin tables with int index
		//write_data.BeginTable(boost::lexical_cast<std::string>(index).c_str());
		std::ostringstream index_str;
		index_str<<index;
		write_data.BeginTable(index_str.str().c_str());
		write_data.WriteString(0,(*it).second.file_name);
		write_data.WriteInt(1, (*it).second.walkability);
		write_data.EndTable();
		index++;
	}
	write_data.EndTable();

	write_data.CloseFile();
}

DbTile& TileDatabase::GetTile(const std::string& tile_name)
{
	return _tiles[tile_name];
}

TileSet& TileDatabase::GetGlobalSet()
{
	return _global_set;
}

bool TileDatabase::TileExists(const std::string& tile_name)
{
	return (_tiles.find(tile_name)!=_tiles.end());
}

TileSet::TileSet(TileDatabase* db)
{
	_db=db;
}

TileSet::TileSet(TileDatabase* db, const std::string& name)
{
	_db=db;
	_name=name;

	DataDescriptor read_data;
	read_data.OpenFile(("dat/tilesets/tileset_"+name+".lua").c_str(),READ);
	read_data.OpenTable("tile_names");
	for(int i=1; i<=read_data.GetTableSize(); i++)
	{
		std::string tile_name=read_data.ReadString(i);
		AddTile(tile_name);
	}
	read_data.CloseTable();
	read_data.CloseFile();
}

void TileSet::AddTile(const std::string& tile_name)
{
	_tile_names.push_back(tile_name);
}

void TileSet::RemoveTile(const std::string& tile_name)
{
	_tile_names.remove(tile_name);
}

DbTile& TileSet::GetTile(const std::string& tile_name)
{
	return _db->GetTile(tile_name);
}

std::list<DbTile> TileSet::GetTiles() const
{
	std::list<DbTile> ret;
	for(std::list<std::string>::const_iterator it=_tile_names.begin(); it!=_tile_names.end(); it++)
	{
		ret.push_back(_db->GetTile(*it));
	}
	return ret;
}

void TileSet::Save()
{
	if(_name=="") 
	{
		QMessageBox::warning(NULL,"Error","tileset needs to have a name in order to save it!");
		return;
	}

	DataDescriptor write_data;
	write_data.OpenFile(("dat/tilesets/tileset_"+_name+".lua").c_str(),WRITE);

	write_data.BeginTable("tile_names");
	int index=1;
	for(std::list<std::string>::const_iterator it=_tile_names.begin(); it!=_tile_names.end(); it++)
	{
		write_data.WriteString(index,(*it).c_str());
		index++;
	}
	write_data.EndTable();
	write_data.CloseFile();
}

void TileSet::SetName(const std::string& name)
{
	_name=name;
}

const std::string& TileSet::GetName() const
{
	return _name;
}