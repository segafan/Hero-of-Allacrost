///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    dialogs.cpp
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \brief   Source file for all of the editor's dialog windows
*** **************************************************************************/

#include "editor.h"
#include "map_data.h"
#include "dialogs.h"

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// MapPropertiesDialog class
///////////////////////////////////////////////////////////////////////////////

MapPropertiesDialog::MapPropertiesDialog(QWidget* parent, const QString& name, bool prop) :
	QDialog(parent, (const char*)name)
{
	setWindowTitle("Map Properties...");

	// Set up the height spinbox
	_height_label = new QLabel("Map Height:", this);
	_height_sbox = new QSpinBox(this);
	_height_sbox->setMinimum(MINIMUM_MAP_HEIGHT);
	_height_sbox->setMaximum(MAXIMUM_MAP_HEIGHT);

	// Set up the length spinbox
	_length_label = new QLabel("Map Length:", this);
	_length_sbox = new QSpinBox(this);
	_length_sbox->setMinimum(MINIMUM_MAP_LENGTH);
	_length_sbox->setMaximum(MAXIMUM_MAP_LENGTH);

	// If map data already exists, get the length and height. Otherwise use the minimum values as the default
	MapData* existing_data = static_cast<Editor*>(parent)->GetMapData();
	if (existing_data->IsInitialized() == true) {
		_length_sbox->setValue(existing_data->GetMapLength());
		_height_sbox->setValue(existing_data->GetMapHeight());
	}
	else {
		_length_sbox->setValue(MINIMUM_MAP_LENGTH);
		_height_sbox->setValue(MINIMUM_MAP_HEIGHT);
	}

	// Set up the cancel and okay push buttons
	_cancel_pbut = new QPushButton("Cancel", this);
	_ok_pbut = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);

	// At construction no tilesets are checked, so disable the ok button
	_ok_pbut->setEnabled(false);
	connect(_ok_pbut, SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	// Set up the list of selectable tilesets
	QDir tileset_dir("lua/data/tilesets");
	_tileset_tree = new QTreeWidget(this);
	_tileset_tree->setColumnCount(1);
	_tileset_tree->setHeaderLabels(QStringList("Tilesets"));
	connect(_tileset_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(_EnableOKButton()));

	// Loop through all files found in the tileset definition directory (lua/data/tilesets).
	// Start the loop at 2 to skip over the present and parent working directories ("." and "..")
	QList<QTreeWidgetItem*> tilesets;
	for (uint32 i = 2; i < tileset_dir.count(); i++) {
		// Exclude the file autotiling.lua, as it's not a tileset defition files
		if (tileset_dir[i] == QString("autotiling.lua"))
			continue;

		tilesets.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(tileset_dir[i].remove(".lua"))));
		tilesets.back()->setCheckState(0, Qt::Unchecked); // enables checkboxes

		// Indicates that the user wants to edit the map's existing properties
		if (prop) {
// 			Editor* editor = static_cast<Editor*>(parent);

			// TODO: Iterate through the names of the tabs to see which ones in the list are already present
			// and set their checkbox appropriately
// 			for (int j = 0; j < editor->_ed_tabs->count(); j++) {
// 				// Both tileset names must match to set the checkbox
// 				if (tilesets.back()->text(0) == editor->_ed_tabs->tabText(j)) {
// 					tilesets.back()->setCheckState(0, Qt::Checked);
// 					_ok_pbut->setEnabled(true);
// 					break;
// 				}
// 			}
		}
	}
	_tileset_tree->insertTopLevelItems(0, tilesets);

	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	_dia_layout->addWidget(_height_label, 0, 0);
	_dia_layout->addWidget(_height_sbox,  1, 0);
	_dia_layout->addWidget(_length_label,  2, 0);
	_dia_layout->addWidget(_length_sbox,   3, 0);
	_dia_layout->addWidget(_tileset_tree, 0, 1, 5, -1);
	_dia_layout->addWidget(_cancel_pbut,  6, 0);
	_dia_layout->addWidget(_ok_pbut,      6, 1);
} // MapPropertiesDialog::MapPropertiesDialog(QWidget* parent, const QString& name, bool prop)



MapPropertiesDialog::~MapPropertiesDialog() {
	delete _tileset_tree;
	delete _height_label;
	delete _height_sbox;
	delete _length_label;
	delete _length_sbox;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _dia_layout;
}



