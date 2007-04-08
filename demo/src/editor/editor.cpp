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

Editor::Editor() : Q3MainWindow(0, 0, Qt::WDestructiveClose)
{
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

	// set the window icon
	setWindowIcon(QIcon("img/logos/program_icon.bmp"));
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
			_ed_tabs->setTabPosition(QTabWidget::South);

			Q3CheckListItem* tiles = static_cast<Q3CheckListItem*> (new_map->GetTilesetListView()->firstChild());
			vector<StillImage> temp(256);
			while (tiles)
			{
				if (tiles->isOn())
				{
					TilesetTable* table = new TilesetTable(_ed_widget, tiles->text());
					_ed_tabs->addTab(table, tiles->text());
					_ed_scrollview->_map->tileset_names.append(tiles->text());
					temp = table->tiles;
					_ed_scrollview->_map->tileset_tiles.push_back(temp);
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
			_ed_scrollview = new EditorScrollView(_ed_widget, "map", 0, 0);

			if (_ed_tabs != NULL)
				delete _ed_tabs;
			_ed_tabs = new QTabWidget(_ed_widget);
			_ed_tabs->setTabPosition(QTabWidget::South);

			_ed_layout->addWidget(_ed_scrollview);
			_ed_layout->addWidget(_ed_tabs);
			_ed_scrollview->show();

			_ed_scrollview->_map->SetFileName(file_name);
			_ed_scrollview->_map->LoadMap();

			for (QStringList::ConstIterator it = _ed_scrollview->_map->tileset_names.begin();
				it != _ed_scrollview->_map->tileset_names.end(); it++)
				_ed_tabs->addTab(new TilesetTable(_ed_widget, *it), *it);
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
		_ed_scrollview->_map->tileset_names.clear();
		while (tiles)
		{
			if (tiles->isOn())
			{
				_ed_tabs->addTab(new TilesetTable(_ed_widget, tiles->text()), tiles->text());
				_ed_scrollview->_map->tileset_names.push_back(tiles->text());
			} // tileset must be selected
			tiles = static_cast<Q3CheckListItem*> (tiles->nextSibling());
		} // iterate through all possible tilesets
	
		_ed_layout->addWidget(_ed_tabs);
		_ed_tabs->show();
	} // only if the user pressed OK
	else
		_stat_bar->message("Map not resized!", 5000);

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
	TilesetTable* table = static_cast<TilesetTable*> (this->_ed_tabs->currentPage());

	// put selected tile from tileset into tile array at correct position
	int tileset_index = table->currentRow() * 16 + table->currentColumn();
	int multiplier = _ed_scrollview->_map->tileset_names.findIndex(table->tileset_name);
	if (multiplier == -1)
	{
		_ed_scrollview->_map->tileset_names.append(table->tileset_name);
		multiplier = _ed_scrollview->_map->tileset_names.findIndex(table->tileset_name);
	} // calculate index of current tileset

	vector<int32>::iterator it;    // used to iterate over an entire layer
	vector<int32>& CurrentLayer = _ed_scrollview->GetCurrentLayer();
	for (it = CurrentLayer.begin(); it != CurrentLayer.end(); it++)
		*it = tileset_index + multiplier * 256;
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
		"<center><h2><font color=blue>Copyright (c) 2004-2007<font></h2></center>"
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


/************************
  NewMapDialog class functions follow
************************/

NewMapDialog::NewMapDialog(QWidget* parent, const QString& name)
	: QDialog(parent, (const char*) name)
{
	setCaption("Map Properties...");

	_dia_layout    = new Q3GridLayout(this, 7, 2, 5);
	
	_height_label  = new QLabel("Height (in tiles):", this);
	_height_sbox   = new QSpinBox(this);
	_height_sbox->setMinimum(24);
	_height_sbox->setMaximum(1000);
	
	_width_label   = new QLabel(" Width (in tiles):", this);
	_width_sbox    = new QSpinBox(this);
	_width_sbox->setMinimum(32);
	_width_sbox->setMaximum(1000);
	
	_tileset_lview = new Q3ListView(this, "tileset_lview", Qt::WStaticContents|Qt::WNoAutoErase);
	
	_cancel_pbut   = new QPushButton("Cancel", this);
	_ok_pbut       = new QPushButton("OK", this);
	
	_cancel_pbut->setDefault(true);
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	QDir tileset_dir("img/tilesets");
	_tileset_lview->addColumn("Tilesets");
//	Q3CheckListItem* global = new Q3CheckListItem(_tileset_lview, "Global", Q3CheckListItem::CheckBox);
//	global->setOn(true);
	for (uint32 i = 0; i < tileset_dir.count(); i++)
	{
//		if (tileset_dir[i].contains("tileset") != 0)
//		{
			(void) new Q3CheckListItem(_tileset_lview, tileset_dir[i].remove(".png"), Q3CheckListItem::CheckBox);
//		}
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

EditorScrollView::EditorScrollView(QWidget* parent, const QString& name, int width, int height)
	: Q3ScrollView(parent, (const char*) name, Qt::WNoAutoErase|Qt::WStaticContents)
{
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
	_walk_checkbox[0] = new QCheckBox(QString("NW corner"), checkboxes, QString("walk_checkbox[0]"));
	_walk_checkbox[1] = new QCheckBox(QString("NE corner"), checkboxes, QString("walk_checkbox[1]"));
	_walk_checkbox[2] = new QCheckBox(QString("SW corner"), checkboxes, QString("walk_checkbox[2]"));
	_walk_checkbox[3] = new QCheckBox(QString("SE corner"), checkboxes, QString("walk_checkbox[3]"));
	connect(_allwalk_checkbox, SIGNAL(toggled(bool)), this, SLOT(_ToggleWalkCheckboxes(bool)));
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
				TilesetTable* table = static_cast<TilesetTable*> (editor->_ed_tabs->currentPage());

				// put selected tile from tileset into tile array at correct position
				int tileset_index = table->currentRow() * 16 + table->currentColumn();
				int multiplier = _map->tileset_names.findIndex(table->tileset_name);
				if (multiplier == -1)
				{
					_map->tileset_names.append(table->tileset_name);
					multiplier = _map->tileset_names.findIndex(table->tileset_name);
				} // calculate index of current tileset
				
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
				int file_index = GetCurrentLayer()[_tile_index];

				// delete the tile
				GetCurrentLayer()[_tile_index] = -1;

				// No longer needed
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
					TilesetTable* table = static_cast<TilesetTable*> (editor->_ed_tabs->currentPage());

					// put selected tile from tileset into tile array at correct position
					int tileset_index = table->currentRow() * 16 + table->currentColumn();
					int multiplier = _map->tileset_names.findIndex(table->tileset_name);
					if (multiplier == -1)
					{
						_map->tileset_names.append(table->tileset_name);
						multiplier = _map->tileset_names.findIndex(table->tileset_name);
					} // calculate index of current tileset

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
					int file_index = GetCurrentLayer()[_tile_index];

					// delete the tile
					GetCurrentLayer()[_tile_index] = -1;

					//_RemoveIfUnused(file_index);
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
} // contentsContextMenuEvent(...)



// ********** Private slots **********

void EditorScrollView::_ContextMenuSetup()
{
	unsigned int walkable = 0;
	
	// Individual walkability supersedes everything else
	if (_map->indiv_walkable[_tile_index] != 0)
		walkable = _map->indiv_walkable[_tile_index];
	// Look up walkability property from the map
	else if (_map->tiles_walkable[_tile_index] != 0)
		walkable = _map->tiles_walkable[_tile_index];
	// Look up walkability property in global tiles database
	else
	{
	// FIXME
//		QString tile_name = _map->file_name_list[
//			_map->GetLayer(LOWER_LAYER)[_tile_index]];
//		walkable = _db->GetGlobalSet().GetTile(tile_name).walkability;
	}

	// Set checkboxes
	_allwalk_checkbox->setChecked(true);
	for (uint8 i = 0; i < 3; i++)
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
	for (uint8 i = 0; i < 3; i++)
		if (_walk_checkbox[i]->isChecked())
			_map->indiv_walkable[_tile_index] |= (1 << i);
} // _ContextMenuEvaluate()

void EditorScrollView::_ToggleWalkCheckboxes(bool on)
{
	if (on)
		for (uint8 i = 0; i < 3; i++)
			_walk_checkbox[i]->setChecked(true);
	else
		for (uint8 i = 0; i < 3; i++)
			_walk_checkbox[i]->setChecked(false);
} // _ToggleWalkCheckboxes(...)



// ********** Private Functions **********

void EditorScrollView::_RemoveIfUnused(int file_index) 
{
	// find other instances of the tile 
	bool found = false;
	for (LAYER_TYPE layer = LOWER_LAYER; layer <= UPPER_LAYER && !found; layer++)
	{
		vector<int32>::iterator it;
		vector<int32>& current_layer = _map->GetLayer(layer);
		// Loop until we either find something or we are at the end of the vector
		for (it = current_layer.begin(); it != current_layer.end() && *it != file_index; it++);
			if (it != current_layer.end())
				found = true;
	}

	// If tile was not found, remove it from the list
	//if (!found)
		//_map->file_name_list.removeAt(file_index);
} // _RemoveIfUnused(...)

