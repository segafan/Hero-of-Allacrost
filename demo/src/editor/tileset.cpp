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

TilesetTable::TilesetTable(QWidget* parent, const QString& name)
	: Q3Table(parent, (const char*)name)
{
	// Set some table properties.
	setReadOnly(true);
	setShowGrid(false);
	setSelectionMode(Q3Table::Single);
	setTopMargin(0);
	setLeftMargin(0);
	
	// Create filename from name.
	tileset_name = name;
	QString filename = QString("img/tilesets/" + name + ".png");
	tiles.resize(256);
	for (int i = 0; i < 256; i++)
		tiles[i].SetDimensions(1.0f, 1.0f);

	// NOTE: the following line currently causes a seg fault when the editor exits!
	if (VideoManager->LoadMultiImageFromNumberElements(tiles, filename.toStdString(), 16, 16) == false)
		qDebug("LoadMultiImage failed to load tileset " + filename);
// 	else
// 		qDebug("LoadMultiImage successful! " + filename);

	// Set up the table.
	//int num_columns = visibleWidth() / TILE_WIDTH - 1;
	//setNumCols(num_columns);
	setNumCols(16);
	setNumRows(16);
	//setNumRows(static_cast<int> (ceil(
	//		static_cast<double> (tiles.size()) / static_cast<double> (num_columns))));
	for (int i = 0; i < numRows(); i++)
		setRowHeight(i, TILE_HEIGHT);
	for (int i = 0; i < numCols(); i++)
		setColumnWidth(i, TILE_WIDTH);

	// Read in tiles and create table items.
	QRect rectangle;
	QDir dir = QDir::current();
	for (int row = 0; row < 16; row++)
	{
		for (int col = 0; col < 16; col++)
		{
			QImageReader reader(filename, "png");
			rectangle.setRect(col * TILE_WIDTH, row * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
			reader.setClipRect(rectangle);
			QImage tile_img = reader.read();
			if (!tile_img.isNull())
			{
				// FIXME: conversion from image to pixmap isn't working, so save the tile and then read it for now
				//tile_pixmap.fromImage(tile_img);
				if (!tile_img.save(QString("tile%1.png").arg(col + row * 16)))
					qDebug("Saving error during tileset table creation");
				QPixmap tile_pixmap(QString("tile%1.png").arg(col + row * 16));
				setPixmap(row, col, tile_pixmap);
				dir.remove(QString("tile%1.png").arg(col + row * 16));
			} // image of the tile must not be null
			else
				qDebug(QString("%1").arg(reader.error()));
		} // iterate through the columns of the tileset
	} // iterate through the rows of the tileset
} // TilesetTable constructor

TilesetTable::~TilesetTable()
{
} // TilesetTable destructor
