///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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

using namespace hoa_data;
using namespace hoa_utils;
using namespace hoa_editor;
using namespace hoa_video;
using namespace std;

Editor::Editor() : QMainWindow(0, 0, WDestructiveClose)
{
	// create the statusbar
	_stat_bar = new QStatusBar(this);
	
	// file menu creation
	_file_menu = new QPopupMenu(this);
	connect(_file_menu, SIGNAL(aboutToShow()), this, SLOT(_FileMenuSetup()));
	menuBar()->insertItem("&File", _file_menu);

	// view menu creation
	_view_menu = new QPopupMenu(this);
	menuBar()->insertItem("&View", _view_menu);
	_grid_id = _view_menu->insertItem("&Grid", this, SLOT(_ViewToggleGrid()));
	_view_menu->insertSeparator();
	_ll_id = _view_menu->insertItem("&Lower Tile Layer", this, SLOT(_ViewToggleLL()));
	_ml_id = _view_menu->insertItem("&Middle Tile Layer", this, SLOT(_ViewToggleML()));
	_ul_id = _view_menu->insertItem("&Upper Tile Layer", this, SLOT(_ViewToggleUL()));
	_view_menu->setCheckable(true);

	// tile menu creation
	_tiles_menu = new QPopupMenu(this);
	menuBar()->insertItem("&Tiles", _tiles_menu);
	_tiles_menu->insertItem("&Fill current layer", this, SLOT(_TileLayerFill()));
	_tiles_menu->insertItem("&Clear current layer", this, SLOT(_TileLayerClear()));
	_tiles_menu->insertSeparator();
	// store id's for setting and removing checks
	int paint_mode_id = _tiles_menu->insertItem("&Paint mode", this, SLOT(_TileModePaint()));
	_mode_ids.insert(std::pair<TILE_MODE_TYPE,int>(PAINT_TILE,paint_mode_id));
	int move_mode_id = _tiles_menu->insertItem("&Move mode", this, SLOT(_TileModeMove()));
	_mode_ids.insert(std::pair<TILE_MODE_TYPE,int>(MOVE_TILE,move_mode_id));
	int delete_mode_id = _tiles_menu->insertItem("&Delete mode", this, SLOT(_TileModeDelete()));
	_mode_ids.insert(std::pair<TILE_MODE_TYPE,int>(DELETE_TILE,delete_mode_id));
	_tiles_menu->insertSeparator();
	int edit_ll_id = _tiles_menu->insertItem("Edit &lower layer", this, SLOT(_TileEditLL()));
	_layer_ids.insert(std::pair<LAYER_TYPE,int>(LOWER_LAYER,edit_ll_id));
	int edit_ml_id = _tiles_menu->insertItem("Edit &middle layer", this, SLOT(_TileEditML()));
	_layer_ids.insert(std::pair<LAYER_TYPE,int>(MIDDLE_LAYER,edit_ml_id));
	int edit_ul_id = _tiles_menu->insertItem("Edit &upper layer", this, SLOT(_TileEditUL()));
	_layer_ids.insert(std::pair<LAYER_TYPE,int>(UPPER_LAYER,edit_ul_id));
	_tiles_menu->insertSeparator();
	_tiles_menu->insertItem("&Manage database...", this, SLOT(_TileDatabase()), CTRL+Key_D);
	
	// help menu creation
	_help_menu = new QPopupMenu(this);
	menuBar()->insertSeparator();
	menuBar()->insertItem("&Help", _help_menu);
	_help_menu->insertItem("&Help", this, SLOT(_HelpHelp()), Key_F1);
	_help_menu->insertItem("&About", this, SLOT(_HelpAbout()));
	_help_menu->insertItem("About &Qt", this, SLOT(_HelpAboutQt()));

	// initialize viewing items
	_grid_on = true;
	_ll_on = true;
	_ml_on = false;
	_ul_on = false;
	_view_menu->setItemChecked(_grid_id, _grid_on);
	_view_menu->setItemChecked(_ll_id,   _ll_on);
	_view_menu->setItemChecked(_ml_id,   _ml_on);
	_view_menu->setItemChecked(_ul_id,   _ul_on);

	// create the main widget and layout
	_ed_widget = new QWidget(this);
	_ed_layout = new QVBoxLayout(_ed_widget);
	_ed_scrollview = NULL;
	_ed_tabs = NULL;
	setCentralWidget(_ed_widget);
	resize(600, 400);
} // Editor constructor

Editor::~Editor()
{
//	if (_tiles != NULL)
//		delete _tiles;
	if (_ed_scrollview != NULL)
		delete _ed_scrollview;
	if (_ed_tabs != NULL)
		delete _ed_tabs;
	delete _ed_layout;
	delete _ed_widget;
} // Editor destructor



// ********** Protected function **********

void Editor::closeEvent(QCloseEvent*)
{
    _FileQuit();
} // closeEvent(...)



// ********** Private slots **********

void Editor::_FileMenuSetup()
{
	_file_menu->clear();
	_file_menu->insertItem("&New...", this, SLOT(_FileNew()), CTRL+Key_N);
	_file_menu->insertItem("&Open...", this, SLOT(_FileOpen()), CTRL+Key_O);
	int save_id = _file_menu->insertItem("&Save", this, SLOT(_FileSave()), CTRL+Key_S);
	int save_as_id = _file_menu->insertItem("Save &As...", this, SLOT(_FileSaveAs()));
	_file_menu->insertSeparator();
	int resize_id = _file_menu->insertItem("&Resize Map...", this, SLOT(_FileResize()));
	_file_menu->insertSeparator();
	_file_menu->insertItem("&Quit", this, SLOT(_FileQuit()), CTRL+Key_Q);

	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_file_menu->setItemEnabled(save_id, _ed_scrollview->_map->GetChanged());
		_file_menu->setItemEnabled(save_as_id, true);
	//	_file_menu->setItemEnabled(resize_id, true);
	} // map must exist in order to save or resize it
	else
	{
		_file_menu->setItemEnabled(save_id, false);
		_file_menu->setItemEnabled(save_as_id, false);
	} // map does not exist, can't save or resize it
	_file_menu->setItemEnabled(resize_id, false);
} // _FileMenuSetup()

