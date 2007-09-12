///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    editor.cpp
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Source file for editor's main window and user interface.
 *****************************************************************************/

#include "editor.h"

using namespace hoa_script;
using namespace hoa_utils;
using namespace hoa_editor;
using namespace hoa_video;
using namespace std;

Editor::Editor() : QMainWindow()
{
	// create actions, menus, and toolbars
	_CreateActions();
	_CreateMenus();
	_CreateToolbars();
	_TilesEnableActions();

	// initialize viewing items
	_grid_on = false;
	_ll_on = false;
	_ml_on = false;
	_ul_on = false;
	
	// create the main widget and layout
	_ed_splitter = new QSplitter(this);
	_ed_splitter->setOrientation(Qt::Vertical);
	_ed_scrollview = NULL;
	_ed_tabs = NULL;
	setCentralWidget(_ed_splitter);
	resize(600, 400);

	// set the window icon
	setWindowIcon(QIcon("img/logos/program_icon.bmp"));
} // Editor constructor

Editor::~Editor()
{
	if (_ed_scrollview != NULL)
		delete _ed_scrollview;
	if (_ed_tabs != NULL)
		delete _ed_tabs;
	delete _ed_splitter;
} // Editor destructor



// ********** Protected function **********

void Editor::closeEvent(QCloseEvent*)
{
    _FileQuit();
} // closeEvent(...)



// ********** Private slots **********

void Editor::_FileMenuSetup()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_save_as_action->setEnabled(true);
		_save_action->setEnabled(_ed_scrollview->_map->GetChanged());
	} // map must exist in order to save or resize it
	else
	{
		_save_as_action->setEnabled(false);
		_save_action->setEnabled(false);
	} // map does not exist, can't save or resize it*/
} // _FileMenuSetup()

void Editor::_ViewMenuSetup()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_toggle_grid_action->setEnabled(true);
		_toggle_ll_action->setEnabled(true);
		_toggle_ml_action->setEnabled(true);
		_toggle_ul_action->setEnabled(true);
	} // map must exist in order to set view options
	else
	{
		_toggle_grid_action->setEnabled(false);
		_toggle_ll_action->setEnabled(false);
		_toggle_ml_action->setEnabled(false);
		_toggle_ul_action->setEnabled(false);
	} // map does not exist, can't view it*/
} // _ViewMenuSetup()

void Editor::_TilesEnableActions()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_layer_fill_action->setEnabled(true);
		_layer_clear_action->setEnabled(true);
		_mode_paint_action->setEnabled(true);
		_mode_move_action->setEnabled(true);
		_mode_delete_action->setEnabled(true);
		_edit_ll_action->setEnabled(true);
		_edit_ml_action->setEnabled(true);
		_edit_ul_action->setEnabled(true);
	} // map must exist in order to paint it
	else
	{
		_layer_fill_action->setEnabled(false);
		_layer_clear_action->setEnabled(false);
		_mode_paint_action->setEnabled(false);
		_mode_move_action->setEnabled(false);
		_mode_delete_action->setEnabled(false);
		_edit_ll_action->setEnabled(false);
		_edit_ml_action->setEnabled(false);
		_edit_ul_action->setEnabled(false);
	} // map does not exist, can't paint it*/
} // _TilesEnableActions()

void Editor::_MapMenuSetup()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_select_music_action->setEnabled(true);
		_map_properties_action->setEnabled(true);
	} // map must exist in order to set music
	else
	{
		_select_music_action->setEnabled(false);
		_map_properties_action->setEnabled(false);
	} // map does not exist, can't modify it*/
} // _MapMenuSetup()

void Editor::_FileNew()
{
	if (_EraseOK())
	{
		MapPropertiesDialog* new_map = new MapPropertiesDialog(this, "new_map", false);

		if (new_map->exec() == QDialog::Accepted)
		{
			if (_ed_scrollview != NULL)
				delete _ed_scrollview;
			_ed_scrollview = new EditorScrollView(NULL, "map", new_map->GetWidth(), new_map->GetHeight());

			if (_ed_tabs != NULL)
				delete _ed_tabs;
			_ed_tabs = new QTabWidget();
			_ed_tabs->setTabPosition(QTabWidget::South);

			_ed_splitter->addWidget(_ed_scrollview);
			_ed_splitter->addWidget(_ed_tabs);

			QTreeWidget* tilesets = new_map->GetTilesetTree();
			int num_items = tilesets->topLevelItemCount();
			for (int i = 0; i < num_items; i++)
			{
				if (tilesets->topLevelItem(i)->checkState(0) == Qt::Checked)
				{
					Tileset* a_tileset = new Tileset(this, tilesets->topLevelItem(i)->text(0));
					_ed_tabs->addTab(a_tileset->table, tilesets->topLevelItem(i)->text(0));
					_ed_scrollview->_map->tilesets.push_back(a_tileset);
				} // tileset must be checked
			} // iterate through all possible tilesets

			_ed_scrollview->resize(new_map->GetWidth() * TILE_WIDTH, new_map->GetHeight() * TILE_HEIGHT);
			_ed_splitter->show();

			_grid_on = false;
			_ll_on   = false;
			_ml_on   = false;
			_ul_on   = false;
			_ViewToggleGrid();
			_ViewToggleLL();
			_ViewToggleML();
			_ViewToggleUL();
			
			// Enable appropriate actions
			_TilesEnableActions();

			// Set default edit mode
			_ed_scrollview->_layer_edit = LOWER_LAYER;
			_ed_scrollview->_tile_mode  = PAINT_TILE;
			
			statusBar()->showMessage("New map created", 5000);
		} // only if the user pressed OK
		else
			statusBar()->showMessage("No map created!", 5000);

		delete new_map;
	} // make sure an unsaved map is not lost
} // _FileNew()

