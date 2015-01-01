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
	_height_label = new QLabel("Map Height (tiles):", this);
	_height_sbox = new QSpinBox(this);
	_height_sbox->setMinimum(MINIMUM_MAP_HEIGHT);
	_height_sbox->setMaximum(MAXIMUM_MAP_HEIGHT);

	// Set up the length spinbox
	_length_label = new QLabel("Map Length (tiles):", this);
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

} // namespace hoa_editor
