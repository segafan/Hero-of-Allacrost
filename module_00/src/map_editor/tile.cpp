/******************************************************************************
 *
 *	Hero of Allacrost Tile class implementation
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Filename: map.cpp
 *
 *	$Id$
 *
 *	Description: This class constructs and manipulates a tile used in the map
 *               editor.
 *
 *****************************************************************************/

#include "tile.h"

using namespace hoa_mapEd;

Tile::Tile(QImage img, QCanvas *canvas) : QCanvasRectangle(canvas)
{
	image = img;
	setSize(image.width(), image.height());
    pixmap.convertFromImage(image, OrderedAlphaDither);
	tileInfo.upper_layer = -1;
	tileInfo.lower_layer = -1;
	tileInfo.event_mask = 0;
} // Tile constructor

void Tile::drawShape(QPainter &p)
{
    p.drawPixmap(int(x()), int(y()), pixmap);
} // drawShape(...)

bool Tile::hit(const QPoint &p) const
{
    int ix = p.x()-int(x());
    int iy = p.y()-int(y());
    if (!image.valid(ix, iy))
        return FALSE;
    QRgb pixel = image.pixel(ix, iy);
    return qAlpha(pixel) != 0;
} // hit(...)
