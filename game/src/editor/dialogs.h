///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    dialogs.h
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \brief   Header file for all of the editor's dialog windows
*** **************************************************************************/

#ifndef __DIALOGS_HEADER__
#define __DIALOGS_HEADER__

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QSpinBox>
#include <QTreeWidget>

#include "map_data.h"

namespace hoa_editor {

/** ***************************************************************************
*** \brief A dialog window that allows the user to modify some properties of an existing map.
***
*** The properties that may be modified through this dialog include the following:
*** - The map dimensions (in tiles)
*** - Which tilesets are used by this map
*** **************************************************************************/
class MapPropertiesDialog : public QDialog {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The widget from which this dialog was invoked.
	*** \param name The name of this widget.
	*** \param prop True when accessing an already loaded map's properties, false otherwise.
	***
	*** This class is used in two instances. For presenting a dialog to the user to (1) create a new map,
	*** or (2) modify the properties of an already existing map (such as height, length, or tilesets loaded in the bottom
	*** portion of the editor). For case #1, the parameter prop is false, and for case #2, it is true.
	**/
	MapPropertiesDialog(QWidget* parent, const QString& name, bool prop);

	~MapPropertiesDialog();

	//! \name Class member accessor functions
	//@{
	uint32 GetHeight() const
		{ return _height_sbox->value(); }

	uint32 GetLength() const
		{ return  _length_sbox->value(); }

	QTreeWidget* GetTilesetTree() const
		{ return _tileset_tree; }
	//@}

private slots:
	//! \brief Enables or disables the OK push button of this dialog depending on whether any tilesets are checked or not.
	void _EnableOKButton();

private:
	//! \brief A tree for showing all available tilesets
	QTreeWidget* _tileset_tree;

	//! \brief A spinbox for specifying the map's height
	QSpinBox* _height_sbox;

	//! \brief A spinbox for specifying the map's length
	QSpinBox* _length_sbox;

	//! \brief A label used to visually name the height spinbox
	QLabel* _height_label;

	//! \brief A label used to visually name the length spinbox
	QLabel* _length_label;

	//! \brief A pushbutton for canceling the new map dialog.
	QPushButton* _cancel_pbut;

	//! \brief A pushbutton for okaying the new map dialog.
	QPushButton* _ok_pbut;

	//! \brief A layout to manage all the labels, spinboxes, and listviews.
	QGridLayout* _dia_layout;
}; // class MapPropertiesDialog : public QDialog


/** ***************************************************************************
*** \brief Allows the user to resize the map by adding or removing rows and columns from its end
***
*** This dialog allows the user to specify the new height and length of the map. New rows and columns
*** are either added or removed from the right and bottom sides.
***
*** \todo This class needs to be enhanced in the future. Instead of the current format, the user should
*** be able to select the new height and length of the map and an x/y offset that determines where rows
*** and columns are added and removed. Refer to the Tiled map editor's "Resize Map" menu option for how
*** this should be done.
*** **************************************************************************/
class MapResizeDialog : public QDialog {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The widget from which this dialog was invoked
	*** \param data A pointer to the active map data
	**/
	MapResizeDialog(QWidget* parent, MapData* data);

	~MapResizeDialog();

	//! \brief Makes the changes to the map data and redraws the map
	void ModifyMapData();

private:
	//! \brief A pointer to the active map data containing the list of open tilesets
	MapData* _map_data;

	//! \brief Spinboxes that allow the user to specify the new dimensions of the map
	//@{
	QSpinBox* _height_spinbox;
	QSpinBox* _length_spinbox;
	//@}

	//! \brief Labels used to identify the height or length segements of the dialog
	//@{
	QLabel* _height_title;
	QLabel* _length_title;
	//@}

	//! \brief Labels used to specify the number of rows or columns that will be added or deleted
	//@{
	QLabel* _height_change;
	QLabel* _length_change;
	//@}

	//! \brief A button to confirm the resize operation
	QPushButton* _ok_button;

	//! \brief A button for cancelling the resize operation
	QPushButton* _cancel_button;

	//! \brief Defines the layout of all widgets in the dialog window
	QGridLayout* _grid_layout;

private slots:
	//! \brief Processes changes in height to update the _height_change label
	void _HeightChanged();

	//! \brief Processes changes in height to update the _height_change label
	void _LengthChanged();
}; // class MapResizeDialog : public QDialog


/** ***************************************************************************
*** \brief A dialog window that allows the user to insert or delete multiple rows or columns of tiles from a chosen location
***
***
*** **************************************************************************/
class MapResizeInternalDialog : public QDialog {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The widget from which this dialog was invoked
	*** \param data A pointer to the active map data
	*** \param start_row The starting tile row coordinate for the operation
	*** \param start_column The starting tile column coordinate for the operation
	*** \param insert_operation If true, this widget should be inserting rows and columns. If false, it will be deleting them
	***
	**/
	MapResizeInternalDialog(QWidget* parent, MapData* data, uint32 start_row, uint32 start_column, bool insert_operation);

	~MapResizeInternalDialog();

private:
	//! \brief A pointer to the active map data containing the list of open tilesets
	MapData* _map_data;

	//! \brief A spinbox for specifying the number of rows
	QSpinBox* _row_spinbox;

	//! \brief A spinbox for specifying the map's length
	QSpinBox* _column_spinbox;

	//! \brief A label used to visually name the row spinbox
	QLabel* _row_label;

	//! \brief A label used to visually name the column spinbox
	QLabel* _column_label;

	//! \brief A button to confirm the insert/delete operation
	QPushButton* _ok_button;

	//! \brief A button for cancelling the insert/delete operation
	QPushButton* _cancel_button;

	//! \brief Defines the layout of all widgets in the dialog window
	QGridLayout* _widget_layout;

private slots:
	//! \brief Used to determine whether the Ok button should be enabled based on the values of the spinbox widgets
	void _EnableOkButton();
}; // class MapResizeInternalDialog : public QDialog


/** ***************************************************************************
*** \brief A dialog window that allows the user to add additional tilesets to a map
***
*** This presents the user with a list of all available tilesets that can be added to
*** the map. Tilesets which are already loaded and in use by the map are also shown, but
*** they are greyed out and the user can not interact with them. The user can add more than
*** one tileset to the map at a time with this widget.
*** **************************************************************************/
class AddTilesetsDialog : public QDialog {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The widget from which this dialog was invoked
	*** \param data A pointer to the active map data
	**/
	AddTilesetsDialog(QWidget* parent, MapData* data);

	~AddTilesetsDialog();

	/** \brief Adds the tilesets selected by the user to the map data
	*** \return The number of tilesets that were added
	***
	*** This should be called only after the user clicks the "Add" button. It may generate error message dialogs to the user if any
	*** of the tilesets failed to load.
	**/
	uint32 AddTilesetsToMapData();

private:
	//! \brief A pointer to the active map data containing the list of open tilesets
	MapData* _map_data;

	//! \brief A tree for showing all available tilesets
	QTreeWidget* _tileset_tree;

	//! \brief A button to confirm adding the new tilesets
	QPushButton* _add_button;

	//! \brief A button for cancelling the add operation
	QPushButton* _cancel_button;

	//! \brief Defines the layout of all widgets in the dialog window
	QGridLayout* _widget_layout;

private slots:
	//! \brief Enables or disables the add push button of this dialog depending on whether any tilesets are selected
	void _EnableAddButton();
}; // class AddTilesetsDialog : public QDialog

} // namespace hoa_editor

#endif // __DIALOGS_HEADER__
