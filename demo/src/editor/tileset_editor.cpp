///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
* \file    tileset_editor.cpp
* \author  Barýþ Soner Uþaklý, blackkknight@hotmail.com
* \brief   Source file for editor's tileset editor dialog
*****************************************************************************/
#include "tileset_editor.h"

using namespace hoa_editor;

TilesetEditor::TilesetEditor(QWidget* parent,const QString& name,bool prop)
: QDialog(parent, (const char*) name)
{
	setCaption("Tileset Definition File Editor");
	
		
	_cancel_pbut   = new QPushButton("Cancel", this);
	_ok_pbut       = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);
	// TODO: connect the OK push button to a signal and slot such that it is
	// disabled if no tilesets are selected
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));
	
	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	
	_dia_layout->addWidget(_cancel_pbut, 6, 1);
	_dia_layout->addWidget(_ok_pbut, 6, 0);

}
TilesetEditor::~TilesetEditor()
{
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _dia_layout;
}

