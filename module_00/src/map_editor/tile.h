///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2004, 2005 by The Allacrost Project
// All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file tile.h
 * \author Philip Vorsilak, gorzuate@allacrost.org
 * \date Last Updated: October 30th, 2005
 * \brief Header file for representing a tile in the editor.
 *
 * This code provides a class to store a tile's properties and attributes in
 * the editor.
 *****************************************************************************/

#ifndef __TILE_HEADER__
#define __TILE_HEADER__

#include <qcanvas.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstring.h>

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

//! A unique number used to distinguish tiles from other objects on the grid.
static const int TILE_RTTI = 973952;
//! A tile's width in pixels
const int TILE_WIDTH = 32;
//! A tile's height in pixels
const int TILE_HEIGHT = 32;

/*!****************************************************************************
 * \brief Represents a tile as a QCanvasRectangle and stores its properties
 * appropriately.
 *
 * \note Inherits QCanvasRectangle.
 *****************************************************************************/
class Tile: public QCanvasRectangle
{
	public:
    	Tile(QString name, QImage img, QCanvas *canvas);

		/*!
		 *  \brief Reimplemented Qt function used to easily identify objects on
		 *  the grid. Do NOT rename to conform to code standards!!!
		 *  \return Tile's RTTI constant.
		 */
    	int rtti() const { return TILE_RTTI; }

		/*!
		 *  \brief Determines if a tile is "hit" by the mouse when clicked on.
		 *  \param &p The point where the mouse was clicked.
		 *  \return The pixel where the mouse clicked. If non-zero, a tile was
		 *  "hit".
		 */
		bool Hit(const QPoint &p) const;

		/*!
		 *  \brief Used to get the tile's file name.
		 *  \return A QString of the tile's file name.
		 */
    	QString GetName() { return _file_name; }
		
	protected:
		/*!
		 *  \brief Reimplemented Qt function used to draw the tile onto the
		 *  grid. Do NOT rename to conform to code standards!!!
		 *  \param &p Object on which to draw the tile, in this case Qt's
		 *  painter.
		 */
    	void drawShape(QPainter &p);

	private:
		QString _file_name;
    	QImage _image;
    	QPixmap _pixmap;
}; // class Tile

} // namespace hoa_editor

#endif
// __TILE_HEADER__
