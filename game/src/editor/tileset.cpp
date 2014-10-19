///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tileset.cpp
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for tileset data and display code
*** **************************************************************************/

#include <QFile>
#include <QImage>
#include <QVariant>
#include <QHeaderView>

#include "common.h"
#include "tileset.h"

using namespace std;
using namespace hoa_script;
using namespace hoa_common;

namespace hoa_editor {

////////////////////////////////////////////////////////////////////////////////
// Tileset class
////////////////////////////////////////////////////////////////////////////////

Tileset::Tileset() :
	_initialized(false),
	_tileset_name(""),
	_tileset_image_filename(""),
	_tileset_definition_filename(""),
	_tile_images(TILESET_NUM_ROWS * TILESET_NUM_COLS, QPixmap()),
	_tile_collisions(TILESET_NUM_ROWS * TILESET_NUM_COLS * COLLISION_QUADRANTS, 0)
{}



bool Tileset::New(const QString& img_filename, bool single_image) {
	if (img_filename.isEmpty()) {
		qDebug("Missing image filename");
		return false;
	}

	_ClearData();

	_tileset_image_filename = img_filename;
	_CreateTilesetNameFromFilename(img_filename);
	if (_LoadImageData(single_image) == false) {
		_ClearData();
		return false;
	}

	_initialized = true;
	return true;
}



bool Tileset::Load(const QString& def_filename, bool single_image) {
	_ClearData();

	if (def_filename.isEmpty())
		return false;

	// ---------- 1) Open the tileset definition file
	ReadScriptDescriptor read_file;
	_tileset_definition_filename = def_filename;
	if (read_file.OpenFile(_tileset_definition_filename.toStdString()) == false) {
		_ClearData();
		return false;
	}

	// ---------- 2) Load the tileset name and image data
	if (read_file.DoesStringExist("tileset_name")) {
		_tileset_name = QString::fromStdString(read_file.ReadString("tileset_name"));
	}
	else {
		_CreateTilesetNameFromFilename(def_filename);
	}
	_tileset_image_filename = QString::fromStdString(read_file.ReadString("image"));

	if (_LoadImageData(single_image) == false) {
		_ClearData();
		return false;
	}

	// ---------- 3) Load in the collision data
	if (read_file.DoesTableExist("collisions") == false) {
		_ClearData();
		return false;
	}

	read_file.OpenTable("collisions");
	for (uint32 i = 0; i < TILESET_NUM_ROWS; ++i) {
		vector<uint32> collision_row;  // Temporarily stores one tile row worth of collision data
		read_file.ReadUIntVector(i, collision_row);
		// Ensure that the row is the correct size
		if (collision_row.size() != (TILESET_NUM_COLS * COLLISION_QUADRANTS)) {
			_ClearData();
			return false;
		}

		for (uint32 j = 0; j < collision_row.size(); ++j) {
			_tile_collisions[(i * (TILESET_NUM_COLS * COLLISION_QUADRANTS)) + j] = collision_row[j];
		}
	}
	read_file.CloseTable();

	// ---------- 4) Read in animation data if the TDF contains animations
	if (read_file.DoesTableExist("animations") == true) {
		uint32 table_size = read_file.GetTableSize("animations");
		read_file.OpenTable("animations");

		for (uint32 i = 0; i < table_size; ++i) {
			// Determine how many entries (tile + time) are in this animations
			vector<uint32> animation_data;
			read_file.ReadUIntVector(i, animation_data);
			_tile_animations.push_back(vector<AnimatedTileData>());

			for (uint32 j = 0; j < animation_data.size(); j += 2) {
				_tile_animations.back().push_back(AnimatedTileData(animation_data[j], animation_data[j + 1]));
			}
		}
		read_file.CloseTable();
	}

	// ---------- 5) Read in autotiling data if the TDF contains autotiling
	if (read_file.DoesTableExist("autotiling") == true) {
		// Read in the set of autotiling keys followed by their string values
		vector<uint32> keys;
		read_file.OpenTable("autotiling");
		read_file.ReadTableKeys(keys);

		for (uint32 i = 0; i < keys.size(); ++i) {
			_tile_autotiles[keys[i]] = read_file.ReadString(keys[i]);
		}
		read_file.CloseTable();
	}

	read_file.CloseFile();
	_initialized = true;
	return true;
} // bool Tileset::Load(const QString& def_filename, bool single_image)



bool Tileset::Save() {
	// We can't save the data if we don't have a file specified
	if (_tileset_definition_filename.isEmpty() == true) {
		return false;
	}


	WriteScriptDescriptor write_file;
	if (write_file.OpenFile(_tileset_definition_filename.toStdString()) == false) {
		return false;
	}

	// Write the tablespace header to the TDF, using a transformation of the TDF filename as the tablespace name
	string tablespace_name = DetermineLuaFileTablespaceName(_tileset_definition_filename.toStdString());
	write_file.WriteNamespace(tablespace_name);
	write_file.InsertNewLine();

	// Write the basic properties: the tileset name and image filename
	write_file.WriteString("tileset_name", _tileset_name.toStdString());
	write_file.WriteString("image", _tileset_image_filename.toStdString());
	write_file.InsertNewLine();

	// Write the collision data one row at a time
	write_file.BeginTable("collisions");
	for (uint32 row = 0; row < TILESET_NUM_ROWS; row++) {
		// Extract a sub vector from the collision data to write
		vector<uint32>::const_iterator row_start = _tile_collisions.begin() + (row * TILESET_NUM_COLS * COLLISION_QUADRANTS);
		vector<uint32>::const_iterator row_end = row_start + (TILESET_NUM_COLS * COLLISION_QUADRANTS);
		vector<uint32> collision_row(row_start, row_end);
		write_file.WriteUIntVector(row, collision_row);
	}
	write_file.EndTable();
	write_file.InsertNewLine();

	// Write the animation data, if the tileset has animations
	if (_tile_animations.empty() == false) {
 		write_file.BeginTable("animations");


		for (uint32 i = 0; i < _tile_animations.size(); ++i) {
			vector<uint32> animation_data;
			for (uint32 j = 0; j < _tile_animations[i].size(); ++j) {
				animation_data.push_back(_tile_animations[i][j].tile_id);
				animation_data.push_back(_tile_animations[i][j].time);
			}
			write_file.WriteUIntVector(i, animation_data);
		}

 		write_file.EndTable();
		write_file.InsertNewLine();
	}

	// Write the autotile data, if the tileset has autotiling
	if (_tile_autotiles.empty() == false) {
	 	write_file.BeginTable("autotiling");
	 	for (map<uint32, string>::iterator i = _tile_autotiles.begin(); i != _tile_autotiles.end(); ++i)
 			write_file.WriteString((*i).first, (*i).second);
 		write_file.EndTable();
		write_file.InsertNewLine();
	} // data must exist in order to save it


	if (write_file.IsErrorDetected() == true) {
		cerr << "Errors were detected when saving tileset file. The errors include:\n" << write_file.GetErrorMessages() << endl;
		write_file.CloseFile();
		return false;
	}

	write_file.CloseFile();
	return true;
} // bool Tileset::Save()



bool Tileset::SaveAs(const QString& def_filename) {
	QString old_def_filename = _tileset_definition_filename;
	_tileset_definition_filename = def_filename;

	if (Save() == false) {
		_tileset_definition_filename = old_def_filename;
		return false;
	}

	return true;
}



void Tileset::_ClearData() {
	_initialized = false;
	_tileset_name = "";
	_tileset_image_filename = "";
	_tileset_definition_filename = "";

	for (uint32 i = 0; i < _tile_images.size(); ++i)
		_tile_images[i] = QPixmap();
	for (uint32 i = 0; i < _tile_collisions.size(); ++i)
		_tile_collisions[i] = 0;
	_tile_animations.clear();
	_tile_autotiles.clear();
}



bool Tileset::_LoadImageData(bool single_image) {
	QRect rectangle;
	QImage entire_tileset;
	if (entire_tileset.load(_tileset_image_filename, "png") == false) {
		qDebug("Failed to load tileset image: %s", _tileset_image_filename.toStdString().c_str());
		_ClearData();
		return false;
	}

	if (single_image == true) {
		_tile_images[0].convertFromImage(entire_tileset);
	}
	else {
		for (uint32 row = 0; row < TILESET_NUM_ROWS; ++row) {
			for (uint32 col = 0; col < TILESET_NUM_COLS; ++col) {
				rectangle.setRect(col * TILE_LENGTH, row * TILE_HEIGHT, TILE_LENGTH, TILE_HEIGHT);
				QImage tile = entire_tileset.copy(rectangle);
				if (tile.isNull()) {
					qDebug("Image loading error!");
				} else {
					uint32 index = (row * TILESET_NUM_ROWS) + col;
					_tile_images[index].convertFromImage(tile);
				}
			}
		}
	}

	return true;
}



void Tileset::_CreateTilesetNameFromFilename(const QString &filename) {
	_tileset_name = filename;
	// Remove everything up to and including the final '/' character
	_tileset_name.remove(0, _tileset_name.lastIndexOf("/") + 1);
	// Chop off the appended four characters (the filename extension)
	_tileset_name.chop(4);
}

///////////////////////////////////////////////////////////////////////////////
// TilesetTable class
///////////////////////////////////////////////////////////////////////////////

TilesetTable::TilesetTable() :
	Tileset(),
	table(new QTableWidget(TILESET_NUM_ROWS, TILESET_NUM_COLS))
{
	// Set up the QT table
	table->setShowGrid(false);
	table->setSelectionMode(QTableWidget::ContiguousSelection);
	table->setEditTriggers(QTableWidget::NoEditTriggers);
	table->setContentsMargins(0, 0, 0, 0);
	table->setDragEnabled(false);
	table->setAcceptDrops(false);
	table->setHorizontalHeaderLabels(QStringList());
	table->setVerticalHeaderLabels(QStringList());
	table->verticalHeader()->hide();
	table->verticalHeader()->setContentsMargins(0, 0, 0, 0);
	table->horizontalHeader()->hide();
	table->horizontalHeader()->setContentsMargins(0, 0, 0, 0);

	for (uint32 i = 0; i < TILESET_NUM_ROWS; ++i)
		table->setRowHeight(i, TILE_HEIGHT);
	for (uint32 i = 0; i < TILESET_NUM_COLS; ++i)
		table->setColumnWidth(i, TILE_LENGTH);
}



bool TilesetTable::Load(const QString& def_filename) {
	if (Tileset::Load(def_filename) == false)
		return false;

	// Load the tileset image into a single file, then make a copy of the individual tiles
	// within it and place them in the pixmap
	QImage entire_tileset;
	if (entire_tileset.load(_tileset_image_filename) == false) {
		qDebug("ERROR: failed to load tileset image file");
		return false;
	}

	// Go through every tile location in the tileset image, copy out the image data for that tile,
	// and place it in the table's pixmap
	QRect rectangle;
	QVariant variant;
	QPixmap tile_pixmap;
	for (uint32 row = 0; row < TILESET_NUM_ROWS; ++row) {
		for (uint32 col = 0; col < TILESET_NUM_COLS; ++col) {
			rectangle.setRect(col * TILE_LENGTH, row * TILE_HEIGHT, TILE_LENGTH, TILE_HEIGHT);
			variant = entire_tileset.copy(rectangle);
			if (variant.isNull() == true) {
				qDebug("Error occurred while copying a tileset image tile");
			}
			else {
				QTableWidgetItem* item = new QTableWidgetItem(QTableWidgetItem::UserType);
				item->setData(Qt::DecorationRole, variant);
				item->setFlags(item->flags() &~ Qt::ItemIsEditable);
				table->setItem(row, col, item);
			}
		}
	}

	// Select the top left item
	table->setCurrentCell(0, 0);

	return true;
}

} // namespace hoa_editor