void Editor::_FileOpen()
{
	if (_EraseOK())
	{
		// file to open
		QString file_name = QFileDialog::getOpenFileName(this, "HoA Level Editor -- File Open",
			"dat/maps", "Maps (*.lua)");

		if (!file_name.isEmpty())
		{
			if (_ed_scrollview != NULL)
				delete _ed_scrollview;
			_ed_scrollview = new EditorScrollView(NULL, "map", 0, 0);

			if (_ed_tabs != NULL)
				delete _ed_tabs;
			_ed_tabs = new QTabWidget();
			_ed_tabs->setTabPosition(QTabWidget::South);

			_ed_splitter->addWidget(_ed_scrollview);
			_ed_splitter->addWidget(_ed_tabs);

			_ed_scrollview->_map->SetFileName(file_name);
			_ed_scrollview->_map->LoadMap();

			for (QStringList::ConstIterator it = _ed_scrollview->_map->tileset_names.begin();
				it != _ed_scrollview->_map->tileset_names.end(); it++)
			{
				Tileset* a_tileset = new Tileset(this, *it);
				_ed_tabs->addTab(a_tileset->table, *it);
				_ed_scrollview->_map->tilesets.push_back(a_tileset);
			} // iterate through all tilesets in the map

			_ed_scrollview->resize(_ed_scrollview->_map->GetWidth(), _ed_scrollview->_map->GetHeight());
			_ed_splitter->show();

			_grid_on = false;
			_ll_on   = false;
			_ml_on   = false;
			_ul_on   = false;
			_ViewToggleGrid();
			_ViewToggleLL();
			_ViewToggleML();
			_ViewToggleUL();

			// Enable appropriate actions
			_TilesEnableActions();

			// Set default edit mode
			_ed_scrollview->_layer_edit = LOWER_LAYER;
			_ed_scrollview->_tile_mode  = PAINT_TILE;

			statusBar()->showMessage(QString("Opened \'%1\'").arg(_ed_scrollview->_map->GetFileName()), 5000);
		} // file must exist in order to open it
	} // make sure an unsaved map is not lost
} // _FileOpen()

void Editor::_FileSaveAs()
{
	// get the file name from the user
	QString file_name = QFileDialog::getSaveFileName(this, "HoA Level Editor -- File Save",
		"dat/maps", "Maps (*.lua)");
		
	if (!file_name.isEmpty())
	{
		int answer = 0;		// button pressed by user
		
		// ask to overwrite existing file
		if (QFile::exists(file_name))
			answer = QMessageBox::warning(this, "Overwrite File",
				QString("Overwrite\n\'%1\'?" ).arg(file_name),
				"&Yes", "&No", QString::null, 1, 1);
				
		if (answer == 0)
		{
			_ed_scrollview->_map->SetFileName(file_name);
			_FileSave();
			return;
		} // save the file
    } // make sure the file name is not blank
	
    statusBar()->showMessage("Save abandoned.", 5000);
} // _FileSaveAs()

void Editor::_FileSave()
{
	if (_ed_scrollview->_map->GetFileName().isEmpty() ||
		_ed_scrollview->_map->GetFileName() == "Untitled")
	{
		_FileSaveAs();
		return;
	} // gets a file name if it is blank

	_ed_scrollview->_map->SaveMap();      // actually saves the map
	setCaption(QString("%1").arg(_ed_scrollview->_map->GetFileName()));
	statusBar()->showMessage(QString("Saved \'%1\' successfully!").
		arg(_ed_scrollview->_map->GetFileName()), 5000);
} // _FileSave()

void Editor::_FileQuit()
{
	// Checks to see if the map is unsaved.
	if (_EraseOK())
		qApp->exit(0);
} // _FileQuit()

void Editor::_ViewToggleGrid()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_grid_on = !_grid_on;
		_toggle_grid_action->setChecked(_grid_on);
		_ed_scrollview->_map->SetGridOn(_grid_on);
	} // map must exist in order to view things on it
} // _ViewToggleGrid()

void Editor::_ViewToggleLL()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_ll_on = !_ll_on;
		_toggle_ll_action->setChecked(_ll_on);
		_ed_scrollview->_map->SetLLOn(_ll_on);
	} // map must exist in order to view things on it
} // _ViewToggleLL()

void Editor::_ViewToggleML()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_ml_on = !_ml_on;
		_toggle_ml_action->setChecked(_ml_on);
		_ed_scrollview->_map->SetMLOn(_ml_on);
	} // map must exist in order to view things on it
} // _ViewToggleML()

void Editor::_ViewToggleUL()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_ul_on = !_ul_on;
		_toggle_ul_action->setChecked(_ul_on);
		_ed_scrollview->_map->SetULOn(_ul_on);
	} // map must exist in order to view things on it
} // _ViewToggleUL()

void Editor::_TileLayerFill()
{
	// get reference to current tileset
	Q3Table* table = static_cast<Q3Table*> (_ed_tabs->currentPage());

	// put selected tile from tileset into tile array at correct position
	int tileset_index = table->currentRow() * 16 + table->currentColumn();
	int multiplier = _ed_scrollview->_map->tileset_names.findIndex(_ed_tabs->tabText(_ed_tabs->currentIndex()));
	if (multiplier == -1)
	{
		_ed_scrollview->_map->tileset_names.append(_ed_tabs->tabText(_ed_tabs->currentIndex()));
		multiplier = _ed_scrollview->_map->tileset_names.findIndex(_ed_tabs->tabText(_ed_tabs->currentIndex()));
	} // calculate index of current tileset

	vector<int32>::iterator it;    // used to iterate over an entire layer
	vector<int32>& CurrentLayer = _ed_scrollview->GetCurrentLayer();
	for (it = CurrentLayer.begin(); it != CurrentLayer.end(); it++)
		*it = tileset_index + multiplier * 256;

	// Draw the changes
	_ed_scrollview->_map->updateGL();
} // _TileLayerFill()

void Editor::_TileLayerClear()
{
	vector<int32>::iterator it;    // used to iterate over an entire layer
	vector<int32>& CurrentLayer = _ed_scrollview->GetCurrentLayer();
	for (it = CurrentLayer.begin(); it != CurrentLayer.end(); it++)
		*it = -1;

	// Draw the changes
	_ed_scrollview->_map->updateGL();
} // _TileLayerClear()

void Editor::_TileModePaint()
{
	if (_ed_scrollview != NULL)
		_ed_scrollview->_tile_mode = PAINT_TILE;
} // _TileModePaint()

void Editor::_TileModeMove()
{
	if (_ed_scrollview != NULL)
		_ed_scrollview->_tile_mode = MOVE_TILE;
} // _TileModeMove()

void Editor::_TileModeDelete()
{
	if (_ed_scrollview != NULL)
		_ed_scrollview->_tile_mode = DELETE_TILE;
} // _TileModeDelete()

