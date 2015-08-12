///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    grid.h
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Header file for editor's grid, used for the OpenGL map portion
 *          where tiles are painted, edited, etc.
 *****************************************************************************/

#ifndef __GRID_HEADER__
#define __GRID_HEADER__

#include <QGLWidget>
#include <QStringList>
#include <QMessageBox>

#include "tileset.h"
#include "sprites.h"

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

//! Different tile layers in the map.
enum LAYER_TYPE
{
	INVALID_LAYER = -1,
	LOWER_LAYER   = 0,
	MIDDLE_LAYER  = 1,
	UPPER_LAYER   = 2,
	SELECT_LAYER  = 3,
	OBJECT_LAYER  = 4,
	TOTAL_LAYER   = 5
};

//! objects in the object layer.
enum OBJECT_TYPE
{
	INVALID_OBJECT = -1,
	VIRTUAL_SPRITE_OBJECT = 0,
	SPRITE_OBJECT = 1,
	TOTAL_OBJECT = 2
};

LAYER_TYPE& operator++(LAYER_TYPE& value, int dummy);

class EditorScrollView;

class Grid: public QGLWidget
{
	Q_OBJECT     // macro needed to use QT's slots and signals

	public:
		Grid(QWidget *parent = 0, const QString &name = QString("Untitled"),
			int width = 0, int height = 0); // constructor
		~Grid();                            // destructor

		//! Returns the map's modified status.
		bool GetChanged() const { return _changed; }
		//! Sets the map's modified status.
		void SetChanged(bool value);
		QString GetFileName() const { return _file_name; }
		void    SetFileName(QString filename); // sets the map's file name
		int  GetHeight()  const { return _height; }
		int  GetWidth()   const { return _width; }
		int  GetContext() const { return _context; }
		void SetHeight(int height);         // sets the map's height in tiles (rows)
		void SetWidth(int width);           // sets the map's width in tiles (columns)
		void SetContext(int context);       // sets the map's active context
		void SetInitialized(bool ready);    // sets the map's ready to be drawn, completely loaded flag
		void SetLLOn(bool value);           // sets lower layer on/off
		void SetMLOn(bool value);           // sets middle layer on/off
		void SetULOn(bool value);           // sets upper layer on/off
		void SetOLOn(bool value);			// sets object layer on/off
		void SetGridOn(bool value);         // sets grid on/off
		void SetSelectOn(bool value);       // sets selection layer on/off
		void SetTexturesOn(bool value);     // sets textures on/off

		std::vector<int32>& GetLayer(LAYER_TYPE layer, int context);

		/*!
		 *  \brief Creates a new context for each layer.
		 *  \param inherit_context - the index of the context to inherit from.
		 */
		void CreateNewContext(int inherit_context);

		/*!
		 *  \brief Loads a map from a config (lua) file when the user selects Open Map... from the File menu.
		 */
		void LoadMap();

		/*!
		 *  \brief Saves the map to a config (lua) file when the user selects Save, Save as..., or Quit
		 *         from the File menu.
		 */
		void SaveMap();

		//! \name Context (Right-Clicking) Modification Functions
		//! \brief Functions to insert or delete rows or columns from the map.
		//! \note Currently accessed by right-clicking on the map. Could be used elsewhere if the proper
		//!       tile index is passed as a parameter.
		//! \param tile_index An ID (0 - width*height-1) of the tile used to determine the row or column
		//!                   upon which to perform the operation.
		//{@
		void InsertRow(int tile_index);
		void InsertCol(int tile_index);
		void DeleteRow(int tile_index);
		void DeleteCol(int tile_index);
		//@}

		//! List of the tileset names being used.
		QStringList tileset_names;
		//! A vector which contains a pointer to each tileset and the tiles it has loaded via LoadMultiImage.
		std::vector<Tileset*> tilesets;
		//! A list which contains a pointer to each sprite, I use list which is because of its efficient.
		std::list<MapSprite*> sprites;

		//! A list containing the names of each context.
		//! \note Should have a max size of 32. That's the max amount of contexts.
		QStringList context_names;
		//! A list storing the background music filenames.
		QStringList music_files;
	
		//! pointer to scrollview
		EditorScrollView * _ed_scrollview;

	protected:
		//! \brief Sets up the rendering context of the OpenGL portion of the editor.
		void initializeGL();

		//! \brief Paints the entire map with the Allacrost video engine.
		void paintGL();

		//! \brief Performs a resize operation of the OpenGL widget when appropriate.
		void resizeGL(int w, int h);

	private:
		//! The map's file name.
		QString _file_name;
		//! The height of the map in tiles.
		int _height;
		//! The width of the map in tiles.
		int _width;
		//! The active context for editing tiles.
		int _context;

		//! When TRUE the map has been modified.
		bool _changed;
		//! When TRUE the map is ready to be drawn.
		bool _initialized;
		//! When TRUE the grid between tiles is displayed.
		bool _grid_on;
		//! When TRUE the rectangle of chosen tiles is displayed.
		bool _select_on;
		//! When TRUE the texture sheets are displayed.
		bool _textures_on;
		//! When TRUE the lower layer of tiles is displayed.
		bool _ll_on;
		//! When TRUE the middle layer of tiles is displayed.
		bool _ml_on;
		//! When TRUE the upper layer of tiles is displayed.
		bool _ul_on;
		//! When TRUE the object layer of tiles is displayed.
		bool _ol_on;

		//! A vector of tiles in the lower layer.
		std::vector<std::vector<int32> > _lower_layer;
		//! A vector of tiles in the middle layer.
		std::vector<std::vector<int32> > _middle_layer;
		//! A vector of tiles in the upper layer.
		std::vector<std::vector<int32> > _upper_layer;
		//! A vector of sprites in the object layer.
		std::vector<int32 > _object_layer;
		//! A vector of tiles in the selection rectangle. Exists only in the editor,
		//! not the game. Acts similarly to an actual layer as far as drawing
		//! is concerned.
		std::vector<int32> _select_layer;
}; // class Grid

} // namespace hoa_editor

#endif
// __GRID_HEADER__
