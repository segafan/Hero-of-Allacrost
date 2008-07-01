///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    tileset_editor.cpp
*** \author  Bar�� Soner U�akl? blackkknight@hotmail.com
*** \brief   Source file for editor's tileset editor dialog
*******************************************************************************/

#include "tileset_editor.h"

using namespace hoa_editor;
using namespace hoa_script;
using namespace hoa_video;

////////////////////////////////////////////////////////////////////////////////
////////// OverlayGrid class
////////////////////////////////////////////////////////////////////////////////

OverlayGrid::OverlayGrid()
{
	tileset = new TilesetTable();
	tileset->tiles.resize(1);
	_overlayInitialized = false;
}



OverlayGrid::~OverlayGrid()
{
	delete tileset;
	VideoManager->SingletonDestroy();
}



void OverlayGrid::initializeGL()
{
	// Destroy and recreated the video engine
	VideoManager->SingletonDestroy();
	VideoManager = GameVideo::SingletonCreate();
	VideoManager->SetTarget(VIDEO_TARGET_QT_WIDGET);

	VideoManager->SingletonInitialize();
	// changed because allacrost had to delay some video loading code

	VideoManager->ApplySettings();
	VideoManager->FinalizeInitialization();
	VideoManager->ToggleFPS();
}

void OverlayGrid::paintGL()
{
	VideoManager->SetCoordSys(0.0f, VideoManager->GetScreenWidth() / TILE_WIDTH,
		VideoManager->GetScreenHeight() / TILE_HEIGHT, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);

	VideoManager->Clear(Color::blue);

	// Draw the tile as one big image
	tileset->tiles[0].Draw();
	VideoManager->DrawGrid(0.0f, 0.0f, 0.5f, 0.5f, Color::black);
}



void OverlayGrid::resizeGL(int w,int h)
{
	VideoManager->SetResolution(512, 512);
	VideoManager->ApplySettings();
}







void OverlayGrid::contentsMousePressEvent(QMouseEvent* evt)
{
	// don't draw outside the map
	//if ((evt->y() / TILE_HEIGHT) >= _map->GetHeight() ||
	//	(evt->x() / TILE_WIDTH)  >= _map->GetWidth())
	//	return;

	//TODO : Check for mouse press events and paint walkability

} // contentsMousePressEvent(...)

////////////////////////////////////////////////////////////////////////////////
////////// TilesetEditor class
////////////////////////////////////////////////////////////////////////////////

TilesetEditor::TilesetEditor(QWidget* parent, const QString& name, bool prop)
	: QDialog(parent, (const char*)name)
{
	setCaption("Tileset Definition File Editor");

	// Create GUI Items
	_opentileset_pbut = new QPushButton("Open",this);
	_cancel_pbut = new QPushButton("Cancel", this);
	_ok_pbut = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);

	// Create the overlay grid
	_walkability_grid = new OverlayGrid;
	_walkability_grid->resize(512,512);
	_walkability_grid->setFixedWidth(512);
	_walkability_grid->setFixedHeight(512);

	// connect button signals
	connect(_ok_pbut, SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));
	connect(_opentileset_pbut, SIGNAL(clicked()), this, SLOT(_openTDF()));

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
	// Get the filename to open through the OpenFileName dialog
	QString file_name = QFileDialog::getOpenFileName(this, "HoA Level Editor -- File Open",
		"dat/tilesets", "Tilesets (*.lua)");

	// File name will contain only the name of the tileset.
	int i = file_name.lastIndexOf("/");
	file_name = file_name.remove(0, i + 1);
	file_name.chop(4);

	QString img_filename = QString("img/tilesets/" + file_name + ".png");
	QString dat_filename = QString("dat/tilesets/" + file_name + ".lua");

	// Load the tileset image.
// 	_walkability_grid->tileset->Load(file_name);
	_walkability_grid->tileset->tiles.clear();
	_walkability_grid->tileset->tiles.resize(1);

	_walkability_grid->tileset->tiles[0].SetDimensions(16.0f, 16.0f);
	// Load image as still image
	_walkability_grid->tileset->tiles[0].Load(std::string(img_filename.toAscii()),16,16);

	_walkability_grid->updateGL();
	//TODO : Load the tileset definition file
	//walkability autotiling etc.
}



