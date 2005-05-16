/******************************************************************************
 *
 *	Hero of Allacrost Tileset class
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Filename: tileset.cpp
 *
 *	$Id$
 *
 *	Description: This class controls and manipulates a tileset for
 *               editing Hero of Allacrost maps.
 *
 *****************************************************************************/

#include "tileset.h"

using namespace hoa_mapEd;

/******************************************************************************
 *
 *  Function: dragObject
 *
 *  Inputs: none
 *
 *  Outputs: QImageDrag* imgDrag - object to drag, typecasted to QDragObject*
 *
 *  Description: This function implements high level drag and drop operations.
 *				 It determines the desired object to drag from the QIconView.
 *
 *****************************************************************************/
QDragObject* Tileset::dragObject()
{
	// creates an image to drag
	QImageDrag *imgDrag = new QImageDrag(currentItem()->pixmap()
		->convertToImage(), this);
	
	// creates the image user will see whilst dragging
	imgDrag->setPixmap(*(currentItem()->pixmap()),
		QPoint(currentItem()->pixmapRect().width() / 2,
			currentItem()->pixmapRect().height() / 2));

	return imgDrag;
} // dragObject()
