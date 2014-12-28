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
*** it is also responsible for the creation and layout of all lower-level widgets
*** as well as holding the active instance of the map data. Several of the actions
*** that a user takes that are processed by this class will make calls into the
*** appropriate sub widget to reflect the changes.
***
*** The MapView widget, which represents the viewble and editable area of the map,
*** is located on the left side of the window. The right side of the window contains
*** three widgets. From top to bottom, they are: the LayerView widget, ContextView widget,
*** and TilesetTab widget.
***
*** \todo Add save/restore state information for the QSplitter objects used by this
*** class so that the editor remembers the size that the user last left the editor
*** window at. This information should be saved to a Lua file called "editor_state.lua"
*** or something similar.
***
*** \todo In the File menu, add a "Recent Files >" action with a submenu below the Open action.
*** The submenu should contain around 5 files maximum along with a "Clear Recent Files" option.
*** Mimic the way this is done in the Tiled map editor.
*** ***************************************************************************/
class Editor : public QMainWindow {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	Editor();

	~Editor();

	//! \brief Returns a pointer to the map data
	MapData* GetMapData()
		{ return &_map_data; }

	QTabWidget* GetTilesetTabs() const
		{ return _tileset_tabs; }

	//! \brief Used by other subwidgets to update the map view when the map data has been modified external to it
	void UpdateMapView()
		{ _map_view->DrawMap(); }

private:
	//! \brief Contains all data for the open map file and methods for manipulating that data
	MapData _map_data;

	//! \brief The toolbar at the top of the window containing icons for various edit options
	QToolBar* _tiles_toolbar;

	//! \brief Splits the widget into two horizontal sections
	QSplitter* _horizontal_splitter;

	//! \brief Splits the right horizontal section into three vertical sections
	QSplitter* _right_vertical_splitter;

	//! \brief The left sub-widget containing the editable map area
	MapView* _map_view;

	//! \brief Widget used to display and edit the ordered list of all tile layers
	LayerView* _layer_view;

	//! \brief Widget used to display and edit the properties of map contexts
	ContextView* _context_view;

	//! \brief Tabbed widget that contains each tileset open
	QTabWidget* _tileset_tabs;

	//! \brief The stack that contains the undo and redo operations.
	QUndoStack* _undo_stack;

	/** \name Application Menus
	*** \brief The top-level menus found in the menu bar: File, Edit, View, Tools, Help
	**/
	//{@
	QMenu* _file_menu;
	QMenu* _edit_menu;
	QMenu* _view_menu;
	QMenu* _tools_menu;
	QMenu* _help_menu;
	//@}

	/** \name Application Menu Actions
	*** \brief Actions that are executed when particular menus are selected
	***
	*** These are Qt's way of associating the same back-end functionality to occur whether a user
	*** invokes a menu through the menu bar, a keyboard shortcut, a toolbar button, or other means.
	*** They are organized in the order in which they appear in the application menus.
	**/
	//{@
	QAction* _new_action;
	QAction* _open_action;
	QAction* _save_action;
	QAction* _save_as_action;
	QAction* _close_action;
	QAction* _quit_action;

	QAction* _undo_action;
	QAction* _redo_action;
	QAction* _cut_action;
	QAction* _copy_action;
	QAction* _paste_action;
	QAction* _tileset_properties_action;
	QAction* _map_properties_action;

	QAction* _view_grid_action;
	QAction* _view_collisions_action;

	QAction* _edit_mode_paint_action;
	QAction* _edit_mode_move_action;
	QAction* _edit_mode_erase_action;
	QAction* _edit_fill_action;
	QAction* _edit_clear_action;
	QAction* _toggle_select_action;

	QAction* _help_action;
	QAction* _about_action;
	QAction* _about_qt_action;
	//@}

	//! \brief Used to group the various edit modes together so that only one may be active at a given time
	QActionGroup* _edit_mode_action_group;

	//! \brief Handles close and quit events
	void closeEvent(QCloseEvent* event)
		{ _FileQuit(); }

	//! \brief Creates actions for use by menus, toolbars, and keyboard shortcuts
	void _CreateActions();

	//! \brief Creates the main menus
	void _CreateMenus();

	//! \brief Creates the main toolbar
	void _CreateToolbars();

	/** \brief Sets the editor to its default state for editing mode, checkboxes, and so on
	*** This is called whenever the application starts, a new map is created, or an existing map is loaded
	**/
	void _ClearEditorState();

	/** \brief Called whenever an operation occurs that could discard unsaved map data
	*** \return False if the user cancelled the operation that would cause the data to be discarded
	***
	*** The options that are present to the user include: save the map data, discard the map data, or cancel the operation
	*** that caused this dialog to be invoked. A return value of true means that the user either saved or intentionally
	*** discarded the data.
	**/
	bool _UnsavedDataPrompt();

private slots:
	/** \name Action Setup Slots
	*** \brief Used to enable or disable actions in the menus and toolbars based on the current state of the editor
	**/
	//{@
	void _CheckFileActions();
	void _CheckEditActions();
	void _CheckViewActions();
	void _CheckToolsActions();
	//@}

	/** \name Action Execution Slots
	*** \brief Processes the various actions commanded by the user
	**/
	//{@
	void _FileNew();
	void _FileOpen();
	void _FileSave();
	void _FileSaveAs();
	void _FileClose();
	void _FileQuit();

	void _CutSelection();
	void _CopySelection();
	void _PasteSelection();
	void _EditTilesetProperties();
	void _EditMapProperties();

	void _ViewTileGrid();
	void _ViewCollisionData();

	void _SelectPaintMode();
	void _SelectMoveMode();
	void _SelectEraseMode();
	void _FillSelection();
	void _ClearSelection();
	void _ToggleSelectArea();

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
