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
using namespace hoa_script;
using namespace hoa_video;


TilesetEditor::TilesetEditor(QWidget* parent,const QString& name,bool prop)
: QDialog(parent, (const char*) name)
{
	setCaption("Tileset Definition File Editor");
	
	//Create current tileset
	_current_tileset = new Tileset;	
	
	//Create GUI Items
	_opentileset_pbut = new QPushButton("Open",this);
	_cancel_pbut   = new QPushButton("Cancel", this);
	_ok_pbut       = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);
	_tileset_label = new QLabel("Open a tileset...",this);

	_tileset_label->resize(512,512);
	
	// connect button signals
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));
	connect(_opentileset_pbut,SIGNAL(clicked()),this,SLOT(_openTDF()));
	
	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	
	_dia_layout->addWidget(_opentileset_pbut, 0, 1);
	_dia_layout->addWidget(_ok_pbut, 1, 1);
	_dia_layout->addWidget(_cancel_pbut, 2, 1);
	_dia_layout->addWidget(_tileset_label,0,0,3,1);
	
	

}

TilesetEditor::~TilesetEditor()
{
	delete _opentileset_pbut;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _dia_layout;

	delete _current_tileset;
}

void TilesetEditor::_openTDF()
{

	//get the filename to open through the OpenFileName dialog
	QString file_name = QFileDialog::getOpenFileName(this, "HoA Level Editor -- File Open",
		"dat/tilesets", "Tilesets (*.lua)");

	//file name will contain only the name of the tileset.
	int i = file_name.lastIndexOf("/");
	file_name = file_name.remove(0,i+1);
	file_name.chop(4);
	
	QString img_filename = QString("img/tilesets/" + file_name + ".png");
	QString dat_filename = QString("dat/tilesets/" + file_name + ".lua");
	

	// Load the tileset image.
	_current_tileset->tiles.resize(1);
	
	_current_tileset->tiles[0].SetDimensions(16.0f, 16.0f);
	if (ImageDescriptor::LoadMultiImageFromElementGrid(_current_tileset->tiles, std::string(img_filename.toAscii()), 1, 1) == false)
		qDebug("LoadMultiImage failed to load tileset " + img_filename);
	
	
	QRect rectangle;
	QImageReader reader(img_filename, "png");
	rectangle.setRect(0, 0, 512, 512);
	reader.setClipRect(rectangle);
	QImage tile_img = reader.read();
	QVariant variant = tile_img;
	if (!tile_img.isNull())
	{
		QPixmap tile_pixmap = variant.value<QPixmap>();
		_tileset_label->setPixmap(tile_pixmap);
	} // image of the tile must not be null
	else
		qDebug(QString("%1").arg(reader.error()));


	//TODO : Load the tileset definition file
	//walkability autotiling etc.
}


