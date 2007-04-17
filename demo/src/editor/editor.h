///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <Q3FileDialog>
#include <QGLWidget>
#include <Q3IconView>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <Q3ListView>
#include <Q3MainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <Q3PopupMenu>
#include <QPushButton>
#include <Q3ScrollView>
#include <QSpinBox>
#include <QStatusBar>
#include <QStringList>
#include <Q3TabDialog>
#include <QTabWidget>
#include <QContextMenuEvent>
#include <QCloseEvent>
#include <Q3GridLayout>
#include <QMouseEvent>
#include <Q3VBoxLayout>
#include <Q3VButtonGroup>

#include <map>

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

class EditorScrollView;

class Editor: public Q3MainWindow
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
		//@}

		//! \name Map Menu Item Slots
		//! \brief These slots process selection for their item in the Map menu.
		//{@
		void _MapSelectMusic();
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
		//! Sets current edit mode
		void _SetEditMode(TILE_MODE_TYPE new_mode);
		//! Sets currently edited layer
		void _SetEditLayer(LAYER_TYPE new_layer);		

		//! This is used to represent the File menu.
		Q3PopupMenu* _file_menu;
		//! This is used to represent the View menu.
		Q3PopupMenu* _view_menu;
		//! This is used to represent the Tiles menu.
		Q3PopupMenu* _tiles_menu;
		//! This is used to represent the Tiles menu.
		Q3PopupMenu* _map_menu;
		//! This is used to represent the Help menu.
		Q3PopupMenu* _help_menu;

		//! This is used to display status messages.
		QStatusBar*       _stat_bar;
		//! Tabbed widget of tilesets.
		QTabWidget*       _ed_tabs;
		//! Used to add scrollbars to the QGLWidget of the map.
		EditorScrollView* _ed_scrollview;
		//! Main window layout.
		Q3BoxLayout*       _ed_layout;
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

		//! Edit layer items in Tile menu
		std::map<LAYER_TYPE, int> _layer_ids;
		//! Mode items in Tile menu
		std::map<TILE_MODE_TYPE, int> _mode_ids;
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
		Q3ListView* GetTilesetListView() const { return _tileset_lview; }

	private:
		//! A listview for showing all available tilesets.
		Q3ListView* _tileset_lview;
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
		Q3GridLayout* _dia_layout;
}; // class NewMapDialog

class MusicDialog: public QDialog
{
public:
	MusicDialog(QWidget* parent, const QString& name, const QString& selected_music);
	~MusicDialog();

	QString GetSelectedFile();
private:
	//! A pushbutton for canceling the new map dialog.
	QPushButton* _cancel_pbut;
	//! A pushbutton for okaying the new map dialog.
	QPushButton* _ok_pbut;
	//! Label telling you to select some music
	QLabel* _select_label;
	//! A layout to manage all the labels, spinboxes, and listviews.
	Q3GridLayout* _dia_layout;
	//! List with all music files
	Q3ListView* _music_list;

	//! Puts music files in the QListView and selects the specified file.
	//! \param selected_str - this file will be selected
	void _PopulateMusicList(const QString& selected_str);
}; // class MusicDialog

class EditorScrollView: public Q3ScrollView
{
	//! Macro needed to use Qt's slots and signals.
	Q_OBJECT 
	
	public:
		EditorScrollView(QWidget* parent, const QString& name, int width, int height);
		~EditorScrollView();

		//! Resizes the map.
		//! \param width Width of the map.
		//! \param height Height of the map.
		void Resize(int width, int height); 

		//! Gets currently edited layer
		std::vector<int32>& GetCurrentLayer();		

		//! Needed for changing the editing mode and painting.
		friend class Editor;

	protected:
		//! \name Mouse Processing Functions
		//! \brief Functions to process mouse events on the map.
		//! \note Reimplemented from QScrollView.
		//! \param evt A pointer to the QMouseEvent generated by the mouse.
		//{@
		void contentsMousePressEvent(QMouseEvent *evt);
		void contentsMouseMoveEvent(QMouseEvent *evt);
		void contentsMouseReleaseEvent(QMouseEvent *evt);
//		void contentsContextMenuEvent(QContextMenuEvent *evt);
		//@}

	private slots:
/*		//! \name Context Menu Slots
		//! \brief These slots are used to correctly setup and process the context
		//!        menu, which pops up on right-clicks of the mouse on the map.
		//{@
		void _ContextMenuSetup();
		void _ContextMenuEvaluate();
		//@}
*/
	private:
		//! Current working map.
		Grid *_map;
		//! Current tile edit mode being used.
		TILE_MODE_TYPE _tile_mode;
		//! Current layer being edited.
		LAYER_TYPE _layer_edit;
		//! Mouse is at this tile index on the map.
		int _tile_index;
		//! Menu used on right-clicks of the mouse on the map.
		//Q3PopupMenu *_context_menu;

		//! Stores source index of moved tiles
		int _move_source_index;
}; // class EditorScrollView

} // namespace hoa_editor

#endif
// __EDITOR_HEADER__