void Editor::_FileNew()
{
	_CreateTileDatabase();
	if (_EraseOK())
	{
		NewMapDialog* new_map = new NewMapDialog(this, "new_map");

		if (new_map->exec() == QDialog::Accepted)
		{
			if (_ed_scrollview != NULL)
				delete _ed_scrollview;
			_ed_scrollview = new EditorScrollView(_ed_widget, "map",
				new_map->GetWidth(), new_map->GetHeight());
			_ed_scrollview->resize(new_map->GetWidth() * TILE_WIDTH, new_map->GetHeight() * TILE_HEIGHT);

			if (_ed_tabs != NULL)
				delete _ed_tabs;
			_ed_tabs = new QTabWidget(_ed_widget);
			_ed_tabs->setTabPosition(QTabWidget::Bottom);

			QCheckListItem* tiles = static_cast<QCheckListItem*> (new_map->GetTilesetListView()->firstChild());
			while (tiles)
			{
				if (tiles->isOn())
				{
					_ed_tabs->addTab(new Tileset(_ed_widget, tiles->text()), tiles->text());
					_ed_scrollview->_map->tileset_list.push_back(tiles->text());
				} // tileset must be selected
				tiles = static_cast<QCheckListItem*> (tiles->nextSibling());
			} // iterate through all possible tilesets
	
			_ed_layout->addWidget(_ed_scrollview);
			_ed_layout->addWidget(_ed_tabs);
			_ed_scrollview->show();
			_ed_tabs->show();

			// Set default edit mode
			_SetEditLayer(LOWER_LAYER);
			_SetEditMode(PAINT_TILE);
		} // only if the user pressed OK
		else
			_stat_bar->message("No map created!",5000);

		delete new_map;
	} // make sure an unsaved map is not lost
} // _FileNew()

void Editor::_FileOpen()
{
	_CreateTileDatabase();
	if (_EraseOK())
	{
		// file to open
		QString file_name = QFileDialog::getOpenFileName(
			"dat/maps", "Maps (*.lua)", this, "file open",
			"HoA Level Editor -- File Open");

		if (!file_name.isEmpty())
		{
			if (_ed_scrollview != NULL)
				delete _ed_scrollview;
			_ed_scrollview = new EditorScrollView(_ed_widget, "map", 0, 0);

			if (_ed_tabs != NULL)
				delete _ed_tabs;
			_ed_tabs = new QTabWidget(_ed_widget);
			_ed_tabs->setTabPosition(QTabWidget::Bottom);

			_ed_layout->addWidget(_ed_scrollview);
			_ed_layout->addWidget(_ed_tabs);
			_ed_scrollview->show();

			_ed_scrollview->_map->SetFileName(file_name);
			_ed_scrollview->_map->LoadMap();

			for (QValueListIterator<QString> it = _ed_scrollview->_map->tileset_list.begin();
				it != _ed_scrollview->_map->tileset_list.end(); it++)
				_ed_tabs->addTab(new Tileset(_ed_widget, *it), *it);
			_ed_tabs->show();

			_ed_scrollview->resize(_ed_scrollview->_map->GetWidth(),
				_ed_scrollview->_map->GetHeight());
			_grid_on = false;
			_ll_on = false;
			_ml_on = false;
			_ul_on = false;
			_ViewToggleGrid();
			_ViewToggleLL();
			_ViewToggleML();
			_ViewToggleUL();
			_stat_bar->message(QString("Opened \'%1\'").
				arg(_ed_scrollview->_map->GetFileName()), 5000);

			// Set default edit mode
			_SetEditLayer(LOWER_LAYER);
			_SetEditMode(PAINT_TILE);
		} // file must exist in order to open it
	} // make sure an unsaved map is not lost
} // _FileOpen()

void Editor::_FileSaveAs()
{
	// get the file name from the user
	QString file_name = QFileDialog::getSaveFileName(
		"dat/maps", "Maps (*.lua)", this, "file save",
		"HoA Level Editor -- File Save");
		
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
	
    _stat_bar->message("Save abandoned.", 5000);
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
	_stat_bar->message(QString("Saved \'%1\' successfully!").
		arg(_ed_scrollview->_map->GetFileName()), 5000);
} // _FileSave()

void Editor::_FileResize()
{
	NewMapDialog* resize = new NewMapDialog(this, "map_resize");
	
	if (resize->exec() == QDialog::Accepted)
	{
		_ed_scrollview->_map->SetHeight(resize->GetHeight());
		_ed_scrollview->_map->SetWidth(resize->GetWidth());
		_ed_scrollview->_map->resize(resize->GetWidth() * TILE_WIDTH, resize->GetHeight() * TILE_HEIGHT);
		_ed_scrollview->resize(resize->GetWidth() * TILE_WIDTH, resize->GetHeight() * TILE_HEIGHT);

		if (_ed_tabs != NULL)
			delete _ed_tabs;
		_ed_tabs = new QTabWidget(_ed_widget);
		_ed_tabs->setTabPosition(QTabWidget::Bottom);

		QCheckListItem* tiles = static_cast<QCheckListItem*> (resize->GetTilesetListView()->firstChild());
		_ed_scrollview->_map->tileset_list.clear();
		while (tiles)
		{
			if (tiles->isOn())
			{
				_ed_tabs->addTab(new Tileset(_ed_widget, tiles->text()), tiles->text());
				_ed_scrollview->_map->tileset_list.push_back(tiles->text());
			} // tileset must be selected
			tiles = static_cast<QCheckListItem*> (tiles->nextSibling());
		} // iterate through all possible tilesets
	
		_ed_layout->addWidget(_ed_tabs);
		_ed_tabs->show();
	} // only if the user pressed OK
	else
		_stat_bar->message("Map not resized!",5000);

	delete resize;
} // _FileResize()

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
		_view_menu->setItemChecked(_grid_id, _grid_on);
		_ed_scrollview->_map->SetGridOn(_grid_on);
	} // map must exist in order to view things on it
} // _ViewToggleGrid()

