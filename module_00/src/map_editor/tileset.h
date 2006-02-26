///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2004, 2005 by The Allacrost Project
// All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    tileset.h
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \date    Last Updated: February 7th, 2006
 * \brief   Header file for editor's tileset, mainly used for drag'n'drop.
 *
 * This code displays map tiles in a grid and implements drag and drop
 * capability for these tiles.
 *****************************************************************************/
			   
#ifndef __TILESET_HEADER__
#define __TILESET_HEADER__

#include "utils.h"
#include "defs.h"
#include "data.h"
#include "tile.h"

//#include <qcursor.h>
#include <qmessagebox.h>
//#include <qpainter.h>
#include <qpixmap.h>
//#include <qpoint.h>
#include <qstring.h>
#include <qtable.h>

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

/*!****************************************************************************
 * \brief Manages individual tiles into a QTable with drag and drop
 * capability.
 * 
 * \note Inherits QTable.
 *****************************************************************************/
class Tileset : public QTable
{
	public:
		Tileset(QWidget* parent, const QString& name);    // constructor
		~Tileset();                                       // destructor

		/*!
		 *  \brief Implements high-level drag and drop functionality.
		 *  \return A pointer to the object being dragged.
		 */
//		QDragObject* dragObject();

	protected:
//		void paintCell(QPainter* painter, int row, int col);
		
	private:
//		QPainter* _painter;        // needed for draw operations on the grid
//		std::vector<QPixmap> _pixmap_vect;
//		std::vector<QPixmap>::iterator _it;
}; // class Tileset

} // namespace hoa_editor

#endif
// __TILESET_HEADER__