void Editor::_TileEditLL()
{
	if (_ed_scrollview != NULL)
		_ed_scrollview->_layer_edit = LOWER_LAYER;
} // _TileEditLL()

void Editor::_TileEditML()
{
	if (_ed_scrollview != NULL)
		_ed_scrollview->_layer_edit = MIDDLE_LAYER;
} // _TileEditML()

void Editor::_TileEditUL()
{
	if (_ed_scrollview != NULL)
		_ed_scrollview->_layer_edit = UPPER_LAYER;
} // _TileEditUL()

void Editor::_MapSelectMusic()
{
	if(_ed_scrollview == NULL)
		return;

	MusicDialog* music = new MusicDialog(this, "music_dialog", _ed_scrollview->_map->GetMusic());
	if(music->exec() == QDialog::Accepted) {
		_ed_scrollview->_map->SetMusic(music->GetSelectedFile());
		_ed_scrollview->_map->SetChanged(true);
	}
	delete music;
} // _MapSelectMusic()

void Editor::_MapProperties()
{
	MapPropertiesDialog* props = new MapPropertiesDialog(this, "map_properties", true);
	
	if (props->exec() == QDialog::Accepted)
	{
		if (_ed_scrollview->_map->GetWidth() < props->GetWidth())
		{
			// User wants to make map wider so we must insert columns of tiles at the edge of the map.
			
			int map_width     = _ed_scrollview->_map->GetWidth();
			int map_height    = _ed_scrollview->_map->GetHeight();
			int extra_columns = props->GetWidth() - map_width;
			
			// Add in the extra columns one by one.
			for (int col = extra_columns; col > 0; col--)
			{
				vector<int32>& lower_layer = _ed_scrollview->_map->GetLayer(LOWER_LAYER);
				vector<int32>::iterator it = lower_layer.begin() + map_width;
				for (int row = 0; row < map_height; row++)
				{
					lower_layer.insert(it, -1);
					it += map_width + 1;
				} // iterate through the rows of the lower layer
	
				vector<int32>& middle_layer = _ed_scrollview->_map->GetLayer(MIDDLE_LAYER);
				it = middle_layer.begin() + map_width;
				for (int row = 0; row < map_height; row++)
				{
					middle_layer.insert(it, -1);
					it += map_width + 1;
				} // iterate through the rows of the middle layer
	
				vector<int32>& upper_layer = _ed_scrollview->_map->GetLayer(UPPER_LAYER);
				it = upper_layer.begin() + map_width;
				for (int row = 0; row < map_height; row++)
				{
					upper_layer.insert(it, -1);
					it += map_width + 1;
				} // iterate through the rows of the upper layer

				map_width++;
				_ed_scrollview->_map->SetWidth(map_width);
			} // add in all the extra columns
		} // insert columns
		else if (_ed_scrollview->_map->GetWidth() > props->GetWidth())
		{
			// User wants to make map less wide so we must delete columns of tiles from the edge of the map.
			
			int map_width     = _ed_scrollview->_map->GetWidth();
			int map_height    = _ed_scrollview->_map->GetHeight();
			int extra_columns = map_width - props->GetWidth();
			
			// Delete all the extra columns one by one.
			for (int col = extra_columns; col > 0; col--)
			{
				vector<int32>& lower_layer = _ed_scrollview->_map->GetLayer(LOWER_LAYER);
				vector<int32>::iterator it = lower_layer.begin() + map_width;
				for (int row = 0; row < map_height; row++)
				{
					lower_layer.erase(it);
					it += map_width - 1;
				} // iterate through the rows of the lower layer
	
				vector<int32>& middle_layer = _ed_scrollview->_map->GetLayer(MIDDLE_LAYER);
				it = middle_layer.begin() + map_width;
				for (int row = 0; row < map_height; row++)
				{
					middle_layer.erase(it);
					it += map_width - 1;
				} // iterate through the rows of the middle layer
	
				vector<int32>& upper_layer = _ed_scrollview->_map->GetLayer(UPPER_LAYER);
				it = upper_layer.begin() + map_width;
				for (int row = 0; row < map_height; row++)
				{
					upper_layer.erase(it);
					it += map_width - 1;
				} // iterate through the rows of the upper layer

				map_width--;
				_ed_scrollview->_map->SetWidth(map_width);
			} // delete all the extra columns
		} // delete columns
		
		if (_ed_scrollview->_map->GetHeight() < props->GetHeight())
		{
			// User wants to make map taller so we must insert rows of tiles at the edge of the map.

			int map_width = _ed_scrollview->_map->GetWidth();
			int extra_rows = props->GetHeight() - _ed_scrollview->_map->GetHeight();
	
			vector<int32>& lower_layer  = _ed_scrollview->_map->GetLayer(LOWER_LAYER);
			vector<int32>& middle_layer = _ed_scrollview->_map->GetLayer(MIDDLE_LAYER);
			vector<int32>& upper_layer  = _ed_scrollview->_map->GetLayer(UPPER_LAYER);
			lower_layer.insert( lower_layer.end(),  extra_rows * map_width, -1);
			middle_layer.insert(middle_layer.end(), extra_rows * map_width, -1);
			upper_layer.insert( upper_layer.end(),  extra_rows * map_width, -1);
		} // add rows
		else if (_ed_scrollview->_map->GetHeight() > props->GetHeight())
		{
			// User wants to make map less tall so we must delete rows of tiles from the edge of the map.
			
			int map_width  = _ed_scrollview->_map->GetWidth();
			int extra_rows = _ed_scrollview->_map->GetHeight() - props->GetHeight();
	
			vector<int32>& lower_layer  = _ed_scrollview->_map->GetLayer(LOWER_LAYER);
			vector<int32>& middle_layer = _ed_scrollview->_map->GetLayer(MIDDLE_LAYER);
			vector<int32>& upper_layer  = _ed_scrollview->_map->GetLayer(UPPER_LAYER);
			lower_layer.erase( lower_layer.end()  - extra_rows * map_width, lower_layer.end());
			middle_layer.erase(middle_layer.end() - extra_rows * map_width, middle_layer.end());
			upper_layer.erase( upper_layer.end()  - extra_rows * map_width, upper_layer.end());
	
		} // delete rows

		// Resize the map, QOpenGL and QScrollView widgets.
		_ed_scrollview->_map->SetHeight(props->GetHeight());
		_ed_scrollview->_map->resize(props->GetWidth() * TILE_WIDTH, props->GetHeight() * TILE_HEIGHT);
		_ed_scrollview->resize(props->GetWidth() * TILE_WIDTH, props->GetHeight() * TILE_HEIGHT);



		// User has the ability to add or remove tilesets being used. We don't want
		// to reload tilesets that have already been loaded before.

		QTreeWidget* tilesets = props->GetTilesetTree();

		// Put the names of the tabs into a nice list that can be easily searched
		// with one command instead of a loop.
		QStringList tab_names;
		for (int i = 0; i < _ed_tabs->count(); i++)
			tab_names << _ed_tabs->tabText(i);

		// Go through the list of tilesets, adding selected tilesets and removing
		// any unwanted tilesets.
		int num_items = tilesets->topLevelItemCount();
		for (int i = 0; i < num_items; i++)
		{
			if (tilesets->topLevelItem(i)->checkState(0) == Qt::Checked)
			{
				if (tab_names.contains(tilesets->topLevelItem(i)->text(0)) == false)
				{
					Tileset* a_tileset = new Tileset(this, tilesets->topLevelItem(i)->text(0));
					_ed_tabs->addTab(a_tileset->table, tilesets->topLevelItem(i)->text(0));
					_ed_scrollview->_map->tilesets.push_back(a_tileset);
				} // only add a tileset if it isn't already loaded
			} // tileset must be checked in order to add it
			else if (tilesets->topLevelItem(i)->checkState(0) == Qt::Unchecked &&
			         tab_names.contains(tilesets->topLevelItem(i)->text(0)))
				_ed_tabs->removeTab(tab_names.indexOf(tilesets->topLevelItem(i)->text(0)));
				// FIXME:
				// Where to add and remove tileset name from the tilesets list
				// in the _map? Do it here or when actually painting and deleting
				// tiles? Here the assumption is made that if the user is adding a
				// tileset, then s/he expects to use tiles from that tileset and we
				// can safely add the tileset name to the _map. Otherwise we would
				// have to constantly check every time a paint operation occurs
				// whether or not the tileset name of the selected tile was present
				// in the tileset name list in _map. That's cumbersome.
				//
				// When removing a tileset however, there might still be tiles in
				// the map from that tileset, and the user is only removing the
				// tileset from the view in the bottom of the map to unclutter
				// things. In this case we wouldn't want to remove the tileset name
				// from the list in _map.
		} // iterate through all possible tilesets

		_ed_splitter->addWidget(_ed_tabs);
	} // only if the user pressed OK
	else
		statusBar()->showMessage("Properties not modified!", 5000);

	delete props;
} // _MapProperties()

