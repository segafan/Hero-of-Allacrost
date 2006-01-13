///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2004, 2005 by The Allacrost Project
// All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include "editor.h"

using namespace std;
using namespace hoa_data;
using namespace hoa_utils;
using namespace hoa_editor;

const QString APP_KEY = "/map_editor/";

Editor::Editor() : QMainWindow(0, 0, WDestructiveClose)
{
	// Create and initialize the GameData singleton
	DataManager = GameData::Create();
	DataManager = GameData::GetReference();
	if (!DataManager->Initialize())
	{
		cerr << "ERROR: unable to initialize DataManager" << endl;
		_FileQuit();
	} // Make sure DataManager is properly initialized.
	
	// create the main widget and tile icons
	QSplitter *split = new QSplitter(this);
	_tiles = new Tileset(split);
	_map = new Grid(split);

	setCentralWidget(split);
	split->show();
	resize(600, 400);

	// create the statusbar
	_stat_bar = new QStatusBar(this);
	
	// file menu creation
	_file_menu = new QPopupMenu(this);
	connect(_file_menu, SIGNAL(aboutToShow()), this, SLOT(_FileMenuSetup()));
	menuBar()->insertItem("&File", _file_menu);
	
	// help menu creation
	_help_menu = new QPopupMenu(this);
	menuBar()->insertItem("&Help", _help_menu);
	_help_menu->insertItem("&Help", this, SLOT(_HelpHelp()), Key_F1);
	_help_menu->insertItem("&About", this, SLOT(_HelpAbout()));
	_help_menu->insertItem("About &Qt", this, SLOT(_HelpAboutQt()));
	
	// FIXME: these settings don't seem to be working...? hmm
/*	QSettings settings;		// contains saveable settings, like recent files
	QString filename;		// file to add to list of recently used files
	for (int i = 0; i < MAX_RECENTFILES; ++i)
	{
		filename = settings.readEntry(APP_KEY + "File" + 
			QString::number(i + 1));
		if (!filename.isEmpty())
			masterRecentFiles.push_back(filename);
	} // puts saved files into a list of recently used files

	// updates the recent files list in the File menu
	if (masterRecentFiles.count())
		updateRecentFilesMenu();
*/	
	// loads the tileset for drag 'n' drop operation
	_TileInit();
	
	// FIXME: opens the most recently used file
//	if (!filename.isEmpty())
//		load(filename);
} // Editor constructor

Editor::~Editor()
{
	// Destroy GameData singleton
	GameData::Destroy();
	if (_map != NULL)
		delete _map;
	if (_tiles != NULL)
		delete _tiles;
} // Editor destructor

void Editor::closeEvent(QCloseEvent *)
{
    _FileQuit();
} // closeEvent(...)

void Editor::_FileMenuSetup()
{
	_file_menu->clear();
	_file_menu->insertItem("&New...", this, SLOT(_FileNew()), CTRL+Key_N);
	_file_menu->insertItem("&Open...", this, SLOT(_FileOpen()), CTRL+Key_O);
	int save_id = _file_menu->insertItem("&Save", this, SLOT(_FileSave()),
		CTRL+Key_S);
	int save_as_id = _file_menu->insertItem("Save &As...", this,
		SLOT(_FileSaveAs()));
	_file_menu->insertSeparator();
	int resize_id = _file_menu->insertItem("&Resize Map...", this,
		SLOT(_FileResize()));
	_file_menu->insertSeparator();
	_file_menu->insertItem("&Quit", this, SLOT(_FileQuit()), CTRL+Key_Q);

	if (_map->GetChanged())
		_file_menu->setItemEnabled(save_id, true);
	else
		_file_menu->setItemEnabled(save_id, false);

	if (_map->canvas() == NULL)
	{
		_file_menu->setItemEnabled(save_as_id, false);
		_file_menu->setItemEnabled(resize_id, false);
	}
	else
	{
		_file_menu->setItemEnabled(save_as_id, true);
		_file_menu->setItemEnabled(resize_id, true);
	}
} // _FileMenuSetup()

void Editor::_FileNew()
{
	if (_EraseOK())
	{
		QCanvas *canvas = new QCanvas(this);
		_map->setCanvas(canvas);
		_FileResize();
	} // make sure an unsaved map is not lost
} // _FileNew()

void Editor::_FileOpen()
{
	if (_EraseOK())
	{
		// file to open
		QString file_name = QFileDialog::getOpenFileName(
			"dat/maps", "Maps (*.lua)", this, "file open",
			"HoA Level Editor -- File Open");

		if (!file_name.isEmpty())
		{
			_map->SetFileName(file_name);
			_map->LoadMap();
			_stat_bar->message(QString("Opened \'%1\'").
				arg(_map->GetFileName()), 5000);
//			_UpdateRecentFiles(file_name);  <-- FIXME
		} // file must exist in order to open it
	} // make sure an unsaved map is not lost
} // _FileOpen()
/*
void Editor::_FileOpenRecent(int index)
{
FIXME
	_Load(masterRecentFiles[index]);
} // _FileOpenRecent(...)
*/
void Editor::_FileSaveAs()
{
	// Get the file name from the user.
	QString file_name = QFileDialog::getSaveFileName(
		"dat/maps", "Maps (*.lua)", this, "file save",
		"HoA Level Editor -- File Save");
		
	if (!file_name.isEmpty())
	{
		int answer = 0;		// button pressed by user
		
		// Ask to overwrite existing file.
		if (QFile::exists(file_name))
			answer = QMessageBox::warning(
				this, "Overwrite File",
				QString("Overwrite\n\'%1\'?" ).arg(file_name),
				"&Yes", "&No", QString::null, 1, 1);
				
		if (answer == 0)
		{
			_map->SetFileName(file_name);
//			_UpdateRecentFiles(file_name);  <-- FIXME
			_FileSave();
			return;
		} // Save the file.
    } // Make sure the file name is not blank.
	
    _stat_bar->message("Save abandoned.", 5000);
} // _FileSaveAs()