void Editor::_ViewToggleLL()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_ll_on = !_ll_on;
		_view_menu->setItemChecked(_ll_id, _ll_on);
		_ed_scrollview->_map->SetLLOn(_ll_on);
	} // map must exist in order to view things on it
} // _ViewToggleLL()

void Editor::_ViewToggleML()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_ml_on = !_ml_on;
		_view_menu->setItemChecked(_ml_id, _ml_on);
		_ed_scrollview->_map->SetMLOn(_ml_on);
	} // map must exist in order to view things on it
} // _ViewToggleML()

void Editor::_ViewToggleUL()
{
	if (_ed_scrollview != NULL && _ed_scrollview->_map != NULL)
	{
		_ul_on = !_ul_on;
		_view_menu->setItemChecked(_ul_id, _ul_on);
		_ed_scrollview->_map->SetULOn(_ul_on);
	} // map must exist in order to view things on it
} // _ViewToggleUL()

void Editor::_TileLayerFill()
{
	// get reference to current tileset
	//Editor* editor = static_cast<Editor*> (topLevelWidget());
	QTable* table = static_cast<QTable*> (this->_ed_tabs->currentPage());

	// put selected tile from tileset into tile array at correct position
	QString name = table->text(table->currentRow(), table->currentColumn());
	int file_index = _ed_scrollview->_map->file_name_list.findIndex(name);
	if (file_index == -1)
	{
		_ed_scrollview->_map->file_name_list.append(name);
		file_index = _ed_scrollview->_map->file_name_list.findIndex(name);
	} // add tile filename to the list in use

	vector<int32>::iterator it;    // used to iterate over an entire layer
	vector<int32>& CurrentLayer=_ed_scrollview->GetCurrentLayer();
	for (it = CurrentLayer.begin();
					it != CurrentLayer.end(); it++)
				*it = file_index;
} // _TileLayerFill()

void Editor::_TileLayerClear()
{
	vector<int32>::iterator it;    // used to iterate over an entire layer
	vector<int32>& CurrentLayer=_ed_scrollview->GetCurrentLayer();
	for (it = CurrentLayer.begin();
					it != CurrentLayer.end(); it++)
				*it = -1;
} // _TileLayerClear()

void Editor::_SetEditMode(TILE_MODE_TYPE new_mode)
{
	if(_ed_scrollview==NULL)
		return;

	// Unset old check
	_tiles_menu->setItemChecked(_mode_ids[_ed_scrollview->_tile_mode], false);

	// Change mode and apply new check
	_ed_scrollview->_tile_mode = new_mode;
	_tiles_menu->setItemChecked(_mode_ids[_ed_scrollview->_tile_mode], true);
} // _SetEditMode(TILE_MODE_TYPE new_mode)

void Editor::_TileModePaint()
{
	if (_ed_scrollview != NULL)
		_SetEditMode(PAINT_TILE);
} // _TileModePaint()

void Editor::_TileModeMove()
{
	if (_ed_scrollview != NULL)
		_SetEditMode(MOVE_TILE);
} // _TileModeMove()

void Editor::_TileModeDelete()
{
	if (_ed_scrollview != NULL)
		_SetEditMode(DELETE_TILE);
} // _TileModeDelete()

void Editor::_SetEditLayer(LAYER_TYPE new_layer)
{
	if(_ed_scrollview == NULL)
		return;

	// Unset old check
	_tiles_menu->setItemChecked(_layer_ids[_ed_scrollview->_layer_edit],false);

	// Set new edit layer and set check
	_ed_scrollview->_layer_edit=new_layer;
	_tiles_menu->setItemChecked(_layer_ids[_ed_scrollview->_layer_edit],true);
}

void Editor::_TileEditLL()
{
	if (_ed_scrollview != NULL)
		_SetEditLayer(LOWER_LAYER);
} // _TileEditLL()

void Editor::_TileEditML()
{
	if (_ed_scrollview != NULL)
		_SetEditLayer(MIDDLE_LAYER);
} // _TileEditML()

void Editor::_TileEditUL()
{
	if (_ed_scrollview != NULL)
		_SetEditLayer(UPPER_LAYER);
} // _TileEditUL()

void Editor::_TileDatabase()
{
	DatabaseDialog* tile_db = new DatabaseDialog(this, "tile_db_dialog");
	tile_db->exec();
	delete tile_db;
} // _TileDatabase()

void Editor::_HelpHelp()
{
	_stat_bar->message(QString("See http://allacrost.sourceforge.net/wiki/index.php/Code_Documentation#Map_Editor_Documentation for more details"), 10000);
} // _HelpHelp()

void Editor::_HelpAbout()
{
    QMessageBox::about(this, "HoA Level Editor -- About",
		"<center><h1><font color=blue>Hero of Allacrost Level Editor<font>"
		"</h1></center>"
		"<center><h2><font color=blue>Copyright (c) 2004-2006<font></h2></center>"
		"<p>A level editor created for the Hero of Allacrost project."
		" See 'http://www.allacrost.org/' for more details</p>");
} // _HelpAbout()

void Editor::_HelpAboutQt()
{
    QMessageBox::aboutQt(this, "HoA Level Editor -- About Qt");
} // _HelpAboutQt()



// ********** Private functions **********

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
					_stat_bar->message("Save abandoned", 5000);
    	    		return false;
	    	} // warn the user to save
	    } // map has been modified
	} // map must exist first

    return true;
} // _EraseOK()

