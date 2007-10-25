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



//! OverlayGrid Implementation
OverlayGrid::OverlayGrid()
{
	tileset = new Tileset;		
	tileset->tiles.resize(1);
}

OverlayGrid::~OverlayGrid()
{
	delete tileset;
}
void OverlayGrid::paintGL()
{
	VideoManager->SetCoordSys(0.0f, VideoManager->GetScreenWidth() / TILE_WIDTH,
		VideoManager->GetScreenHeight() / TILE_HEIGHT, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	VideoManager->Clear(Color::white);
	
	
	
	
	
	
	tileset->tiles[0].Draw();
	
	
	
	
	
	
	VideoManager->DrawGrid(0.0f, 0.0f, 0.5f, 0.5f, Color::black);

}
void OverlayGrid::resizeGL(int w,int h)
{
	VideoManager->SetResolution(512, 512);
	VideoManager->ApplySettings();
}
void OverlayGrid::initializeGL()
{
	VideoManager->SetTarget(VIDEO_TARGET_QT_WIDGET);
	VideoManager->SingletonInitialize();
	VideoManager->ToggleFPS();
}


//! TilesetEditor Implementation

TilesetEditor::TilesetEditor(QWidget* parent,const QString& name,bool prop)
: QDialog(parent, (const char*) name)
{
	setCaption("Tileset Definition File Editor");
	
	//Create GUI Items
	_opentileset_pbut = new QPushButton("Open",this);
	_cancel_pbut   = new QPushButton("Cancel", this);
	_ok_pbut       = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);
	

	//Create the overlay grid
	_walkability_grid = new OverlayGrid;
	_walkability_grid->resize(512,512);
	_walkability_grid->setFixedWidth(512);
	_walkability_grid->setFixedHeight(512);

	// connect button signals
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));
	connect(_opentileset_pbut,SIGNAL(clicked()),this,SLOT(_openTDF()));
	
	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	
	_dia_layout->addWidget(_opentileset_pbut, 0, 1);
	_dia_layout->addWidget(_ok_pbut, 1, 1);
	_dia_layout->addWidget(_cancel_pbut, 2, 1);
	_dia_layout->addWidget(_walkability_grid,0,0,3,1);
	
	
}

TilesetEditor::~TilesetEditor()
{
	delete _opentileset_pbut;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _dia_layout;

	delete _walkability_grid;
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
	_walkability_grid->tileset->tiles.resize(1);
	
	_walkability_grid->tileset->tiles[0].SetDimensions(16.0f, 16.0f);
	if (ImageDescriptor::LoadMultiImageFromElementGrid(_walkability_grid->tileset->tiles, std::string(img_filename.toAscii()), 1, 1) == false)
		qDebug("LoadMultiImage failed to load tileset " + img_filename);
	
	
	
	//TODO : Load the tileset definition file
	//walkability autotiling etc.
}



