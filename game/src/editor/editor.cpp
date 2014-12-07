///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    editor.cpp
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for the map editor main window
*** ***************************************************************************/

#include <QGraphicsView>
#include <QScrollBar>
#include <QTableWidgetItem>

#include "script.h"
#include "editor.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_script;

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// Editor class -- public functions
///////////////////////////////////////////////////////////////////////////////

Editor::Editor() :
	QMainWindow(),
	_tiles_toolbar(NULL),
	_horizontal_splitter(NULL),
	_right_vertical_splitter(NULL),
	_map_view(NULL),
	_tileset_tabs(NULL),
	_layer_view(NULL),
	_layer_toolbar(NULL),
	_undo_stack(NULL),
	_error_max_contexts(NULL),
	_file_menu(NULL),
	_view_menu(NULL),
	_tiles_menu(NULL),
	_map_menu(NULL),
	_help_menu(NULL),
	_tileset_menu(NULL),
	_script_menu(NULL),
	_new_action(NULL),
	_open_action(NULL),
	_save_action(NULL),
	_save_as_action(NULL),
	_close_action(NULL),
	_quit_action(NULL),
	_toggle_grid_action(NULL),
	_coord_tile_action(NULL),
	_coord_collision_action(NULL),
	_undo_action(NULL),
	_redo_action(NULL),
	_layer_fill_action(NULL),
	_layer_clear_action(NULL),
	_toggle_select_action(NULL),
	_mode_paint_action(NULL),
	_mode_move_action(NULL),
	_mode_delete_action(NULL),
	_mode_group(NULL),
	_edit_tileset_action(NULL),
	_map_properties_action(NULL),
	_help_action(NULL),
	_about_action(NULL),
	_about_qt_action(NULL)
{
	// Create and initialize singleton objects that the editor code uses
	ScriptManager = ScriptEngine::SingletonCreate();
	ScriptManager->SingletonInitialize();

	// Create actions, menus, and toolbars
	_CreateActions();
	_CreateMenus();
	_CreateToolbars();
	_TilesMenuSetup();

	_undo_stack = new QUndoStack();
	connect(_undo_stack, SIGNAL(canRedoChanged(bool)), _redo_action, SLOT(setEnabled(bool)));
	connect(_undo_stack, SIGNAL(canUndoChanged(bool)), _undo_action, SLOT(setEnabled(bool)));

	// Created the central widget and widget layout with the QSplitter objects
	_horizontal_splitter = new QSplitter(this);
	_horizontal_splitter->setOrientation(Qt::Horizontal);
	_right_vertical_splitter = new QSplitter(_horizontal_splitter);
	_right_vertical_splitter->setOrientation(Qt::Vertical);
	setCentralWidget(_horizontal_splitter);
	resize(800, 600);

	setWindowIcon(QIcon("img/logos/program_icon.ico"));
}



Editor::~Editor() {
	delete _new_action;
	delete _open_action;
	delete _save_action;
	delete _save_as_action;
	delete _close_action;
	delete _quit_action;
	delete _toggle_grid_action;
	delete _undo_action;
	delete _redo_action;
	delete _layer_fill_action;
	delete _layer_clear_action;
	delete _toggle_select_action;
	delete _mode_paint_action;
	delete _mode_move_action;
	delete _mode_delete_action;
	delete _mode_group;
	delete _edit_tileset_action;
	delete _map_properties_action;
	delete _help_action;
	delete _about_action;
	delete _about_qt_action;

	if (_map_view != NULL)
		delete _map_view;
	if (_tileset_tabs != NULL)
		delete _tileset_tabs;
	if (_layer_toolbar)
		delete _layer_toolbar;
	if (_layer_view != NULL)
		delete _layer_view;

	delete _right_vertical_splitter;
	delete _horizontal_splitter;
	delete _undo_stack;

	ScriptEngine::SingletonDestroy();
}

///////////////////////////////////////////////////////////////////////////////
// Editor class -- private functions
///////////////////////////////////////////////////////////////////////////////

