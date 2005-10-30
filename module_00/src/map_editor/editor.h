///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2004, 2005 by The Allacrost Project
// All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef __EDITOR_HEADER__
#define __EDITOR_HEADER__

#include "grid.h"
#include "tileset.h"

#include <qapplication.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qinputdialog.h>
//#include <qlayout.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qsettings.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qstringlist.h>

//class QPopupMenu;
//class QString;
//class QStringList;

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

class Editor: public QMainWindow
{
	Q_OBJECT	// macro needed to use QT's slots and signals
	
	public:
		Editor();				// constructor
		~Editor();				// destructor
		
		// maximum number of recently used files to keep track of
		// not sure why this is enum
		enum { MAX_RECENTFILES = 5 };

		friend class Grid;		// needed for "painting" tiles

	protected:
		// Handles close and/or quit events, reimplemented from QMainWindow
		virtual void closeEvent(QCloseEvent *);
		// not sure if I need this:
		//void resizeEvent(QResizeEvent *); where does this go?
	
	private slots:
		// the following slot is used to gray out items in file the menu
		void _FileMenuSetup();
		
		//void _SaveOptions();	// saves the list of most recently used files

		// the following slots are used in the file menu
		void _FileNew();
		void _FileOpen();
	//	void _FileOpenRecent(int index);
		void _FileSaveAs();
		void _FileSave();
		void _FileResize();
		void _FileQuit();
				
		// the following slots are used in the view menu
	//	void viewToggleGrid();
		
		// the following slots are used in the help menu
		void _HelpHelp();
		void _HelpAbout();
		void _HelpAboutQt();
		
	private:
		void _Load(const QString &file_name);	// loads a map
		void _TileInit();		// loads the tiles for drag 'n' drop
		bool _EraseOK();		// saves the map if it is unsaved

		// updates the list of recently used files with fileName
		//void updateRecentFiles(const QString &fileName);
		
		// updates the list of recently used files in the File menu
		//void updateRecentFilesMenu();
		
		QPopupMenu* _file_menu;	// this is used for the File menu
	//	QPopupMenu *viewMenu;	// this is used for the View menu
		QPopupMenu* _help_menu;	// this is used for the Help menu

		QStatusBar* _stat_bar;	// this is used to display messages
	//	QStringList _master_recent_files;	// list of recently used files
		Tileset* _tiles;		// iconview of tiles
		Grid* _map;				// current working map
}; // class Editor

} // namespace hoa_editor

#endif
// __EDITOR_HEADER__
