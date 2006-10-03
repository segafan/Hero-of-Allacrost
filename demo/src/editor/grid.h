///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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

#include "utils.h"
#include "defs.h"
#include "script.h"
#include "video.h"

#include "tile.h"

#include <qcanvas.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qgl.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qstringlist.h>
//#include <qvaluevector.h>
#include <qvbuttongroup.h>
#include <qwmatrix.h>

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

	//! Different layers
enum LAYER_TYPE
{
	INVALID_LAYER = -1,
	LOWER_LAYER   = 0,
	MIDDLE_LAYER  = 1,
	UPPER_LAYER   = 2,
	TOTAL_LAYER   = 3
};

LAYER_TYPE& operator++(LAYER_TYPE& value, int dummy);

class Grid: public QGLWidget
{
	Q_OBJECT     // macro needed to use QT's slots and signals
	
	public:
		Grid(QWidget *parent = 0, const QString &name = QString("Untitled"),
			int width = 0, int height = 0); // constructor
		~Grid();                            // destructor

		bool GetChanged() const { return _changed; }
		void SetChanged(bool value);        // sets the map's modified status
		QString GetFileName() const { return _file_name; }
		void    SetFileName(QString filename); // sets the map's file name
		int  GetHeight() const { return _height; }
		int  GetWidth() const {return _width; }
		void SetHeight(int height);         // sets the map's height in tiles (rows)
		void SetWidth(int width);           // sets the map's width in tiles (columns)
		void SetLLOn(bool value);           // sets lower layer on/off
		void SetMLOn(bool value);           // sets middle layer on/off
		void SetULOn(bool value);           // sets upper layer on/off
		void SetGridOn(bool value);         // sets grid on/off

		std::vector<int32>& GetLayer(LAYER_TYPE layer);

		//! Sets background music
		void SetMusic(const std::string& music_file);
		//! Gets background music
		const std::string& GetMusic() const;

		void LoadMap();                     // loads a map from a config file
		void SaveMap();                     // saves the map to a config file

		QStringList file_name_list;         // list of tile file names
		QStringList tileset_list;           // list of tileset names
		std::vector<int32> tiles_walkable;  // vector of walkability of tiles
		std::vector<int32> indiv_walkable;  // vector of walkability of individual tiles

	protected:
		void initializeGL();                // sets up the rendering context
		void paintGL();                     // paints the entire map
		void resizeGL(int w, int h);        // resizes the widget

		// I think these are protected
		// Low-level drag and drop
/*		void dragEnterEvent(QDragEnterEvent *evt);
		void dropEvent(QDropEvent *evt);
		//void mousePressEvent(QMouseEvent *);
		//void mouseMoveEvent(QMouseEvent *);
		
		void contentsMousePressEvent(QMouseEvent *evt);
		void contentsMouseMoveEvent(QMouseEvent *evt);
		void contentsMouseReleaseEvent(QMouseEvent *evt);
		void contentsMouseDoubleClickEvent(QMouseEvent *evt);
		void contentsContextMenuEvent(QContextMenuEvent *);
*/		
	private slots:
		// the following slots are used to gray out items in the menu
/*		void _EditMenuSetup();
		void _ViewMenuSetup();
		void _ViewMenuEvaluate();
		void _TileMenuSetup();
		void _TileMenuEvaluate();
		
		// the following slots are used in the edit menu
		void _EditUndo();		// undoes last action
		void _EditRedo();		// redoes last action
		void _EditClear();		// clears all items on the map
//		void _EditMode();		// sets the editing mode
		
		// the following slots are used in the view menu
		void _ViewToggleGrid();	// toggles the map's grid on or off

		// the following slots are used in the tile menu
//		void _TileFlipHorizontal(); <-- Told I don't need this capability
//		void _TileFlipVertical();
//		void _TileRotateClockwise();
//		void _TileRotateCounterClockwise();
		void _TileMode();		// sets the status of the tile
*/
	private:
/*		void _GetMapData();		// gets map height and width & loads into DOM
//		void _PaintMap();		// paints the map to the screen
		void _CreateMenus();	// creates the various context menus
		
		QPoint _menu_position;	// position at which theMenu was executed
		QPopupMenu* _the_menu;	// menu pops up on right click
		QPopupMenu* _edit_menu;	// internal to theMenu
		QPopupMenu* _view_menu;	// internal to theMenu
		QPopupMenu* _tile_menu;	// internal to theMenu

		QRadioButton* _view_none;
		QRadioButton* _view_treasure;
		QRadioButton* _view_event;
		QRadioButton* _view_occupied;
		QRadioButton* _view_no_walk;

		QVButtonGroup* _properties;			// tile status button group
		QCheckBox* _tile_treasure;
		QCheckBox* _tile_event;
		QCheckBox* _tile_occupied;
		
		QCanvasItem* _moving;	// set if an object is currently being moved
		QPoint _moving_start;	// moving object's starting point

*/		QString _file_name;		// map's file name
/*		int _view_property;		// hex. bit mask of which property being viewed
		int _tile_properties;	// hex. bit mask of a tile's properties
*/		int _height;			// height of map in tiles
		int _width;				// width of map in tiles
		hoa_video::StillImage _tile;
		bool _changed;			// TRUE = map modified, FALSE otherwise
		//bool _dragging;		// TRUE = something being dragged, else FALSE
		bool _grid_on;			// TRUE = grid is displayed, else FALSE
		bool _ll_on;            // TRUE = lower layer is displayer, else FALSE
		bool _ml_on;            // TRUE = middle layer is displayer, else FALSE
		bool _ul_on;            // TRUE = upper layer is displayer, else FALSE

		std::vector<int32> _lower_layer;     // vector of tiles in the lower layer
		std::vector<int32> _middle_layer;    // vector of tiles in the middle layer
		std::vector<int32> _upper_layer;     // vector of tiles in the upper layer

		
		//! Stores background music
		std::string _music_file;
		bool _random_encounters;
		int _encounter_rate;

/*		//bool _drag_on;		// TRUE = dragging is enabled, else painting
		bool _walk_on;			// TRUE = walkable is set, else not-walkable
*/}; // class Grid

} // namespace hoa_editor

#endif
// __GRID_HEADER__
