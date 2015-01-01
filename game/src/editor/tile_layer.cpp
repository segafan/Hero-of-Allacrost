///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tile_layer.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for tile layer data and view classes
*** **************************************************************************/

#include <QDebug>
#include <QMouseEvent>

#include "editor_utils.h"
#include "editor.h"
#include "map_data.h"
#include "tile_layer.h"

using namespace std;

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// TileLayer class
///////////////////////////////////////////////////////////////////////////////

int32 TileLayer::GetTile(uint32 x, uint32 y) const {
	// Make sure that there is at least one column and one row
	if (_tiles.empty() == true)
		return NO_TILE;
	if (_tiles[0].empty() == true)
		return NO_TILE;

	// Check that the coordinates do not exceed the bounds of the layer
	if (y >= _tiles.size())
		return NO_TILE;
	if (x >= _tiles[0].size())
		return NO_TILE;

	return _tiles[y][x];
}



void TileLayer::SetTile(uint32 x, uint32 y, int32 value) {
	// Make sure that there is at least one column and one row
	if (_tiles.empty() == true)
		return;
	if (_tiles[0].empty() == true)
		return;

	// Check that the coordinates do not exceed the bounds of the layer
	if (y >= _tiles.size())
		return;
	if (x >= _tiles[0].size())
		return;

	_tiles[y][x] = value;
}



void TileLayer::FillLayer(int32 tile_id) {
	for (uint32 y = 0; y < _tiles.size(); ++y) {
		for (uint32 x = 0; x < _tiles[y].size(); ++x) {
			_tiles[y][x] = tile_id;
		}
	}
}



void TileLayer::_AddLayerRow(uint32 row_index, int32 value) {
	uint32 old_height = GetHeight();
	uint32 length = GetLength();

	if (old_height == 0) {
		return;
	}
	if (row_index > old_height) {
		return;
	}

	// And an empty tile row to the bottom
	_tiles.push_back(vector<int32>(length, NO_TILE));
	// Shift the appropriate rows one position down to make room for the row to add
	for (uint32 i = _tiles.size() - 1; i > row_index; --i) {
		_tiles[i] = _tiles[i-1];
	}
	// Now set the tiles at the new row to NO_TILE
	for (uint32 i = 0; i < length; ++i) {
		_tiles[row_index][i] = NO_TILE;
	}
}



void TileLayer::_AddLayerCol(uint32 col_index, int32 value) {
	uint32 height = GetHeight();
	uint32 old_length = GetLength();

	if (height == 0) {
		return;
	}
	if (col_index > old_length) {
		return;
	}

	for (uint32 i = 0; i < height; ++i) {
		// Add an empty tile column to the back of each row.
		_tiles[i].push_back(NO_TILE);
		// Shift the columns one position right to make room for the column to add.
		for (uint32 j = _tiles[i].size() - 1; j > col_index; --j) {
			_tiles[i][j] = _tiles[i][j-1];
		}
		// Set the value in the new column to NO_TILE
		_tiles[i][col_index] = NO_TILE;
	}
}



void TileLayer::_DeleteLayerRow(uint32 row_index) {
	uint32 old_height = GetHeight();

	if (old_height == 0) {
		return;
	}
	if (row_index >= old_height) {
		return;
	}

	// Swap all rows below the index one position up to replace the deleted row
	for (uint32 i = row_index; i < _tiles.size() - 1; ++i) {
		_tiles[i] = _tiles[i+1];
	}
	// Now remove the last row
	_tiles.pop_back();
}



void TileLayer::_DeleteLayerCol(uint32 col_index) {
	uint32 height = GetHeight();
	uint32 old_length = GetLength();

	if (height == 0) {
		return;
	}
	if (col_index > old_length) {
		return;
	}

	// Swap all columns to the right of col_index one position left to replace the deleted column
	for (uint32 i = 0; i < height; ++i) {
		for (uint32 j = col_index; j < old_length - 1; ++j) {
			_tiles[i][j] = _tiles[i][j+1];
		}
		// Now remove the last column
		_tiles[i].pop_back();
	}
}



