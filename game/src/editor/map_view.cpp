///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
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

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QScrollBar>

#ifndef QT_NO_OPENGL
	#include <QGLWidget>
#endif

#include "map_view.h"
#include "editor.h"

using namespace std;

namespace hoa_editor {

MapView::MapView(QWidget* parent, MapData* data) :
	QGraphicsScene(),
	_map_data(data),
	_grid_visible(false),
	_selection_visible(false),
	_cursor_tile_x(-1),
	_cursor_tile_y(-1),
	_move_source_tile_x(-1),
	_move_source_tile_y(-1),
	_selection_start_tile_x(-1),
	_selection_start_tile_y(-1),
	_tile_mode(PAINT_TILE),
	_select_layer(data->GetMapLength(), data->GetMapHeight())
{
	// Create the graphics view
	_graphics_view = new QGraphicsView(parent);
	_graphics_view->setRenderHints(QPainter::Antialiasing);
	_graphics_view->setBackgroundBrush(QBrush(Qt::black));
	_graphics_view->setScene(this);
	// Enable OpenGL for rendering the graphics view if it is supported
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

	// Right-click menu action creation
	_insert_row_action = new QAction("Insert Tile Rows", this);
	_insert_row_action->setStatusTip("Inserts one or more rows of empty tiles above or below the selected tile");
	connect(_insert_row_action, SIGNAL(triggered()), this, SLOT(_InsertTileRow()));
	_insert_column_action = new QAction("Insert Tile Columns", this);
	_insert_column_action->setStatusTip("Inserts one or more columns of empty tiles above or below the selected tile");
	connect(_insert_column_action, SIGNAL(triggered()), this, SLOT(_InsertTileColumn()));
	_delete_row_action = new QAction("Delete Tile Rows", this);
	_delete_row_action->setStatusTip("Deletes the currently selected row of tiles from all layers");
	connect(_delete_row_action, SIGNAL(triggered()), this, SLOT(_DeleteTileRow()));
	_delete_column_action = new QAction("Delete Tile Columns", this);
 	_delete_column_action->setStatusTip("Deletes the currently selected column of tiles from all layers");
	connect(_delete_column_action, SIGNAL(triggered()), this, SLOT(_DeleteTileColumn()));

	_right_click_menu = new QMenu(_graphics_view);
	_right_click_menu->addAction(_insert_row_action);
	_right_click_menu->addAction(_insert_column_action);
	_right_click_menu->addSeparator();
	_right_click_menu->addAction(_delete_row_action);
	_right_click_menu->addAction(_delete_column_action);

	// Blue selection tile with 50% transparency
	_selection_tile = QPixmap(TILE_LENGTH, TILE_HEIGHT);
	_selection_tile.fill(QColor(0, 0, 255, 125));
} // MapView::MapView(QWidget* parent, MapData* data)



MapView::~MapView() {
	// TODO: uncomment when tileset data is moved to this class


	delete _insert_row_action;
	delete _insert_column_action;
	delete _delete_row_action;
	delete _delete_column_action;
	delete _right_click_menu;
	delete _graphics_view;
}



void MapView::mousePressEvent(QGraphicsSceneMouseEvent* event) {
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

	if (_selection_visible == false && event->button() == Qt::LeftButton) {
		switch (_tile_mode) {
			case PAINT_TILE:
				_PaintTile(_cursor_tile_x, _cursor_tile_y);
				_map_data->SetMapModified(true);
				DrawMap();
				break;

			case MOVE_TILE:
				_move_source_tile_x = _cursor_tile_x;
				_move_source_tile_y = _cursor_tile_y;
				break;

			case DELETE_TILE:
				_DeleteTile(_cursor_tile_x, _cursor_tile_y);
				_map_data->SetMapModified(true);
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
			_select_layer.SetTile(_cursor_tile_x, _cursor_tile_y, TILE_SELECTED);
		}
	}
}



void MapView::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
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

