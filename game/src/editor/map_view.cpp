///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    map_view.cpp
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for the map view widget
*** **************************************************************************/

#include <queue>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QScrollBar>

#ifndef QT_NO_OPENGL
	#include <QGLWidget>
#endif

#include "dialogs.h"
#include "editor.h"
#include "map_view.h"
#include "tileset.h"

using namespace std;

namespace hoa_editor {

MapView::MapView(QWidget* parent, MapData* data) :
	QGraphicsScene(parent),
	_map_data(data),
	_selection_area_active(false),
	_grid_visible(false),
	_missing_overlay_visible(false),
	_inherited_overlay_visible(false),
	_collision_overlay_visible(false),
	_cursor_tile_x(-1),
	_cursor_tile_y(-1),
	_move_source_tile_x(-1),
	_move_source_tile_y(-1),
	_selection_start_tile_x(-1),
	_selection_start_tile_y(-1),
	_edit_mode(PAINT_MODE),
	_selection_area(data->GetMapLength(), data->GetMapHeight()),
	_right_click_menu(NULL),
	_insert_menu(NULL),
	_delete_menu(NULL),
	_selection_menu(NULL),
	_insert_single_row_action(NULL),
	_insert_multiple_rows_action(NULL),
	_insert_single_column_action(NULL),
	_insert_multiple_columns_action(NULL),
	_delete_single_row_action(NULL),
	_delete_multiple_rows_action(NULL),
	_delete_single_column_action(NULL),
	_delete_multiple_columns_action(NULL)
{
	// Create the graphics view
	_graphics_view = new QGraphicsView(parent);
	_graphics_view->setRenderHints(QPainter::Antialiasing);
	_graphics_view->setBackgroundBrush(QBrush(Qt::black));
	_graphics_view->setScene(this);
	// Use OpenGL for rendering the graphics view if it is supported
	#ifndef QT_NO_OPENGL
	if (QGLFormat::hasOpenGL() && !qobject_cast<QGLWidget*>(_graphics_view->viewport())) {
		QGLFormat format = QGLFormat::defaultFormat();
		format.setDepth(false); // No depth buffer is needed for 2D surfaces
		format.setSampleBuffers(true); // Enable anti-aliasing
		_graphics_view->setViewport(new QGLWidget(format));
	}
	else {
		// Helps with rendering when not using OpenGL
		_graphics_view->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
	}
	#endif
	_graphics_view->setMouseTracking(true);
	_graphics_view->viewport()->setAttribute(Qt::WA_StaticContents); // Helps during screen resizing

	setSceneRect(0, 0, data->GetMapLength() * TILE_LENGTH, data->GetMapHeight() * TILE_HEIGHT);

	// Create the right-click menu and corresponding actions
	_insert_single_row_action = new QAction("Insert Single Row", this);
	_insert_single_row_action->setStatusTip("Inserts a single row of empty tiles at the selected location");
	connect(_insert_single_row_action, SIGNAL(triggered()), this, SLOT(_InsertSingleTileRow()));
	_insert_multiple_rows_action = new QAction("Insert Multiple Rows...", this);
	_insert_multiple_rows_action->setStatusTip("Opens a dialog window to insert one or more empty tile rows at the selected location");
	connect(_insert_multiple_rows_action, SIGNAL(triggered()), this, SLOT(_InsertMultipleTileRows()));
	_insert_single_column_action = new QAction("Insert Single Column", this);
	_insert_single_column_action->setStatusTip("Inserts a single column of empty tiles at the selected location");
	connect(_insert_single_column_action, SIGNAL(triggered()), this, SLOT(_InsertSingleTileColumn()));
	_insert_multiple_columns_action = new QAction("Insert Multiple Columns...", this);
	_insert_multiple_columns_action->setStatusTip("Opens a dialog window to insert one or more empty tile columns at the selected location");
	connect(_insert_multiple_columns_action, SIGNAL(triggered()), this, SLOT(_InsertMultipleTileColumns()));
	_delete_single_row_action = new QAction("Delete Single Row", this);
	_delete_single_row_action->setStatusTip("Deletes a single row of tiles corresponding to the selected location");
	connect(_delete_single_row_action, SIGNAL(triggered()), this, SLOT(_DeleteSingleTileRow()));
	_delete_multiple_rows_action = new QAction("Delete Multiple Rows...", this);
	_delete_multiple_rows_action->setStatusTip("Opens a dialog window to delete one or more rows of tiles at the selected location");
	connect(_delete_multiple_rows_action, SIGNAL(triggered()), this, SLOT(_DeleteMultipleTileRows()));
	_delete_single_column_action = new QAction("Delete Single Column", this);
 	_delete_single_column_action->setStatusTip("Deletes a single column of tiles corresponding to the selected location");
	connect(_delete_single_column_action, SIGNAL(triggered()), this, SLOT(_DeleteSingleTileColumn()));
	_delete_multiple_columns_action = new QAction("Delete Multiple Columns...", this);
 	_delete_multiple_columns_action->setStatusTip("Opens a dialog window to delete one or more columns of tiles at the selected location");
	connect(_delete_multiple_columns_action, SIGNAL(triggered()), this, SLOT(_DeleteMultipleTileColumns()));

	_right_click_menu = new QMenu(_graphics_view);
	_insert_menu = new QMenu("Insert", _right_click_menu);
	_delete_menu = new QMenu("Delete", _right_click_menu);
	_selection_menu = new QMenu("Selection", _right_click_menu);

	_right_click_menu->addMenu(_insert_menu);
	_right_click_menu->addMenu(_delete_menu);
	_right_click_menu->addMenu(_selection_menu);

	_insert_menu->addAction(_insert_single_row_action);
	_insert_menu->addAction(_insert_multiple_rows_action);
	_insert_menu->addAction(_insert_single_column_action);
	_insert_menu->addAction(_insert_multiple_columns_action);

	_delete_menu->addAction(_delete_single_row_action);
	_delete_menu->addAction(_delete_multiple_rows_action);
	_delete_menu->addAction(_delete_single_column_action);
	_delete_menu->addAction(_delete_multiple_columns_action);

	// Blue tile with 50% transparency
	_selection_tile = QPixmap(TILE_LENGTH, TILE_HEIGHT);
	_selection_tile.fill(QColor(0, 0, 255, 128));
	// Orange tile with 20% transparency
	_missing_tile = QPixmap(TILE_LENGTH, TILE_HEIGHT);
	_missing_tile.fill(QColor(255, 128, 0, 50));
	// Yellow tile with 20% transparency
	_inherited_tile = QPixmap(TILE_LENGTH, TILE_HEIGHT);
	_inherited_tile.fill(QColor(255, 255, 0, 50));
	// Red tile quadrant with 20% transparency
	_collision_element = QPixmap(TILE_QUADRANT_LENGTH, TILE_QUADRANT_HEIGHT);
	_collision_element.fill(QColor(255, 0, 0, 50));
} // MapView::MapView(QWidget* parent, MapData* data)



MapView::~MapView() {
	delete _insert_single_row_action;
	delete _insert_multiple_rows_action;
	delete _insert_single_column_action;
	delete _insert_multiple_columns_action;
	delete _delete_single_row_action;
	delete _delete_multiple_rows_action;
	delete _delete_single_column_action;
	delete _delete_multiple_columns_action;
	delete _right_click_menu;
	delete _insert_menu;
	delete _delete_menu;
	delete _selection_menu;
	delete _graphics_view;
}



void MapView::MapSizeModified() {
	// TODO: resize _selection_area

	_selection_area.ClearLayer();
	_selection_area_active = false;
}



void MapView::DrawMap() {
	clear();
	if (_map_data->IsInitialized() == false) {
		return;
	}

	// Setup drawing parameters
	setSceneRect(0, 0, _map_data->GetMapLength() * TILE_LENGTH, _map_data->GetMapHeight() * TILE_HEIGHT);
	setBackgroundBrush(QBrush(Qt::gray));

	vector<TileLayer>* tile_layers = &(_map_data->GetSelectedTileContext()->GetTileLayers());
	vector<TileLayer>* inherited_tile_layers = NULL;
	vector<TileLayerProperties>& layer_properties = _map_data->GetTileLayerProperties();
	vector<Tileset*>& tilesets = _map_data->GetTilesets();

	// If this is an inheriting context, we also want to pull in the tile layers for the inherited context
	if (_map_data->GetSelectedTileContext()->IsInheritingContext() == true) {
		// Inherited context should never be NULL in this case
		TileContext* inherited_context = _map_data->FindTileContextByID(_map_data->GetSelectedTileContext()->GetInheritedContextID());
		inherited_tile_layers = &(inherited_context->GetTileLayers());
	}

	// Start drawing each tile from the tile layers in order
	for (uint32 l = 0; l < tile_layers->size(); ++l) {
		// Holds the value of the tile from a specific X/Y coordinate on this layer
		int32 tile = MISSING_TILE;
		// The index value of the tile within its tileset
		int32 tile_index = 0;
		// The index of the tileset that holds the value referred to by tile
		int32 tileset_index = 0;
		// Set to true whenever we are dealing with an inherited tile
		bool inherited_tile = false;
		// Set to true if this layer is the currently selected layer that the user is viewing or editing
		bool selected_layer = (_map_data->GetSelectedTileLayer() == &(*tile_layers)[l]);

		if (layer_properties[l].IsVisible() == false)
			continue;

		for (uint32 x = 0; x < _map_data->GetMapLength(); ++x) {
			for (uint32 y = 0; y < _map_data->GetMapHeight(); ++y) {
				tile = (*tile_layers)[l].GetTile(x, y);
				inherited_tile = (tile == INHERITED_TILE);
				if (inherited_tile == true) {
					tile = (*inherited_tile_layers)[l].GetTile(x, y);
				}

				// If we're pointing to a valid tile, retrieve the appropriate indexes to the image and draw it
				if (tile >= 0) {
					tileset_index = tile / TILESET_NUM_TILES;
					tile_index = tile;
					if (tileset_index != 0) // Don't divide by zero
						tile_index = tile % (tileset_index * TILESET_NUM_TILES);
					addPixmap(*tilesets[tileset_index]->GetTileImage(tile_index))->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
				}

				// Draw the missing tile overlay if needed and move on to the next tile
				if (selected_layer == true) {
					if (inherited_tile == false && tile == MISSING_TILE && _missing_overlay_visible == true) {
						addPixmap(_missing_tile)->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
					}
					// Draw the inherited overlay over the inherited tile
					if (inherited_tile == true && _inherited_overlay_visible == true) {
						addPixmap(_inherited_tile)->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
					}
				}
			}
		}
	}

	// If the selection tool is active, draw the overlay for all tiles currently selected
	if (_selection_area_active) {
		for (uint32 x = 0; x < _map_data->GetMapLength(); ++x) {
			for (uint32 y = 0; y < _map_data->GetMapHeight(); ++y) {
				if (_selection_area.GetTile(x, y) != MISSING_TILE) {
					addPixmap(_selection_tile)->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
				}
			}
		}
	}

	if (_grid_visible)
		_DrawGrid();

	// Finally, draw the borders of the map in a red outline
	QPen pen;
	pen.setColor(Qt::red);
	addLine(0, 0, _map_data->GetMapLength() * TILE_LENGTH, 0, pen);
	addLine(0, _map_data->GetMapHeight() * TILE_HEIGHT, _map_data->GetMapLength() * TILE_LENGTH, _map_data->GetMapHeight() * TILE_HEIGHT, pen);
	addLine(0, 0, 0, _map_data->GetMapHeight() * TILE_HEIGHT, pen);
	addLine(_map_data->GetMapLength() * TILE_LENGTH, 0, _map_data->GetMapLength() * TILE_LENGTH, _map_data->GetMapHeight() * TILE_HEIGHT, pen);
} // void MapView::DrawMap()



void MapView::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (_map_data->IsInitialized() == false)
		return;

