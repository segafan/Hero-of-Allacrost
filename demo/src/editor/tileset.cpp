///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
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
using namespace hoa_video;
using namespace hoa_script;


namespace hoa_editor
{

////////////////////////////////////////////////////////////////////////////////
////////// Tileset class
////////////////////////////////////////////////////////////////////////////////

Tileset::Tileset() :
	tileset_name(""),
	_initialized(false)
{}



Tileset::~Tileset()
{
	for (std::vector<hoa_video::StillImage>::iterator it = tiles.begin(); it != tiles.end(); it++)
		 (*it).Clear();
	tiles.clear();
}



QString Tileset::CreateImageFilename(const QString& tileset_name)
{
	return QString("img/tilesets/" + tileset_name + ".png");
}



QString Tileset::CreateDataFilename(const QString& tileset_name)
{
	return QString("dat/tilesets/" + tileset_name + ".lua");
}



QString Tileset::CreateTilesetName(const QString& filename)
{
	QString tname = filename;
	// Remove everything up to and including the final '/' character
	tname.remove(0, tname.lastIndexOf("/") + 1);
	// Chop off the appendend four characters the (filename extension)
	tname.chop(4);
	return tname;
}



bool Tileset::New(const QString& img_filename, bool one_image)
{
	_initialized = false;

	// Retreive the tileset name from the image filename
	tileset_name = CreateTilesetName(img_filename);

	// Prepare the tile vector and load the tileset image
	if (one_image == true) {
		tiles.clear();
		tiles.resize(1);
		tiles[0].SetDimensions(16.0f, 16.0f);
		if (tiles[0].Load(string(img_filename.toAscii()), 16, 16) == false) {
			qDebug("Failed to load tileset image: " + img_filename);
			return false;
		}
	}
	else {
		tiles.clear();
		tiles.resize(256);
		for (uint32 i = 0; i < 256; i++)
			tiles[i].SetDimensions(1.0f, 1.0f);
		if (ImageDescriptor::LoadMultiImageFromElementGrid(tiles, string(img_filename.toAscii()), 16, 16) == false) {
			qDebug("Failed to load tileset image: " + img_filename);
			return false;
		}
	}

	// Initialize the rest of the tileset data
	vector<int32> blank_entry(4, 0);
	for (uint32 i = 0; i < 16; i++) {
		for (uint32 j = 0; j < 16; j++) {
			walkability.insert(make_pair(i * 16 + j, blank_entry));
		}
	}

	autotileability.clear();
//	_animated_tiles.clear();

	_initialized = true;
	return true;
}



bool Tileset::Load(const QString& set_name, bool one_image)
{
	_initialized = false;
	tileset_name = set_name;

	// Create filenames from the tileset name
	QString img_filename = CreateImageFilename(set_name);
	QString dat_filename = CreateDataFilename(set_name);

	// Prepare the tile vector and load the tileset image
	if (one_image == true) {
		tiles.clear();
		tiles.resize(1);
		tiles[0].SetDimensions(16.0f, 16.0f);
		if (tiles[0].Load(string(img_filename.toAscii()), 16, 16) == false)
			return false;
	}
	else {
		tiles.clear();
		tiles.resize(256);
		for (uint32 i = 0; i < 256; i++)
			tiles[i].SetDimensions(1.0f, 1.0f);
		if (ImageDescriptor::LoadMultiImageFromElementGrid(tiles,
		        string(img_filename.toAscii()), 16, 16) == false)
			return false;
	}

	// Set up for reading the tileset definition file.
	ReadScriptDescriptor read_data;
	if (read_data.OpenFile(string(dat_filename.toAscii()), true) == false) {
		_initialized = false;
		return false;
	}

	read_data.OpenTable(string(tileset_name.toAscii()));

	// Read in walkability information.
	if (read_data.DoesTableExist("walkability") == true)
	{
		vector<int32> vect;  // used to read in vectors from the data file
		read_data.OpenTable("walkability");
		for (int32 i = 0; i < 16; i++)
		{
			read_data.OpenTable(i);
			// Make sure that at least one row exists
			if (read_data.IsErrorDetected() == true)
			{
				read_data.CloseTable();
				read_data.CloseTable();
				read_data.CloseFile();
				_initialized = false;
				return false;
			}

			for (int32 j = 0; j < 16; j++)
			{
				read_data.ReadIntVector(j, vect);
				if (read_data.IsErrorDetected() == false)
					walkability[i * 16 + j] = vect;
				vect.clear();
			} // iterate through all tiles in a row
			read_data.CloseTable();
		} // iterate through all rows of the walkability table
		read_data.CloseTable();
	} // make sure table exists first

	// Read in autotiling information.
	if (read_data.DoesTableExist("autotiling") == true)
	{
		uint32 table_size = read_data.GetTableSize("autotiling");
		read_data.OpenTable("autotiling");
		vector<int32> keys; // will contain the keys (indeces, if you will) of this table's entries
		read_data.ReadTableKeys(keys);
		for (uint32 i = 0; i < table_size; i++)
			autotileability[keys[i]] = read_data.ReadString(keys[i]);
		read_data.CloseTable();
	} // make sure table exists first

	// TODO: editor does not yet have animated tile support
	/*
	// Read in animated tiles.
	uint32 animated_table_size = read_data.GetTableSize("animated_tiles");
	read_data.OpenTable("animated_tiles");
	for (uint32 i = 1; i <= animated_table_size; i++)
	{
		_animated_tiles.push_back(vector<AnimatedTileData>());
		vector<AnimatedTileData>& tiles = _animated_tiles.back();
		// Calculate loop end: an animated tile is comprised of a tile id and a time, so the loop end
		// is really half the table size.
		uint32 tile_count = read_data.GetTableSize(i) / 2;
		read_data.OpenTable(i);
		for(uint32 index = 1; index <= tile_count; index++)
		{
			tiles.push_back(AnimatedTileData());
			AnimatedTileData& tile_data = tiles.back();
			tile_data.tile_id = read_data.ReadUInt(index * 2 - 1);
			tile_data.time = read_data.ReadUInt(index * 2);
		}
		read_data.CloseTable();
	}
	read_data.CloseTable();
	*/

	read_data.CloseTable();
	read_data.CloseFile();

	_initialized = true;
	return true;
} // bool Tileset::Load(const QString& name)



bool Tileset::Save() {
	string dat_filename = string(CreateDataFilename(tileset_name).toAscii());
	string img_filename = string(CreateImageFilename(tileset_name).toAscii());
	WriteScriptDescriptor write_data;

	if (write_data.OpenFile(dat_filename) == false) {
		return false;
	}

	// Write the localization namespace for the tileset file
	write_data.WriteNamespace(tileset_name.toStdString());
	write_data.InsertNewLine();

	// Write basic tileset properties
	write_data.WriteString("file_name", dat_filename);
	write_data.WriteString("image", img_filename);
	write_data.WriteInt("num_tile_cols", 16);
	write_data.WriteInt("num_tile_rows", 16);
	write_data.InsertNewLine();

	// Write walkability data
	write_data.BeginTable("walkability");
	for (uint32 row = 0; row < 16; row++) {
		write_data.BeginTable(row);
		for (uint32 col = 0; col < 16; col++) {
			write_data.WriteIntVector(col, walkability[row * 16 + col]);
		}
		write_data.EndTable();
	}
	write_data.EndTable();

	// Write animated tile data
	// TODO: animated tiles not supported in editor yet
// 	write_data.BeginTable("animated_tiles");
// 	for (uint32 i = 0; i < _animated_tiles.size(); i++) {
// 		vector<int32> data;
// 		for (uint32 c = 0; c <_animated_tiles[i].size(); c++) {
// 			data.push_back(_animated_tiles[i][c].tile_id);
// 			data.push_back(_animated_tiles[i][c].time);
// 		}
// 		write_data.WriteIntVector(i + 1,data);
// 	}
// 	write_data.EndTable();

	// Write autotiling data
	// TODO: autotiling not supported in editor yet
// 	write_data.BeginTable("autotiling");
// 	for (map<int, string>::iterator i = autotileability.begin(); i != autotileability.end(); i++) {
// 		write_data.WriteString((*i).first, (*i).second);
// 	}
// 	write_data.EndTable();

	if (write_data.IsErrorDetected() == true) {
		cerr << "Errors were detected when saving tileset file. The errors include: " << endl;
		cerr << write_data.GetErrorMessages() << endl;
		write_data.CloseFile();
		return false;
	}
	else {
		write_data.CloseFile();
		return true;
	}
} // bool Tileset::Save()

////////////////////////////////////////////////////////////////////////////////
////////// TilesetTable class
////////////////////////////////////////////////////////////////////////////////

TilesetTable::TilesetTable() :
	Tileset()
{
	// Set up the QT table
	table = new Q3Table(16, 16);
	table->setReadOnly(true);
	table->setShowGrid(false);
	table->setSelectionMode(Q3Table::Multi);
	table->setTopMargin(0);
	table->setLeftMargin(0);
	for (int32 i = 0; i < table->numRows(); i++)
		table->setRowHeight(i, TILE_HEIGHT);
	for (int32 i = 0; i < table->numCols(); i++)
		table->setColumnWidth(i, TILE_WIDTH);
}



TilesetTable::~TilesetTable()
{
	delete table;
}


// TilesetTable::New(const QString& img_filename)
// {
// }


bool TilesetTable::Load(const QString& set_name)
{
	if (Tileset::Load(set_name) == false)
		return false;

	// Read in tiles and create table items.
	// FIXME: this is one ugly hack. It loads each individual tile's image and puts it into a table. But each
	// tile's image only exists together with a bunch of other tiles in a tileset image. So we have to split them
	// up. Qt has no built-in function to split a big image into little ones. This image information has already
	// been loaded by the above call to LoadMultiImageFromElementGrid(...). If we could somehow take that info
	// and put it into a Qt table, that would be ideal.
	// This piece of code is what takes so long for the editor to load a tileset.
	// <<FIXED>>: The ugly hack has been fixed, I use the QImage to handle directly to the bits, it's much faster.
	// Contact me if there's any problem with this fix, eguitarz (dalema22@gmail.com)
	QRect rectangle;
	QImage entire_tileset;
	entire_tileset.load(CreateImageFilename(set_name), "png");
	for (uint32 row = 0; row < 16; row++)
	{
		for (uint32 col = 0; col < 16; col++)
		{
			rectangle.setRect(col * TILE_WIDTH, row * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
			QVariant variant = entire_tileset.copy(rectangle);
			if (!variant.isNull())
			{
				QPixmap tile_pixmap = variant.value<QPixmap>();
				table->setPixmap(row, col, tile_pixmap);
			}
			else
				qDebug(QString("%1").arg("Image loading error!!"));
		} // iterate through the columns of the tileset
	} // iterate through the rows of the tileset

	return true;
}

} // namespace hoa_editor