	// Check if the user has moved the cursor over a different tile
	if (tile_x != _cursor_tile_x || tile_y != _cursor_tile_y) {
		_cursor_tile_x = tile_x;
		_cursor_tile_y = tile_y;

		if (_selection_visible == true && event->buttons() == Qt::LeftButton) {
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
					_select_layer.SetTile(x, y, TILE_SELECTED);
				}
			}
		}

		if (_selection_visible == false && event->buttons() == Qt::LeftButton) {
			switch (_tile_mode) {
				case PAINT_TILE:
					_PaintTile(_cursor_tile_x, _cursor_tile_y);
					DrawMap();
					break;

				case MOVE_TILE:
					break;

				case DELETE_TILE:
					_DeleteTile(_cursor_tile_x, _cursor_tile_y);
					DrawMap();
					break;

				default:
					QMessageBox::warning(_graphics_view, "Tile editing mode", "ERROR: Invalid tile editing mode!");
					break;
			}
		}
	}

	// Display the mouse position coordinates and the tile that the position it corresponds to
	QString position = QString("Position [%1,  %2]").arg(static_cast<float>(mouse_x * 2 / TILE_LENGTH), 0, 'f', 2)
		.arg(static_cast<float>(mouse_y * 2 / TILE_HEIGHT), 0, 'f', 2);
	position.append(QString(" - Tile [%1,  %2]").arg(tile_x).arg(tile_y));
	editor->statusBar()->showMessage(position);
} // void MapView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)



void MapView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	int32 mouse_x = event->scenePos().x();
	int32 mouse_y = event->scenePos().y();

	switch (_tile_mode) {
		case PAINT_TILE: {
			if (_selection_visible == true) {
				vector<vector<int32> > select_layer = _select_layer.GetTiles();
				for (uint32 x = 0; x < _select_layer.GetLength(); ++x) {
					for (uint32 y = 0; y < _select_layer.GetHeight(); ++y) {
						if (select_layer[y][x] != NO_TILE)
							_PaintTile(x, y);
					}
				}
				DrawMap();
			}
			// TODO: add the paint command to the Editor undo stack
// 			LayerCommand *paint_command = new LayerCommand(_tile_indeces, _previous_tiles, _modified_tiles, layer_id, editor, "Paint");
// 			editor->_undo_stack->push(paint_command);
// 			_tile_indeces.clear();
// 			_previous_tiles.clear();
// 			_modified_tiles.clear();
			break;
		}

		case MOVE_TILE: {
			_cursor_tile_x = mouse_x / TILE_LENGTH;
			_cursor_tile_y = mouse_y / TILE_HEIGHT;
			std::vector<std::vector<int32> >& layer = _map_data->GetSelectedTileLayer()->GetTiles();

			if (_selection_visible == false) {
				// TODO: Record information for undo/redo action.
	// 			_tile_indeces.push_back(_move_source_index);
	// 			_previous_tiles.push_back(layer[_move_source_tile_y][_move_source_tile_x]);
	// 			_modified_tiles.push_back(NO_TILE);
	// 			_tile_indeces.push_back(_tile_index);
	// 			_previous_tiles.push_back(layer[_cursor_tile_y][_cursor_tile_x]);
	// 			_modified_tiles.push_back(layer[_move_source_tile_y][_move_source_tile_x]);

				layer[_cursor_tile_y][_cursor_tile_x] = layer[_move_source_tile_y][_move_source_tile_x];
				layer[_move_source_tile_y][_move_source_tile_x] = NO_TILE;
			}
			else {
				vector<vector<int32> > select_layer = _select_layer.GetTiles();
				for (int32 y = 0; y < static_cast<int32>(select_layer.size()); ++y) {
					for (int32 x = 0; x < static_cast<int32>(select_layer[y].size()); ++x) {
						if (select_layer[y][x] != NO_TILE) {
							// TODO: Record information for undo/redo action.
	// 						_tile_indeces.push_back(QPoint(x, y));
	// 						_previous_tiles.push_back(layer[y][x]);
	// 						_modified_tiles.push_back(-1);
	// 						_tile_indeces.push_back(QPoint(x + _cursor_tile_x - _move_source_tile_x, y + _cursor_tile_y - _move_source_tile_y));
	// 						_previous_tiles.push_back(layer[y + _cursor_tile_y - _move_source_tile_y][x + _cursor_tile_x - _move_source_tile_x]);
	// 						_modified_tiles.push_back(layer[y][x]);

							layer[y + _cursor_tile_y - _move_source_tile_y][x + _cursor_tile_x - _move_source_tile_x] = layer[y][x];
							layer[y][x] = NO_TILE;
						}
					}
				}
			}

			// TODO: Push the move command on to the undo stack
	// 		LayerCommand *move_command = new LayerCommand(_tile_indeces, _previous_tiles, _modified_tiles, _layer_id, editor, "Move");
	// 		editor->_undo_stack->push(move_command);
	// 		_tile_indeces.clear();
	// 		_previous_tiles.clear();
	// 		_modified_tiles.clear();

			DrawMap();
			break;
		}

		case DELETE_TILE: {
			if (_selection_visible == true) {
				vector<vector<int32> > select_layer = _select_layer.GetTiles();
				for (int32 y = 0; y < static_cast<int32>(select_layer.size()); ++y) {
					for (int32 x = 0; x < static_cast<int32>(select_layer[y].size()); ++x) {
						if (select_layer[y][x] != NO_TILE)
							_DeleteTile(x, y);
					}
				}
				DrawMap();
			}

			// TODO: add the delete command to the Editor undo stack
	// 		LayerCommand* delete_command = new LayerCommand(_tile_indeces, _previous_tiles, _modified_tiles, _layer_id, editor, "Delete");
	// 		editor->_undo_stack->push(delete_command);
	// 		_tile_indeces.clear();
	// 		_previous_tiles.clear();
	// 		_modified_tiles.clear();
			break;
		}

		default:
			QMessageBox::warning(_graphics_view, "Tile editing mode", "ERROR: Invalid tile editing mode!");
	} // switch (_tile_mode)
} // void MapView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)