	// Don't allow edits to the selected layer if it's not visible
	if (_map_data->GetSelectedTileLayerProperties()->IsVisible() == false)
		return;

	// Takes into account the current scrolling
	int32 x = event->scenePos().x();
	int32 y = event->scenePos().y();

	// Ensure that the coordinates map to a valid tile x and y coordinate
	if (x < 0 || (x / TILE_LENGTH) >= static_cast<uint32>(_map_data->GetMapLength()) ||
		y < 0 || (y / TILE_HEIGHT) >= static_cast<uint32>(_map_data->GetMapHeight())) {
		return;
	}

	// Determine the coordinates of the tile that was click during the mouse press
	_cursor_tile_x = x / TILE_LENGTH;
	_cursor_tile_y = y / TILE_HEIGHT;

	if (_selection_area_active == false && event->button() == Qt::LeftButton) {
		switch (_edit_mode) {
			case PAINT_MODE:
				_PaintTile(_cursor_tile_x, _cursor_tile_y);
				_map_data->SetMapModified(true);
				DrawMap();
				break;

			case SWAP_MODE:
				_move_source_tile_x = _cursor_tile_x;
				_move_source_tile_y = _cursor_tile_y;
				break;

			case ERASE_MODE:
				_SetTile(_cursor_tile_x, _cursor_tile_y, MISSING_TILE);
				_map_data->SetMapModified(true);
				DrawMap();
				break;

			case INHERIT_MODE:
				_SetTile(_cursor_tile_x, _cursor_tile_y, INHERITED_TILE);
				_map_data->SetMapModified(true);
				DrawMap();
				break;

			case SELECT_AREA_MODE:
				// TODO:
				break;

			case FILL_AREA_MODE:
// 				_FillArea(_cursor_tile_x, _cursor_tile_y, ?? value from tileset);
				DrawMap();
				break;

			case CLEAR_AREA_MODE:
				_FillArea(_cursor_tile_x, _cursor_tile_y, MISSING_TILE);
				DrawMap();
				break;

			case INHERIT_AREA_MODE:
				_FillArea(_cursor_tile_x, _cursor_tile_y, INHERITED_TILE);
				DrawMap();
				break;

			default:
				QMessageBox::warning(_graphics_view, "Tile editing mode", "ERROR: Invalid tile editing mode!");
				break;
		}
	}
	else {
		// If the mutli selection is on, record the location of the beginning of the selection rectangle
		if (event->button() == Qt::LeftButton) {
			_selection_start_tile_x = _cursor_tile_x;
			_selection_start_tile_y = _cursor_tile_y;
			_selection_area.SetTile(_cursor_tile_x, _cursor_tile_y, SELECTED_TILE);
		}
	}
}



