////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2013 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    editor.h
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for the map editor main window
*** ***************************************************************************/

#ifndef __EDITOR_HEADER__
#define __EDITOR_HEADER__

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QErrorMessage>
#include <QFileDialog>
#include <QLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProgressDialog>
#include <QSplitter>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QUndoCommand>

#include "editor_utils.h"
#include "dialogs.h"
#include "map_data.h"
#include "map_view.h"
#include "tile_layer.h"
#include "tileset_editor.h"

//! \brief All editor code is contained within this namespace.
namespace hoa_editor {

/** ****************************************************************************
*** \brief The main window of the editor program and the top-level widget
***
*** This class is responsible for creating the application menus and toolbars and
*** processing the actions when those items are selected. As the top-level widget,
*** it is also responsible for the creation and layout of all lower-level widgets.
*** Many of the user actions processed by this class will make calls into the
*** appropriate sub widget to reflect the changes.
***
*** \todo Add a section here listing the subwidgets that are created and how they
*** are layed out within the application window.
***
*** \todo Add save/restore state information for the QSplitter objects used by this
*** class so that the editor remembers the size that the user last left the editor
*** window at. This information should be saved to a Lua file called "editor_state.lua"
*** or something similar.
*** ***************************************************************************/
class Editor : public QMainWindow {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	Editor();

	~Editor();

	MapData* GetMapData()
		{ return &_map_data; }

	QTabWidget* GetTilesetTabs() const
		{ return _tileset_tabs; }

	//! \brief Used to update the map view visuals when map data has been modified
	void UpdateMapView()
		{ _map_view->DrawMap(); }

protected:
	/** \brief Handles close and/or quit events
	*** \param event Pointer to a QCloseEvent object
	**/
	void closeEvent(QCloseEvent* event)
		{ _FileQuit(); }

private:
	//! \brief Contains all data for the open map file and methods for manipulating that data
	MapData _map_data;

	//! \brief The toolbar at the top of the window containing icons for various edit options
	QToolBar* _tiles_toolbar;

	//! \brief Splits the widget into two horizontal sections
	QSplitter* _horizontal_splitter;

	//! \brief Splits the right horizontal section into two vertical sections
	QSplitter* _right_vertical_splitter;

	//! \brief The left sub-widget containing the editable map area
	MapView* _map_view;

	//! \brief Tabbed widget that contains each tileset open
	QTabWidget* _tileset_tabs;

	//! \brief Widget used to display and edit the ordered list of all tile layers
	LayerView* _layer_view;

	//! \brief Widget used to display and edit the properties of map contexts
	ContextView* _context_view;

	//! \brief The stack that contains the undo and redo operations.
	QUndoStack* _undo_stack;

	//! \brief An error dialog for exceeding the maximum allowable number of contexts.
	QErrorMessage* _error_max_contexts;

	/** \name Application Menus
	*** \brief Represent the various top-level menus found in the menu bar
	**/
	//{@
	QMenu* _file_menu;
	QMenu* _view_menu;
	QMenu* _tiles_menu;
	QMenu* _map_menu;
	QMenu* _help_menu;
	QMenu* _tileset_menu;
	QMenu* _script_menu;
	//@}

	/** \name Application Menu Actions
	*** \brief Actions that are executed when particular menus are selected
	***
	*** These are Qt's way of associating the same back-end functionality to occur whether a user
	*** invokes a menu through the menu bar, a keyboard shortcut, a toolbar button, or other means.
	**/
	//{@
	QAction* _new_action;
	QAction* _open_action;
	QAction* _save_action;
	QAction* _save_as_action;
	QAction* _close_action;
	QAction* _quit_action;

	QAction* _toggle_grid_action;
	QAction* _coord_tile_action;
	QAction* _coord_collision_action;

	QAction* _undo_action;
	QAction* _redo_action;
	QAction* _layer_fill_action;
	QAction* _layer_clear_action;
	QAction* _toggle_select_action;
	QAction* _mode_paint_action;
	QAction* _mode_move_action;
	QAction* _mode_delete_action;
	QActionGroup* _mode_group;

	QAction* _edit_tileset_action;