void MapView::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
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
	static_cast<Editor*>(_graphics_view->topLevelWidget())->statusBar()->clearMessage();
}



void MapView::keyPressEvent(QKeyEvent* event) {
	// TODO: map key presses to different actions
	if (event->key() == Qt::Key_Delete) {
		// Handle object deletion
	}
}



void MapView::DrawMap() {
	if (_map_data->IsInitialized() == false) {
		return;
	}

	// Setup drawing parameters
	clear();
	setSceneRect(0, 0, _map_data->GetMapLength() * TILE_LENGTH, _map_data->GetMapHeight() * TILE_HEIGHT);
	setBackgroundBrush(QBrush(Qt::gray));

	vector<TileLayer>& tile_layers = _map_data->GetSelectedTileContext()->GetTileLayers();
	vector<TileLayerProperties>& layer_properties = _map_data->GetTileLayerProperties();
	vector<Tileset*>& tilesets = _map_data->GetTilesets();

	// Start drawing from the top left through all the visible tile layers in order
	for (uint32 x = 0; x < _map_data->GetMapLength(); ++x) {
		for (uint32 y = 0; y < _map_data->GetMapHeight(); ++y) {
			for (uint32 layer_id = 0; layer_id < tile_layers.size(); ++layer_id) {
				// Don't draw the layer if its not visible
 				if (layer_properties[layer_id].IsVisible() == false)
 					continue;

				 int32 layer_index = tile_layers[layer_id].GetTile(x, y);
				 // Draw tile if one exists at this location
				 if (layer_index == NO_TILE)
					 continue;

				int32 tileset_index = layer_index / TILESET_NUM_TILES;
				// Don't divide by zero
				int32 tile_index = 0;
				if (tileset_index == 0)
					tile_index = layer_index;
				else
					tile_index = layer_index % (tileset_index * TILESET_NUM_TILES);

				addPixmap(*tilesets[tileset_index]->GetTileImage(tile_index))->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
			}

			// Draw the selection square over this tile if selection mode is active and the tile is inside the selected area
			if (_selection_visible && _select_layer.GetTile(x, y) != NO_TILE) {
				addPixmap(_selection_tile)->setPos(x * TILE_LENGTH, y * TILE_HEIGHT);
			}
		}
	}

	// Draw the tile grid over the tiles if it's visibility is enabled
	if (_grid_visible)
		_DrawGrid();

	// Finally, draw the borders of the map in a red outline
	QPen pen;
	pen.setColor(Qt::red);
	addLine(0, 0, _map_data->GetMapLength() * TILE_LENGTH, 0, pen);
	addLine(0, _map_data->GetMapHeight() * TILE_HEIGHT, _map_data->GetMapLength() * TILE_LENGTH, _map_data->GetMapHeight() * TILE_HEIGHT, pen);
	addLine(0, 0, 0, _map_data->GetMapHeight() * TILE_HEIGHT, pen);
	addLine(_map_data->GetMapLength() * TILE_LENGTH, 0, _map_data->GetMapLength() * TILE_LENGTH, _map_data->GetMapHeight() * TILE_HEIGHT, pen);
}



void MapView::_InsertTileRow() {
	_map_data->InsertTileLayerRows(_cursor_tile_y);
	DrawMap();
}



void MapView::_InsertTileColumn() {
	_map_data->InsertTileLayerColumns(_cursor_tile_x);
	DrawMap();
}



void MapView::_DeleteTileRow() {
	_map_data->RemoveTileLayerRows(_cursor_tile_y);
	DrawMap();
}