void MapView::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if (_map_data->IsInitialized() == false)
		return;

	Editor* editor = static_cast<Editor*>(_graphics_view->topLevelWidget());

	int32 mouse_x = event->scenePos().x();
	int32 mouse_y = event->scenePos().y();

	// Ensure that the coordinates map to a valid tile x and y coordinate
	if (mouse_x < 0 || (static_cast<uint32>(mouse_x) / TILE_LENGTH) >= _map_data->GetMapLength() ||
		mouse_y < 0 || (static_cast<uint32>(mouse_y) / TILE_HEIGHT) >= _map_data->GetMapHeight()) {
		editor->statusBar()->clearMessage(); // Clear any position information since we are outside the map bounds
		return;
	}

	// Determine the tile that maps to the mouse coordinates
	int32 tile_x = mouse_x / TILE_LENGTH;
	int32 tile_y = mouse_y / TILE_HEIGHT;

	// Don't allow edits to the selected layer if it's not visible
	if (_map_data->GetSelectedTileLayerProperties()->IsVisible() == false) {
		// TODO: this is duplicated at the end of the function, figure out a way to reuse this code
		QString position = QString("Tile: [%1,  %2]").arg(tile_x).arg(tile_y);
		position.append(QString(" -- Position: [%1,  %2]").arg(event->scenePos().x() * 2 / TILE_LENGTH, 0, 'f', 2)
			.arg(event->scenePos().y() * 2 / TILE_HEIGHT, 0, 'f', 2));
		editor->statusBar()->showMessage(position);
		return;
	}

	// Check if the user has moved the cursor over a different tile
	if (tile_x != _cursor_tile_x || tile_y != _cursor_tile_y) {
		_cursor_tile_x = tile_x;
		_cursor_tile_y = tile_y;

		if (_selection_area_active == true && event->buttons() == Qt::LeftButton) {
			uint32 left_x, right_x, top_y, bottom_y;

			// Use the current tile and the selection start tiles to determine which tiles are selected
			if (_cursor_tile_x <= _selection_start_tile_x) {
				left_x = _cursor_tile_x;
				right_x = _selection_start_tile_x;
			}
			else {
				left_x = _selection_start_tile_x;
				right_x = _cursor_tile_x;
			}
			if (_cursor_tile_y <= _selection_start_tile_y) {
				top_y = _cursor_tile_y;
				bottom_y = _selection_start_tile_y;
			}
			else {
				top_y = _selection_start_tile_y;
				bottom_y = _cursor_tile_y;
			}

			for (uint32 x = left_x; x <= right_x; ++x) {
				for (uint32 y = top_y; y <= bottom_y; ++y) {
					_selection_area.SetTile(x, y, SELECTED_TILE);
				}
			}
		}

		if (_selection_area_active == false && event->buttons() == Qt::LeftButton) {
			switch (_edit_mode) {
				case PAINT_MODE:
					_PaintTile(_cursor_tile_x, _cursor_tile_y);
					DrawMap();
					break;

				case SWAP_MODE:
					break;

				case ERASE_MODE:
					_SetTile(_cursor_tile_x, _cursor_tile_y, MISSING_TILE);
					DrawMap();
					break;

				case INHERIT_MODE:
					_SetTile(_cursor_tile_x, _cursor_tile_y, INHERITED_TILE);
					DrawMap();
					break;

				case SELECT_AREA_MODE:
					break;

				case FILL_AREA_MODE:
					// TODO
					break;

				case CLEAR_AREA_MODE:
					// TODO
					break;

				case INHERIT_AREA_MODE:
					// TODO
					break;

				default:
					break;
			}
		}
	}

	// Display the mouse position coordinates and the tile that the position corresponds to.
	// Note that the position coordinates are in units of the collision grid, not the tile grid.
	QString position = QString("Tile: [%1,  %2]").arg(tile_x).arg(tile_y);
	position.append(QString(" -- Position: [%1,  %2]").arg(event->scenePos().x() * 2 / TILE_LENGTH, 0, 'f', 2)
		.arg(event->scenePos().y() * 2 / TILE_HEIGHT, 0, 'f', 2));
	editor->statusBar()->showMessage(position);
} // void MapView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)



