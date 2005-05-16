/******************************************************************************
 *
 *	Hero of Allacrost Tileset class
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Filename: tileset.h
 *
 *	$Id$
 *
 *	Description: This class controls and manipulates a tileset for
 *               editing Hero of Allacrost maps.
 *
 *****************************************************************************/

#ifndef TILESET_H
#define TILESET_H

#include <qcursor.h>
#include <qiconview.h>
#include <qpoint.h>

namespace hoa_mapEd
{

/******************************************************************************
 *
 *  Class: Tileset
 *
 *  Inherits: QIconView
 *
 *  Description: This class displays map tiles as icons and implements drag
 *				 and drop capability for these tiles.
 *
 *****************************************************************************/
class Tileset : public QIconView
{
	public:
		Tileset(QWidget* parent = 0, const char* name = 0, WFlags f = 0) :
			QIconView(parent, name, f) {}

		QDragObject* dragObject();		// high-level drag and drop
}; // class Tileset

} // namespace hoa_mapEd

#endif
// TILESET_H