void Editor::_HelpHelp()
{
	statusBar()->showMessage(tr("See http://allacrost.sourceforge.net/wiki/index.php/Code_Documentation#Map_Editor_Documentation for more details"), 10000);
} // _HelpHelp()

void Editor::_HelpAbout()
{
    QMessageBox::about(this, "HoA Level Editor -- About",
		"<center><h1><font color=blue>Hero of Allacrost Level Editor<font>"
		"</h1></center>"
		"<center><h2><font color=blue>Copyright (c) 2004-2007<font></h2></center>"
		"<p>A level editor created for the Hero of Allacrost project."
		" See 'http://www.allacrost.org/' for more details</p>");
} // _HelpAbout()

void Editor::_HelpAboutQt()
{
    QMessageBox::aboutQt(this, "HoA Level Editor -- About Qt");
} // _HelpAboutQt()



// ********** Private functions **********

void Editor::_CreateActions()
{
	// Create menu actions related to the File menu
	
	_new_action = new QAction("&New...", this);
	_new_action->setShortcut(tr("Ctrl+N"));
	_new_action->setStatusTip("Create a new map");
	connect(_new_action, SIGNAL(triggered()), this, SLOT(_FileNew()));
	
	_open_action = new QAction("&Open...", this);
	_open_action->setShortcut(tr("Ctrl+O"));
	_open_action->setStatusTip("Open an existing map");
	connect(_open_action, SIGNAL(triggered()), this, SLOT(_FileOpen()));
	
	_save_as_action = new QAction("&Save As...", this);
	_save_as_action->setStatusTip("Save the map with another name");
	connect(_save_as_action, SIGNAL(triggered()), this, SLOT(_FileSaveAs()));
	
	_save_action = new QAction("&Save", this);
	_save_action->setShortcut(tr("Ctrl+S"));
	_save_action->setStatusTip("Save the map");
	connect(_save_action, SIGNAL(triggered()), this, SLOT(_FileSave()));
	
	_quit_action = new QAction("&Quit", this);
	_quit_action->setShortcut(tr("Ctrl+Q"));
	_quit_action->setStatusTip("Quits from the editor");
	//_quit_action->setMenuRole(QAction::QuitRole);
	connect(_quit_action, SIGNAL(triggered()), this, SLOT(_FileQuit()));



	// Create menu actions related to the View menu

	_toggle_grid_action = new QAction("&Grid", this);
	_toggle_grid_action->setStatusTip("Switches the grid on and off");
	_toggle_grid_action->setCheckable(true);
	connect(_toggle_grid_action, SIGNAL(triggered()), this, SLOT(_ViewToggleGrid()));
	
	_toggle_ll_action = new QAction("&Lower Layer", this);
	_toggle_ll_action->setStatusTip("Switches the lower layer on and off");
	_toggle_ll_action->setCheckable(true);
	connect(_toggle_ll_action, SIGNAL(triggered()), this, SLOT(_ViewToggleLL()));
	
	_toggle_ml_action = new QAction("&Middle Layer", this);
	_toggle_ml_action->setStatusTip("Switches the middle layer on and off");
	_toggle_ml_action->setCheckable(true);
	connect(_toggle_ml_action, SIGNAL(triggered()), this, SLOT(_ViewToggleML()));
	
	_toggle_ul_action = new QAction("&Upper Layer", this);
	_toggle_ul_action->setStatusTip("Switches the upper layer on and off");
	_toggle_ul_action->setCheckable(true);
	connect(_toggle_ul_action, SIGNAL(triggered()), this, SLOT(_ViewToggleUL()));



	// Create menu actions related to the Tiles menu

	_layer_fill_action = new QAction(
		QIcon("img/misc/editor-tools/stock-tool-bucket-fill-22.png"),
		"&Fill layer", this);
	_layer_fill_action->setStatusTip("Fills current layer with selected tile");
	connect(_layer_fill_action, SIGNAL(triggered()), this, SLOT(_TileLayerFill()));

	_layer_clear_action = new QAction("&Clear layer", this);
	_layer_clear_action->setStatusTip("Clears current layer from any tiles");
	connect(_layer_clear_action, SIGNAL(triggered()), this, SLOT(_TileLayerClear()));
	
	_mode_paint_action = new QAction(
		QIcon("img/misc/editor-tools/stock-tool-pencil-22.png"),
		"&Paint mode", this);
	_mode_paint_action->setStatusTip("Switches to paint mode to draw tiles on the map");
	_mode_paint_action->setCheckable(true);
	connect(_mode_paint_action, SIGNAL(triggered()), this, SLOT(_TileModePaint()));

	_mode_move_action = new QAction("&Move mode", this);
	_mode_move_action->setStatusTip("Switches to move mode to move tiles around on the map");
	_mode_move_action->setCheckable(true);
	connect(_mode_move_action, SIGNAL(triggered()), this, SLOT(_TileModeMove()));

	_mode_delete_action = new QAction(
		QIcon("img/misc/editor-tools/stock-tool-eraser-22.png"),
		"&Delete mode", this);
	_mode_delete_action->setStatusTip("Switches to delete mode to erase tiles from the map");
	_mode_delete_action->setCheckable(true);
	connect(_mode_delete_action, SIGNAL(triggered()), this, SLOT(_TileModeDelete()));
	
	_mode_group = new QActionGroup(this);
	_mode_group->addAction(_mode_paint_action);
	_mode_group->addAction(_mode_move_action);
	_mode_group->addAction(_mode_delete_action);
	_mode_paint_action->setChecked(true);
	
	_edit_ll_action = new QAction("Edit &lower layer", this);
	_edit_ll_action->setStatusTip("Makes lower layer of the map current");
	_edit_ll_action->setCheckable(true);
	connect(_edit_ll_action, SIGNAL(triggered()), this, SLOT(_TileEditLL()));
	
	_edit_ml_action = new QAction("Edit &middle layer", this);
	_edit_ml_action->setStatusTip("Makes middle layer of the map current");
	_edit_ml_action->setCheckable(true);
	connect(_edit_ml_action, SIGNAL(triggered()), this, SLOT(_TileEditML()));
	
	_edit_ul_action = new QAction("Edit &upper layer", this);
	_edit_ul_action->setStatusTip("Makes upper layer of the map current");
	_edit_ul_action->setCheckable(true);
	connect(_edit_ul_action, SIGNAL(triggered()), this, SLOT(_TileEditUL()));

	_edit_group = new QActionGroup(this);
	_edit_group->addAction(_edit_ll_action);
	_edit_group->addAction(_edit_ml_action);
	_edit_group->addAction(_edit_ul_action);
	_edit_ll_action->setChecked(true);
	
	

	// Create menu actions related to the Map menu

	_select_music_action = new QAction("&Select map music...", this);
	_select_music_action->setStatusTip("Choose background music for the map");
	connect(_select_music_action, SIGNAL(triggered()), this, SLOT(_MapSelectMusic()));
	
	_map_properties_action = new QAction("&Properties...", this);
	_map_properties_action->setStatusTip("Modify the properties of the map");
	connect(_map_properties_action, SIGNAL(triggered()), this, SLOT(_MapProperties()));



	// Create menu actions related to the Help menu

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
} // _CreateActions()