void MapView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	if (_map_data->IsInitialized() == false)
		return;

	// Don't allow edits to the selected layer if it's not visible
	if (_map_data->GetSelectedTileLayerProperties()->IsVisible() == false)
		return;

	int32 mouse_x = event->scenePos().x();
	int32 mouse_y = event->scenePos().y();

	switch (_edit_mode) {
		case PAINT_MODE: {
			if (_selection_area_active == true) {
				vector<vector<int32> > select_layer = _selection_area.GetTiles();
				for (uint32 x = 0; x < _selection_area.GetLength(); ++x) {
					for (uint32 y = 0; y < _selection_area.GetHeight(); ++y) {
						if (select_layer[y][x] != MISSING_TILE)
							_PaintTile(x, y);
					}
				}
				DrawMap();
			}
			// TODO: Record information for undo/redo stack
			break;
		}

		case SWAP_MODE: {
			_cursor_tile_x = mouse_x / TILE_LENGTH;
			_cursor_tile_y = mouse_y / TILE_HEIGHT;
			vector<vector<int32> >& layer = _map_data->GetSelectedTileLayer()->GetTiles();

			if (_selection_area_active == false) {
				// TODO: Record information for undo/redo stack
				int32 temp = layer[_cursor_tile_y][_cursor_tile_x];
				layer[_cursor_tile_y][_cursor_tile_x] = layer[_move_source_tile_y][_move_source_tile_x];
				layer[_move_source_tile_y][_move_source_tile_x] = temp;
			}
			else {
				vector<vector<int32> > select_layer = _selection_area.GetTiles();
				for (int32 y = 0; y < static_cast<int32>(select_layer.size()); ++y) {
					for (int32 x = 0; x < static_cast<int32>(select_layer[y].size()); ++x) {
						if (select_layer[y][x] != MISSING_TILE) {
							// TODO: Record information for undo/redo stack

							layer[y + _cursor_tile_y - _move_source_tile_y][x + _cursor_tile_x - _move_source_tile_x] = layer[y][x];
							layer[y][x] = MISSING_TILE;
						}
					}
				}
			}

			// TODO: Record information for undo/redo stack

			DrawMap();
			break;
		}

		case ERASE_MODE: {
			if (_selection_area_active == true) {
				vector<vector<int32> > select_layer = _selection_area.GetTiles();
				for (int32 y = 0; y < static_cast<int32>(select_layer.size()); ++y) {
					for (int32 x = 0; x < static_cast<int32>(select_layer[y].size()); ++x) {
						if (select_layer[y][x] != MISSING_TILE)
							_SetTile(x, y, MISSING_TILE);
					}
				}
				DrawMap();
			}

			// TODO: Record information for undo/redo stack
			break;
		}

		case INHERIT_MODE: {
			if (_selection_area_active == true) {
				vector<vector<int32> > select_layer = _selection_area.GetTiles();
				for (int32 y = 0; y < static_cast<int32>(select_layer.size()); ++y) {
					for (int32 x = 0; x < static_cast<int32>(select_layer[y].size()); ++x) {
						if (select_layer[y][x] != INHERITED_TILE)
							_SetTile(x, y, INHERITED_TILE);
					}
				}
				DrawMap();
			}

			// TODO: Record information for undo/redo stack
			break;
		}

		case SELECT_AREA_MODE:
			// TODO
			break;


		case FILL_AREA_MODE:
			break;

		case CLEAR_AREA_MODE:
			break;

		case INHERIT_AREA_MODE:
			break;

		default:
			QMessageBox::warning(_graphics_view, "Tile editing mode", "ERROR: Invalid tile editing mode!");
	} // switch (_edit_mode)
} // void MapView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)