void TileLayer::_ResizeLayer(uint32 length, uint height) {
	_tiles.resize(height, vector<int32>(length));
	for (uint32 y = 0; y < height; ++y) {
		_tiles[y].resize(length, NO_TILE);
	}
}

///////////////////////////////////////////////////////////////////////////////
// LayerView class
///////////////////////////////////////////////////////////////////////////////

const uint32 LayerView::ID_COLUMN;
const uint32 LayerView::VISIBLE_COLUMN;
const uint32 LayerView::NAME_COLUMN;
const uint32 LayerView::COLLISION_COLUMN;

LayerView::LayerView(MapData* data) :
	QTreeWidget(),
	_map_data(data),
	_original_layer_name(),
	_visibility_icon(QString("img/misc/editor_tools/eye.png")),
	_right_click_item(NULL)
{
	if (data == NULL) {
		qDebug() << "constructor received NULL map data argument" << endl;
		return;
	}

	// Enable settings so that layers can be dragged and reordered
	setSelectionMode(QAbstractItemView::SingleSelection);
	setDragEnabled(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::InternalMove);

	// Create column dimensions, headers, and properties
    setColumnCount(4);
	hideColumn(ID_COLUMN); // Hide the ID column as we only use it internally
	setColumnWidth(VISIBLE_COLUMN, 25); // Make this column small as it only contains the eye icon
	setColumnWidth(NAME_COLUMN, 200);
	QStringList layer_headers;
	layer_headers << "ID" << "" << "Layer" << "Collisions";
	setHeaderLabels(layer_headers);
	setIndentation(0);

	// Setup actions for the right click menu
	_add_layer_action = new QAction("Add New Layer", this);
	_add_layer_action->setStatusTip("Adds a new empty tile layer to the end of the layer list");
	_rename_layer_action = new QAction("Rename Layer", this);
	_rename_layer_action->setStatusTip("Renames the selected layer (can also be activated by double-clicking the layer's name)");
	_delete_layer_action = new QAction("Delete Tile Layer", this);
	_delete_layer_action->setStatusTip("Deletes the selected layer");

	_right_click_menu = new QMenu(this);
	_right_click_menu->addAction(_add_layer_action);
	_right_click_menu->addAction(_rename_layer_action);
	_right_click_menu->addAction(_delete_layer_action);

	// Connect all signals and slots
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(_ChangeSelectedLayer()));
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(_SetTileLayerName(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(_ChangeLayerProperties(QTreeWidgetItem*, int)));
	connect(_add_layer_action, SIGNAL(triggered()), this, SLOT(_AddTileLayer()));
	connect(_rename_layer_action, SIGNAL(triggered()), this, SLOT(_RenameTileLayer()));
	connect(_delete_layer_action, SIGNAL(triggered()), this, SLOT(_DeleteTileLayer()));
}



LayerView::~LayerView() {
	delete _right_click_menu;
	delete _add_layer_action;
	delete _rename_layer_action;
	delete _delete_layer_action;
}



void LayerView::mousePressEvent(QMouseEvent* event) {
	// Handle left clicks the standard way. Right clicks bring up the layer action menu
	if (event->button() == Qt::LeftButton) {
		QTreeWidget::mousePressEvent(event);
	}
	else {
		// Determine which QTreeWidgetItem was selected, if any. Enable/disable menu actions appropriately
		_right_click_item = itemAt(event->pos());
		if (_right_click_item != NULL) {
			_rename_layer_action->setEnabled(true);
			_delete_layer_action->setEnabled(true);
		}
		else {
			// Clicked a space in the widget that did not point to any item
			_rename_layer_action->setEnabled(false);
			_delete_layer_action->setEnabled(false);
		}

		_right_click_menu->exec(QCursor::pos());
		return;
	}
}



