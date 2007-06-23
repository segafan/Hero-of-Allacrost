///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
using namespace hoa_editor;

Tileset::Tileset(QWidget* parent, const QString& name)
{	
	// Create filename from name.
	tileset_name = name;
	QString img_filename = QString("img/tilesets/" + name + ".png");
	QString dat_filename = QString("dat/tilesets/" + name + ".lua");

	// Load the tileset image.
	tiles.resize(256);
	for (int i = 0; i < 256; i++)
		tiles[i].SetDimensions(1.0f, 1.0f);
	// NOTE: the following line currently causes a seg fault when the editor exits!
	if (VideoManager->LoadMultiImageFromNumberElements(tiles, std::string(img_filename.toAscii()), 16, 16) == false)
		qDebug("LoadMultiImage failed to load tileset " + img_filename);

	// Set up the table.
	table = new Q3Table(16, 16);
	table->setReadOnly(true);
	table->setShowGrid(false);
	table->setSelectionMode(Q3Table::Multi);
	table->setTopMargin(0);
	table->setLeftMargin(0);
	for (int i = 0; i < table->numRows(); i++)
		table->setRowHeight(i, TILE_HEIGHT);
	for (int i = 0; i < table->numCols(); i++)
		table->setColumnWidth(i, TILE_WIDTH);

	// Read in tiles and create table items.
	QRect rectangle;
	for (int row = 0; row < 16; row++)
	{
		for (int col = 0; col < 16; col++)
		{
			QImageReader reader(img_filename, "png");
			rectangle.setRect(col * TILE_WIDTH, row * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
			reader.setClipRect(rectangle);
			QImage tile_img = reader.read();
			QVariant variant = tile_img;
			if (!tile_img.isNull())
			{
				QPixmap tile_pixmap = variant.value<QPixmap>();
				table->setPixmap(row, col, tile_pixmap);
			} // image of the tile must not be null
			else
				qDebug(QString("%1").arg(reader.error()));
		} // iterate through the columns of the tileset
	} // iterate through the rows of the tileset


	// Read in walkability information.
	ReadScriptDescriptor read_data;
	vector<int32> vect;           // used to read in vectors from the data file
	
	if (!read_data.OpenFile(std::string(dat_filename.toAscii())))
		QMessageBox::warning(parent, "Loading File...", QString("ERROR: could not open %1 for reading!").arg(dat_filename));

	read_data.OpenTable("walkability");
	//uint32 table_size = read_data.ReadGetTableSize();
	for (int32 i = 0; i < 16; i++)
	{
		read_data.OpenTable(i);
		if (read_data.IsErrorDetected() == false)
		{
			for (int32 j = 0; j < 16; j++)
			{
				read_data.ReadIntVector(j, vect);
				if (read_data.IsErrorDetected() == false)
					walkability[i * 16 + j] = vect;
				vect.clear();
			} // iterate through all tiles in a row
			read_data.CloseTable();
		} // make sure a row exists
	} // iterate through all rows of the walkability table
	read_data.CloseTable();

	//Read animated tiles
	uint32 animated_table_size = read_data.GetTableSize("animated_tiles");
	read_data.OpenTable("animated_tiles");
	for (uint32 i=1; i<=animated_table_size; i++) 
	{
		_animated_tiles.push_back(std::vector<AnimatedTileData>());
		std::vector<AnimatedTileData>& tiles=_animated_tiles.back();
		// Tile count is table size / 2 (tile id + time)
		uint32 tile_count=read_data.GetTableSize(i) / 2;
		read_data.OpenTable(i);
		for(uint32 index=1; index<=tile_count; index++) 
		{
			tiles.push_back(AnimatedTileData());
			AnimatedTileData& tile_data=tiles.back();
			tile_data.tile_id=read_data.ReadUInt(index*2-1);
			tile_data.time=read_data.ReadUInt(index*2);
		}
		read_data.CloseTable();
	}
	read_data.CloseTable();
} // Tileset constructor

Tileset::~Tileset()
{
	delete table;
} // TilesetTable destructor

void Tileset::Save() {
	std::string dat_filename = "dat/tilesets/" + std::string(tileset_name.toAscii()) + ".lua";
	std::string img_filename = "img/tilesets/" + std::string(tileset_name.toAscii()) + ".png";
	WriteScriptDescriptor write_data;

	// Write global infos
	write_data.OpenFile(dat_filename); // NOTE: return value of this function should be checked!!!
	write_data.InsertNewLine();
	write_data.WriteString("file_name",dat_filename);
	write_data.WriteString("image",dat_filename);
	write_data.WriteInt("num_tile_cols",table->numCols());
	write_data.WriteInt("num_tile_rows",table->numRows());
	write_data.InsertNewLine();

	// Write walkability data
	write_data.BeginTable("walkability");
	for(int row=0;row<table->numRows();row++) {
		write_data.BeginTable(row);
		for(int col=0;col<table->numCols();col++) {
			write_data.WriteIntVector(col,this->walkability[row*16+col]);
		}
		write_data.EndTable();
	}
	write_data.EndTable();

	// Write animated tiles
	write_data.BeginTable("animated_tiles");
	for(int i=0;i<_animated_tiles.size();i++) {
		std::vector<int32> data;
		for(int c=0;c<_animated_tiles[i].size();c++) {
			data.push_back(_animated_tiles[i][c].tile_id);
			data.push_back(_animated_tiles[i][c].time);
		}
		write_data.WriteIntVector(i+1,data);
	}
	write_data.EndTable();

	write_data.CloseFile();
}
