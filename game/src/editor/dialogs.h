///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
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
#include <QTreeWidget>

#include "utils.h"

#include "editor.h"

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

} // namespace hoa_editor

#endif // __DIALOGS_HEADER__
