/******************************************************************************
 *
 *	Hero of Allacrost Map class
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Filename: map.h
 *
 *	$Id$
 *
 *	Description: This class constructs and manipulates a map used in the map
 *               editor.
 *
 *****************************************************************************/

#ifndef MAP_H
#define MAP_H

#include <qcursor.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtable.h>
#include <qvaluevector.h>

//class QFile;
//class QPopupMenu;
//class QStringList;

namespace hoa_map
{

class Map: public QTable
{
	Q_OBJECT     // macro needed to use QT's slots and signals
	
	public:
		Map(QWidget *parent = 0, const QString &name = QString("Untitled"));
		~Map();
		
		bool getChanged();  		// returns mapChanged
		QString getFileName();		// returns mapFileName
		void setWidth(int width);	// sets the map's width in tiles (columns)
		void setHeight(int height);	// sets the map's height in tiles (rows)
		void setFileName(QString filename);	// sets the map's file name
		void saveMap(QFile &);		// saves the map to a config file

	public slots:
		void ToggleGrid();			// toggles the map's grid on or off
		
	protected:
		// I think these are protected
		// Low-level drag and drop
		void dragEnterEvent(QDragEnterEvent *evt);
		void dropEvent(QDropEvent *evt );
		void mousePressEvent(QMouseEvent *evt);
		void mouseMoveEvent(QMouseEvent *);
		
		//void paintCell(QPainter *p, int row, int col);
		//void paintEvent(QPaintEvent *);
		
//		virtual void closeEvent(QCloseEvent *);
		void contextMenuEvent(QContextMenuEvent *);
		
	private slots:
		// the following slots are used in the edit menu
		//void editUndo();
		//void editRedo();
		
		// the following slots are used in the tile menu
		//void tileFlipHorizontal();
		//void tileFlipVertical();
		//void tileRotateClockwise();
		//void tileRotateCounterClockwise();
		
	private:
		void getMapData();		// gets map height and width & loads into DOM
//		void paintMap();		// paints the map to the screen
//		void createMenus();		// creates the various context menus
		bool eraseOK();			// checks to make sure map is not modified
		
		QPopupMenu *theMenu;				// menu pops up on right click
		//QPopupMenu *fileMenu;				// context file menu
		//QPopupMenu *editMenu;				// context edit menu
		//QPopupMenu *tileMenu;				// context tile menu
		QStringList fileNameList;			// list of tile file names
		QValueVector<int> locationVector;	// vector of tiles in each cell
		
		QString mapFileName;	// map's file name
		int mapHeight;			// height of map in tiles
		int mapWidth;			// width of map in tiles
		bool mapChanged;		// TRUE = map modified, FALSE otherwise
		bool dragging;			// TRUE = something being dragged, else FALSE
}; // class Map

} // namespace hoa_map

#endif
// MAP_H
