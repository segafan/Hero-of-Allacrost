/******************************************************************************
 *
 *	Hero of Allacrost MapEditor class
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Filename: map_editor.h
 *
 *	$Id$
 *
 *	Description: This class constructs and manipulates a map editor used for
 *               editing Hero of Allacrost maps. 
 *
 *****************************************************************************/

#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include "map.h"
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
#include <qstringlist.h>

class QPopupMenu;
class QString;
class QStringList;

namespace hoa_map
{

class MapEditor: public QMainWindow
{
	Q_OBJECT	// macro needed to use QT's slots and signals
	
	public:
		MapEditor();		// constructor
		~MapEditor();		// destructor
		
		// maximum number of recently used files to keep track of
		// not sure why this is enum
		enum { MAX_RECENTFILES = 5 };
	
	protected:
		// Handles close and/or quit events, reimplemented from QMainWindow
		virtual void closeEvent(QCloseEvent *);
		// not sure if I need this:
		//void resizeEvent(QResizeEvent *); where does this go?
	
	private slots:
		void saveOptions();		// saves the list of most recently used files

		// the following slots are used in the file menu
		void fileNew();
		void fileOpen();
		void fileOpenRecent(int index);
		void fileSaveAs();
		void fileSave();
		void fileQuit();
				
		// the following slots are used in the view menu
		void viewToggleGrid();
		
		// the following slots are used in the help menu
		void helpHelp();
		void helpAbout();
		void helpAboutQt();
		
	private:
		void load(const QString &fileName);  // loads a map
		void tileInit();  // loads the tiles for drag 'n' drop
		
		// updates the list of recently used files with fileName
		void updateRecentFiles(const QString &fileName);
		
		// updates the list of recently used files in the File menu
		void updateRecentFilesMenu();
		
		QPopupMenu *fileMenu;	// this is used for the File menu
		QPopupMenu *viewMenu;	// this is used for the View menu
		QPopupMenu *helpMenu;	// this is used for the Help menu

		QStatusBar *statBar;	// this is used to display messages
		QStringList masterRecentFiles;	// list of recently used files
		Tileset *tiles;		// iconview of tiles
		Map *map;			// current working map
}; // class MapEditor

} // namespace hoa_map

#endif
// MAPEDITOR_H