void MapView::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (_map_data->IsInitialized() == false) {
		// Show the menu, but disable all options
		_insert_single_row_action->setEnabled(false);
		_insert_multiple_rows_action->setEnabled(false);
		_insert_single_column_action->setEnabled(false);
		_insert_multiple_columns_action->setEnabled(false);
		_delete_single_row_action->setEnabled(false);
		_delete_multiple_rows_action->setEnabled(false);
		_delete_single_column_action->setEnabled(false);
		_delete_multiple_columns_action->setEnabled(false);
		_right_click_menu->exec(QCursor::pos());
		return;
	}

	// We could check the map size here to see if an insert or delete operation is possible or not. We leave these options enabled because
	// we don't want to confuse the user as to why these options would suddenly be disabled. Instead, the slot functions for these actions
	// do the check and if they find it to be invalid, they'll present a warning dialog window to the user.
	_insert_single_row_action->setEnabled(true);
	_insert_multiple_rows_action->setEnabled(true);
	_insert_single_column_action->setEnabled(true);
	_insert_multiple_columns_action->setEnabled(true);
	_delete_single_row_action->setEnabled(true);
	_delete_multiple_rows_action->setEnabled(true);
	_delete_single_column_action->setEnabled(true);
	_delete_multiple_columns_action->setEnabled(true);

	// Retrieve the coordinates of the right click event and translate them into tile coordinates
	int32 mouse_x = event->scenePos().x();
	int32 mouse_y = event->scenePos().y();

	// Ensure that the coordinates map to a valid tile x and y coordinate
	if (mouse_x < 0 || (static_cast<uint32>(mouse_x) / TILE_LENGTH) >= _map_data->GetMapLength() ||
		mouse_y < 0 || (static_cast<uint32>(mouse_y) / TILE_HEIGHT) >= _map_data->GetMapHeight()) {
		return;
	}

	_cursor_tile_x = mouse_x / TILE_LENGTH;
	_cursor_tile_y = mouse_y / TILE_HEIGHT;
	_right_click_menu->exec(QCursor::pos());
}