void MapPropertiesDialog::_EnableOKButton() {
	// Disable the ok button if no tilesets are checked, otherwise enable it
	int num_items = _tileset_tree->topLevelItemCount();
	for (int i = 0; i < num_items; i++) {
		// At least one tileset must be checked in order to enable push button
		if (_tileset_tree->topLevelItem(i)->checkState(0) == Qt::Checked) {
			_ok_pbut->setEnabled(true);
			return;
		}
	}

	// If this point is reached, no tilesets are checked.
	_ok_pbut->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////
// MapResizeDialog class
///////////////////////////////////////////////////////////////////////////////

MapResizeDialog::MapResizeDialog(QWidget* parent, MapData* data) :
	QDialog(parent),
	_map_data(data),
	_height_spinbox(NULL),
	_length_spinbox(NULL),
	_height_title(NULL),
	_length_title(NULL),
	_height_change(NULL),
	_length_change(NULL),
	_ok_button(NULL),
	_cancel_button(NULL),
	_grid_layout(NULL)
{
	setWindowTitle("Resize Map");

	if (_map_data == NULL) {
		qDebug("ERROR: MapResizesDialog constructor received a NULL map data pointer");
	}

	_height_spinbox = new QSpinBox(this);
	_height_spinbox->setMinimum(MINIMUM_MAP_HEIGHT);
	_height_spinbox->setMaximum(MAXIMUM_MAP_HEIGHT);
	_height_spinbox->setValue(_map_data->GetMapHeight());
	connect(_height_spinbox, SIGNAL(valueChanged(int)), this, SLOT(_HeightChanged()));
	_length_spinbox = new QSpinBox(this);
	_length_spinbox->setMinimum(MINIMUM_MAP_LENGTH);
	_length_spinbox->setMaximum(MAXIMUM_MAP_LENGTH);
	_length_spinbox->setValue(_map_data->GetMapLength());
	connect(_length_spinbox, SIGNAL(valueChanged(int)), this, SLOT(_LengthChanged()));

	_height_title = new QLabel("Map Height:", this);
	_length_title = new QLabel("Map Length:", this);
	_height_change = new QLabel("Change: 0", this);
	_length_change = new QLabel("Change: 0", this);

	_cancel_button = new QPushButton("Cancel", this);
	_cancel_button->setDefault(true);
	connect(_cancel_button, SIGNAL(released()), this, SLOT(reject()));
	_ok_button = new QPushButton("OK", this);
	connect(_ok_button, SIGNAL(released()), this, SLOT(accept()));

	_grid_layout = new QGridLayout(this);
	_grid_layout->addWidget(_height_title, 0, 0);
	_grid_layout->addWidget(_height_spinbox, 0, 1);
	_grid_layout->addWidget(_height_change, 0, 2);
	_grid_layout->addWidget(_length_title, 1, 0);
	_grid_layout->addWidget(_length_spinbox, 1, 1);
	_grid_layout->addWidget(_length_change, 1, 2);
	_grid_layout->addWidget(_ok_button, 2, 1);
	_grid_layout->addWidget(_cancel_button, 2, 2);
}



MapResizeDialog::~MapResizeDialog() {
	delete _height_spinbox;
	delete _length_spinbox;
	delete _height_title;
	delete _length_title;
	delete _height_change;
	delete _length_change;
	delete _ok_button;
	delete _cancel_button;
	delete _grid_layout;
}



void MapResizeDialog::ModifyMapData() {
	uint32 new_height = static_cast<uint32>(_height_spinbox->value());
	uint32 new_length = static_cast<uint32>(_length_spinbox->value());
	Editor* editor = static_cast<Editor*>(parent());

	if ((new_height == _map_data->GetMapHeight()) && (new_length == _map_data->GetMapLength())) {
		editor->statusBar()->showMessage("Map size was not changed", 5000);
		return;
	}

	_map_data->ResizeMap(new_length, new_height);
	editor->UpdateMapView();
	editor->statusBar()->showMessage("Map resized", 5000);
}



void MapResizeDialog::_HeightChanged() {
	int32 change = _height_spinbox->value() - static_cast<int32>(_map_data->GetMapHeight());
	if (change <= 0) {
		_height_change->setText("Change: " + QString::number(change));
	}
	else {
		_height_change->setText("Change: +" + QString::number(change));
	}
}



void MapResizeDialog::_LengthChanged() {
	int32 change = _length_spinbox->value() - static_cast<int32>(_map_data->GetMapLength());
	if (change <= 0) {
		_length_change->setText("Change: " + QString::number(change));
	}
	else {
		_length_change->setText("Change: +" + QString::number(change));
	}
}

///////////////////////////////////////////////////////////////////////////////
// MapResizeInternalDialog class
///////////////////////////////////////////////////////////////////////////////

MapResizeInternalDialog::MapResizeInternalDialog(QWidget* parent, MapData* data, uint32 start_row, uint32 start_column, bool insert_operation) :
	QDialog(parent),
	_map_data(data)
{

}



MapResizeInternalDialog::~MapResizeInternalDialog() {

}



void MapResizeInternalDialog::_EnableOkButton() {

}

///////////////////////////////////////////////////////////////////////////////
// AddTilesetsDialog class
///////////////////////////////////////////////////////////////////////////////

AddTilesetsDialog::AddTilesetsDialog(QWidget* parent, MapData* data) :
	QDialog(parent),
	_map_data(data),
	_tileset_tree(NULL),
	_add_button(NULL),
	_cancel_button(NULL),
	_widget_layout(NULL)
{
	setWindowTitle("Add Tilesets...");

	if (_map_data == NULL) {
		qDebug("ERROR: AddTilesetsDialog constructor received a NULL map data pointer");
	}

	_add_button = new QPushButton("Add", this);
	_cancel_button = new QPushButton("Cancel", this);
	_cancel_button->setDefault(true);

	connect(_add_button, SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_button, SIGNAL(released()), this, SLOT(reject()));

	// Set up the list of selectable tilesets
	_tileset_tree = new QTreeWidget(this);
	_tileset_tree->setColumnCount(1);
	_tileset_tree->setHeaderLabels(QStringList("Tilesets"));
	connect(_tileset_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(_EnableAddButton()));

	// Retrieve all files found in the tileset definition directory (lua/data/tilesets).
	QList<QTreeWidgetItem*> file_items;
	QDir tileset_filenames("lua/data/tilesets");
	QStringList loaded_tileset_filenames = _map_data->GetTilesetFilenames();

	// Start the loop at 2 to skip over the present and parent working directories ("." and "..")
	for (uint32 i = 2; i < tileset_filenames.count(); i++) {
		// Exclude the file autotiling.lua, as it is not a tileset defition files
		if (tileset_filenames[i] == QString("autotiling.lua"))
			continue;

		QTreeWidgetItem* new_item = new QTreeWidgetItem(_tileset_tree, QStringList(tileset_filenames[i].remove(".lua")));
		if (loaded_tileset_filenames.contains("lua/data/tilesets/" + tileset_filenames[i]) == true) {
			new_item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
			new_item->setCheckState(0, Qt::Checked);
		}
		else {
			new_item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			new_item->setCheckState(0, Qt::Unchecked);
		}
		file_items.append(new_item);
	}

	_tileset_tree->insertTopLevelItems(0, file_items);

	_widget_layout = new QGridLayout(this);
	_widget_layout->addWidget(_tileset_tree, 0, 0, 10, -1);
	_widget_layout->addWidget(_cancel_button, 11, 0);
	_widget_layout->addWidget(_add_button, 11, 1);
}



AddTilesetsDialog::~AddTilesetsDialog() {
	delete _tileset_tree;
	delete _add_button;
	delete _cancel_button;
	delete _widget_layout;
}



uint32 AddTilesetsDialog::AddTilesetsToMapData() {
	uint32 tilesets_added = 0;

	for (int i = 0; i < _tileset_tree->topLevelItemCount(); i++) {
		QTreeWidgetItem* item = _tileset_tree->topLevelItem(i);
		// At least one tileset must be checked in order to enable push button
		if (item->checkState(0) == Qt::Checked && item->isDisabled() == false) {
			Tileset* tileset = new Tileset();
			QString filename = QString("lua/data/tilesets/") + item->text(0) + (".lua");

			if (tileset->Load(filename) == false) {
				QMessageBox::critical(this, APP_NAME, "Failed to load tileset: " + filename);
				delete tileset;
				item->setCheckState(0, Qt::Unchecked);
				continue;
			}

			if (_map_data->AddTileset(tileset) == false) {
				QMessageBox::critical(this, APP_NAME, "Failed to add tileset to map data: " + _map_data->GetErrorMessage());
				delete tileset;
				item->setCheckState(0, Qt::Unchecked);
				continue;
			}

			item->setFlags(item->flags() ^ Qt::ItemIsEnabled); // Disable this item now that it has been loaded
			tilesets_added++;
		}
	}

	return tilesets_added;
}



void AddTilesetsDialog::_EnableAddButton() {
	// Disable the add button if no tilesets are checked, otherwise enable it
	for (int i = 0; i < _tileset_tree->topLevelItemCount(); i++) {
		// At least one tileset must be checked in order to enable push button
		if (_tileset_tree->topLevelItem(i)->checkState(0) == Qt::Checked && _tileset_tree->topLevelItem(i)->isDisabled() == false) {
			_add_button->setEnabled(true);
			return;
		}
	}

	// If this point is reached, no tilesets are checked.
	_add_button->setEnabled(false);
}

} // namespace hoa_editor