void Editor::_CreateActions() {
	// Create actions related to the File menu
	_new_action = new QAction("&New...", this);
	_new_action->setShortcut(tr("Ctrl+N"));
	_new_action->setStatusTip("Create a new map");
	connect(_new_action, SIGNAL(triggered()), this, SLOT(_FileNew()));

	_open_action = new QAction("&Open...", this);
	_open_action->setShortcut(tr("Ctrl+O"));
	_open_action->setStatusTip("Open an existing map file");
	connect(_open_action, SIGNAL(triggered()), this, SLOT(_FileOpen()));

	_save_as_action = new QAction("Save &As...", this);
	_save_as_action->setStatusTip("Save the map as a new file");
	connect(_save_as_action, SIGNAL(triggered()), this, SLOT(_FileSaveAs()));

	_save_action = new QAction("&Save", this);
	_save_action->setShortcut(tr("Ctrl+S"));
	_save_action->setStatusTip("Save map");
	connect(_save_action, SIGNAL(triggered()), this, SLOT(_FileSave()));

	_close_action = new QAction("&Close", this);
	_close_action->setShortcut(tr("Ctrl+W"));
	_close_action->setStatusTip("Close the open map");
	connect(_close_action, SIGNAL(triggered()), this, SLOT(_FileClose()));

	_quit_action = new QAction("&Quit", this);
	_quit_action->setShortcut(tr("Ctrl+Q"));
	_quit_action->setStatusTip("Exits from the editor application");
	connect(_quit_action, SIGNAL(triggered()), this, SLOT(_FileQuit()));

	// Create actions related to the View menu
	_toggle_grid_action = new QAction("&Grid", this);
	_toggle_grid_action->setStatusTip("Toggles display of the tile grid on the map");
	_toggle_grid_action->setShortcut(tr("G"));
	_toggle_grid_action->setCheckable(true);
	connect(_toggle_grid_action, SIGNAL(triggered()), this, SLOT(_ViewToggleGrid()));

	// Create actions related to the Tiles menu
	_undo_action = new QAction(QIcon("img/misc/editor_tools/arrow_left.png"), "&Undo", this);
	_undo_action->setShortcut(tr("Ctrl+Z"));
	_undo_action->setStatusTip("Undoes the previous command");
	connect(_undo_action, SIGNAL(triggered()), _undo_stack, SLOT(undo()));

	_redo_action = new QAction(QIcon("img/misc/editor_tools/arrow_right.png"), "&Redo", this);
	_redo_action->setShortcut(tr("Ctrl+Y"));
	_redo_action->setStatusTip("Redoes the next command");
	connect(_redo_action, SIGNAL(triggered()), _undo_stack, SLOT(redo()));

	_layer_fill_action = new QAction(QIcon("img/misc/editor_tools/fill.png"), "&Fill layer", this);
	_layer_fill_action->setStatusTip("Fills current layer with selected tile");
	connect(_layer_fill_action, SIGNAL(triggered()), this, SLOT(_TileLayerFill()));

	_layer_clear_action = new QAction("&Clear layer", this);
	_layer_clear_action->setStatusTip("Clears current layer from any tiles");
	connect(_layer_clear_action, SIGNAL(triggered()), this, SLOT(_TileLayerClear()));

	_toggle_select_action = new QAction(QIcon("img/misc/editor_tools/selection_rectangle.png"), "Marquee &Select", this);
	_toggle_select_action->setShortcut(tr("Shift+S"));
	_toggle_select_action->setStatusTip("Rectangularly select tiles on the map");
	_toggle_select_action->setCheckable(true);
	connect(_toggle_select_action, SIGNAL(triggered()), this, SLOT(_TileToggleSelect()));

	_mode_paint_action = new QAction(QIcon("img/misc/editor_tools/pencil.png"), "&Paint mode", this);
	_mode_paint_action->setShortcut(tr("Shift+P"));
	_mode_paint_action->setStatusTip("Switches to paint mode to draw tiles on the map");
	_mode_paint_action->setCheckable(true);
	connect(_mode_paint_action, SIGNAL(triggered()), this, SLOT(_TileModePaint()));

	_mode_move_action = new QAction(QIcon("img/misc/editor_tools/arrow.png"), "Mo&ve mode", this);
	_mode_move_action->setShortcut(tr("Shift+V"));
	_mode_move_action->setStatusTip("Switches to move mode to move tiles around on the map");
	_mode_move_action->setCheckable(true);
	connect(_mode_move_action, SIGNAL(triggered()), this, SLOT(_TileModeMove()));

	_mode_delete_action = new QAction(QIcon("img/misc/editor_tools/eraser.png"), "&Delete mode", this);
	_mode_delete_action->setShortcut(tr("Shift+D"));
	_mode_delete_action->setStatusTip("Switches to delete mode to erase tiles from the map");
	_mode_delete_action->setCheckable(true);
	connect(_mode_delete_action, SIGNAL(triggered()), this, SLOT(_TileModeDelete()));

	_mode_group = new QActionGroup(this);
	_mode_group->addAction(_mode_paint_action);
	_mode_group->addAction(_mode_move_action);
	_mode_group->addAction(_mode_delete_action);
	_mode_paint_action->setChecked(true);

	// Create actions related to the Tileset menu
	_edit_tileset_action = new QAction("Edit &Tileset", this);
	_edit_tileset_action->setStatusTip("Edit the properties of the tileset file");
	connect(_edit_tileset_action, SIGNAL(triggered()), this, SLOT(_TilesetEdit()));

	// Create actions related to the Map menu
	_map_properties_action = new QAction("&Properties...", this);
	_map_properties_action->setStatusTip("Modify the properties of the map");
	connect(_map_properties_action, SIGNAL(triggered()), this, SLOT(_MapProperties()));

	// Create actions related to the Help menu
	_help_action = new QAction("&Help", this);
	_help_action->setShortcut(Qt::Key_F1);
	_help_action->setStatusTip("Brings up help documentation for the editor");
	connect(_help_action, SIGNAL(triggered()), this, SLOT(_HelpHelp()));

	_about_action = new QAction("&About", this);
	_about_action->setStatusTip("Brings up information about the editor");
	connect(_about_action, SIGNAL(triggered()), this, SLOT(_HelpAbout()));

	_about_qt_action = new QAction("About &Qt", this);
	_about_qt_action->setStatusTip("Brings up information about Qt");
	connect(_about_qt_action, SIGNAL(triggered()), this, SLOT(_HelpAboutQt()));
} // void Editor::_CreateActions()