void LayerView::dropEvent(QDropEvent* event) {
	QTreeWidget::dropEvent(event);
	vector<uint32> layer_order; // Holds the new layer positions

	// Update the IDs for each tile layer to correspond to the new layer order
	QTreeWidgetItem* root = invisibleRootItem();
	for (uint32 i = 0; i < static_cast<uint32>(root->childCount()); ++i) {
		QTreeWidgetItem* child = root->child(i);
		layer_order.push_back(child->text(ID_COLUMN).toUInt());
		child->setText(ID_COLUMN, QString::number(i));
	}

	// Make the appropriate changes corresponding to the layer order in the map data
	for (uint32 i = 0; i < layer_order.size(); ++i) {
		// Skip over layers that haven't been affected by the reordering
		if (layer_order[i] == i) {
			continue;
		}

		// Find the new location of this layer and swap it with other layer
		for (uint32 j = 0; j < _map_data->GetTileLayerCount(); ++j) {
			if (layer_order[j] == i) {
				uint32 temp = layer_order[i];
				layer_order[i] = layer_order[j];
				layer_order[j] = temp;
				_map_data->SwapTileLayers(i, j);
				break;
			}
		}
	}

	static_cast<Editor*>(topLevelWidget())->UpdateMapView();
}



void LayerView::RefreshView() {
	clear();

	// Add all tile layers from the map data
	vector<TileLayerProperties>& layer_properties = _map_data->GetTileLayerProperties();

	for (uint32 i = 0; i < layer_properties.size(); ++i) {
		QTreeWidgetItem* item = new QTreeWidgetItem(this);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
		item->setText(ID_COLUMN, QString::number(i));
		if (layer_properties[i].IsVisible() == true)
			item->setIcon(VISIBLE_COLUMN, _visibility_icon);
		else
			item->setIcon(VISIBLE_COLUMN, QIcon());
		item->setText(NAME_COLUMN, layer_properties[i].GetName());
		item->setText(COLLISION_COLUMN, layer_properties[i].IsCollisionEnabled() ? QString("Enabled") : QString("Disabled"));
	}

	setCurrentItem(itemAt(0, 0));
}



void LayerView::_ChangeSelectedLayer() {
	// We only allow one selected layer at a time. The size of selected items should only ever be 0 or 1.
	QList<QTreeWidgetItem*> selected_items = selectedItems();
	if (selected_items.size() != 1) {
		return;
	}

	QTreeWidgetItem* selection = selected_items.first();
	uint32 layer_id = selection->text(ID_COLUMN).toUInt();
	if (_map_data->ChangeSelectedTileLayer(layer_id) == NULL) {
		QMessageBox::warning(this, "Layer Selection Failure", _map_data->GetErrorMessage());
	}
}



void LayerView::_ChangeLayerProperties(QTreeWidgetItem* item, int column) {
	if (item == NULL)
		return;

	uint32 layer_id = item->text(ID_COLUMN).toUInt();
	vector<TileLayerProperties>& layer_properties = _map_data->GetTileLayerProperties();
	if (column == VISIBLE_COLUMN) {
		_map_data->ToggleTileLayerVisibility(layer_id);
		if (layer_properties[layer_id].IsVisible() == true)
			item->setIcon(VISIBLE_COLUMN, _visibility_icon);
		else
			item->setIcon(VISIBLE_COLUMN, QIcon());

		static_cast<Editor*>(topLevelWidget())->UpdateMapView();
	}
	else if (column == NAME_COLUMN) {
		// While technically this was not a right-click event, this allows us to use the same code path for performing rename operations
		_right_click_item = item;
		_RenameTileLayer();
	}
	else if (column == COLLISION_COLUMN) {
		_map_data->ToggleTileLayerCollision(layer_id);
		item->setText(COLLISION_COLUMN, layer_properties[layer_id].IsCollisionEnabled() ? QString("Enabled") : QString("Disabled"));
	}
	else {
		QMessageBox::warning(this, "Layer Property Change Failure", "Invalid column clicked");
	}
}