	QAction* _map_properties_action;

	QAction* _help_action;
	QAction* _about_action;
	QAction* _about_qt_action;
	//@}

	//! \brief Helper function to the class constructor which creates actions for use by menus and toolbars
	void _CreateActions();

	//! \brief Helper function to the class constructor which creates the menus
	void _CreateMenus();

	//! \brief Helper function to the class constructor which creates the toolbars
	void _CreateToolbars();

	/** \brief Called whenever an operation occurs that could discard unsaved map data
	*** \return False if the user cancelled the operation that would cause the data to be discarded
	***
	*** The options that are present to the user include: save the map data, discard the map data, or cancel the operation
	*** that caused this dialog to be invoked. A return value of true means that the user either saved or intentionally
	*** discarded the data.
	**/
	bool _UnsavedDataPrompt();

private slots:
	/** \name Menu Toolbar Setup Slots
	*** \brief Used to gray out items in the menus and toolbars
	**/
	//{@
	void _FileMenuSetup();
	void _ViewMenuSetup();
	void _TilesMenuSetup();
	void _TilesetMenuSetup();
	void _MapMenuSetup();
	//@}

	/** \name File Menu Item Slots
	*** \brief Process user selection for items in the File menu
	**/
	//{@
	void _FileNew();
	void _FileOpen();
	void _FileSave();
	void _FileSaveAs();
	void _FileClose();
	void _FileQuit();
	//@}

	/** \name View Menu Item Slots
	*** \brief Process user selection for items in the View menu
	**/
	//{@
	void _ViewToggleGrid();
	//@}

	/** \name Tiles Menu Item Slots
	*** \brief Process user selection for items in the Tiles menu
	**/
	//{@
	void _TileLayerFill();
	void _TileLayerClear();
	void _TileToggleSelect();
	void _TileModePaint();
	void _TileModeMove();
	void _TileModeDelete();
	//@}

	/** \name Tileset Menu Item Slots
	*** \brief Process user selection for items in the Tileset menu
	**/
	//{@
	void _TilesetEdit();
	//@}

	/** \name Map Menu Item Slots
	*** \brief Process user selection for items in the Map menu
	**/
	//{@
	void _MapProperties();
	//@}

	/** \name Help Menu Item Slots
	*** \brief Process user selection for items in the Help menu
	**/
	//{@
	void _HelpHelp();
	void _HelpAbout();
	void _HelpAboutQt();
	//@}
}; // class Editor : public QMainWindow


/** ****************************************************************************
*** \brief Holds the previous state of map tiles during editing, used for undo/redo actions
***
*** \todo This entire class needs to be revisited and modified so that it can handle operations
*** such as map resizing, multi layer changes, and multi context changes.
*** ***************************************************************************/
class LayerCommand : public QUndoCommand {
	// Needed for accessing the current map's layers.
	friend class Editor;

public:
	LayerCommand(std::vector<int32> indeces, std::vector<int32> previous, std::vector<int32> modified, int context, Editor* editor,
		const QString& text = "Layer Operation", QUndoCommand* parent = 0);

	//! \name Undo Functions
	//! \brief Reimplemented from the QUndoCommand class to provide specific undo/redo capability towards the map.
	//{@
	void undo();
	void redo();
	//@}

private:
	//! \name Tile Vectors
	//! \brief The following three vectors are used to know how to perform undo and redo operations
	//!        for this command. They should be the same size and one-to-one. So, the j-th element
	//!        of each vector should correspond to the j-th element of the other vectors.
	//{@
	std::vector<int32> _tile_indeces;  //! A vector of tile indeces in the map that were modified by this command.
	std::vector<int32> _previous_tiles;//! A vector of indeces into tilesets of the modified tiles before they were modified.
	std::vector<int32> _modified_tiles;//! A vector of indeces into tilesets of the modified tiles after they were modified.
	//@}

	//! A record of the active context when this command was performed.
	int _context;

	//! A reference to the main window so we can get the current map.
	Editor* _editor;
}; // class LayerCommand : public QUndoCommand

} // namespace hoa_editor

#endif // __EDITOR_HEADER__