void MapView::_InsertSingleTileRow() {
	if (_map_data->GetMapHeight() == MAXIMUM_MAP_HEIGHT) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Insert Tile Row Failure",
			"Could not insert additional tile rows as the map height is currently at its maximum limit.");
		return;
	}

	_map_data->InsertTileLayerRows(_cursor_tile_y);
	DrawMap();
}



void MapView::_InsertMultipleTileRows() {
	if (_map_data->GetMapHeight() == MAXIMUM_MAP_HEIGHT) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Insert Tile Row Failure",
			"Could not insert additional tile rows as the map height is currently at its maximum limit.");
		return;
	}

	MapResizeInternalDialog resize_dialog(_graphics_view->topLevelWidget(), _map_data, _cursor_tile_y, _cursor_tile_x, true, false);
	if (resize_dialog.exec() == QDialog::Accepted) {
		resize_dialog.ModifyMapData();
	}
}



void MapView::_InsertSingleTileColumn() {
	if (_map_data->GetMapHeight() == MAXIMUM_MAP_LENGTH) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Insert Tile Column Failure",
			"Could not insert additional tile columns as the map length is currently at its maximum limit.");
		return;
	}

	_map_data->InsertTileLayerColumns(_cursor_tile_x);
	DrawMap();
}



void MapView::_InsertMultipleTileColumns() {
	if (_map_data->GetMapHeight() == MAXIMUM_MAP_LENGTH) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Insert Tile Column Failure",
			"Could not insert additional tile columns as the map length is currently at its maximum limit.");
		return;
	}

	MapResizeInternalDialog resize_dialog(_graphics_view->topLevelWidget(), _map_data, _cursor_tile_y, _cursor_tile_x, true, true);
	if (resize_dialog.exec() == QDialog::Accepted) {
		resize_dialog.ModifyMapData();
	}
}



