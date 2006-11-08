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

using namespace hoa_script;
using namespace hoa_utils;
using namespace hoa_editor;
using namespace hoa_video;
using namespace std;

Editor::Editor() : Q3MainWindow(0, 0, Qt::WDestructiveClose)
{
	_tile_db=NULL;

	// create the statusbar
	_stat_bar = new QStatusBar(this);
	
	// file menu creation
	_file_menu = new Q3PopupMenu(this);
	connect(_file_menu, SIGNAL(aboutToShow()), this, SLOT(_FileMenuSetup()));
	menuBar()->insertItem("&File", _file_menu);

	// view menu creation
	_view_menu = new Q3PopupMenu(this);
	menuBar()->insertItem("&View", _view_menu);
	_grid_id = _view_menu->insertItem("&Grid", this, SLOT(_ViewToggleGrid()));
	_view_menu->insertSeparator();
	_ll_id = _view_menu->insertItem("&Lower Tile Layer", this, SLOT(_ViewToggleLL()));
	_ml_id = _view_menu->insertItem("&Middle Tile Layer", this, SLOT(_ViewToggleML()));
	_ul_id = _view_menu->insertItem("&Upper Tile Layer", this, SLOT(_ViewToggleUL()));
	_view_menu->setCheckable(true);

	// tile menu creation
	_tiles_menu = new Q3PopupMenu(this);
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
	_tiles_menu->insertItem("&Manage database...", this, SLOT(_TileDatabase()), Qt::CTRL+Qt::Key_D);
	
	// map menu creation
	_map_menu = new Q3PopupMenu(this);
	menuBar()->insertItem("&Map",_map_menu);
	_map_menu->insertItem("Set background &music...", this, SLOT(_MapSelectMusic()));

	// help menu creation
	_help_menu = new Q3PopupMenu(this);
	menuBar()->insertSeparator();
	menuBar()->insertItem("&Help", _help_menu);
	_help_menu->insertItem("&Help", this, SLOT(_HelpHelp()), Qt::Key_F1);
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
	_ed_layout = new Q3VBoxLayout(_ed_widget);
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

	if(_tile_db!=NULL)
		delete _tile_db;
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
	_file_menu->insertItem("&New...", this, SLOT(_FileNew()), Qt::CTRL+Qt::Key_N);
	_file_menu->insertItem("&Open...", this, SLOT(_FileOpen()), Qt::CTRL+Qt::Key_O);
	int save_id = _file_menu->insertItem("&Save", this, SLOT(_FileSave()), Qt::CTRL+Qt::Key_S);
	int save_as_id = _file_menu->insertItem("Save &As...", this, SLOT(_FileSaveAs()));
	_file_menu->insertSeparator();
	int resize_id = _file_menu->insertItem("&Resize Map...", this, SLOT(_FileResize()));
	_file_menu->insertSeparator();
	_file_menu->insertItem("&Quit", this, SLOT(_FileQuit()), Qt::CTRL+Qt::Key_Q);

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
	_OpenTileDatabase();
	if (_EraseOK())
	{
		NewMapDialog* new_map = new NewMapDialog(this, "new_map");

		if (new_map->exec() == QDialog::Accepted)
		{
			if (_ed_scrollview != NULL)
				delete _ed_scrollview;
			_ed_scrollview = new EditorScrollView(_ed_widget, "map",
				new_map->GetWidth(), new_map->GetHeight(), _tile_db);
			_ed_scrollview->resize(new_map->GetWidth() * TILE_WIDTH, new_map->GetHeight() * TILE_HEIGHT);

			if (_ed_tabs != NULL)
				delete _ed_tabs;
			_ed_tabs = new QTabWidget(_ed_widget);
			_ed_tabs->setTabPosition(QTabWidget::South);

			Q3CheckListItem* tiles = static_cast<Q3CheckListItem*> (new_map->GetTilesetListView()->firstChild());
			while (tiles)
			{
				if (tiles->isOn())
				{
					_ed_tabs->addTab(new TilesetTable(_ed_widget, tiles->text(), _tile_db), tiles->text());
					_ed_scrollview->_map->tileset_list.push_back(tiles->text());
				} // tileset must be selected
				tiles = static_cast<Q3CheckListItem*> (tiles->nextSibling());
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
	_OpenTileDatabase();
	if (_EraseOK())
	{
		// file to open
		QString file_name = Q3FileDialog::getOpenFileName(
			"dat/maps", "Maps (*.lua)", this, "file open",
			"HoA Level Editor -- File Open");

		if (!file_name.isEmpty())
		{
			if (_ed_scrollview != NULL)
				delete _ed_scrollview;
			_ed_scrollview = new EditorScrollView(_ed_widget, "map", 0, 0, _tile_db);

			if (_ed_tabs != NULL)
				delete _ed_tabs;
			_ed_tabs = new QTabWidget(_ed_widget);
			_ed_tabs->setTabPosition(QTabWidget::South);

			_ed_layout->addWidget(_ed_scrollview);
			_ed_layout->addWidget(_ed_tabs);
			_ed_scrollview->show();

			_ed_scrollview->_map->SetFileName(file_name);
			_ed_scrollview->_map->LoadMap();

			for (QStringList::ConstIterator it = _ed_scrollview->_map->tileset_list.begin();
				it != _ed_scrollview->_map->tileset_list.end(); it++)
				_ed_tabs->addTab(new TilesetTable(_ed_widget, *it, _tile_db), *it);
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
	QString file_name = Q3FileDialog::getSaveFileName(
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
		_ed_tabs->setTabPosition(QTabWidget::South);

		Q3CheckListItem* tiles = static_cast<Q3CheckListItem*> (resize->GetTilesetListView()->firstChild());
		_ed_scrollview->_map->tileset_list.clear();
		while (tiles)
		{
			if (tiles->isOn())
			{
				_ed_tabs->addTab(new TilesetTable(_ed_widget, tiles->text(), _tile_db), tiles->text());
				_ed_scrollview->_map->tileset_list.push_back(tiles->text());
			} // tileset must be selected
			tiles = static_cast<Q3CheckListItem*> (tiles->nextSibling());
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
	Q3Table* table = static_cast<Q3Table*> (this->_ed_tabs->currentPage());

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
	DatabaseDialog* tile_db = new DatabaseDialog(this, "tile_db_dialog", _tile_db);
	tile_db->exec();
	delete tile_db;
} // _TileDatabase()

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
}

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

void Editor::_OpenTileDatabase()
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

	if (QFile::exists("dat/tilesets/tiles_database.lua"))
	{
		_tile_db=new TileDatabase("dat/tilesets/tiles_database.lua");
	} 
	else 
	{
		QMessageBox::warning(this, "Tile Database", "Tile database does not exist. Creating one now...");
		_stat_bar->message("Please wait...");
		_tile_db=new TileDatabase();
		_tile_db->Update("img/tiles");
		_tile_db->Save("dat/tilesets/tiles_database.lua");
		_stat_bar->message("Database successfully created!", 5000);
	} // tile database has not yet been setup
} // _CreateTileDatabase()

/************************
  NewMapDialog class functions follow
************************/

NewMapDialog::NewMapDialog(QWidget* parent, const QString& name)
	: QDialog(parent, (const char*) name)
{
	setCaption("Map Properties...");

	_dia_layout    = new Q3GridLayout(this, 7, 2, 5);
	
	_height_label  = new QLabel("Height (in tiles):", this);
	_height_sbox   = new QSpinBox(1, 1000, 1, this);
	_width_label   = new QLabel(" Width (in tiles):", this);
	_width_sbox    = new QSpinBox(1, 1000, 1, this);
	
	_tileset_lview = new Q3ListView(this, "tileset_lview", Qt::WStaticContents|Qt::WNoAutoErase);
	
	_cancel_pbut   = new QPushButton("Cancel", this);
	_ok_pbut       = new QPushButton("OK", this);
	
	_cancel_pbut->setDefault(true);
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	QDir tileset_dir("dat/tilesets");
	_tileset_lview->addColumn("Tilesets");
	Q3CheckListItem* global = new Q3CheckListItem(_tileset_lview, "Global", Q3CheckListItem::CheckBox);
	global->setOn(true);
	for (uint32 i = 0; i < tileset_dir.count(); i++)
	{
		if (tileset_dir[i].contains("tileset") != 0)
		{
			(void) new Q3CheckListItem(_tileset_lview, tileset_dir[i].remove("tileset_").remove(".lua"),
				Q3CheckListItem::CheckBox);
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
  MusicDialog class functions follow
************************/

MusicDialog::MusicDialog(QWidget* parent, const QString& name, const QString& selected_music)
	: QDialog(parent,name)
{
	setCaption("Select map music");
	_dia_layout    = new Q3GridLayout(this, 7, 2, 5);

	_cancel_pbut   = new QPushButton("Cancel", this);
	_ok_pbut       = new QPushButton("OK", this);
	_select_label  = new QLabel("Select the music for this map:",this);
	_music_list    = new Q3ListView(this, "tileset_lview", Qt::WStaticContents|Qt::WNoAutoErase);

	//Turn off sorting
	_music_list->setSorting(-1);

	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	_dia_layout->addWidget(_select_label,0,0);
	_dia_layout->addWidget(_music_list,1,0);
	_dia_layout->addWidget(_ok_pbut,2,0);
	_dia_layout->addWidget(_cancel_pbut,2,1);

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
	_music_list->addColumn("Filename");

	// Add music files
	for (unsigned int i = 0; i < music_dir.count(); i++) 
	{
		if (music_dir[i].contains(".ogg"))
		{
			QString file_name=music_dir[i];
			Q3ListViewItem* Item=new Q3ListViewItem(_music_list, file_name);
			if (selected_str.endsWith(file_name) && !selected_str.isEmpty())
				_music_list->setSelected(Item, true);
		} // if .ogg
	} // for i

	// Add "None" option
	Q3ListViewItem* NoneItem = new Q3ListViewItem(_music_list, "None");
	if (selected_str.isEmpty() || selected_str == "None")
		_music_list->setSelected(NoneItem, true);
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

EditorScrollView::EditorScrollView(QWidget* parent, const QString& name, int width,
	int height, TileDatabase* db) : Q3ScrollView(parent, (const char*) name, Qt::WNoAutoErase|Qt::WStaticContents)
{
	_db=db;

	// Set default editing modes.
	_tile_mode  = PAINT_TILE;
	_layer_edit = LOWER_LAYER;
	
	// Create a new map.
	_map = new Grid(viewport(), "Untitled", width, height);
	addChild(_map);

	// Context menu creation.
	_context_menu = new Q3PopupMenu(this);
	// Create the walkability checkboxes and add them to a QVButtonGroup.
	Q3VButtonGroup* checkboxes = new Q3VButtonGroup("Walkability", _context_menu,
		"checkboxes");
	_allwalk_checkbox = new QCheckBox("All", checkboxes, "allwalk_checkbox");
	for (uint32 i = 0; i < 8; i++)
		_walk_checkbox[i] = new QCheckBox(QString("Level %1").arg(i+1), checkboxes,
			QString("walk_checkbox[%1]").arg(i));
	connect(_allwalk_checkbox, SIGNAL(toggled(bool)), this,
		SLOT(_ToggleWalkCheckboxes(bool)));
	connect(_context_menu, SIGNAL(aboutToShow()), this, SLOT(_ContextMenuSetup()));
	connect(_context_menu, SIGNAL(aboutToHide()), this, SLOT(_ContextMenuEvaluate()));
	_context_menu->insertItem("Walkability", checkboxes, NULL);
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
				Q3Table* table = static_cast<Q3Table*> (editor->_ed_tabs->currentPage());

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
					Q3Table* table = static_cast<Q3Table*> (editor->_ed_tabs->currentPage());

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
	unsigned int walkable = 0;
	
	// Individual walkability supersedes everything else
	if (_map->indiv_walkable[_tile_index] != -1)
		walkable = _map->indiv_walkable[_tile_index];
	// Look up walkability property from the map
	else if (_map->tiles_walkable[_tile_index] != -1)
		walkable = _map->tiles_walkable[_tile_index];
	// Look up walkability property in global tiles database
	else
	{
		QString tile_name = _map->file_name_list[
			_map->GetLayer(LOWER_LAYER)[_tile_index]];
		walkable = _db->GetGlobalSet().GetTile(tile_name).walkability;
	}

	// Set checkboxes
	_allwalk_checkbox->setChecked(true);
	for (uint8 i = 0; i < 8; i++)
		if (walkable & (1 << i))
			_walk_checkbox[i]->setChecked(true);
		else
		{
			_allwalk_checkbox->setChecked(false);
			_walk_checkbox[i]->setChecked(false);
		}
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



// ********** Private Functions **********

void EditorScrollView::_RemoveIfUnused(int file_index) 
{
	// find other instances of the tile 
	bool found = false;
	for (LAYER_TYPE layer = LOWER_LAYER;layer <= UPPER_LAYER && !found; layer++)
	{
		vector<int32>::iterator it;
		vector<int32>& CurrentLayer = _map->GetLayer(layer);
		// Loop until we either find something or we are at the end of the vector
		for (it = CurrentLayer.begin(); it != CurrentLayer.end() && *it != file_index; it++);
		if (it != CurrentLayer.end())
			found = true;
	}

	// If tile was not found, remove it from the list
	if (!found)
		_map->file_name_list.removeAt(file_index);
} // _RemoveIfUnused(...)


/************************
  DatabaseDialog class functions follow
************************/

DatabaseDialog::DatabaseDialog(QWidget* parent, const QString& name, TileDatabase* db)
	: Q3TabDialog(parent, (const char*) name)
{
	_db=db;
	_set_modified=false;

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

		// Create a QLabel for a read-only QComboBox (drop-down list) and add all existing tilesets,
		// and connect it to a slot.
		QLabel* tilesets_label   = new QLabel("Tileset to modify:", tilesets_widget, "tilesets_label");
		tilesets_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		_tilesets_cbox = new QComboBox(false, tilesets_widget, "tilesets_cbox");
		_tilesets_cbox->insertItem("Select Tileset...");
		_tilesets_cbox->insertItem("New Tileset");
		for (uint32 i = 0; i < tileset_dir.count(); i++)
		{
			if (tileset_dir[i].contains("tileset") != 0)
				_tilesets_cbox->insertItem(tileset_dir[i].remove("tileset_").remove(".lua"));
		} // looks for tileset files in the tileset directory
		connect(_tilesets_cbox, SIGNAL(activated(const QString&)), this, SLOT(_TilesetsTabPopulateTileset(const QString&)));

		// Create a QLineEdit and a QLabel for it.
		_tileset_ledit        = new QLineEdit(tilesets_widget, "tileset_ledit");
		_tileset_ledit->setEnabled(false);
		QLabel* tileset_label = new QLabel("Tileset Name:", tilesets_widget, "tileset_label");
		tileset_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		// Create QIconViews and set up their properties appropriately.
		_all_tiles   = new Q3IconView(tilesets_widget, "all_tiles_iview");
		_mod_tileset = new Q3IconView(tilesets_widget, "new_tileset_iview");
		_all_tiles->setItemTextPos(Q3IconView::Right);
		_mod_tileset->setItemTextPos(Q3IconView::Right);
		_all_tiles->setSelectionMode(Q3IconView::Single);
		_mod_tileset->setSelectionMode(Q3IconView::Single);
		_all_tiles->setWordWrapIconText(false);
		_mod_tileset->setWordWrapIconText(false);
		_all_tiles->setItemsMovable(false);
		_mod_tileset->setItemsMovable(false);
		_mod_tileset->setSorting(true);
		_all_tiles->setGridX(300);
		_mod_tileset->setGridX(300);
		_mod_tileset->setAcceptDrops(true);
		_mod_tileset->setEnabled(false);
		_all_tiles->setEnabled(false);

		// Populate the QIconView for the global tileset.
		std::list<DbTile> global_set_files=_db->GetGlobalSet().GetTiles();
		for(std::list<DbTile>::const_iterator it=global_set_files.begin(); it!=global_set_files.end(); it++)
		{
			(void) new Q3IconViewItem(_all_tiles, (*it).file_name, QPixmap("img/tiles/" + (*it).file_name));
		}		

		// Create QPushButtons and connect them to their appropriate slots.
		QPushButton* add_tile_pbut = new QPushButton("Add Tile", tilesets_widget);
		QPushButton* del_tile_pbut = new QPushButton("Remove Tile", tilesets_widget);
		connect(add_tile_pbut, SIGNAL(clicked()), this, SLOT(_AddTile()));
		connect(del_tile_pbut, SIGNAL(clicked()), this, SLOT(_DelTile()));

		// Create a QGridLayout and add all the created widgets to it.
		Q3GridLayout* tilesets_tab = new Q3GridLayout(tilesets_widget, 4, 2, 5);
		tilesets_tab->addWidget(tilesets_label, 0, 0);
		tilesets_tab->addWidget(_tilesets_cbox, 0, 1);
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
		_proptsets_cbox  = new QComboBox(false, properties_widget, "proptsets_cbox");
		_proptsets_cbox->insertItem("Select Tileset...");
		for (uint32 i = 0; i < tileset_dir.count(); i++)
		{
			if (tileset_dir[i].contains("tileset") != 0)
				_proptsets_cbox->insertItem(tileset_dir[i].remove("tileset_").remove(".lua"));
		} // looks for tileset files in the tileset directory
		connect(_proptsets_cbox, SIGNAL(activated(const QString&)), this, SLOT(_PropertiesTabPopulateTileset(const QString&)));

		// Create QIconView and set up its properties appropriately.
		_prop_tileset = new Q3IconView(properties_widget, "prop_tileset_iview");
		_prop_tileset->setItemTextPos(Q3IconView::Right);
		_prop_tileset->setSelectionMode(Q3IconView::Single);
		_prop_tileset->setWordWrapIconText(false);
		_prop_tileset->setItemsMovable(false);
		_prop_tileset->setSorting(true);
		_prop_tileset->setGridX(300);
		connect(_prop_tileset, SIGNAL(currentChanged(Q3IconViewItem *)), this, SLOT(_ProcessWalkability(Q3IconViewItem *)));
		_tile_index = 0;    // no changes made yet

		// Create the walkability checkboxes and add them to a QVButtonGroup.
		Q3VButtonGroup* checkboxes = new Q3VButtonGroup("Walkability", properties_widget, "checkboxes");
		_allwalk_checkbox = new QCheckBox("All", checkboxes, "allwalk_checkbox");
		for (uint32 i = 0; i < 8; i++)
			_walk_checkbox[i] = new QCheckBox(QString("Level %1").arg(i+1), checkboxes, QString("walk_checkbox[%1]").arg(i));
		connect(_allwalk_checkbox, SIGNAL(toggled(bool)), this, SLOT(_ToggleWalkCheckboxes(bool)));

		// Create the animation stuff. FIXME
		QLabel* anim_label = new QLabel("Placeholder for animation settings", properties_widget, "anim_label");

		// Create a QGridLayout and add all the created widgets to it.
		Q3GridLayout* properties_tab = new Q3GridLayout(properties_widget, 2, 3, 5);
		properties_tab->addWidget(_proptsets_cbox,  0, 0);
		properties_tab->addWidget(_prop_tileset,   1, 0);
		properties_tab->addWidget(checkboxes,      1, 1);
		properties_tab->addWidget(anim_label,      1, 2);

		// Insert the tab into the dialog window.
		addTab(properties_widget, "Properties");
		
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

void DatabaseDialog::_CreateTileSet()
{
	if(_selected_set)
	{
		TileSet* new_set=new TileSet(_db);
		new_set->SetName(_tileset_ledit->text());
		_SwitchTileset(new_set);

		_tilesets_cbox->setCurrentText(_tileset_ledit->text());
		_tileset_ledit->setEnabled(false);
	}
}

void DatabaseDialog::_UpdateData()
{
	// Save current tileset if necessary.
	if (_set_modified && !_tileset_ledit->text().isNull() && _tileset_ledit->text() != "" && _selected_set != NULL)
	{
		_selected_set->SetName(_tileset_ledit->text());
		_selected_set->Save();
	} // only if the QLineEdit is not empty
	
	// User might change some properties then immediately click Ok.
	_ProcessWalkability(_prop_tileset->currentItem());
	
	// Save tile db
	_db->Save("dat/tilesets/tiles_database.lua");
	
	// ***************************************************
	// End of saving data related to the Properties tab.
	// ***************************************************
} // _UpdateData()

void DatabaseDialog::_AddTile()
{
	_CreateTileSet();

	Q3IconViewItem* CurrentItem=_all_tiles->currentItem();
	//If no item is selected, show a warning
	if(CurrentItem == 0) {
		QMessageBox::warning(this,"Error","No tile selected!");
	}
	// Otherwise: Only add new tile if it doesn't already exist in the new tileset.
	else if (_mod_tileset->findItem(CurrentItem->text(), Q3IconView::ExactMatch) == 0)
	{
		(void) new Q3IconViewItem(_mod_tileset, _all_tiles->currentItem()->text(), *_all_tiles->currentItem()->pixmap());
		_selected_set->AddTile(_all_tiles->currentItem()->text());
		_set_modified=true;
	}
} // _AddTile()

void DatabaseDialog::_DelTile()
{
	_selected_set->RemoveTile(_mod_tileset->currentItem()->text());
	delete _mod_tileset->currentItem();
	_set_modified=true;
} // _DelTile()

void DatabaseDialog::_TilesetsTabPopulateTileset(const QString& name)
{
	if (name != "New Tileset" && name != "Select Tileset...")
	{
		// Fill in the QLineEdit.
		_tileset_ledit->setText(name);

		// Do the populating.
		_PopulateTilesetHelper(_mod_tileset, name);

		_tileset_ledit->setEnabled(false);
		_mod_tileset->setEnabled(true);
		_all_tiles->setEnabled(true);
	} // no populating necessary otherwise
	else
	{
		_tileset_ledit->setText("");
		if (name == "New Tileset")
		{
			_tileset_ledit->setEnabled(true);
			_mod_tileset->setEnabled(true);
			_all_tiles->setEnabled(true);
		}
		else
		{
			_tileset_ledit->setEnabled(false);
			_mod_tileset->setEnabled(false);
			_all_tiles->setEnabled(false);
		}
	}
} // _TilesetsTabPopulateTileset(...)

void DatabaseDialog::_PropertiesTabPopulateTileset(const QString& name)
{
	if (name != "Select Tileset...")
		_PopulateTilesetHelper(_prop_tileset, name);
} // _PropertiesTabPopulateTileset(...)

void DatabaseDialog::_ProcessWalkability(Q3IconViewItem* item)
{
	if (item != NULL)
	{
		if (_selected_item != "")
		{
			DbTile& tile=_selected_set->GetTile(_selected_item);

			int old_walk=tile.walkability;
			// Record the new walkability options.
			tile.walkability = 0;
			for (uint8 i = 0; i < 8; i++)
				if (_walk_checkbox[i]->isChecked())
					tile.walkability |= (1 << i);
			if(tile.walkability!=old_walk)
				_set_modified=true;
		} // must have made some changes in order to record them
		
		DbTile& new_tile=_selected_set->GetTile(item->text());
		_allwalk_checkbox->setChecked(true);
		for (uint8 i = 0; i < 8; i++)
			if (new_tile.walkability & (1 << i))
				_walk_checkbox[i]->setChecked(true);
			else
			{
				_allwalk_checkbox->setChecked(false);
				_walk_checkbox[i]->setChecked(false);
			}
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

void DatabaseDialog::_PopulateTilesetHelper(Q3IconView *tileset, const QString& name)
{
	if (tileset != NULL)
	{
		tileset->clear();
		_SwitchTileset(new TileSet(_db,name));

		std::list<DbTile> tiles=_selected_set->GetTiles();
		for(std::list<DbTile>::const_iterator it=tiles.begin(); it!=tiles.end(); it++)
		{
			(void) new Q3IconViewItem(tileset, (*it).file_name, QPixmap("img/tiles/" + (*it).file_name));
		}		
	} // no populating necessary otherwise
} // _PopulateTilesetHelper()

void DatabaseDialog::_SwitchTileset(TileSet* new_set)
{
	// If the tileset has been modified, ask if it should be saved.
	if(_set_modified && _selected_set)
	{
		int ret = QMessageBox::question(this, "Tileset has been changed",
			"Do you want to save your changes?", QMessageBox::Yes, QMessageBox::No);
		if (ret == QMessageBox::Yes)
			_selected_set->Save();
	} // must not be NULL

	_selected_set.reset(new_set);
	_set_modified=false;
} // _SwitchTileset(...)