void Editor::_CreateMenus()
{
	// file menu creation
	_file_menu = menuBar()->addMenu("&File");
	_file_menu->addAction(_new_action);
	_file_menu->addAction(_open_action);
	_file_menu->addSeparator();
	_file_menu->addAction(_save_action);
	_file_menu->addAction(_save_as_action);
	_file_menu->addSeparator();
	_file_menu->addAction(_quit_action);
	connect(_file_menu, SIGNAL(aboutToShow()), this, SLOT(_FileMenuSetup()));

	// view menu creation
	_view_menu = menuBar()->addMenu("&View");
	_view_menu->addAction(_toggle_grid_action);
	_view_menu->addSeparator();
	_view_menu->addAction(_toggle_ll_action);
	_view_menu->addAction(_toggle_ml_action);
	_view_menu->addAction(_toggle_ul_action);
	_view_menu->setCheckable(true);
	_view_menu->setTearOffEnabled(true);
	connect(_view_menu, SIGNAL(aboutToShow()), this, SLOT(_ViewMenuSetup()));

	// tile menu creation
	_tiles_menu = menuBar()->addMenu("&Tiles");
	_tiles_menu->addAction(_layer_fill_action);
	_tiles_menu->addAction(_layer_clear_action);
	_tiles_menu->addSeparator()->setText("Editing Mode");
	_tiles_menu->addAction(_mode_paint_action);
	_tiles_menu->addAction(_mode_move_action);
	_tiles_menu->addAction(_mode_delete_action);
	_tiles_menu->addSeparator()->setText("Current Layer");
	_tiles_menu->addAction(_edit_ll_action);
	_tiles_menu->addAction(_edit_ml_action);
	_tiles_menu->addAction(_edit_ul_action);
	_tiles_menu->setTearOffEnabled(true);
	connect(_tiles_menu, SIGNAL(aboutToShow()), this, SLOT(_TilesEnableActions()));
	
	// map menu creation
	_map_menu = menuBar()->addMenu("&Map");
	_map_menu->addAction(_select_music_action);
	_map_menu->addAction(_map_properties_action);
	connect(_map_menu, SIGNAL(aboutToShow()), this, SLOT(_MapMenuSetup()));

	// help menu creation
	_help_menu = menuBar()->addMenu("&Help");
	_help_menu->addAction(_help_action);
	_help_menu->addAction(_about_action);
	_help_menu->addAction(_about_qt_action);
} // _CreateMenus()

void Editor::_CreateToolbars()
{
	_tiles_toolbar = addToolBar("Tiles");
	_tiles_toolbar->addAction(_layer_fill_action);
	_tiles_toolbar->addSeparator();
	_tiles_toolbar->addAction(_mode_paint_action);
	_tiles_toolbar->addAction(_mode_delete_action);
} // _CreateToolbars()