void Editor::_CreateTileDatabase()
{
	QDir database_dir("./dat");    // tiles database directory
	if (!database_dir.exists("tilesets"))
	{
		QString database_name = QDir::convertSeparators("dat/tilesets");
		QString message("Tile database directory ");
		message.append(database_name);
		message.append(" does not exist.\nCreate ");
		message.append(database_name);
		message.append(" directory?\n");
		message.append("(Warning: editor will not function without this directory!)");
		int answer = QMessageBox::warning(this, "Tile Database", QString(message), "&Yes", "&No", QString::null, 0, 1);

		if (answer == 0)
		{
			if (!database_dir.mkdir("tilesets"))
			{
				QMessageBox::warning(this, "Tile Database", "Unable to create tile database directory! Exiting...");
				_FileQuit();
			} // only if mkdir was unsuccessful
		} // user pressed yes
		else
			_FileQuit();
	} // make sure tile database directory exists

	if (!QFile::exists("dat/tilesets/tiles_database.lua"))
	{
		QMessageBox::warning(this, "Tile Database", "Tile database does not exist. Creating one now...");
		_stat_bar->message("Please wait...");
		_GenerateDatabase();
		_stat_bar->message("Database successfully created!", 5000);
	} // tile database has not yet been setup
} // _CreateTileDatabase()

void Editor::_GenerateDatabase()
{
	QDir tile_dir("img/tiles/", "*.png");			// tile set directory
	if (!tile_dir.exists())
	{
       	QMessageBox::warning(this, "Tile Images", "Cannot find the tile image directory! Try reinstalling Hero of Allacrost.");
		_FileQuit();
	} // make sure directory exists

	if (tile_dir.count() == 0)
	{
		QMessageBox::warning(this, "Tile Images", "No tiles were found in the image directory!"
			"Please see the Level Editor documentation at 'http://allacrost.sourceforge.net/wiki' for more details.");
		_FileQuit();
	} // make sure there are images in the directory

	DataDescriptor write_data;
	if (!write_data.OpenFile("dat/tilesets/tiles_database.lua", WRITE))
		QMessageBox::warning(this, "Tile Images", "ERROR: could not open dat/tilesets/tiles_database.lua for writing!");
	else
	{
		write_data.WriteComment("File: tiles_database.lua");
		write_data.InsertNewLine();

		// Add individual tiles to the database.
		write_data.WriteComment("Names of all possible tile image files, with the path and file extension omitted (note that the indeces begin with 1, not 0)");
		write_data.BeginTable("tile_filenames");
		for (uint32 i = 0; i < tile_dir.count(); i++)
			write_data.WriteString(i+1, tile_dir[i].remove(".png").ascii());
		write_data.EndTable();
		write_data.InsertNewLine();

		// Add default properties to individual tiles.
		write_data.WriteComment("Properties of all possible tiles (valid range: 0-255, non-zero being walkable)");
		write_data.BeginTable("tile_properties");
		for (uint32 i = 0; i < tile_dir.count(); i++)
			write_data.WriteInt(i+1, 255);
		write_data.EndTable();
		write_data.InsertNewLine();

		write_data.CloseFile();
	} // file was opened successfully
} // _GenerateDatabase()



/************************
  NewMapDialog class functions follow
************************/

NewMapDialog::NewMapDialog(QWidget* parent, const QString& name)
	: QDialog(parent, (const char*) name)
{
	setCaption("Map Properties...");

	_dia_layout    = new QGridLayout(this, 7, 2, 5);
	
	_height_label  = new QLabel("Height (in tiles):", this);
	_height_sbox   = new QSpinBox(1, 1000, 1, this);
	_width_label   = new QLabel(" Width (in tiles):", this);
	_width_sbox    = new QSpinBox(1, 1000, 1, this);
	
	_tileset_lview = new QListView(this, "tileset_lview", WStaticContents|WNoAutoErase);
	
	_cancel_pbut   = new QPushButton("Cancel", this);
	_ok_pbut       = new QPushButton("OK", this);
	
	_cancel_pbut->setDefault(true);
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	QDir tileset_dir("dat/tilesets");
	_tileset_lview->addColumn("Tilesets");
	QCheckListItem* global = new QCheckListItem(_tileset_lview, "Global", QCheckListItem::CheckBox);
	global->setOn(true);
	for (uint32 i = 0; i < tileset_dir.count(); i++)
	{
		if (tileset_dir[i].contains("tileset") != 0)
		{
			(void) new QCheckListItem(_tileset_lview, tileset_dir[i].remove("tileset_").remove(".lua"),
				QCheckListItem::CheckBox);
		}
	} // looks for tileset files in the tileset directory
	
	_dia_layout->addWidget(_height_label, 0, 0);
	_dia_layout->addWidget(_height_sbox, 1, 0);
	_dia_layout->addWidget(_width_label, 2, 0);
	_dia_layout->addWidget(_width_sbox, 3, 0);
	_dia_layout->addMultiCellWidget(_tileset_lview, 0, 5, 1, 1);
	_dia_layout->addWidget(_cancel_pbut, 6, 1);
	_dia_layout->addWidget(_ok_pbut, 6, 0);
} // NewMapDialog constructor

NewMapDialog::~NewMapDialog()
{
	delete _height_label;
	delete _height_sbox;
	delete _width_label;
	delete _width_sbox;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _dia_layout;
} // NewMapDialog destructor



/************************
  EditorScrollView class functions follow
************************/