void Editor::_CreateMenus() {
	// File menu
	_file_menu = menuBar()->addMenu("&File");
	_file_menu->addAction(_new_action);
	_file_menu->addAction(_open_action);
	_file_menu->addSeparator();
	_file_menu->addAction(_save_action);
	_file_menu->addAction(_save_as_action);
	_file_menu->addSeparator();
	_file_menu->addAction(_close_action);
	_file_menu->addAction(_quit_action);
	connect(_file_menu, SIGNAL(aboutToShow()), this, SLOT(_FileMenuSetup()));

	// View menu
	_view_menu = menuBar()->addMenu("&View");
	_view_menu->addAction(_toggle_grid_action);
	connect(_view_menu, SIGNAL(aboutToShow()), this, SLOT(_ViewMenuSetup()));

	// Tile menu
	_tiles_menu = menuBar()->addMenu("&Tiles");
	_tiles_menu->addAction(_undo_action);
	_tiles_menu->addAction(_redo_action);
	_tiles_menu->addSeparator();
	_tiles_menu->addAction(_layer_fill_action);
	_tiles_menu->addAction(_layer_clear_action);
	_tiles_menu->addSeparator();
	_tiles_menu->addAction(_toggle_select_action);
	_tiles_menu->addSeparator()->setText("Editing Mode");
	_tiles_menu->addAction(_mode_paint_action);
	_tiles_menu->addAction(_mode_move_action);
	_tiles_menu->addAction(_mode_delete_action);
	connect(_tiles_menu, SIGNAL(aboutToShow()), this, SLOT(_TilesMenuSetup()));

	// Tileset menu
	_tileset_menu = menuBar()->addMenu("Tile&set");
	_tileset_menu->addAction(_edit_tileset_action);
	connect(_tileset_menu, SIGNAL(aboutToShow()), this, SLOT(_TilesetMenuSetup()));

	// Map menu
	_map_menu = menuBar()->addMenu("&Map");
	_map_menu->addAction(_map_properties_action);
	connect(_map_menu, SIGNAL(aboutToShow()), this, SLOT(_MapMenuSetup()));

	// Help menu
	_help_menu = menuBar()->addMenu("&Help");
	_help_menu->addAction(_help_action);
	_help_menu->addAction(_about_action);
	_help_menu->addAction(_about_qt_action);
} // void Editor::_CreateMenus()