bool Editor::_EraseOK()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
	    if (_ed_scrollview->_map->GetChanged())
		{
			switch(QMessageBox::warning(this, "Unsaved File", "The document contains unsaved changes\n"
				"Do you want to save the changes before proceeding?", "&Save", "&Discard", "Cancel",
				0,		// Enter == button 0
        		2))		// Escape == button 2
			{
    			case 0: // Save clicked or Alt+S pressed or Enter pressed.
        			// save and exit
					_FileSave();
					break;
				case 1: // Discard clicked or Alt+D pressed
					// don't save but exit
					break;
				default: // Cancel clicked or Escape pressed
    	    		// don't exit
					statusBar()->showMessage("Save abandoned", 5000);
    	    		return false;
	    	} // warn the user to save
	    } // map has been modified
	} // map must exist first

    return true;
} // _EraseOK()



/************************
  MapPropertiesDialog class functions follow
************************/

MapPropertiesDialog::MapPropertiesDialog(QWidget* parent, const QString& name, bool prop)
	: QDialog(parent, (const char*) name)
{
	setCaption("Map Properties...");

	// Set up the height spinbox
	_height_label  = new QLabel("Height (in tiles):", this);
	_height_sbox   = new QSpinBox(this);
	_height_sbox->setMinimum(24);
	_height_sbox->setMaximum(1000);
	
	// Set up the width spinbox
	_width_label   = new QLabel(" Width (in tiles):", this);
	_width_sbox    = new QSpinBox(this);
	_width_sbox->setMinimum(32);
	_width_sbox->setMaximum(1000);
	
	// Set up the cancel and okay push buttons
	_cancel_pbut   = new QPushButton("Cancel", this);
	_ok_pbut       = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);
	// TODO: connect the OK push button to a signal and slot such that it is
	// disabled if no tilesets are selected
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	// Set up the list of selectable tilesets
	QDir tileset_dir("img/tilesets");
	_tileset_tree = new QTreeWidget(this);
	_tileset_tree->setColumnCount(1);
	_tileset_tree->setHeaderLabels(QStringList("Tilesets"));
	QList<QTreeWidgetItem*> tilesets;
	// Start the loop at 2 to skip over . (present working directory) and .. (parent directory).
	for (uint32 i = 2; i < tileset_dir.count(); i++)
	{
		tilesets.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(tileset_dir[i].remove(".png"))));
		tilesets.back()->setCheckState(0, Qt::Unchecked);  // enables checkboxes
		if (prop)
		{
			// get reference to the Editor
			Editor* editor = static_cast<Editor*> (parent);

			// Iterate through the names of the tabs to see which ones in the list are already present and set
			// their checkbox appropriately.
			for (int j = 0; j < editor->_ed_tabs->count(); j++)
				if (tilesets.back()->text(0) == editor->_ed_tabs->tabText(j))
				{
					tilesets.back()->setCheckState(0, Qt::Checked);
					break;
				} // the two tileset names must match in order to set the checkbox
		} // user wants to edit the map's properties
	} // loop through all files in the tileset directory
	_tileset_tree->insertTopLevelItems(0, tilesets);

	if (prop)
	{
		// get reference to the Editor
		Editor* editor = static_cast<Editor*> (parent);

		_height_sbox->setValue(editor->_ed_scrollview->_map->GetHeight());
		_width_sbox->setValue(editor->_ed_scrollview->_map->GetWidth());
	} // user wants to edit the map's properties
	
	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	_dia_layout->addWidget(_height_label, 0, 0);
	_dia_layout->addWidget(_height_sbox, 1, 0);
	_dia_layout->addWidget(_width_label, 2, 0);
	_dia_layout->addWidget(_width_sbox, 3, 0);
	_dia_layout->addWidget(_tileset_tree, 0, 1, 5, -1);
	_dia_layout->addWidget(_cancel_pbut, 6, 1);
	_dia_layout->addWidget(_ok_pbut, 6, 0);
} // MapPropertiesDialog constructor

MapPropertiesDialog::~MapPropertiesDialog()
{
	delete _height_label;
	delete _height_sbox;
	delete _width_label;
	delete _width_sbox;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _tileset_tree;
	delete _dia_layout;
} // MapPropertiesDialog destructor



/************************
  MusicDialog class functions follow
************************/

MusicDialog::MusicDialog(QWidget* parent, const QString& name, const QString& selected_music)
	: QDialog(parent, name)
{
	setCaption("Select map music");
	_dia_layout   = new QGridLayout(this);

	_cancel_pbut  = new QPushButton("Cancel", this);
	_ok_pbut      = new QPushButton("OK", this);
	_select_label = new QLabel("Select the music for this map:",this);
	_music_list   = new QTreeWidget(this);

	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	_dia_layout->addWidget(_select_label, 0, 0);
	_dia_layout->addWidget(_music_list, 1, 0);
	_dia_layout->addWidget(_ok_pbut, 2, 0);
	_dia_layout->addWidget(_cancel_pbut, 2, 1);

	_PopulateMusicList(selected_music);
} // MusicDialog::MusicDialog

MusicDialog::~MusicDialog()
{
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _select_label;
	delete _music_list;
	delete _dia_layout;
} // MusicDialog::~MusicDialog

void MusicDialog::_PopulateMusicList(const QString& selected_str)
{
	QDir music_dir("mus");
	_music_list->setColumnCount(1);
	_music_list->setHeaderLabels(QStringList("Filename"));
	
	// Add music files
	QList<QTreeWidgetItem*> music;
	music.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList("None")));
	for (unsigned int i = 0; i < music_dir.count(); i++) 
	{
		if (music_dir[i].contains(".ogg"))
		{
			music.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(music_dir[i])));
			if (selected_str.endsWith(music_dir[i]) && !selected_str.isEmpty())
				music.back()->setSelected(true);
		} // only look for .ogg files
	} // iterate through all files in the music directory

	if (selected_str.isEmpty() || selected_str == "None")
		music.first()->setSelected(true);

	_music_list->insertTopLevelItems(0, music);
} // MusicDialog::_PopulateMusicList

QString MusicDialog::GetSelectedFile()
{
	if (_music_list->currentItem() == 0)
		return QString("None");

	return QString("mus/" + _music_list->currentItem()->text(0));
} // MusicDialog::GetSelectedFile