EditorScrollView::EditorScrollView(QWidget* parent, const QString& name, int width,
	int height) : QScrollView(parent, (const char*) name, WNoAutoErase|WStaticContents)
{
	// Set default editing modes.
	_tile_mode  = PAINT_TILE;
	_layer_edit = LOWER_LAYER;
	
	// Create a new map.
	_map = new Grid(viewport(), "Untitled", width, height);
	addChild(_map);

	// Context menu creation.
	_context_menu = new QPopupMenu(this);
	// Create the walkability checkboxes and add them to a QVButtonGroup.
	QVButtonGroup* checkboxes = new QVButtonGroup("Walkability", _context_menu,
		"checkboxes");
	_allwalk_checkbox = new QCheckBox("All", checkboxes, "allwalk_checkbox");
	for (uint32 i = 0; i < 8; i++)
		_walk_checkbox[i] = new QCheckBox(QString("Level %1").arg(i+1), checkboxes,
			QString("walk_checkbox[%1]").arg(i));
	connect(_allwalk_checkbox, SIGNAL(toggled(bool)), this,
		SLOT(_ToggleWalkCheckboxes(bool)));
	connect(_context_menu, SIGNAL(aboutToShow()), this, SLOT(_ContextMenuSetup()));
	connect(_context_menu, SIGNAL(aboutToHide()), this, SLOT(_ContextMenuEvaluate()));
	_context_menu->insertItem(checkboxes);
} // EditorScrollView constructor

