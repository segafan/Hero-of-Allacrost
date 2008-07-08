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

using namespace hoa_script;
using namespace hoa_video;

namespace hoa_editor
{

////////////////////////////////////////////////////////////////////////////////
////////// TilesetDisplay class
////////////////////////////////////////////////////////////////////////////////

TilesetDisplay::TilesetDisplay()
{
	tileset = new Tileset();
	// Red color with 50% transparency
	_red_square.SetColor(Color(1.0f, 0.0f, 0.0f, 0.5f));
	_red_square.SetDimensions(0.5f, 0.5f);

	setMouseTracking(true);
}



TilesetDisplay::~TilesetDisplay()
{
	delete tileset;
	VideoManager->SingletonDestroy();
}



void TilesetDisplay::initializeGL()
{
	// Destroy and recreate the video engine
	// NOTE: This is actually a very bad practice to do. We have to figure out an alternative.
	VideoManager->SingletonDestroy();
	VideoManager = GameVideo::SingletonCreate();
	VideoManager->SetTarget(VIDEO_TARGET_QT_WIDGET);

	VideoManager->SingletonInitialize();

	VideoManager->ApplySettings();
	VideoManager->FinalizeInitialization();
	VideoManager->ToggleFPS();
}



void TilesetDisplay::paintGL()
{
	VideoManager->SetCoordSys(0.0f, VideoManager->GetScreenWidth() / TILE_WIDTH,
		VideoManager->GetScreenHeight() / TILE_HEIGHT, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	VideoManager->Clear(Color::blue);

	if (tileset->IsInitialized() == true) {
		// Draw the tileset as a single image
		tileset->tiles[0].Draw();

		// Draw transparent red over the unwalkable tile quadrants
		for (uint32 i = 0; i < 16; i++) {
			for (uint32 j = 0; j < 16; j++) {
				VideoManager->Move(j, i);

				if (tileset->walkability[i * 16 + j][0] != 0) {
					_red_square.Draw();
				}

				VideoManager->MoveRelative(0.5f, 0.0f);
				if (tileset->walkability[i * 16 + j][1] != 0) {
					_red_square.Draw();
				}

				VideoManager->MoveRelative(-0.5f, 0.5f);
				if (tileset->walkability[i * 16 + j][2] != 0) {
					_red_square.Draw();
				}

				VideoManager->MoveRelative(0.5f, 0.0f);
				if (tileset->walkability[i * 16 + j][3] != 0) {
					_red_square.Draw();
				}
			}
		}
	}

	// Draws the grid that visually seperates each tile in the tileset image
	VideoManager->DrawGrid(0.0f, 0.0f, 0.5f, 0.5f, Color::black);
}



void TilesetDisplay::resizeGL(int w, int h)
{
	VideoManager->SetResolution(512, 512);
	VideoManager->ApplySettings();
}



void TilesetDisplay::mousePressEvent(QMouseEvent* evt)
{
	// Don't process clicks outside of the tileset image
	if ((evt->x() < 0) || (evt->y() < 0) || evt->x() >= 512 || evt->y() >= 512)
		return;

	// Determine which tile the user clicked
	int32 tile_x, tile_y;
	tile_x = evt->x() / 32;
	tile_y = evt->y() / 32;

	// Now determine which quadrant of that tile was clicked, and change it's walkability status
	if (((evt->x() % 32) < 16) && ((evt->y() % 32) < 16)) { // Upper left quadrant (index 0)
		tileset->walkability[tile_y * 16 + tile_x][0] = (tileset->walkability[tile_y * 16 + tile_x][0] ? 0 : 1);
	}
	else if (((evt->x() % 32) >= 16) && ((evt->y() % 32) < 16)) { // Upper right quadrant (index 1)
		tileset->walkability[tile_y * 16 + tile_x][1] = (tileset->walkability[tile_y * 16 + tile_x][1] ? 0 : 1);
	}
	else if (((evt->x() % 32) < 16) && ((evt->y() % 32) >= 16)) { // Lower left quadrant (index 2)
		tileset->walkability[tile_y * 16 + tile_x][2] = (tileset->walkability[tile_y * 16 + tile_x][2] ? 0 : 1);
	}
	else if (((evt->x() % 32) >= 16) && ((evt->y() % 32) >= 16)) { // Lower right quadrant (index 3)
		tileset->walkability[tile_y * 16 + tile_x][3] = (tileset->walkability[tile_y * 16 + tile_x][3] ? 0 : 1);
	}

	updateGL();
} // contentsMousePressEvent(...)

////////////////////////////////////////////////////////////////////////////////
////////// TilesetEditor class
////////////////////////////////////////////////////////////////////////////////

TilesetEditor::TilesetEditor(QWidget* parent, const QString& name, bool prop)
	: QDialog(parent, (const char*)name)
{
	setCaption("Tileset Editor");

	// Create GUI Items
	_opentileset_pbut = new QPushButton("Open", this);
	_cancel_pbut = new QPushButton("Cancel", this);
	_ok_pbut = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);

	// Create the window
	_tset_display = new TilesetDisplay;
	_tset_display->resize(512, 512);
	_tset_display->setFixedWidth(512);
	_tset_display->setFixedHeight(512);

	// connect button signals
	connect(_ok_pbut, SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));
	connect(_opentileset_pbut, SIGNAL(clicked()), this, SLOT(_OpenFile()));

	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	_dia_layout->addWidget(_opentileset_pbut, 0, 1);
	_dia_layout->addWidget(_ok_pbut, 1, 1);
	_dia_layout->addWidget(_cancel_pbut, 2, 1);
	_dia_layout->addWidget(_tset_display, 0, 0, 3, 1);
}



TilesetEditor::~TilesetEditor()
{
	delete _opentileset_pbut;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _dia_layout;
	delete _tset_display;
}



void TilesetEditor::_OpenFile()
{
	// Get the filename to open through the OpenFileName dialog
	QString file_name = QFileDialog::getOpenFileName(this, "HoA Level Editor -- File Open",
		"dat/tilesets", "Tilesets (*.lua)");

	// File name will contain only the name of the tileset->
	int i = file_name.lastIndexOf("/");
	file_name = file_name.remove(0, i + 1);
	file_name.chop(4);

	_tset_display->tileset->Load(file_name, true);
	_tset_display->updateGL();
}

} // namespace hoa_editor
