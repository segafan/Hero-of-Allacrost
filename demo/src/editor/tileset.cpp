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

Tileset::Tileset(QWidget* parent, const QString& name)
	: QTable(parent, (const char*) name)
{
	// Set some table properties.
	setReadOnly(true);
	setShowGrid(false);
	setSelectionMode(QTable::Single);
	setTopMargin(0);
	setLeftMargin(0);
	
	// Create filename from name.
	QString file_name;
	if (name == "Global")
		file_name = "tiles_database.lua";
	else
		file_name = name.lower().append(".lua").prepend("tileset_");

	DataDescriptor read_data;
	if (!read_data.OpenFile(file_name.prepend("dat/tilesets/"), READ))
		QMessageBox::warning(parent, "Tilesets", QString("ERROR: could not open %1 for reading!").arg(file_name));
	else
	{
		QString tile_name;     // a tile's filename
		QPixmap tile_pixmap;   // a tile's pixmap of it's image
		read_data.OpenTable("tile_filenames");
		uint32 table_size = read_data.GetTableSize();

		// Dynamically set the table size based on number of tiles in the tileset.
		// Subtract 1 because visibleWidth() does not account for the scrollbar.
		int num_columns = visibleWidth() / TILE_WIDTH - 1;
		setNumCols(num_columns);
		setNumRows(static_cast<int> (ceil(
			static_cast<double> (table_size) / static_cast<double> (num_columns))));
		for (int i = 0; i < numRows(); i++)
			setRowHeight(i, TILE_HEIGHT);
		for (int i = 0; i < numCols(); i++)
			setColumnWidth(i, TILE_WIDTH);

		int32 row, col;
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
			} // end of the column has been reached
		} // construct image from tile filename and place it in the grid
		read_data.CloseTable();
		read_data.CloseFile();
	} // file was opened successfully
} // Tileset constructor

Tileset::~Tileset()
{
} // Tileset destructor
