///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2004, 2005, 2006 by The Allacrost Project
// All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    tileset.h
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Header file for editor's tileset, used for maintaining a visible
 *          "list" of tiles to select from for painting on a map.
 *****************************************************************************/
			   
#ifndef __TILESET_HEADER__
#define __TILESET_HEADER__

#include "utils.h"
#include "defs.h"
#include "data.h"
#include "tile.h"

#include <qmessagebox.h>
#include <qpixmap.h>
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

		// TODO: implement some sort of dynamic table resizing on window resize
}; // class Tileset

} // namespace hoa_editor

#endif
// __TILESET_HEADER__
