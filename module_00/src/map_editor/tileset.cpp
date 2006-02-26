///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2004, 2005 by The Allacrost Project
// All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    tileset.cpp
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \date    Last Updated: January 26th, 2006
 * \brief   Source file for editor's tileset, mainly used for drag'n'drop.
 *****************************************************************************/

#include "tileset.h"

using namespace std;
using namespace hoa_data;
using namespace hoa_editor;

Tileset::Tileset(QWidget* parent, const QString& name)
	: QTable(parent, (const char*) name)
{
	// Setup table properties.FIXME: should make this somewhat smart based on tileset it's loading
	setReadOnly(true);
	setShowGrid(false);
	setSelectionMode(QTable::Single);
	setTopMargin(0);
	setLeftMargin(0);
	setNumCols(13);
	setNumRows(6);
	for (int i = 0; i < numRows(); i++)
		setRowHeight(i, TILE_HEIGHT);
	for (int i = 0; i < numCols(); i++)
		setColumnWidth(i, TILE_WIDTH);
	
	// Create file name from name.
	QString file_name;
	if (name == "Global")
		file_name = "tiles_database.lua";
	else
		file_name = name.lower().append(".lua").prepend("tileset_");

	ReadDataDescriptor read_data;
	if (!read_data.OpenFile(file_name.prepend("dat/tilesets/")))
		QMessageBox::warning(parent, "Tilesets", QString("ERROR: could not open %1 for reading!").arg(file_name));
	else
	{
		QString tile_name;
		QPixmap tile_pixmap;
		read_data.OpenTable("tile_filenames");
		uint32 table_size = read_data.GetTableSize();
		uint32 row, col;
		row = col = 0;
		for (uint32 i = 1; i <= table_size; i++)
		{
			tile_name = QString(read_data.ReadString(i)).append(".png").prepend("img/tiles/");
			tile_pixmap = QPixmap(tile_name);
			setPixmap(row, col, tile_pixmap);
			setText(row, col, tile_name);
			col++;
			if (col == numCols())
			{
				col = 0;
				row++;
			}
		} // construct image from tile filename and place it in the grid
		read_data.CloseTable();
		read_data.CloseFile();
	} // file was opened succuessfully

} // Tileset constructor

Tileset::~Tileset()
{
} // Tileset destructor

//void Tileset::paintCell(QPainter* painter, int row, int col)
//{
	//painter->drawPixmap(0, 0, *_it);
//	bitBlt(_it, 0, 0, _it);
//	if (_it == _pixmap_vect.end())
//		_it = _pixmap_vect.begin();
//	else
//		_it++;
//} // paintCell(...)

/******************************************************************************
 *
 *  Function: dragObject
 *
 *  Inputs: none
 *
 *  Outputs: QImageDrag* img_drag - object to drag, typecasted to QDragObject*
 *
 *  Description: This function implements high level drag and drop operations.
 *				 It determines the desired object to drag from the QIconView.
 *
 *****************************************************************************/
//QDragObject* Tileset::dragObject()
//{
	// creates an image to drag
//	QImageDrag* img_drag = new QImageDrag(currentItem()->pixmap()
//		->convertToImage(), this);
	
	// creates the image user will see whilst dragging
//	img_drag->setPixmap(*(currentItem()->pixmap()),
//		QPoint(currentItem()->pixmapRect().width() / 2,
//			currentItem()->pixmapRect().height() / 2));

//	return img_drag;
//} // dragObject()
