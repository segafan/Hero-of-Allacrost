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

QDragObject *Tileset::dragObject()
{
	return new QImageDrag(currentItem()->pixmap()->convertToImage(), this);
} // dragObject()

