///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    editor.h
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Header file for editor's main window and user interface.
 *****************************************************************************/
			   
#ifndef __EDITOR_HEADER__
#define __EDITOR_HEADER__

#include "grid.h"
#include "tileset.h"

#include <qapplication.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qgl.h>
#include <qiconview.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qspinbox.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtabdialog.h>
#include <qtabwidget.h>
#include <qvbuttongroup.h>

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

//! Different tile editing modes follow.
enum TILE_MODE_TYPE
{
	INVALID_TILE = -1,
	PAINT_TILE   = 0,
	MOVE_TILE    = 1,
	DELETE_TILE  = 2,
	TOTAL_TILE   = 3
};

//! Different layer editing modes follow.
enum LAYER_EDIT_TYPE
{
	INVALID_LAYER = -1,
	LOWER_LAYER   = 0,
	MIDDLE_LAYER  = 1,
	UPPER_LAYER   = 2,
	TOTAL_LAYER   = 3
};

class EditorScrollView;

class Editor: public QMainWindow
{
	//! Macro needed to use Qt's slots and signals.
	Q_OBJECT
	
	public:
		Editor();                       // constructor
		~Editor();                      // destructor

		//! Needed for tile editing.
		friend class EditorScrollView;

	protected:
		//! Handles close and/or quit events.
		//! \note Reimplemented from QMainWindow.
		//! \param QCloseEvent* A pointer to a Qt close event.
		void closeEvent(QCloseEvent*);
		//void resizeEvent(QResizeEvent*); FIXME: do I need this?
	
	private slots:
		//! This slot is used to gray out items in the File menu.
		void _FileMenuSetup();

		//! \name File Menu Item Slots
		//! \brief These slots process selection for their item in the File menu.
		//{@
		void _FileNew();
		void _FileOpen();
		void _FileSaveAs();
		void _FileSave();
		void _FileResize();
		void _FileQuit();
		//@}

		//! \name View Menu Item Slots
		//! \brief These slots process selection for their item in the View menu.
		//{@
		void _ViewToggleGrid();
		void _ViewToggleLL();
		void _ViewToggleML();
		void _ViewToggleUL();
		//@}

		//! \name Tiles Menu Item Slots
		//! \brief These slots process selection for their item in the Tiles menu.
		//{@
		void _TileLayerFill();
		void _TileLayerClear();
		void _TileModePaint();
		void _TileModeMove();
		void _TileModeDelete();
		void _TileEditLL();
		void _TileEditML();
		void _TileEditUL();
		void _TileDatabase();
		//@}

		//! \name Help Menu Item Slots
		//! \brief These slots process selection for their item in the Help menu.
		//{@
		void _HelpHelp();
		void _HelpAbout();
		void _HelpAboutQt();
		//@}
		
	private:
		//! Saves the map if it is unsaved.
		bool _EraseOK();
		//! Sets up the tile database.
		void _CreateTileDatabase();
		//! Adds tiles to the database.
		void _GenerateDatabase();

		//! This is used to represent the File menu.
		QPopupMenu* _file_menu;
		//! This is used to represent the View menu.
		QPopupMenu* _view_menu;
		//! This is used to represent the Tiles menu.
		QPopupMenu* _tiles_menu;
		//! This is used to represent the Help menu.
		QPopupMenu* _help_menu;

		//! This is used to display status messages.
		QStatusBar*       _stat_bar;
		//! Tabbed widget of tilesets.
		QTabWidget*       _ed_tabs;
		//! Used to add scrollbars to the QGLWidget of the map.
		EditorScrollView* _ed_scrollview;
		//! Main window layout.
		QBoxLayout*       _ed_layout;
		//! Needed for _ed_layout for it's children.
		QWidget*          _ed_widget;

		//! Grid item in View menu.
		int  _grid_id;
		//! Lower layer item in View menu.
		int  _ll_id;
		//! Middle layer item in View menu.
		int  _ml_id;
		//! Upper layer item in View menu.
		int  _ul_id;
		//! Grid toggle view switch.
		bool _grid_on;
		//! Lower layer toggle view switch.
		bool _ll_on;
		//! Middle layer toggle view switch.
		bool _ml_on;
		//! Upper layer toggle view switch.
		bool _ul_on;
}; // class Editor

class NewMapDialog: public QDialog
{
	public:
		NewMapDialog(QWidget* parent, const QString& name);   // constructor
		~NewMapDialog();                                      // destructor

		//! Public accessor to get the map height from the height spinbox.
		int GetHeight() const { return _height_sbox->value(); }
		//! Public accessor to get the map width from the width spinbox.
		int GetWidth()  const { return  _width_sbox->value(); }
		//! Public accessor to get the listview containing checkable tilesets.
		QListView* GetTilesetListView() const { return _tileset_lview; }

