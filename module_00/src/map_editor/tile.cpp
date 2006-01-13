///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2004, 2005 by The Allacrost Project
// All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file tile .h
 * \author Philip Vorsilak, gorzuate@allacrost.org
 * \date Last Updated: October 30th, 2005
 * \brief Source file for editor's tile object.
 *****************************************************************************/

#include "tile.h"

using namespace hoa_editor;

/******************************************************************************
 *
 *  Function: Tile
 *
 *  Inputs: img - the image for the tile
 *          canvas - the object on which to place the tile
 *
 *  Outputs: none
 *
 *  Description: Tile's class constructor.
 *
 *****************************************************************************/
Tile::Tile(QString name, QImage img, QCanvas *canvas) : QCanvasRectangle(canvas)
{
	_file_name = name;
	_image = img;
	setSize(_image.width(), _image.height());
	_pixmap.convertFromImage(_image, OrderedAlphaDither);
} // Tile constructor

/******************************************************************************
 *
 *  Function: drawShape
 *
 *  Inputs: p - object on which to draw the tile
 *
 *  Outputs: none
 *
 *  Description: Reimplemented Qt function which draws the tile on the editor.
 *               Do NOT rename!!!
 *
 *****************************************************************************/
void Tile::drawShape(QPainter &p)
{
    p.drawPixmap(int(x()), int(y()), _pixmap);
} // drawShape(...)

/******************************************************************************
 *
 *  Function: Hit
 *
 *  Inputs: p - point where the mouse was clicked
 *
 *  Outputs: A pixel of a tile if there was one where the mouse was clicked,
 *           else zero.
 *
 *  Description: Determines if a tile is located at the point where the mouse
 *               clicked.
 *
 *****************************************************************************/
bool Tile::Hit(const QPoint &p) const
{
    int ix = p.x() - int(x());
    int iy = p.y() - int(y());
    if (!_image.valid(ix, iy))
        return FALSE;
    QRgb pixel = _image.pixel(ix, iy);
    return qAlpha(pixel) != 0;
} // Hit(...)