void MapView::_DeleteTileColumn() {
	_map_data->RemoveTileLayerColumns(_cursor_tile_x);
	DrawMap();
}



void MapView::_PaintTile(uint32 x, uint32 y) {
	// Get a reference to the current tileset
	Editor* editor = static_cast<Editor*>(_graphics_view->topLevelWidget());
	QTableWidget* table = static_cast<QTableWidget*>(editor->GetTilesetTabs()->currentWidget());
	QString tileset_name = editor->GetTilesetTabs()->tabText(editor->GetTilesetTabs()->currentIndex());

	// Detect the first selection range and use to paint an area
	QList<QTableWidgetSelectionRange> selections = table->selectedRanges();
	QTableWidgetSelectionRange selection;
	if (selections.size() > 0)
		selection = selections.at(0);

	// Determine the index of the current tileset in the tileset list to determine its multiplier for calculating the image index
	TilesetTable* tileset_table = static_cast<TilesetTable*>(table);
	vector<Tileset*> all_tilesets = _map_data->GetTilesets();
	uint32 multiplier = all_tilesets.size(); // Initially set to a value that is a known bad tileset index for error checking

	for (uint32 i = 0; i < all_tilesets.size(); ++i) {
		if (all_tilesets[i] == tileset_table->GetTileset()) {
			multiplier = i;
			break;
		}
	}

	if (multiplier == all_tilesets.size()) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "could not paint tile at location [" << x << ", " << y << "] "
			<< "because there was no tileset data that matched the tileset in the tileset table." << endl;
		return;
	}

	if (selections.size() > 0 && (selection.columnCount() * selection.rowCount() > 1)) {
		// Draw tiles from tileset selection onto map, one tile at a time.
		for (int32 i = 0; i < selection.rowCount() && y + i < _map_data->GetMapHeight(); i++) {
			for (int32 j = 0; j < selection.columnCount() && x + j < _map_data->GetMapLength(); j++) {
				int32 tileset_index = (selection.topRow() + i) * 16 + (selection.leftColumn() + j);

				// Perform randomization for autotiles
				// TODO: reenable autotiling feature
				// _AutotileRandomize(multiplier, tileset_index);

				// Record information for undo/redo action.
				// TODO: put this data into a LayerCommand object
// 				_tile_indeces.push_back(QPoint(index_x + j, index_y + i));
// 				_previous_tiles.push_back(GetCurrentLayer()[index_y + i][index_x + j]);
// 				_modified_tiles.push_back(tileset_index + multiplier * 256);

				_map_data->GetSelectedTileLayer()->SetTile(x + j, y + i, tileset_index + multiplier * TILESET_NUM_TILES);
			} // iterate through columns of selection
		} // iterate through rows of selection
	} // multiple tiles are selected
	else {
		// put selected tile from tileset into tile array at correct position
		int32 tileset_index = table->currentRow() * TILESET_NUM_COLS + table->currentColumn();

		// perform randomization for autotiles
		// _AutotileRandomize(multiplier, tileset_index);

		// Record information for undo/redo action.
		// TODO: put this data into a LayerCommand object
// 		_tile_indeces.push_back(QPoint(index_x, index_y));
// 		_previous_tiles.push_back(GetCurrentLayer()[index_y][index_x]);
// 		_modified_tiles.push_back(tileset_index + multiplier * 256);

		_map_data->GetSelectedTileLayer()->SetTile(x, y, tileset_index + multiplier * 256);
	} // a single tile is selected
}



void MapView::_DeleteTile(int32 x, int32 y) {
	// TODO: Record information for undo/redo action.
// 	_tile_indeces.push_back(QPoint(x, y));
// 	_previous_tiles.push_back(GetCurrentLayer()[y][x]);
// 	_modified_tiles.push_back(-1);

	// Delete the tile
	_map_data->GetSelectedTileLayer()->SetTile(x, y, NO_TILE);
}



void MapView::_DrawGrid() {
	for (uint32 x = 0; x < (_map_data->GetMapLength() * TILE_LENGTH); x+= TILE_LENGTH) {
		for (uint32 y = 0; y < (_map_data->GetMapHeight() * TILE_HEIGHT); y+= TILE_HEIGHT) {
			addLine(0, y, _map_data->GetMapLength() * TILE_LENGTH, y, QPen(Qt::DotLine));
			addLine(x, 0, x, _map_data->GetMapHeight() * TILE_HEIGHT, QPen(Qt::DotLine));
		}
	}
}

} // namespace hoa_editor