EditorScrollView::~EditorScrollView()
{
	delete _map;
	delete _allwalk_checkbox;
	for (int i = 0; i < 8; i++)
		delete _walk_checkbox[i];
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
				QTable* table = static_cast<QTable*> (editor->_ed_tabs->currentPage());

				// put selected tile from tileset into tile array at correct position
				QString name = table->text(table->currentRow(), table->currentColumn());
				int file_index = _map->file_name_list.findIndex(name);
				if (file_index == -1)
				{
					_map->file_name_list.append(name);
					file_index = _map->file_name_list.findIndex(name);
				} // add tile filename to the list in use

				GetCurrentLayer()[_tile_index] = file_index;
			} // left mouse button was pressed
			break;
		} // edit mode PAINT_TILE

		case MOVE_TILE: // start moving a tile
		{
			_move_source_index=_tile_index;
			break;
		} // edit mode MOVE_TILE

		case DELETE_TILE: // start deleting tiles
		{
			if (evt->button() == Qt::LeftButton)
			{
				int file_index = GetCurrentLayer()[_tile_index];

				// delete the tile
				GetCurrentLayer()[_tile_index] = -1;

				_RemoveIfUnused(file_index);
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
					QTable* table = static_cast<QTable*> (editor->_ed_tabs->currentPage());

					// put selected tile from tileset into tile array at correct position
					QString name = table->text(table->currentRow(), table->currentColumn());
					int file_index = _map->file_name_list.findIndex(name);
					if (file_index == -1)
					{
						_map->file_name_list.append(name);
						file_index = _map->file_name_list.findIndex(name);
					} // add tile filename to the list in use

					GetCurrentLayer()[_tile_index] = file_index;
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
					int file_index = GetCurrentLayer()[_tile_index];

					// delete the tile
					GetCurrentLayer()[_tile_index] = -1;

					_RemoveIfUnused(file_index);
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
		std::vector<int32>& layer=GetCurrentLayer();
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
} // contentsContextMenuEvent(...)



// ********** Private slots **********

void EditorScrollView::_ContextMenuSetup()
{
	if (_map->indiv_walkable[_tile_index] != -1)
	{
		if (_map->indiv_walkable[_tile_index] == 255)
			_allwalk_checkbox->setChecked(true);
		else
			_allwalk_checkbox->setChecked(false);

		for (uint8 i = 0; i < 8; i++)
			if (_map->indiv_walkable[_tile_index] & (1 << i))
				_walk_checkbox[i]->setChecked(true);
			else
				_walk_checkbox[i]->setChecked(false);
	} // individual walkability supersedes everything else
	else if (_map->tiles_walkable[_tile_index] != -1)
	{
		if (_map->tiles_walkable[_tile_index] == 255)
			_allwalk_checkbox->setChecked(true);
		else
			_allwalk_checkbox->setChecked(false);

		for (uint8 i = 0; i < 8; i++)
			if (_map->tiles_walkable[_tile_index] & (1 << i))
				_walk_checkbox[i]->setChecked(true);
			else
				_walk_checkbox[i]->setChecked(false);
	} // look up walkability property from the map
	else
	{
		// Read from global database to get property of item.
		DataDescriptor read_data;
		if (!read_data.OpenFile("dat/tilesets/tiles_database.lua", READ))
			QMessageBox::warning(this, "Tiles Database",
				"ERROR: could not open dat/tilesets/tiles_database.lua for reading!");
		else
		{
			read_data.OpenTable("tile_filenames");
			uint32 table_size = read_data.GetTableSize();
			uint32 index = 0;
			QString filename = "";
			while (filename != _map->file_name_list[
					_map->GetLayer(LOWER_LAYER)[_tile_index]] && index < table_size)
			{
				index++;
				filename = read_data.ReadString(index);
			} // find index of current tile in the database
			read_data.CloseTable();
			if (filename == _map->file_name_list[_map->GetLayer(LOWER_LAYER)[_tile_index]])
			{
				read_data.OpenTable("tile_filenames");
				uint8 walkable = read_data.ReadInt(index);
				read_data.CloseTable();
				if (walkable == 255)
					_allwalk_checkbox->setChecked(true);
				else
					_allwalk_checkbox->setChecked(false);

				for (uint8 i = 0; i < 8; i++)
					if (walkable & (1 << i))
						_walk_checkbox[i]->setChecked(true);
					else
						_walk_checkbox[i]->setChecked(false);
			} // tile exists in the database
			read_data.CloseFile();
		} // file was successfully opened
	} // look up walkability property in global tiles database
} // _ContextMenuSetup()

void EditorScrollView::_ContextMenuEvaluate()
{
	_map->indiv_walkable[_tile_index] = 0;
	for (uint8 i = 0; i < 8; i++)
		if (_walk_checkbox[i]->isChecked())
			_map->indiv_walkable[_tile_index] |= (1 << i);
} // _ContextMenuEvaluate()

void EditorScrollView::_ToggleWalkCheckboxes(bool on)
{
	if (on)
		for (uint8 i = 0; i < 8; i++)
			_walk_checkbox[i]->setChecked(true);
	else
		for (uint8 i = 0; i < 8; i++)
			_walk_checkbox[i]->setChecked(false);
} // _ToggleWalkCheckboxes(...)

// *************** Private Functions ***************
void EditorScrollView::_RemoveIfUnused(int file_index) 
{
	// find other instances of the tile 
	bool Found=false;
	for(LAYER_TYPE layer=LOWER_LAYER;layer<=UPPER_LAYER && !Found;layer++) {
		vector<int32>::iterator it;
		vector<int32>& CurrentLayer=_map->GetLayer(layer);
		// Loop until we either find something or we are at the end of the vector
		for (it = CurrentLayer.begin(); 
			it != CurrentLayer.end() && *it != file_index; it++);
			if(it!=CurrentLayer.end())
				Found=true;
	}

	// If tile was not found, remove it from the list
	if(!Found)
		_map->file_name_list.erase(_map->file_name_list.at(file_index));
}


/************************
  DatabaseDialog class functions follow
************************/

DatabaseDialog::DatabaseDialog(QWidget* parent, const QString& name)
	: QTabDialog(parent, (const char*) name)
{
	setCaption("Tile Database...");
	resize(600, 500);

	QDir tileset_dir("dat/tilesets");    // tileset directory

	// Make sure directory exists.
	if (!tileset_dir.exists())
       	QMessageBox::warning(this, "Directory Warning", "Cannot find the tileset directory dat/tilesets/!");
	else
	{
		// ***************************************************
		// The following creates the Tilesets tab of the Tile Database manager dialog.
		// ***************************************************

		// Create a widget to put inside a tab of the dialog.
		QWidget* tilesets_widget = new QWidget(this);

		// Create a QLabel for a read-only QComboBox (drop-down list) and add all existing tilesets, and connect it to a slot.
		QLabel* tilesets_label   = new QLabel("Tileset to modify:", tilesets_widget, "tilesets_label");
		tilesets_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QComboBox* tilesets_cbox = new QComboBox(false, tilesets_widget, "tilesets_cbox");
		tilesets_cbox->insertItem("Select Tileset...");
		tilesets_cbox->insertItem("New Tileset");
		for (uint32 i = 0; i < tileset_dir.count(); i++)
		{
			if (tileset_dir[i].contains("tileset") != 0)
				tilesets_cbox->insertItem(tileset_dir[i].remove("tileset_").remove(".lua"));
		} // looks for tileset files in the tileset directory
		connect(tilesets_cbox, SIGNAL(activated(const QString&)), this, SLOT(_TilesetsTabPopulateTileset(const QString&)));

		// Create a QLineEdit and a QLabel for it.
		_tileset_ledit        = new QLineEdit(tilesets_widget, "tileset_ledit");
		QLabel* tileset_label = new QLabel("Tileset Name:", tilesets_widget, "tileset_label");
		tileset_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		// Create QIconViews and set up their properties appropriately.
		_all_tiles   = new QIconView(tilesets_widget, "all_tiles_iview");
		_mod_tileset = new QIconView(tilesets_widget, "new_tileset_iview");
		_all_tiles->setItemTextPos(QIconView::Right);
		_mod_tileset->setItemTextPos(QIconView::Right);
		_all_tiles->setSelectionMode(QIconView::Single);
		_mod_tileset->setSelectionMode(QIconView::Single);
		_all_tiles->setWordWrapIconText(false);
		_mod_tileset->setWordWrapIconText(false);
		_all_tiles->setItemsMovable(false);
		_mod_tileset->setItemsMovable(false);
		_mod_tileset->setSorting(true);
		_all_tiles->setGridX(300);
		_mod_tileset->setGridX(300);
		_mod_tileset->setAcceptDrops(true);

		// Populate the QIconView for the global tileset.
		DataDescriptor read_data;
		if (!read_data.OpenFile("dat/tilesets/tiles_database.lua", READ))
			QMessageBox::warning(this, "Tileset File",
				QString("ERROR: could not open dat/tilesets/tiles_database.lua for reading!"));
		else
		{
			read_data.OpenTable("tile_filenames");
			uint32 table_size = read_data.GetTableSize();
			for (uint32 i = 1; i <= table_size; i++)
			{
				QString filename = read_data.ReadString(i);
				(void) new QIconViewItem(_all_tiles, filename, QPixmap("img/tiles/" + filename.append(".png")));
			} // iterate through all tiles in the tileset
			read_data.CloseTable();
			read_data.CloseFile();
		} // file was successfully opened

		// Create QPushButtons and connect them to their appropriate slots.
		QPushButton* add_tile_pbut = new QPushButton("Add Tile", tilesets_widget);
		QPushButton* del_tile_pbut = new QPushButton("Remove Tile", tilesets_widget);
		connect(add_tile_pbut, SIGNAL(clicked()), this, SLOT(_AddTile()));
		connect(del_tile_pbut, SIGNAL(clicked()), this, SLOT(_DelTile()));

		// Create a QGridLayout and add all the created widgets to it.
		QGridLayout* tilesets_tab = new QGridLayout(tilesets_widget, 4, 2, 5);
		tilesets_tab->addWidget(tilesets_label, 0, 0);
		tilesets_tab->addWidget(tilesets_cbox, 0, 1);
		tilesets_tab->addWidget(tileset_label, 1, 0);
		tilesets_tab->addWidget(_tileset_ledit, 1, 1);
		tilesets_tab->addWidget(_all_tiles, 2, 0);
		tilesets_tab->addWidget(_mod_tileset, 2, 1);
		tilesets_tab->addWidget(add_tile_pbut, 3, 0);
		tilesets_tab->addWidget(del_tile_pbut, 3, 1);

		// Insert the tab into the dialog window.
		addTab(tilesets_widget, "Tilesets");

		// ***************************************************
		// End of Tilesets tab creation.
		// ***************************************************
		
		// ***************************************************
		// The following creates the Properties tab of the Tile Database manager dialog.
		// ***************************************************

		// Create a widget to put inside a tab of the dialog.
		QWidget* properties_widget = new QWidget(this);

		// Create a QLabel for a read-only QComboBox (drop-down list) and add all existing tilesets, and connect it to a slot.
		QComboBox* proptsets_cbox  = new QComboBox(false, properties_widget, "proptsets_cbox");
		proptsets_cbox->insertItem("Select Tileset...");
		for (uint32 i = 0; i < tileset_dir.count(); i++)
		{
			if (tileset_dir[i].contains("tileset") != 0)
				proptsets_cbox->insertItem(tileset_dir[i].remove("tileset_").remove(".lua"));
		} // looks for tileset files in the tileset directory
		connect(proptsets_cbox, SIGNAL(activated(const QString&)), this, SLOT(_PropertiesTabPopulateTileset(const QString&)));

		// Create QIconView and set up its properties appropriately.
		_prop_tileset = new QIconView(properties_widget, "prop_tileset_iview");
		_prop_tileset->setItemTextPos(QIconView::Right);
		_prop_tileset->setSelectionMode(QIconView::Single);
		_prop_tileset->setWordWrapIconText(false);
		_prop_tileset->setItemsMovable(false);
		_prop_tileset->setSorting(true);
		_prop_tileset->setGridX(300);
		connect(_prop_tileset, SIGNAL(currentChanged(QIconViewItem *)), this, SLOT(_ProcessWalkability(QIconViewItem *)));
		_tile_index = 0;    // no changes made yet

		// Create the walkability checkboxes and add them to a QVButtonGroup.
		QVButtonGroup* checkboxes = new QVButtonGroup("Walkability", properties_widget, "checkboxes");
		_allwalk_checkbox = new QCheckBox("All", checkboxes, "allwalk_checkbox");
		for (uint32 i = 0; i < 8; i++)
			_walk_checkbox[i] = new QCheckBox(QString("Level %1").arg(i+1), checkboxes, QString("walk_checkbox[%1]").arg(i));
		connect(_allwalk_checkbox, SIGNAL(toggled(bool)), this, SLOT(_ToggleWalkCheckboxes(bool)));

		// Create the animation stuff. FIXME
		QLabel* anim_label = new QLabel("Placeholder for animation settings", properties_widget, "anim_label");

		// Create a QGridLayout and add all the created widgets to it.
		QGridLayout* properties_tab = new QGridLayout(properties_widget, 2, 3, 5);
		properties_tab->addWidget(proptsets_cbox,  0, 0);
		properties_tab->addWidget(_prop_tileset,   1, 0);
		properties_tab->addWidget(checkboxes,      1, 1);
		properties_tab->addWidget(anim_label,      1, 2);

		// Insert the tab into the dialog window.
		addTab(properties_widget, "Properties");

		// Initialize the _tile_properties array.
		_tile_properties.clear();
		// Read from global database to get the properties.
		if (!read_data.OpenFile("dat/tilesets/tiles_database.lua", READ))
			QMessageBox::warning(this, "Tiles Database",
				QString("ERROR: could not open dat/tilesets/tiles_database.lua for reading!"));
		else
		{
			read_data.OpenTable("tile_properties");
			uint32 table_size = read_data.GetTableSize();
			for (uint32 i = 0; i < table_size; i++)
				_tile_properties.push_back(read_data.ReadInt(i+1));
			read_data.CloseTable();
			read_data.CloseFile();
		} // file was successfully opened
		
		// ***************************************************
		// End of Properties tab creation.
		// ***************************************************
	} // tile image directory exists

	// Create a Cancel button and connect the OK button to a useful slot.
	setCancelButton();
	connect(this, SIGNAL(applyButtonPressed()), this, SLOT(_UpdateData()));
} // DatabaseDialog constructor

DatabaseDialog::~DatabaseDialog()
{
} // DatabaseDialog destructor



// ********** Private slots **********

void DatabaseDialog::_UpdateData()
{
	// ***************************************************
	// Saves data related to the Tilesets tab.
	// ***************************************************
	
	// Assume if there is text in the QLineEdit then a modification has been made.
	if (_tileset_ledit->text() != NULL && _tileset_ledit->text() != "")
	{
		DataDescriptor write_data;
		if (!write_data.OpenFile(QString("dat/tilesets/tileset_%1.lua").arg(_tileset_ledit->text()), WRITE))
			QMessageBox::warning(this, "Tileset File",
				QString("ERROR: could not open dat/tilesets/tileset_%1.lua for writing!").arg(_tileset_ledit->text()));
		else
		{
			write_data.WriteComment(QString("tileset_%1.lua").arg(_tileset_ledit->text()));
			write_data.InsertNewLine();

			write_data.BeginTable("tile_filenames");
			uint32 i = 0;
			for (QIconViewItem* tile = _mod_tileset->firstItem(); tile; tile = tile->nextItem())
			{
				i++;
				write_data.WriteString(i, tile->text().remove(".png"));
			} // iterates through all tiles in the tileset/iconview
			write_data.EndTable();
			write_data.CloseFile();
		} // file was successfully opened
	} // only if the QLineEdit is not empty

	// ***************************************************
	// End of saving data related to the Tilesets tab.
	// ***************************************************
	
	// ***************************************************
	// Saves data related to the Properties tab.
	// ***************************************************
	
	// User might change some properties then immediately click Ok.
	_ProcessWalkability(_prop_tileset->currentItem());
	
	QDir tile_dir("img/tiles/", "*.png");			// tile set directory
	DataDescriptor write_data;
	if (!write_data.OpenFile("dat/tilesets/tiles_database.lua", WRITE))
		QMessageBox::warning(this, "Tiles Database", "ERROR: could not open dat/tilesets/tiles_database.lua for writing!");
	else
	{
		write_data.WriteComment("File: tiles_database.lua");
		write_data.InsertNewLine();

		// Add individual tiles to the database.
		write_data.WriteComment("Names of all possible tile image files, with the path and file extension omitted (note that the indeces begin with 1, not 0)");
		write_data.BeginTable("tile_filenames");
		for (uint32 i = 0; i < tile_dir.count(); i++)
			write_data.WriteString(i+1, tile_dir[i].remove(".png").ascii());
		write_data.EndTable();
		write_data.InsertNewLine();

		// Writes new properties to individual tiles.
		write_data.WriteComment("Properties of all possible tiles (valid range: 0-255, non-zero being walkable)");
		write_data.BeginTable("tile_properties");
		for (uint32 i = 0; i < tile_dir.count(); i++)
			write_data.WriteInt(i+1, _tile_properties[i]);
		write_data.EndTable();
		write_data.InsertNewLine();

		write_data.CloseFile();
	} // file was opened successfully
	
	// ***************************************************
	// End of saving data related to the Properties tab.
	// ***************************************************
} // _UpdateData()

void DatabaseDialog::_AddTile()
{
	QIconViewItem* CurrentItem=_all_tiles->currentItem();
	//If no item is selected, show a warning
	if(CurrentItem == 0) {
		QMessageBox::warning(this,"Error","No tile selected!");
	}
	// Otherwise: Only add new tile if it doesn't already exist in the new tileset.
	else if (_mod_tileset->findItem(CurrentItem->text(), Qt::ExactMatch) == 0)
		(void) new QIconViewItem(_mod_tileset, _all_tiles->currentItem()->text(), *_all_tiles->currentItem()->pixmap());
} // _AddTile()

void DatabaseDialog::_DelTile()
{
	delete _mod_tileset->currentItem();
} // _DelTile()

void DatabaseDialog::_TilesetsTabPopulateTileset(const QString& name)
{
	if (name != "New Tileset" && name != "Select Tileset...")
	{
		// Fill in the QLineEdit.
		_tileset_ledit->setText(name);

		// Do the populating.
		_PopulateTilesetHelper(_mod_tileset, name);
	} // no populating necessary otherwise
} // _TilesetsTabPopulateTileset(...)

void DatabaseDialog::_PropertiesTabPopulateTileset(const QString& name)
{
	if (name != "Select Tileset...")
		_PopulateTilesetHelper(_prop_tileset, name);
} // _PropertiesTabPopulateTileset(...)

void DatabaseDialog::_ProcessWalkability(QIconViewItem* item)
{
	if (item != NULL)
	{
		if (_tile_index != 0)
		{
			// Record the new walkability options.
			_tile_properties[_tile_index-1] = 0;
			for (uint8 i = 0; i < 8; i++)
				if (_walk_checkbox[i]->isChecked())
					_tile_properties[_tile_index-1] |= (1 << i);
		} // must have made some changes in order to record them
		
		// Read from global database to get property of item.
		DataDescriptor read_data;
		if (!read_data.OpenFile("dat/tilesets/tiles_database.lua", READ))
			QMessageBox::warning(this, "Tiles Database", "ERROR: could not open dat/tilesets/tiles_database.lua for reading!");
		else
		{
			read_data.OpenTable("tile_filenames");
			uint32 table_size = read_data.GetTableSize();
			_tile_index = 0;
			QString filename = "";
			while (filename != item->text().remove(".png") && _tile_index < table_size)
			{
				_tile_index++;
				filename = read_data.ReadString(_tile_index);
			} // find index of current tile in the database
			read_data.CloseTable();
			read_data.CloseFile();
			if (filename == item->text().remove(".png"))
			{
				if (_tile_properties[_tile_index-1] == 255)
					_allwalk_checkbox->setChecked(true);
				else
					_allwalk_checkbox->setChecked(false);

				for (uint8 i = 0; i < 8; i++)
					if (_tile_properties[_tile_index-1] & (1 << i))
						_walk_checkbox[i]->setChecked(true);
					else
						_walk_checkbox[i]->setChecked(false);
			} // tile exists in the database
		} // file was successfully opened
	} // nothing to do otherwise
} // _ProcessWalkability(...)

void DatabaseDialog::_ToggleWalkCheckboxes(bool on)
{
	if (on)
		for (uint8 i = 0; i < 8; i++)
			_walk_checkbox[i]->setChecked(true);
	else
		for (uint8 i = 0; i < 8; i++)
			_walk_checkbox[i]->setChecked(false);
} // _ToggleWalkCheckboxes(...)



// ********** Private function **********

void DatabaseDialog::_PopulateTilesetHelper(QIconView *tileset, const QString& name)
{
	if (tileset != NULL)
	{
		tileset->clear();
		
		// Read from tileset Lua file to populate the QIconView.
		DataDescriptor read_data;
		if (!read_data.OpenFile(QString("dat/tilesets/tileset_%1.lua").arg(name), READ))
			QMessageBox::warning(this, "Tileset File",
				QString("ERROR: could not open dat/tilesets/tileset_%1.lua for reading!").arg(name));
		else
		{
			read_data.OpenTable("tile_filenames");
			uint32 table_size = read_data.GetTableSize();
			for (uint32 i = 1; i <= table_size; i++)
			{
				QString filename = read_data.ReadString(i);
				(void) new QIconViewItem(tileset, filename, QPixmap("img/tiles/" + filename.append(".png")));
			} // iterates through all tiles in the tileset
			read_data.CloseTable();
			read_data.CloseFile();
		} // file was successfully opened
	} // no populating necessary otherwise
} // _PopulateTilesetHelper()
