/******************************************************************************
 *
 *	Hero of Allacrost Map class
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Filename: map_grid.h
 *
 *	$Id$
 *
 *	Description: This class constructs and manipulates a map used in the map
 *               editor.
 *
 *****************************************************************************/

#ifndef MAP_GRID_H
#define MAP_GRID_H

#include "tile.h"
#include "map.h"

#include <qcanvas.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qlayout.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include <qvbuttongroup.h>
#include <qwmatrix.h>

namespace hoa_mapEd
{

class MapGrid: public QCanvasView
{
	Q_OBJECT     // macro needed to use QT's slots and signals
	
	public:
		MapGrid(QWidget *parent = 0, const QString &name = QString("Untitled"));
		~MapGrid();
		
		bool getChanged();  		// returns mapChanged
		QString getFileName();		// returns mapFileName
		void setWidth(int width);	// sets the map's width in tiles (columns)
		void setHeight(int height);	// sets the map's height in tiles (rows)
		void setFileName(QString filename);	// sets the map's file name
		void saveMap(QFile &);		// saves the map to a config file
		void createGrid();		// creates grid lines on the map
		
	protected:
		// I think these are protected
		// Low-level drag and drop
		void dragEnterEvent(QDragEnterEvent *evt);
		void dropEvent(QDropEvent *evt );
		//void mousePressEvent(QMouseEvent *);
		//void mouseMoveEvent(QMouseEvent *);
		
		void contentsMousePressEvent(QMouseEvent *evt);
		void contentsMouseMoveEvent(QMouseEvent *evt);
		void contentsMouseReleaseEvent(QMouseEvent *evt);
		void contentsMouseDoubleClickEvent(QMouseEvent *evt);
		void contentsContextMenuEvent(QContextMenuEvent *);
		
	private slots:
		// the following slots are used to gray out items in the menu
		void editMenuSetup();
		void viewMenuSetup();
		void viewMenuEvaluate();
		void tileMenuSetup();
		void tileMenuEvaluate();
		
		// the following slots are used in the edit menu
		void editUndo();	// undoes last action
		void editRedo();	// redoes last action
		void editClear();	// clears all items on the map
		void editMode();	// sets the editting mode
		
		// the following slots are used in the view menu
		void viewToggleGrid();	// toggles the map's grid on or off

		// the following slots are used in the tile menu
		void tileFlipHorizontal();
		void tileFlipVertical();
		void tileRotateClockwise();
		void tileRotateCounterClockwise();
		void tileMode();	// sets the status of the tile

	private:
		void getMapData();		// gets map height and width & loads into DOM
//		void paintMap();		// paints the map to the screen
		void createMenus();		// creates the various context menus
		
		QPoint menuPosition;	// position at which theMenu was executed
		QPopupMenu *theMenu;	// menu pops up on right click
		QPopupMenu *editMenu;	// internal to theMenu
		QPopupMenu *viewMenu;	// internal to theMenu
		QPopupMenu *tileMenu;	// internal to theMenu

		QRadioButton *viewNone;
		QRadioButton *viewTreasure;
		QRadioButton *viewEvent;
		QRadioButton *viewOccupied;
		QRadioButton *viewNoWalk;

		QVButtonGroup *properties;		// tile status button group (needed to gray it out)
		QCheckBox *tileTreasure;
		QCheckBox *tileEvent;
		QCheckBox *tileOccupied;

		QStringList fileNameList;			// list of tile file names
		QValueVector<int> locationVector;	// vector of tiles in each cell
		
		QCanvasItem* moving;	// set if an object is currently being moved
		QPoint moving_start;	// moving object's starting point

		QString mapFileName;	// map's file name
		int viewProperty;		// hex. bit mask of which property being viewed
		int tileProperties;		// hex. bit mask of a tile's properties
		int mapHeight;			// height of map in tiles
		int mapWidth;			// width of map in tiles
		bool mapChanged;		// TRUE = map modified, FALSE otherwise
		bool dragging;			// TRUE = something being dragged, else FALSE
		bool gridOn;			// TRUE = grid is displayed, else FALSE
		bool dragOn;			// TRUE = dragging is enabled, else painting
		bool walkOn;			// TRUE = walkable is set, else not-walkable
}; // class MapGrid

} // namespace hoa_mapEd

#endif
// MAP_GRID_H