	private:
		//! A listview for showing all available tilesets.
		QListView* _tileset_lview;
		//! A spinbox for specifying the map's height.
		QSpinBox* _height_sbox;
		//! A spinbox for specifying the map's width.
		QSpinBox* _width_sbox;
		//! A label used to visually name the height spinbox.
		QLabel* _height_label;
		//! A label used to visually name the width spinbox.
		QLabel* _width_label;
		//! A pushbutton for canceling the new map dialog.
		QPushButton* _cancel_pbut;
		//! A pushbutton for okaying the new map dialog.
		QPushButton* _ok_pbut;
		//! A layout to manage all the labels, spinboxes, and listviews.
		QGridLayout* _dia_layout;
}; // class NewMapDialog

class EditorScrollView: public QScrollView
{
	//! Macro needed to use Qt's slots and signals.
	Q_OBJECT 
	
	public:
		EditorScrollView(QWidget* parent, const QString& name, int width,
			int height);                     // constructor
		~EditorScrollView();                 // destructor

		//! Resizes the map.
		//! \param width Width of the map.
		//! \param height Height of the map.
		void Resize(int width, int height); 

		//! Needed for changing the editing mode and painting.
		friend class Editor;

	protected:
		//! \name Mouse Processing Functions
		//! \brief Functions to process mouse events on the map.
		//! \note Reimplemented from QScrollView.
		//! \param evt A pointer to the QMouseEvent generated by the mouse.
		//{@
		void contentsMousePressEvent(QMouseEvent* evt);
		void contentsMouseMoveEvent(QMouseEvent* evt);
		void contentsMouseReleaseEvent(QMouseEvent* evt);
		void contentsMouseDoubleClickEvent(QMouseEvent* evt);
		//@}

	private:
		//! Current working map.
		Grid* _map;
		//! Current tile edit mode being used.
		TILE_MODE_TYPE  _tile_mode;
		//! Current layer being edited.
		LAYER_EDIT_TYPE _layer_edit;
		//! Mouse is at this tile index on the map.
		int _tile_index;
}; // class EditorScrollView

class DatabaseDialog: public QTabDialog
{
	//! Macro needed to use Qt's slots and signals.
	Q_OBJECT 

	public:
		DatabaseDialog(QWidget* parent, const QString& name);    // constructor
		~DatabaseDialog();                                       // destructor

	private slots:
		//! Writes out/updates all modifications to the tile database.
		void _UpdateData();
		//! Adds a tile to a tileset.
		void _AddTile();
		//! Deletes a tile from a tileset.
		void _DelTile();
		//! Draws the tileset specified onto the window in the Tilesets tab.
		//! \note Also modifies the QLineEdit in the Tilesets tab.
		//! \param name Name of the tileset to draw.
		void _TilesetsTabPopulateTileset(const QString& name);
		//! Draws the tileset specified onto the window in the Properties tab.
		//! \param name Name of the tileset to draw.
		void _PropertiesTabPopulateTileset(const QString& name);
		//! Reads and saves the walkable checkboxes according to the current
		//! selection of the QIconView tileset in the Properties tab.
		//! \param item Selected item in the tileset to check walkability.
		void _ProcessWalkability(QIconViewItem *item);
		//! Uses a single checkbox to toggle the remaining walkable checkboxes.
		//! \param on True if the single checkbox is on, False otherwise.
		void _ToggleWalkCheckboxes(bool on);

	private:
		//! Populates the tileset specified by one of the 2 PopulateTileset slots.
		//! \param tileset Pointer to the tileset to draw.
		//! \param name Name of the tileset to draw.
		void _PopulateTilesetHelper(QIconView *tileset, const QString& name);
		
		//! Lists all available tiles to create new tileset in the Tilesets tab.
		QIconView* _all_tiles; 
		//! Lists tiles added to new/modified tileset in the Tilesets tab.
		QIconView* _mod_tileset; 
		//! Previously selected tile in the tileset of the Properties tab.
		QIconViewItem* _prev_ivitem; 
		//! Lists tiles in selected tileset in the Properties tab.
		QIconView* _prop_tileset; 
		//! Stores index into _tile_properties of previously selected tile in
		//! tileset in the Properties tab.
		uint32 _tile_index; 
		//! Vector of global tile properties from tiles_database.lua. Used in the
		//! Properties tab.
		std::vector<uint8> _tile_properties; 
		//! Editable name of the tileset in the Tilesets tab.
		QLineEdit* _tileset_ledit; 
		//! A checkbox capable of toggling all the other walkable checkboxes of the Properties tab.
		QCheckBox* _allwalk_checkbox;
		//! Array of walkability checkboxes of selected tile in the tileset of the Properties tab.
		QCheckBox* _walk_checkbox[8];
}; // class DatabaseDialog

} // namespace hoa_editor

#endif
// __EDITOR_HEADER__