void LayerView::_SetTileLayerName(QTreeWidgetItem* item, int column) {
	if ((item != _right_click_item) || (column != NAME_COLUMN) || (_original_layer_name.isEmpty() == true))
		return;

	closePersistentEditor(item, column);
	if (_map_data->RenameTileLayer(item->text(ID_COLUMN).toUInt(), item->text(NAME_COLUMN)) == false) {
		// To prevent an infinite recursion loop, we must nullify _right_click_item before restoring the layer's name
		_right_click_item = NULL;
		item->setText(NAME_COLUMN, _original_layer_name);
		_original_layer_name.clear();
		QMessageBox::warning(this, "Layer Rename Failure", _map_data->GetErrorMessage());
		return;
	}

	_original_layer_name.clear();
}



void LayerView::_AddTileLayer() {
	static uint32 new_layer_number = 1; // Used so that each new tile layer added is written as "New Layer (#)"

	// Add the new layer to the map data. If it fails, increment the number to use a different layer name and try again
	QString layer_name;
	while (true) {
		layer_name.clear();
		layer_name = "New Layer (" + QString::number(new_layer_number) + QString(")");

		if (_map_data->AddTileLayer(layer_name, true) == true) {
			_map_data->SetMapModified(true);
			break;
		}
		else {
			new_layer_number++;
		}
	}

	// Add the new item to the view. All new tile layers will have vision and collisions enableed
	QTreeWidgetItem* item = new QTreeWidgetItem(this);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	item->setText(ID_COLUMN, QString::number(_map_data->GetTileLayerCount() - 1));
	item->setIcon(VISIBLE_COLUMN, _visibility_icon);
	item->setText(NAME_COLUMN, layer_name);
	item->setText(COLLISION_COLUMN, "Enabled");

	setCurrentItem(item); // Select the newly added item
	new_layer_number++;
}



void LayerView::_RenameTileLayer() {
	if (_right_click_item == NULL)
		return;

	_original_layer_name = _right_click_item->text(NAME_COLUMN);
	openPersistentEditor(_right_click_item, NAME_COLUMN);
}



void LayerView::_DeleteTileLayer() {
	if (_right_click_item == NULL)
		return;

	if (_map_data->GetTileLayerCount() == 1) {
		QMessageBox::warning(this, "Layer Deletion Failure", "You may not delete the last remaining layer for a map.");
		return;
	}

	// Delete the layer from the map data first and make sure that it was successful
	if (_map_data->DeleteTileLayer(static_cast<uint32>(indexOfTopLevelItem(_right_click_item))) == false) {
		QMessageBox::warning(this, "Layer Deletion Failure", _map_data->GetErrorMessage());
		return;
	}
	_map_data->SetMapModified(true);

	// If the item being deleted is the selected item, change the selction to the item before it (or after if its the first item)
	if (currentItem() == _right_click_item) {
		QTreeWidgetItem* new_selection = itemAbove(_right_click_item);
		if (new_selection == NULL)
			new_selection = itemBelow(_right_click_item);
		setCurrentItem(new_selection);
	}

	// Deleting the item directly also removes it from the QTreeWidget automatically
	delete _right_click_item;
	_right_click_item = NULL;

	// Update the IDs of the remaining layers
	QTreeWidgetItem* root = invisibleRootItem();
	for (uint32 i = 0; i < static_cast<uint32>(root->childCount()); ++i) {
		root->child(i)->setText(ID_COLUMN, QString::number(i));
	}

	// Redraw the map view now that the layer is removed
	static_cast<Editor*>(topLevelWidget())->UpdateMapView();
}

} // namespace hoa_editor