void Editor::_FileSave()
{
	if (_map->GetFileName().isEmpty() || _map->GetFileName() == "Untitled")
	{
		_FileSaveAs();
		return;
    } // gets a file name if it is blank

    _map->SaveMap();	// actually saves the map
    setCaption(QString("%1").arg(_map->GetFileName()));
   	_stat_bar->message(QString("Saved \'%1\' successfully!").
		arg(_map->GetFileName()), 5000);
} // _FileSave()

void Editor::_FileResize()
{
	bool ok_wpressed, ok_hpressed;	// TRUE = user pressed OK, else FALSE
	
	// get map width from user
	int width = QInputDialog::getInteger("Map Size...",
		"Enter map width (in tiles):", 0, 0, 1000, 1, &ok_wpressed, this);

	// get map height from user
	int height = QInputDialog::getInteger("Map Size...",
		"Enter map height (in tiles):", 0, 0, 1000, 1, &ok_hpressed, this);

	if (ok_wpressed && ok_hpressed)
	{
		_map->canvas()->resize(width * TILE_WIDTH, height * TILE_HEIGHT);
		_map->CreateGrid();
		_map->SetWidth(width);
		_map->SetHeight(height);
	} // only if the user entered both the height and the width
	else
		_stat_bar->message("Invalid dimensions, no map created",5000);
} // _FileResize()

void Editor::_FileQuit()
{
	if (_EraseOK())
	{	
		//_SaveOptions();		// FIXME: saves window settings
		qApp->exit(0);
	} // checks to see if the map is unsaved
} // _FileQuit()

bool Editor::_EraseOK()
{
    if (_map->GetChanged())
	{
		switch(QMessageBox::warning(this, "Unsaved File",
			"The document contains unsaved changes\n"
			"Do you want to save the changes before proceeding?",
			"&Save", "&Discard", "Cancel",
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
        		return FALSE;
	    } // warn the user to save
    } // map has been modified

    return TRUE;
} // _EraseOK()

/*void Editor::updateRecentFiles(const QString &fileName)
{
FIXME
    if (masterRecentFiles.find(fileName) == masterRecentFiles.end())
	{
		masterRecentFiles.push_back(fileName);
		if (masterRecentFiles.count() > MAX_RECENTFILES)
			masterRecentFiles.pop_front();
		updateRecentFilesMenu();
	} // the file must not already be in the list of recently used files
} // updateRecentFiles(...)

void Editor::updateRecentFilesMenu()
{
FIXME
    for (int i = 0; i < MAX_RECENTFILES; ++i)
	{
		if (fileMenu->findItem(i))
			fileMenu->removeItem(i);
		if (i < int(masterRecentFiles.count()))
			fileMenu->insertItem(QString("&%1 %2").
				arg(i + 1).arg(masterRecentFiles[i]),
				this, SLOT(fileOpenRecent(int)), 0, i );
    } // loops through the whole list, updating if necessary
} // updateRecentFilesMenu()

void Editor::saveOptions()
{
FIXME
    QSettings settings;		// contains saveable settings, like recent files
	
	// saves the list of recently used files
    for (int i = 0; i < int(masterRecentFiles.count()); ++i)
		settings.writeEntry(APP_KEY + "File" + QString::number(i + 1),
			masterRecentFiles[i]);
} // saveOptions()

void Editor::viewToggleGrid()
{
FIXME
	// toggles the map's grid on or off
	map->viewToggleGrid();
} // viewToggleGrid()
*/
void Editor::_HelpHelp()
{
	_stat_bar->message(QString("Put URL to online editor docs here"), 5000);
} // _HelpHelp()

void Editor::_HelpAbout()
{
    QMessageBox::about(this, "HoA Level Editor -- About",
		"<center><h1><font color=blue>Hero of Allacrost Level Editor<font>"
		"</h1></center>"
		"<center><h2><font color=blue>Copyright (c) 2004<font></h2></center>"
		"<p>A level editor created for the Hero of Allacrost project."
		" See 'http://www.allacrost.org/' for more details</p>");
} // _HelpAbout()

void Editor::_HelpAboutQt()
{
    QMessageBox::aboutQt(this, "HoA Level Editor -- About Qt");
} // _HelpAboutQt()

void Editor::_TileInit()
{
	QDir tile_dir("img/tiles/", "*.png");			// tile set directory
	
	// make sure directory exists
	if (!tile_dir.exists())
        	qWarning("Cannot find the tile directory");
	
	for (int i = 0; i < tile_dir.count(); i++)
		(void) new QIconViewItem(_tiles, tile_dir[i],
			QPixmap("img/tiles/" + tile_dir[i]));

	_map->temp = _tiles;		// FIXME: zOMG what a hack this is
} // _TileInit()
