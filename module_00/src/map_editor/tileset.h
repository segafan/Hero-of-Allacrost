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
 * \date    Last Updated: September 7th, 2005
 * \brief   Header file for editor's tileset, mainly used for drag'n'drop.
 *
 * This code displays map tiles as icons and implements drag and drop
 * capability for these tiles.
 *****************************************************************************/
			   
#ifndef __TILESET_HEADER__
#define __TILESET_HEADER__

#include <qcursor.h>
#include <qiconview.h>
#include <qpoint.h>

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

/*!****************************************************************************
 * \brief Manages individual tiles into a QIconView with drag and drop
 * capability.
 * 
 * \note Inherits QIconView.
 *****************************************************************************/
class Tileset : public QIconView
{
	public:
		Tileset(QWidget* parent = 0, const char* name = 0, WFlags f = 0) :
			QIconView(parent, name, f) {}

		/*!
		 *  \brief Implements high-level drag and drop functionality.
		 *  \return A pointer to the object being dragged.
		 */
		QDragObject* dragObject();
}; // class Tileset

} // namespace hoa_editor

#endif
// __TILESET_HEADER__
