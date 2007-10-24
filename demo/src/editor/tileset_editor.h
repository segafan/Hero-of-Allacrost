///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
* \file    tileset_editor.h
* \author  Barýþ Soner Uþaklý, blackkknight@hotmail.com
* \brief   Header file for editor's tileset editor dialog
*****************************************************************************/


#ifndef __TILESET_EDITOR_HEADER__
#define __TILESET_EDITOR_HEADER__

#include "script.h"
#include "tileset.h"
#include "video.h"

#include <QDialog>
#include <QMessageBox>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>
#include <vector>
using std::vector;

namespace hoa_editor
{

class TilesetEditor : public QDialog
{
	//! Macro needed to use Qt's slots and signals.
	Q_OBJECT

public:
	//! \name TilesetEditor constructor
	//! \brief A constructor for the TilesetEditor class.This class is used to modify the tileset
	//! \definition files through an interface.
	//! \param parent The widget from which this dialog was invoked.
	//! \param name The name of this widget.
	//! \param prop True when accessing an already loaded map's properties, false otherwise.
	TilesetEditor(QWidget* parent,const QString& name,bool prop);
	~TilesetEditor();

private slots:
	//! Loads a TDF file
	void _openTDF();
private :
	//! A pushbutton for opening a new tileset
	QPushButton* _opentileset_pbut;
	//! A pushbutton for canceling the tileset_editor
	QPushButton* _cancel_pbut;
	//! A pushbutton for okaying the tileset_editor
	QPushButton* _ok_pbut;
	//! A layout to manage all the labels, spinboxes, and listviews.
	QGridLayout* _dia_layout;

	//! A label to show the tileset
	QLabel*	_tileset_label;

	//! The current tileset that is loaded and being edited.
	Tileset*	_current_tileset;


};


} // namespace hoa_editor

#endif 
// __TILESET_EDITOR_HEADER__