void MapView::_DeleteSingleTileRow() {
	if (_map_data->GetMapHeight() == MINIMUM_MAP_HEIGHT) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Delete Tile Row Failure",
			"Could not delete any tile rows as the map height is currently at its minimum limit.");
		return;
	}

	_map_data->RemoveTileLayerRows(_cursor_tile_y);
	DrawMap();
}



void MapView::_DeleteMultipleTileRows() {
	if (_map_data->GetMapHeight() == MINIMUM_MAP_HEIGHT) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Delete Tile Row Failure",
			"Could not delete any tile rows as the map height is currently at its minimum limit.");
		return;
	}

	MapResizeInternalDialog resize_dialog(_graphics_view->topLevelWidget(), _map_data, _cursor_tile_y, _cursor_tile_x, false, false);
	if (resize_dialog.exec() == QDialog::Accepted) {
		resize_dialog.ModifyMapData();
	}
}



void MapView::_DeleteSingleTileColumn() {
	if (_map_data->GetMapLength() == MINIMUM_MAP_LENGTH) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Delete Tile Column Failure",
			"Could not delete any tile columns as the map length is currently at its minimum limit.");
		return;
	}

	_map_data->RemoveTileLayerColumns(_cursor_tile_x);
	DrawMap();
}



void MapView::_DeleteMultipleTileColumns() {
	if (_map_data->GetMapLength() == MINIMUM_MAP_LENGTH) {
		QMessageBox::warning(_graphics_view->topLevelWidget(), "Delete Tile Column Failure",
			"Could not delete any tile columns as the map length is currently at its minimum limit.");
		return;
	}

	MapResizeInternalDialog resize_dialog(_graphics_view->topLevelWidget(), _map_data, _cursor_tile_y, _cursor_tile_x, false, true);
	if (resize_dialog.exec() == QDialog::Accepted) {
		resize_dialog.ModifyMapData();
	}
}



void MapView::_PaintTile(uint32 x, uint32 y) {
	// Get a reference to the current tileset
	Editor* editor = static_cast<Editor*>(_graphics_view->topLevelWidget());
	TilesetTable* tileset_table = editor->GetTilesetView()->GetCurrentTilesetTable();

	// Detect the first selection range and use to paint an area
	QList<QTableWidgetSelectionRange> selections = tileset_table->selectedRanges();
	QTableWidgetSelectionRange selection;
	if (selections.size() > 0)
		selection = selections.at(0);

	// Determine the index of the current tileset in the tileset list to determine its multiplier for calculating the image index
	vector<Tileset*> all_tilesets = _map_data->GetTilesets();
	uint32 multiplier = editor->GetTilesetView()->GetCurrentTilesetIndex();
	if (multiplier < 0) {
		qDebug() << "could not paint tile at location [" << x << ", " << y << "] "
			<< "because there was no tileset data that matched the tileset in the tileset table." << endl;
		return;
	}
	multiplier *= TILESET_NUM_TILES;

	if (selections.size() > 0 && (selection.columnCount() * selection.rowCount() > 1)) {
		// Draw tiles from tileset selection onto map, one tile at a time.
		for (int32 i = 0; i < selection.rowCount() && y + i < _map_data->GetMapHeight(); i++) {
			for (int32 j = 0; j < selection.columnCount() && x + j < _map_data->GetMapLength(); j++) {
				int32 tileset_index = (selection.topRow() + i) * 16 + (selection.leftColumn() + j);

				// TODO: Perform randomization for autotiles
				// _AutotileRandomize(multiplier, tileset_index);

				// TODO: Record information for undo/redo stack

				_map_data->GetSelectedTileLayer()->SetTile(x + j, y + i, tileset_index + multiplier);
			} // iterate through columns of selection
		} // iterate through rows of selection
	} // multiple tiles are selected
	else {
		// put selected tile from tileset into tile array at correct position
		int32 tileset_index = tileset_table->currentRow() * TILESET_NUM_COLS + tileset_table->currentColumn();

		// TODO: Perform randomization for autotiles
		// _AutotileRandomize(multiplier, tileset_index);

		// TODO: Record information for undo/redo stack

		_map_data->GetSelectedTileLayer()->SetTile(x, y, tileset_index + multiplier);
	} // a single tile is selected
}