void Editor::_CreateToolbars() {
	_tiles_toolbar = addToolBar("Tiles");
	_tiles_toolbar->addAction(_layer_fill_action);
	_tiles_toolbar->addSeparator();
	_tiles_toolbar->addAction(_mode_paint_action);
	_tiles_toolbar->addAction(_mode_move_action);
	_tiles_toolbar->addAction(_mode_delete_action);
	_tiles_toolbar->addSeparator();
	_tiles_toolbar->addAction(_undo_action);
	_tiles_toolbar->addAction(_redo_action);
	_tiles_toolbar->addSeparator();
	_tiles_toolbar->addAction(_toggle_select_action);
}



void Editor::_SetupMainView() {
    // Can't be initialized if there is no map widget ready
    if (_map_view == NULL)
        return;

	// Create the tileset selection tab widget
    if (_tileset_tabs != NULL)
        delete _tileset_tabs;
    _tileset_tabs = new QTabWidget();
    _tileset_tabs->setTabPosition(QTabWidget::South);

	// Create the tile layer selection tree widget
    if (_layer_view != NULL)
        delete _layer_view;
    _layer_view = new QTreeWidget();

    connect(_layer_view, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(_UpdateSelectedLayer(QTreeWidgetItem*)));
    _layer_view->setColumnCount(3);

    QStringList layer_headers;
    layer_headers << "ID" << " " << "Layer" << "Collisions";
    _layer_view->setHeaderLabels(layer_headers);
    // Hide the ID column as we only use it internally
    _layer_view->setColumnHidden(0, true);

	// Add all tile layers from the map data
	QIcon icon(QString("img/misc/editor_tools/eye.png"));
	QStringList layer_names = _map_data.GetTileLayerNames();
	vector<TileLayerProperties>& layer_properties = _map_data.GetTileLayerProperties();
	for (uint32 i = 0; i < layer_properties.size(); ++i) {
		QTreeWidgetItem* layer_item = new QTreeWidgetItem(_layer_view);
		layer_item->setText(0, QString::number(i));
		// When a map is initially created or loaded, all layers are set to visible
		layer_item->setIcon(1, icon);
		layer_item->setText(2, layer_properties[i].GetName());
		layer_item->setText(3, layer_properties[i].IsCollisionEnabled() ? QString("Enabled") : QString("Disabled"));
	}

    // Create and add all buttons for the layer editing toolbar
    if (_layer_toolbar != NULL)
        delete _layer_toolbar;
    _layer_toolbar = new QToolBar("Layers", _right_vertical_splitter);

    _layer_new_button = new QPushButton(QIcon(QString("img/misc/editor_tools/new.png")), QString(), _layer_toolbar);
    _layer_new_button->setContentsMargins(1, 1, 1, 1);
    _layer_new_button->setFixedSize(20, 20);
    _layer_new_button->setToolTip(tr("Add Layer"));
    connect(_layer_new_button, SIGNAL(clicked(bool)), this, SLOT(_AddLayer()));
    _layer_toolbar->addWidget(_layer_new_button);

    _layer_rename_button = new QPushButton(QIcon(QString("img/misc/editor_tools/rename.png")), QString(), _layer_toolbar);
    _layer_rename_button->setContentsMargins(1, 1, 1, 1);
    _layer_rename_button->setFixedSize(20, 20);
    _layer_rename_button->setToolTip(tr("Rename Layer"));
    _layer_toolbar->addWidget(_layer_rename_button);
    _layer_rename_button->setDisabled(true);

    _layer_delete_button = new QPushButton(QIcon(QString("img/misc/editor_tools/delete.png")), QString(), _layer_toolbar);
    _layer_delete_button->setContentsMargins(1, 1, 1, 1);
    _layer_delete_button->setFixedSize(20, 20);
    _layer_delete_button->setToolTip(tr("Delete Layer"));
    connect(_layer_delete_button, SIGNAL(clicked(bool)), this, SLOT(_DeleteLayer()));
    _layer_toolbar->addWidget(_layer_delete_button);

    _layer_up_button = new QPushButton(QIcon(QString("img/misc/editor_tools/move_up.png")), QString(), _layer_toolbar);
    _layer_up_button->setContentsMargins(1, 1, 1, 1);
    _layer_up_button->setFixedSize(20, 20);
    _layer_up_button->setToolTip(tr("Move Layer Up"));
    _layer_toolbar->addWidget(_layer_up_button);
    connect(_layer_up_button, SIGNAL(clicked(bool)), this, SLOT(_MoveLayerUp()));

    _layer_down_button = new QPushButton(QIcon(QString("img/misc/editor_tools/move_down.png")), QString(), _layer_toolbar);
    _layer_down_button->setContentsMargins(1, 1, 1, 1);
    _layer_down_button->setFixedSize(20, 20);
    _layer_down_button->setToolTip(tr("Move Layer Down"));
    _layer_toolbar->addWidget(_layer_down_button);
    connect(_layer_down_button, SIGNAL(clicked(bool)), this, SLOT(_MoveLayerDown()));

    _layer_visible_button = new QPushButton(QIcon(QString("img/misc/editor_tools/eye.png")), QString(), _layer_toolbar);
    _layer_visible_button->setContentsMargins(1, 1, 1, 1);
    _layer_visible_button->setFixedSize(20, 20);
    _layer_visible_button->setToolTip(tr("Toggle Layer Visibility"));
    _layer_toolbar->addWidget(_layer_visible_button);
    connect(_layer_visible_button, SIGNAL(clicked(bool)), this, SLOT(_ToggleLayerVisibility()));

    // Setup widgets on the left side of the screen
    _horizontal_splitter->addWidget(_map_view->GetGraphicsView());

    // Setup widgets on the right side of the screen
    _right_vertical_splitter->addWidget(_layer_view);
    _right_vertical_splitter->addWidget(_layer_toolbar);
    _right_vertical_splitter->addWidget(_tileset_tabs);

    _horizontal_splitter->addWidget(_right_vertical_splitter);
}



