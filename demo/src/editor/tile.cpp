///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file tile.cpp
 * \author Philip Vorsilak, gorzuate@allacrost.org
 * \brief Header file for representing a tile in the editor, and stores a
 *        tile's properties and attributes.
 *****************************************************************************/

#include "tile.h"

using namespace hoa_editor;

/******************************************************************************
 *
 *  Function: Tile
 *
 *  Description: Tile's class constructor.
 *
 *  Inputs: img - the image for the tile
 *          canvas - the object on which to place the tile
 *
 *  Outputs: none
 *
 *****************************************************************************/
Tile::Tile(QString name, QImage img, Q3Canvas *canvas) : Q3CanvasRectangle(canvas)
{
	_file_name = name;
	_image = img;
	setSize(_image.width(), _image.height());
	_pixmap.convertFromImage(_image, Qt::OrderedAlphaDither);
} // Tile constructor

/******************************************************************************
 *
 *  Function: drawShape
 *
 *  Description: Reimplemented Qt function which draws the tile on the editor.
 *               Do NOT rename!!!
 *
 *  Inputs: p - object on which to draw the tile
 *
 *  Outputs: none
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
 *  Description: Determines if a tile is located at the point where the mouse
 *               clicked.
 *
 *  Inputs: p - point where the mouse was clicked
 *
 *  Outputs: A pixel of a tile if there was one where the mouse was clicked,
 *           else zero.
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
