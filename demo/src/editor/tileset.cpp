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
	ScriptDescriptor read_data;
	vector<int32> vect;           // used to read in vectors from the data file
	
	if (!read_data.OpenFile(std::string(dat_filename.toAscii()), SCRIPT_READ))
		QMessageBox::warning(parent, "Loading File...", QString("ERROR: could not open %1 for reading!").arg(dat_filename));

	read_data.ReadOpenTable("walkability");
	//uint32 table_size = read_data.ReadGetTableSize();
	for (int32 i = 0; i < 16; i++)
	{
		read_data.ReadOpenTable(i);
		if (read_data.GetError() == 0)
		{
			for (int32 j = 0; j < 16; j++)
			{
				read_data.ReadIntVector(j, vect);
				if (read_data.GetError() == 0)
					walkability[i * 16 + j] = vect;
				vect.clear();
			} // iterate through all tiles in a row
			read_data.ReadCloseTable();
		} // make sure a row exists
	} // iterate through all rows of the walkability table
	read_data.ReadCloseTable();
} // Tileset constructor

Tileset::~Tileset()
{
	delete table;
} // TilesetTable destructor
