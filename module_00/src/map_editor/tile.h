/******************************************************************************
 *
 *	Hero of Allacrost Tile class
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Filename: tileset.h
 *
 *	$Id$
 *
 *	Description: This class controls and manipulates a tile for use in
 *               Hero of Allacrost maps.
 *
 *****************************************************************************/

#ifndef TILE_H
#define TILE_H

#include "map.h"

#include <qcanvas.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>

namespace hoa_mapEd
{

static const int TILE_RTTI = 973952;
const int TILE_WIDTH = 32;		// width in pixels
const int TILE_HEIGHT = 32;		// height in pixels

class Tile: public QCanvasRectangle
{
	public:
    	Tile(QImage img, QCanvas *canvas);
    	int rtti () const { return TILE_RTTI; }
    	bool hit(const QPoint&) const;
		hoa_map::MapTile tileInfo;
	protected:
    	void drawShape(QPainter &);
	private:
    	QImage image;
    	QPixmap pixmap;
}; // class Tile

} // namespace hoa_mapEd

#endif
// TILE_H
