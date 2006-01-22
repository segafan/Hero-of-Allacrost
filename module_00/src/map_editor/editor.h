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
#include <qdialog.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qstringlist.h>

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

class Editor: public QMainWindow
{
	Q_OBJECT    // macro needed to use Qt's slots and signals
	
	public:
		Editor();				// constructor
		~Editor();				// destructor
		
		friend class Grid;		// needed for "painting" tiles

	protected:
		// Handles close and/or quit events, reimplemented from QMainWindow
		void closeEvent(QCloseEvent *);
		//void resizeEvent(QResizeEvent *); FIXME: where does this go?
	
	private slots:
		// the following slot is used to gray out items in file the menu
		void _FileMenuSetup();
		
		// the following slots are used in the file menu
		void _FileNew();
		void _FileOpen();
		void _FileSaveAs();
		void _FileSave();
		void _FileResize();
		void _FileQuit();
				
		// the following slots are used in the help menu
		void _HelpHelp();
		void _HelpAbout();
		void _HelpAboutQt();
		
	private:
		void _TileInit();		// loads the tiles for drag 'n' drop
		bool _EraseOK();		// saves the map if it is unsaved

		QPopupMenu* _file_menu;	// this is used for the File menu
		QPopupMenu* _help_menu;	// this is used for the Help menu

		QStatusBar* _stat_bar;	// this is used to display messages
		Tileset* _tiles;		// iconview of tiles
		Grid* _map;				// current working map
}; // class Editor

class SizeDialog: public QDialog
{
	Q_OBJECT    // macro needed to use Qt's slots and signals
	
	public:
		SizeDialog(QWidget* parent, const QString& name);   // constructor
		~SizeDialog();                                      // destructor

		int GetHeight() const { return height_sbox->value(); }
		int GetWidth()  const { return  width_sbox->value(); }

	private:
		QSpinBox* height_sbox;
		QSpinBox* width_sbox;

		QLabel* height_label;
		QLabel* width_label;

		QPushButton* cancel_pbut;
		QPushButton* ok_pbut;

		QGridLayout* grid_lay;
}; // class SizeDialog

} // namespace hoa_editor

#endif
// __EDITOR_HEADER__