/************************
  EditorScrollView class functions follow
************************/

EditorScrollView::EditorScrollView(QWidget* parent, const QString& name, int width, int height)
	: Q3ScrollView(parent, (const char*) name, Qt::WNoAutoErase|Qt::WStaticContents)
{
	// Set default editing modes.
	_tile_mode  = PAINT_TILE;
	_layer_edit = LOWER_LAYER;
	
	// Create a new map.
	_map = new Grid(viewport(), "Untitled", width, height);
	addChild(_map);

	// Create menu actions related to the Context menu.
	_insert_row_action = new QAction("Insert row", this);
	_insert_row_action->setStatusTip("Inserts a row of empty tiles on all layers above the currently selected tile");
	connect(_insert_row_action, SIGNAL(triggered()), this, SLOT(_ContextInsertRow()));
	_insert_column_action = new QAction("Insert column", this);
	_insert_column_action->setStatusTip("Inserts a column of empty tiles on all layers to the left of the currently selected tile");
	connect(_insert_column_action, SIGNAL(triggered()), this, SLOT(_ContextInsertColumn()));
	_delete_row_action = new QAction("Delete row", this);
	_delete_row_action->setStatusTip("Deletes the currently selected row of tiles from all layers");
	connect(_delete_row_action, SIGNAL(triggered()), this, SLOT(_ContextDeleteRow()));
	_delete_column_action = new QAction("Delete column", this);
	_delete_column_action->setStatusTip("Deletes the currently selected column of tiles from all layers");
	connect(_delete_column_action, SIGNAL(triggered()), this, SLOT(_ContextDeleteColumn()));
	
	// Context menu creation.
	_context_menu = new QMenu(this);
	_context_menu->addAction(_insert_row_action);
	_context_menu->addAction(_insert_column_action);
	_context_menu->addSeparator();
	_context_menu->addAction(_delete_row_action);
	_context_menu->addAction(_delete_column_action);
} // EditorScrollView constructor

EditorScrollView::~EditorScrollView()
{
	delete _map;
	delete _context_menu;
} // EditorScrollView destructor

void EditorScrollView::Resize(int width, int height)
{
	_map->resize(width * TILE_WIDTH, height * TILE_HEIGHT);
	_map->SetHeight(height);
	_map->SetWidth(width);
} // Resize(...)

std::vector<int32>& EditorScrollView::GetCurrentLayer()
{
	return _map->GetLayer(_layer_edit);
}

// ********** Protected slots **********

void EditorScrollView::contentsMousePressEvent(QMouseEvent* evt)
{
	// don't draw outside the map
	if ((evt->y() / TILE_HEIGHT) >= _map->GetHeight() ||
		(evt->x() / TILE_WIDTH)  >= _map->GetWidth())
		return;

	_tile_index = evt->y() / TILE_HEIGHT * _map->GetWidth() + evt->x() / TILE_WIDTH;
	_map->SetChanged(true);

	switch (_tile_mode)
	{
		case PAINT_TILE: // start painting tiles
		{
			if (evt->button() == Qt::LeftButton)
			{
				// get reference to current tileset
				Editor* editor = static_cast<Editor*> (topLevelWidget());
				Q3Table* table = static_cast<Q3Table*> (editor->_ed_tabs->currentPage());
				QString tileset_name = editor->_ed_tabs->tabText(editor->_ed_tabs->currentIndex());

				// put selected tile from tileset into tile array at correct position
				int tileset_index = table->currentRow() * 16 + table->currentColumn();
				int multiplier = _map->tileset_names.findIndex(tileset_name);
				if (multiplier == -1)
				{
					_map->tileset_names.append(tileset_name);
					multiplier = _map->tileset_names.findIndex(tileset_name);
				} // calculate index of current tileset
				
				if (_map->tilesets[multiplier]->walkability[tileset_index][0] != -1)
					GetCurrentLayer()[_tile_index] = tileset_index + multiplier * 256;
			} // left mouse button was pressed
			break;
		} // edit mode PAINT_TILE

		case MOVE_TILE: // start moving a tile
		{
			_move_source_index = _tile_index;
			break;
		} // edit mode MOVE_TILE

		case DELETE_TILE: // start deleting tiles
		{
			if (evt->button() == Qt::LeftButton)
			{
				// NOTE: Is file_index going to be used?? If not, no reason for this call
				//int file_index = GetCurrentLayer()[_tile_index];

				// delete the tile
				GetCurrentLayer()[_tile_index] = -1;

				// FIXME: No longer needed
				//_RemoveIfUnused(file_index);
			} // left mouse button was pressed
			break;
		} // edit mode DELETE_TILE

		default:
			QMessageBox::warning(this, "Tile editing mode",
				"ERROR: Invalid tile editing mode!");
	} // switch on tile editing mode

	// Draw the changes
	_map->updateGL();
} // contentsMousePressEvent(...)

void EditorScrollView::contentsMouseMoveEvent(QMouseEvent *evt)
{
	// don't draw outside the map
	if ((evt->y() / TILE_HEIGHT) >= _map->GetHeight() ||
		(evt->x() / TILE_WIDTH)  >= _map->GetWidth())
		return;

	int index = evt->y() / TILE_HEIGHT * _map->GetWidth() + evt->x() / TILE_WIDTH;
	if (index != _tile_index)
	{
		_tile_index = index;
		switch (_tile_mode)
		{
			case PAINT_TILE: // continue painting tiles
			{
				if (evt->state() == Qt::LeftButton)
				{
					// get reference to current tileset
					Editor* editor = static_cast<Editor*> (topLevelWidget());
					Q3Table* table = static_cast<Q3Table*> (editor->_ed_tabs->currentPage());
					QString tileset_name = editor->_ed_tabs->tabText(editor->_ed_tabs->currentIndex());

					// put selected tile from tileset into tile array at correct position
					int tileset_index = table->currentRow() * 16 + table->currentColumn();
					int multiplier = _map->tileset_names.findIndex(tileset_name);
					if (multiplier == -1)
					{
						_map->tileset_names.append(tileset_name);
						multiplier = _map->tileset_names.findIndex(tileset_name);
					} // calculate index of current tileset

					if (_map->tilesets[multiplier]->walkability[tileset_index][0] != -1)
						GetCurrentLayer()[_tile_index] = tileset_index + multiplier * 256;
				} // left mouse button was pressed
				break;
			} // edit mode PAINT_TILE

			case MOVE_TILE: // continue moving a tile
			{
				break;
			} // edit mode MOVE_TILE

			case DELETE_TILE: // continue deleting tiles
			{
				if (evt->state() == Qt::LeftButton)
				{
					// NOTE: file_index is not being used here...
					//int file_index = GetCurrentLayer()[_tile_index];

					// delete the tile
					GetCurrentLayer()[_tile_index] = -1;

					// FIXME: _RemoveIfUnused(file_index);
				} // left mouse button was pressed
				break;
			} // edit mode DELETE_TILE

			default:
				QMessageBox::warning(this, "Tile editing mode",
					"ERROR: Invalid tile editing mode!");
		} // switch on tile editing mode
	} // mouse has moved to a new tile position

	// Draw the changes
	_map->updateGL();
} // contentsMouseMoveEvent(...)

