///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2004, 2005 by The Allacrost Project
// All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    tileset.cpp
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \date    Last Updated: September 7th, 2005
 * \brief   Source file for editor's tileset, mainly used for drag'n'drop.
 *****************************************************************************/

#include "tileset.h"

using namespace hoa_editor;

/******************************************************************************
 *
 *  Function: dragObject
 *
 *  Inputs: none
 *
 *  Outputs: QImageDrag* img_drag - object to drag, typecasted to QDragObject*
 *
 *  Description: This function implements high level drag and drop operations.
 *				 It determines the desired object to drag from the QIconView.
 *
 *****************************************************************************/
QDragObject* Tileset::dragObject()
{
	// creates an image to drag
	QImageDrag* img_drag = new QImageDrag(currentItem()->pixmap()
		->convertToImage(), this);
	
	// creates the image user will see whilst dragging
	img_drag->setPixmap(*(currentItem()->pixmap()),
		QPoint(currentItem()->pixmapRect().width() / 2,
			currentItem()->pixmapRect().height() / 2));

	return img_drag;
} // dragObject()