bool Editor::_UnsavedDataPrompt() {
	if (_map_data.IsInitialized() == false)
		return true;

	if (_map_data.IsMapModified() == false)
		return true;

	switch (QMessageBox::warning(this, "Unsaved File", "The document contains unsaved changes.\n"
		"Do you want to save these changes before proceeding?", "&Save", "&Discard", "Cancel", 0, 2))
	{
		case 0: // Save clicked -or- Alt+S pressed -or- Enter pressed
			_FileSave();
			break;
		case 1: // Discard clicked -or- Alt+D pressed
			break;
		default: // Cancel clicked -or- Escape pressed
			statusBar()->showMessage("Save abandoned", 5000);
			return false;
	}

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Editor class -- private slot functions
///////////////////////////////////////////////////////////////////////////////

void Editor::_FileMenuSetup() {
	if (_map_data.IsInitialized() == true) {
		_save_action->setEnabled(_map_data.IsMapModified());
		_save_as_action->setEnabled(true);
		_close_action->setEnabled(true);
	}
	else {
		_save_action->setEnabled(false);
		_save_as_action->setEnabled(false);
		_close_action->setEnabled(false);
	}
}



void Editor::_ViewMenuSetup() {
	if (_map_data.IsInitialized() == true) {
		_toggle_grid_action->setEnabled(true);
	}
	else {
		_toggle_grid_action->setEnabled(false);
	}
}



void Editor::_TilesMenuSetup() {
	if (_map_data.IsInitialized() == true) {
		_undo_action->setText("Undo " + _undo_stack->undoText());
		_redo_action->setText("Redo " + _undo_stack->redoText());
		_layer_fill_action->setEnabled(true);
		_layer_clear_action->setEnabled(true);
		_toggle_select_action->setEnabled(true);
		_mode_paint_action->setEnabled(true);
		_mode_move_action->setEnabled(true);
		_mode_delete_action->setEnabled(true);
	}
	else {
		_undo_action->setEnabled(false);
		_redo_action->setEnabled(false);
		_layer_fill_action->setEnabled(false);
		_layer_clear_action->setEnabled(false);
		_toggle_select_action->setEnabled(false);
		_mode_paint_action->setEnabled(false);
		_mode_move_action->setEnabled(false);
		_mode_delete_action->setEnabled(false);
	}
}



void Editor::_TilesetMenuSetup() {
	// TODO: Currently tilesets can only be edited when no map is open. This is done because if the tileset data is modified,
	// the editor may be using a stale tileset definition when saving a map. In the future, tilesets should always be able
	// to be edited, and any modified tilesets in use by the map should be reloaded after the tileset file is saved.

	if (_map_data.IsInitialized() == true)
		_edit_tileset_action->setEnabled(false);
	else
		_edit_tileset_action->setEnabled(true);
}



void Editor::_MapMenuSetup() {
	if (_map_data.IsInitialized() == true) {
		_map_properties_action->setEnabled(true);
	}
	else {
		_map_properties_action->setEnabled(false);
	}
}



void Editor::_FileNew() {
	if (_UnsavedDataPrompt() == false) {
		statusBar()->showMessage("New operation cancelled due to existing unsaved map data.", 5000);
		return;
	}

	// ---------- 1) Prompt the user with the dialog for them to enter the map properties
	MapPropertiesDialog* new_dialog = new MapPropertiesDialog(this, "new_dialog", false);
	if (new_dialog->exec() != QDialog::Accepted) {
		delete new_dialog;
		statusBar()->showMessage("New operation cancelled", 5000);
		return;
	}

	// ---------- 2) Initialize the map data and map view widget
	_map_data.DestroyData();
	_map_data.CreateData(new_dialog->GetLength(), new_dialog->GetHeight());

	if (_map_view)
		delete _map_view;
	_map_view = new MapView(_horizontal_splitter, &_map_data);

	_SetupMainView();

	// ---------- 3) Determine the number of tilesets that will be used by the new map and create a load progress dialog
	QTreeWidget* tilesets = new_dialog->GetTilesetTree();
	int32 num_tileset_items = tilesets->topLevelItemCount();
	int32 num_checked_items = 0;
	for (int32 i = 0; i < num_tileset_items; ++i) {
		if (tilesets->topLevelItem(i)->checkState(0) == Qt::Checked)
			num_checked_items++;
	}

	// Used to show the progress of tilesets that have been loaded.
	QProgressDialog* load_tileset_progress = new QProgressDialog("Loading tilesets...", NULL, 0, num_checked_items, this,
		Qt::Widget | Qt::FramelessWindowHint | Qt::WindowTitleHint);
	load_tileset_progress->setWindowTitle("Creating Map...");

	// Set the location of the progress dialog and show it
	load_tileset_progress->move(this->pos().x() + this->width() / 2  - load_tileset_progress->width() / 2,
		this->pos().y() + this->height() / 2 - load_tileset_progress->height() / 2);
	load_tileset_progress->show();

	// ---------- 4) Load each tileset object
	num_checked_items = 0;
	for (int32 i = 0; i < num_tileset_items; i++) {
		if (tilesets->topLevelItem(i)->checkState(0) != Qt::Checked) {
			continue;
		}


		// Increment the progress dialog counter
		load_tileset_progress->setValue(num_checked_items++);

		// Load the tileset data from the definition file, add it to the map data, and create the TilesetTable to display it
		Tileset* tileset = new Tileset();
		QString filename = QString("lua/data/tilesets/") + tilesets->topLevelItem(i)->text(0) + (".lua");
		if (tileset->Load(filename) == false) {
			QMessageBox::critical(this, APP_NAME, "Failed to load tileset: " + filename);
			delete tileset;
		}

		if (_map_data.AddTileset(tileset) == false) {
			QMessageBox::critical(this, APP_NAME, "Failed to add tileset to map data: " + _map_data.GetErrorMessage());
			delete tileset;
		}

		TilesetTable* tileset_table = new TilesetTable(tileset);
		_tileset_tabs->addTab(tileset_table, tilesets->topLevelItem(i)->text(0));
	}

	// ---------- 5) Set the sizes of the splitters
    QList<int> sizes;
    sizes << 600 << 200;
    _horizontal_splitter->setSizes(sizes);

    sizes.clear();
    sizes << 150 << 50 << 400;
    _right_vertical_splitter->setSizes(sizes);

    _horizontal_splitter->show();
	_right_vertical_splitter->show();

	_map_view->SetGridVisible(false);
	_map_view->SetSelectionVisible(false);

	// Enable appropriate menu actions
	_TilesMenuSetup();

	_undo_stack->setClean();

	// Hide and delete progress bar
	load_tileset_progress->hide();
	delete load_tileset_progress;

	statusBar()->showMessage("New map created", 5000);

	delete new_dialog;
	_map_view->DrawMap();
} // void Editor::_FileNew()



void Editor::_FileOpen() {
	if (_UnsavedDataPrompt() == false) {
		statusBar()->showMessage("New operation cancelled due to existing unsaved map data.", 5000);
		return;
	}

	// ---------- 1) Attempt to open the file that the user requested
	QString filename = QFileDialog::getOpenFileName(this, APP_NAME + " -- Open Map File",
		"lua/data/maps", "Maps (*.lua)");
	if (filename.isEmpty() == true) {
		statusBar()->showMessage("No map file was opened (empty filename)", 5000);
		return;
	}

	// ---------- 2) Clear out any existing map data
	_map_data.DestroyData();
	if (_map_view != NULL) {
		delete _map_view;
		_map_view = NULL;
	}


	for (uint32 i = 0; i < static_cast<uint32>(_tileset_tabs->count()); ++i) {
		delete _tileset_tabs->widget(i);
	}
	_tileset_tabs->clear();

	// TODO: delete context view data

	// ---------- 3) Load the map data and setup the tileset tabs
	if (_map_data.LoadData(filename) == false) {
		// TODO: report error message
		return;
	}

	_map_view = new MapView(_horizontal_splitter, &_map_data);
	_SetupMainView();

	vector<Tileset*> tilesets = _map_data.GetTilesets();
	QStringList tileset_names = _map_data.GetTilesetNames();
	for (uint32 i = 0; i < tilesets.size(); ++i) {
		_tileset_tabs->addTab(new TilesetTable(tilesets[i]), tileset_names[i]);
	}
	// _UpdateLayersView();

	// TODO: recreate the context view data here

	_horizontal_splitter->addWidget(_map_view->GetGraphicsView());
	_horizontal_splitter->show();

	_toggle_select_action->setChecked(false);
	_toggle_grid_action->setChecked(false);

	// Enable appropriate user actions
	_TilesMenuSetup();

	// Set default edit mode
	_map_view->SetEditMode(PAINT_TILE);

	_undo_stack->setClean();
	statusBar()->showMessage(QString("Opened map \'%1\'").arg(_map_data.GetMapFilename()), 5000);
} // void Editor::_FileOpen()



void Editor::_FileSave() {
	if (_map_data.IsInitialized() == false) {
		return;
	}

	if (_map_data.SaveData() == false) {
		return;
	}

	_undo_stack->setClean();
	setWindowTitle(QString("%1").arg(_map_data.GetMapFilename()));
	statusBar()->showMessage(QString(tr("Saved \'%1\' successfully!")).arg(_map_data.GetMapFilename()), 5000);
}



void Editor::_FileSaveAs() {
	// Get the file name to save to from the user
	QString filename = QFileDialog::getSaveFileName(this, "Allacrost Map Editor -- File Save", "lua/data/maps", "Maps (*.lua)");

	if (filename.isEmpty() == true) {
		statusBar()->showMessage("Save abandoned.", 5000);
		return;
	}

	if (_map_data.IsInitialized() == false) {
		return;
	}

	if (_map_data.SaveData(filename) == false) {
		return;
	}

	_undo_stack->setClean();
	setWindowTitle(QString("%1").arg(_map_data.GetMapFilename()));
	statusBar()->showMessage(QString(tr("Saved \'%1\' successfully!")).arg(_map_data.GetMapFilename()), 5000);
}



void Editor::_FileClose() {
	if (_UnsavedDataPrompt() == false) {
		return;
	}

	// Clear all existing map data
	_map_data.DestroyData();
	_undo_stack->setClean();

	// TODO: delete/update view objects appropriately

	setWindowTitle("Hero of Allacrost Map Editor");
}



void Editor::_FileQuit() {
	if (_UnsavedDataPrompt() == true)
		qApp->exit(0);
}



void Editor::_ViewToggleGrid() {
	if (_map_view == NULL)
		return;

	bool grid_active = _map_view->ToggleGridVisible();
	_toggle_grid_action->setChecked(grid_active);
}



void Editor::_TileLayerFill() {
	// TODO: fetch the currently selected tile/tileset and fill the entire layer with that tile for the active tile context

	// TODO: perform autotile randomization for the tile when available

	// TODO: record map data for undo/redo stack

	// Draw the changes.
	_map_data.SetMapModified(true);
	_map_view->DrawMap();
}



void Editor::_TileLayerClear() {
	// TODO: fetch the current tile layer and clear it only for the active tile context

	// TODO: record map data for undo/redo stack

	// Draw the changes.
	_map_data.SetMapModified(true);
	_map_view->DrawMap();
}



void Editor::_TileToggleSelect() {
	if (_map_view == NULL)
		return;

	bool selection = _map_view->ToggleSelectionVisible();
	_toggle_select_action->setChecked(selection);
}



void Editor::_TileModePaint() {
	if (_map_view == NULL)
		return;

	_map_view->ClearSelectionLayer();
	_map_view->SetEditMode(PAINT_TILE);
}



void Editor::_TileModeMove() {
	if (_map_view == NULL)
		return;

	_map_view->ClearSelectionLayer();
	_map_view->SetEditMode(MOVE_TILE);
}



void Editor::_TileModeDelete() {
	if (_map_view == NULL)
		return;

	_map_view->ClearSelectionLayer();
	_map_view->SetEditMode(DELETE_TILE);
}



void Editor::_TilesetEdit() {
	TilesetEditor* tileset_editor = new TilesetEditor(this);
	tileset_editor->exec();
	delete tileset_editor;
}



void Editor::_MapProperties() {
	MapPropertiesDialog* props = new MapPropertiesDialog(this, "map_properties", true);

	if (props->exec() != QDialog::Accepted) {
		statusBar()->showMessage("Map properties were not modified", 5000);
		delete props;
		return;
	}

	// TODO: adjust size of map appropriately

	// TODO: add or remove tilesets

	delete props;
}



void Editor::_HelpHelp() {
	statusBar()->showMessage(tr("See http://allacrost.sourceforge.net/wiki/index.php/Code_Documentation#Map_Editor_Documentation for more details"), 10000);
}



void Editor::_HelpAbout() {
    QMessageBox::about(this, "Hero of Allacrost Map Editor -- About",
		"<center><h1><font color=blue>Hero of Allacrost Level Editor<font></h1></center>"
		"<center><h2><font color=blue>Copyright (c) 2004-2015<font></h2></center>"
		"<p>A map editor created for the Hero of Allacrost project."
		" See 'http://www.allacrost.org/' for more details</p>");
}



void Editor::_HelpAboutQt() {
    QMessageBox::aboutQt(this, "Hero of Allacrost Map Editor -- About Qt");
}



void Editor::_UpdateSelectedLayer(QTreeWidgetItem* item) {
	if (item == NULL)
		return;

	// Retrieve the ID of the layer that was just selected
// 	uint32 layer_id = item->text(0).toUInt();

	// TODO: set the layer_id somewhere so it can be known what layer is currently being edited

	// TODO: enable layer buttons appropriately based on the layer's properties
// 	_layer_up_button->setEnabled(_CanLayerMoveUp(item));
// 	_layer_down_button->setEnabled(_CanLayerMoveDown(item));
// 	_delete_layer_button->setEnabled(_CanDeleteLayer(item));
}

///////////////////////////////////////////////////////////////////////////////
// LayerCommand class -- public functions
///////////////////////////////////////////////////////////////////////////////

LayerCommand::LayerCommand(vector<int32> indeces, vector<int32> previous, vector<int32> modified,
		int context, Editor* editor, const QString& text, QUndoCommand* parent) :
	QUndoCommand(text, parent),
	_tile_indeces(indeces),
	_previous_tiles(previous),
	_modified_tiles(modified),
	_context(context),
	_editor(editor)
{}



void LayerCommand::undo() {
// 	for (int32 i = 0; i < static_cast<int32>(_tile_indeces.size()); i++) {
// 		_map_view->GetLayer(_context)[_tile_indeces[i]] = _previous_tiles[i];
// 	}
//
// 	_map_view->updateScene();
}



void LayerCommand::redo() {
// 	for (int32 i = 0; i < static_cast<int32>(_tile_indeces.size()); i++) {
// 		_map_view->GetLayer(_context)[_tile_indeces[i]] = _modified_tiles[i];
// 	}
// 	_map_view->updateScene();
}

} // namespace hoa_editor
