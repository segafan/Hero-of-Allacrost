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

#include <qiconview.h>
//#include <qpixmap.h>

namespace hoa_map
{

class Tileset : public QIconView
{
	//Q_OBJECT
	public:
		Tileset(QWidget * parent = 0, const char * name = 0, WFlags f = 0) :
			QIconView(parent, name, f) {}

		// high-level drag and drop
		QDragObject *dragObject();
}; // class Tileset

} // namespace hoa_map

#endif
// TILESET_H