void EditorScrollView::contentsMouseReleaseEvent(QMouseEvent *evt)
{
	// Should already be finished painting or deleting tiles.

	_tile_index = evt->y() / TILE_HEIGHT * _map->GetWidth() + evt->x() / TILE_WIDTH;

	if (_tile_mode == MOVE_TILE)
	{
		std::vector<int32>& layer = GetCurrentLayer();
		layer[_tile_index] = layer[_move_source_index];
		layer[_move_source_index] = -1;
	} // finish moving a tile
	else if (_tile_mode == INVALID_TILE)
		QMessageBox::warning(this, "Tile editing mode",
			"ERROR: Invalid tile editing mode!");

	// Draw the changes
	_map->updateGL();
} // contentsMouseReleaseEvent(...)

void EditorScrollView::contentsContextMenuEvent(QContextMenuEvent *evt)
{
	// Don't popup a menu outside the map.
	if ((evt->y() / TILE_HEIGHT) >= _map->GetHeight() ||
		(evt->x() / TILE_WIDTH)  >= _map->GetWidth())
		return;

	_tile_index = evt->y() / TILE_HEIGHT * _map->GetWidth() + evt->x() / TILE_WIDTH;
	_context_menu->exec(QCursor::pos());
	(static_cast<Editor*> (topLevelWidget()))->statusBar()->clear();
} // contentsContextMenuEvent(...)



// ********** Private slots **********

void EditorScrollView::_ContextInsertRow()
{
	int map_width = _map->GetWidth();
	int row = _tile_index / map_width;
	
	vector<int32>& lower_layer = _map->GetLayer(LOWER_LAYER);
	lower_layer.insert(lower_layer.begin() + row * map_width, map_width, -1);
	vector<int32>& middle_layer = _map->GetLayer(MIDDLE_LAYER);
	middle_layer.insert(middle_layer.begin() + row * map_width, map_width, -1);
	vector<int32>& upper_layer = _map->GetLayer(UPPER_LAYER);
	upper_layer.insert(upper_layer.begin() + row * map_width, map_width, -1);
	
	Resize(map_width, _map->GetHeight() + 1);
} // _ContextInsertRow()

void EditorScrollView::_ContextInsertColumn()
{
	int map_width = _map->GetWidth();
	int map_height = _map->GetHeight();
	int col = _tile_index % map_width;
	
	vector<int32>& lower_layer = _map->GetLayer(LOWER_LAYER);
	vector<int32>::iterator it = lower_layer.begin() + col;
	for (int row = 0; row < map_height; row++)
	{
		lower_layer.insert(it, -1);
		it += map_width + 1;
	} // iterate through the rows of the lower layer
	
	vector<int32>& middle_layer = _map->GetLayer(MIDDLE_LAYER);
	it = middle_layer.begin() + col;
	for (int row = 0; row < map_height; row++)
	{
		middle_layer.insert(it, -1);
		it += map_width + 1;
	} // iterate through the rows of the middle layer
	
	vector<int32>& upper_layer = _map->GetLayer(UPPER_LAYER);
	it = upper_layer.begin() + col;
	for (int row = 0; row < map_height; row++)
	{
		upper_layer.insert(it, -1);
		it += map_width + 1;
	} // iterate through the rows of the upper layer

	Resize(map_width + 1, map_height);
} // _ContextInsertColumn()

void EditorScrollView::_ContextDeleteRow()
{
	int map_width = _map->GetWidth();
	int row = _tile_index / map_width;
	
	vector<int32>& lower_layer = _map->GetLayer(LOWER_LAYER);
	lower_layer.erase(lower_layer.begin() + row * map_width, lower_layer.begin() + row * map_width + map_width);
	vector<int32>& middle_layer = _map->GetLayer(MIDDLE_LAYER);
	middle_layer.erase(middle_layer.begin() + row * map_width, middle_layer.begin() + row * map_width + map_width);
	vector<int32>& upper_layer = _map->GetLayer(UPPER_LAYER);
	upper_layer.erase(upper_layer.begin() + row * map_width, upper_layer.begin() + row * map_width + map_width);
	
	Resize(map_width, _map->GetHeight() - 1);
} // _ContextDeleteRow()

void EditorScrollView::_ContextDeleteColumn()
{
	int map_width = _map->GetWidth();
	int map_height = _map->GetHeight();
	int col = _tile_index % map_width;
	
	vector<int32>& lower_layer = _map->GetLayer(LOWER_LAYER);
	vector<int32>::iterator it = lower_layer.begin() + col;
	for (int row = 0; row < map_height; row++)
	{
		lower_layer.erase(it);
		it += map_width - 1;
	} // iterate through the rows of the lower layer
	
	vector<int32>& middle_layer = _map->GetLayer(MIDDLE_LAYER);
	it = middle_layer.begin() + col;
	for (int row = 0; row < map_height; row++)
	{
		middle_layer.erase(it);
		it += map_width - 1;
	} // iterate through the rows of the middle layer
	
	vector<int32>& upper_layer = _map->GetLayer(UPPER_LAYER);
	it = upper_layer.begin() + col;
	for (int row = 0; row < map_height; row++)
	{
		upper_layer.erase(it);
		it += map_width - 1;
	} // iterate through the rows of the upper layer

	Resize(map_width - 1, map_height);
} // _ContextDeleteColumn()