void MapView::_SetTile(int32 x, int32 y, int32 value) {
	// TODO: Record information for undo/redo stack
	_map_data->GetSelectedTileLayer()->SetTile(x, y, value);
}



void MapView::_DrawGrid() {
	for (uint32 x = 0; x < (_map_data->GetMapLength() * TILE_LENGTH); x+= TILE_LENGTH) {
		for (uint32 y = 0; y < (_map_data->GetMapHeight() * TILE_HEIGHT); y+= TILE_HEIGHT) {
			addLine(0, y, _map_data->GetMapLength() * TILE_LENGTH, y, QPen(Qt::DotLine));
			addLine(x, 0, x, _map_data->GetMapHeight() * TILE_HEIGHT, QPen(Qt::DotLine));
		}
	}
}



void MapView::_FillArea(uint32 start_x, uint32 start_y, int32 value) {
	if (start_x >= _map_data->GetMapLength() || start_y >= _map_data->GetMapHeight()) {
		return;
	}

	// When there is a selection area active, this function will only perform the fill operation when
	// the tile clicked is one that is inside the selection area
	if (_selection_area_active == true && _selection_area.GetTile(start_x, start_y) == SELECTED_TILE) {
		return;
	}


	// This function is an implementation of a flood fill algorithm. Generally speaking, the algorithm does the following:
	//   1) Maintain a queue of nodes (x,y coordinates) that need to be examined
	//   2) For each node in the queue, if it still needs to be set to the new value then
	//      3) Find the left and right ends of the segment of all tiles that match the old value
	//      4) For each point in the segement, set it to the new value and examine the top and bottom points
	//      5) If the top or bottom point matches the old value, add it to the nodes queue
	//   6) Repeat this process until the queue is empty
	//
	// There may be faster alternatives to this (each node except for the starting node is checked

	TileLayer* layer = _map_data->GetSelectedTileLayer();

	int32 original_value = layer->GetTile(start_x, start_y);
	if (original_value == value) {
		return;
	}

	// The coordinates of the node actively being processed
	uint32 x, y;

	// The left and right nodes that end the current segement
	uint32 x_left_end, x_right_end;

	// Queue that holds the nodes that need to be checked (x, y cooridnate pairs)
	queue<pair<uint32, uint32> > nodes;
	nodes.push(make_pair(start_x, start_y));

	// The algorithm works the same way for both of these conditions. The only difference is whether the comparison
	// check is done on the value of the current tile or whether the tile is currently selected in the selection area
	if (_selection_area_active == false) {
		while (nodes.empty() == false) {
			x = nodes.front().first;
			y = nodes.front().second;
			nodes.pop();
			// In this case, the node has already been set to the new value previously so we can skip over it
			if (layer->GetTile(x, y) != original_value) {
				continue;
			}

			// Find the left and right ends of the current line segment in row y
			x_left_end = x;
			while ((x_left_end > 0) && (layer->GetTile(x_left_end - 1, y) == original_value)) {
				x_left_end--;
			}
			x_right_end = x;
			while ((x_right_end < _map_data->GetMapLength() - 1) && (layer->GetTile(x_right_end + 1, y) == original_value)) {
				x_right_end++;
			}
			// Go through the segment and set the values of each node, adding the element to the top and bottom to the nodes queue if necessary
			for (uint32 i = x_left_end; i <= x_right_end; ++i) {
				layer->SetTile(i, y, value);
				if ((y > 0) && (layer->GetTile(i, y - 1) == original_value)) {
					nodes.push(make_pair(i, y - 1));
				}
				if ((y < _map_data->GetMapHeight() - 1) && (layer->GetTile(i, y + 1) == original_value)) {
					nodes.push(make_pair(i, y + 1));
				}
			}
		}
	}
	else {
		// TODO
	}
}

} // namespace hoa_editor
