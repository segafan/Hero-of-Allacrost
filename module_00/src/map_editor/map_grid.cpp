/******************************************************************************
 *
 *	Hero of Allacrost Map class implementation
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Date: 8/29/04
 *	Filename: map_grid.cpp
 *
 *	$Id$
 *
 *	Description: This class constructs and manipulates a map used in the map
 *               editor.
 *
 *****************************************************************************/

#include <iostream>
#include "map_grid.h"

using namespace hoa_mapEd;
using namespace hoa_map::local_map;

MapGrid::MapGrid(QWidget *parent, const QString &name)
	: QCanvasView(parent, (const char*)name)
{	
	setAcceptDrops(TRUE);	// enable drag 'n' drop
	dragging = FALSE;		// FIXME: currently unneeded
	setCanvas(NULL);		// don't have canvas until "New Map..." is selected
	dragOn = true;			// default value
	walkOn = true;			// default value
	tileProperties = 0;		// default value (tiles are walkable)
	viewProperty = 0;		// default value (viewing mode off)
	mapChanged = false;		// map has not yet been modified
	createMenus();			// initialize the menus
} // MapGrid constructor

MapGrid::~MapGrid()
{
	// do nothing Qt should take care of everything
} // MapGrid destructor

bool MapGrid::getChanged()
{
	return mapChanged;
} // getChanged()

QString MapGrid::getFileName()
{
	return mapFileName;
} // getFileName()

void MapGrid::setWidth(int width)
{
	mapWidth = width;
} //setWidth(...)

void MapGrid::setHeight(int height)
{
	mapHeight = height;
} //setHeight(...)

void MapGrid::setFileName(QString filename)
{
	mapFileName = filename;
} //setFileName(...)

/*void MapGrid::getMapData()
{
	QString fileNames;
	QString locations;
    QDomElement element = document->documentElement();
	QDomNode node = element.firstChild();
	
	if(!node.isNull()) {
        QDomElement e = node.toElement(); // convert the node to an element
        if( !e.isNull() )
            fileNames = e.text(); // the node really is an element
	}
	
    node = node.nextSibling();
	if(!node.isNull()) {
		QDomElement e = node.toElement(); // convert the node to an element
        if( !e.isNull() )
            locations = e.text(); // the node really is an element
	}
	
	fileNames = fileNames.simplifyWhiteSpace();
	fileNameList = QStringList::split(' ', fileNames);
	
	QStringList locationRowList = QStringList::split('\n', locations);
	mapHeight = locationRowList.count() - 1;
	for(int i = 0; i < mapHeight; i++)
		locationRowList[i] = locationRowList[i].simplifyWhiteSpace();
	mapWidth = QStringList::split(' ', locationRowList[0]).count();
	
	for(int i = 0; i < mapHeight; i++)
	{
		QStringList temp = QStringList::split(' ', locationRowList[i]);
		for(int j = 0; j < mapWidth; j++)
			locationVector.push_back(atoi(temp[j]));
	}
} // getMapData()
*/

void MapGrid::dragEnterEvent(QDragEnterEvent *evt)
{
    if (QImageDrag::canDecode(evt))
        evt->accept();
} // dragEnterEvent(...)

void MapGrid::dropEvent(QDropEvent *evt)
{
    QImage img;		// image to insert into the map
    
    if (QImageDrag::decode(evt, img) && (canvas() != NULL))
	{
		Tile *tile = new Tile(img, canvas());
		QPoint point = inverseWorldMatrix().map(evt->pos());
		
		// the division here will effectively snap the tile to the grid
		// yes, it looks dumb. is there another way to round down to the
		// nearest multiple of 32?
		tile->move(point.x() / TILE_WIDTH * TILE_WIDTH,
				   point.y() / TILE_HEIGHT * TILE_HEIGHT);
		tile->setZ(0);		// sets height of tile TODO: create a menu option
		// tile->tileInfo.lower_layer = 1; FIXME: perhaps not needed here
		tile->tileInfo.upper_layer = -1; // TEMPORARY!!! FIXME: this neither
		tile->tileInfo.event_mask = tileProperties;
		tile->show();
		canvas()->update();
		mapChanged = TRUE;
	} // must be able to decode the drag in order to place the image
} // dropEvent(...)

void MapGrid::mousePressEvent(QMouseEvent *)
{
//	QTable::mousePressEvent(evt);
//	dragging = TRUE;
} // mousePressEvent(...)

void MapGrid::mouseMoveEvent(QMouseEvent *)
{
//	if (dragging) {
//		QDragObject *d = new QImageDrag(currentItem()->pixmap(), this );
//		d->dragCopy(); // do NOT delete d.
//		dragging = FALSE;
//	}
} // mouseMoveEvent(...)

void MapGrid::contentsMousePressEvent(QMouseEvent *evt)
{
	if (evt->button() == Qt::LeftButton)
	{
		if (dragOn)
		{
			QPoint p = inverseWorldMatrix().map(evt->pos());
			QCanvasItemList l = canvas()->collisions(p);
			for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it)
			{
				if ((*it)->rtti() == TILE_RTTI)
				{
					Tile *item= (Tile*)(*it);
					if (!item->hit(p))
						continue;

					moving = *it;
					moving_start = p;
					return;
				} // only want to move a tile, nothing else
			} // go through the list of possible items to move
		} // not painting tiles means we want to start moving them instead
	} // only make the left mouse button useful/active

	moving = 0;
} // contentsMousePressEvent(...)

void MapGrid::contentsMouseMoveEvent(QMouseEvent *evt)
{
	if (moving)
	{
		QPoint p = inverseWorldMatrix().map(evt->pos());
		moving->moveBy(p.x() - moving_start.x(), p.y() - moving_start.y());
		moving_start = p;
		canvas()->update();
	} // only if the user is moving an object
} // contentsMouseMoveEvent(...)

void MapGrid::contentsMouseReleaseEvent(QMouseEvent *evt)
{
	if (moving)
	{
		QPoint point = inverseWorldMatrix().map(evt->pos());
		
		// the division here will effectively snap the tile to the grid
		// yes, it looks dumb. is there another way to round down to the
		// nearest multiple of 32?
		moving->move(point.x() / TILE_WIDTH * TILE_WIDTH,
			     point.y() / TILE_HEIGHT * TILE_HEIGHT);
		canvas()->update();
		mapChanged = TRUE;
	} // only if the user is moving an object
} // contentsMouseReleaseEvent(...)

void MapGrid::contextMenuEvent(QContextMenuEvent *)
{
	menuPosition = QCursor::pos();
	theMenu->exec(menuPosition);
} // contextMenuEvent(...)

void MapGrid::createGrid()
{
	QCanvasItemList list = canvas()->allItems();

	// delete existing grid
	for (QCanvasItemList::Iterator it = list.begin(); it != list.end(); ++it)
		if ((*it)->rtti() == QCanvasItem::Rtti_Line)
			delete *it;
	
	for (int h = TILE_HEIGHT; h < canvas()->height(); h += TILE_HEIGHT)
	{
		QCanvasLine *line = new QCanvasLine(canvas());
		line->setPen(QPen(QColor("black"), 2));		// thickness of 2 pixels
		line->setPoints(0, h, canvas()->width(), h);
		line->setZ(3);		// FIXME: make sure it gets painted over the tiles
		line->show();
	} // create horizontal lines

	for (int v = TILE_WIDTH; v < canvas()->width(); v += TILE_WIDTH)
	{
		QCanvasLine *line = new QCanvasLine(canvas());
		line->setPen(QPen(QColor("black"), 2));		// thickness of 2 pixels
		line->setPoints(v, 0, v, canvas()->height());
		line->setZ(3);		// FIXME: make sure it gets painted over the tiles
		line->show();
	} // create vertical lines

	gridOn = true;
} //createGrid()

void MapGrid::createMenus()
{
	// top menu creation
	theMenu = new QPopupMenu(this);
	
	// edit menu creation
	editMenu = new QPopupMenu(theMenu);
	connect(editMenu, SIGNAL(aboutToShow()), this, SLOT(editMenuSetup()));

	// view menu creation
	viewMenu = new QPopupMenu(theMenu);
	connect(viewMenu, SIGNAL(aboutToShow()), this, SLOT(viewMenuSetup()));
	connect(viewMenu, SIGNAL(aboutToHide()), this, SLOT(viewMenuEvaluate()));
	
	// tile menu creation
	tileMenu = new QPopupMenu(theMenu);
	connect(tileMenu, SIGNAL(aboutToShow()), this, SLOT(tileMenuSetup()));
	connect(tileMenu, SIGNAL(aboutToHide()), this, SLOT(tileMenuEvaluate()));
	
	theMenu->insertItem("Edit", editMenu);
	theMenu->insertItem("View", viewMenu);
	theMenu->insertItem("Tile", tileMenu);
} // createMenus()

void MapGrid::editMenuSetup()
{
	editMenu->clear();
	
	int undoID = editMenu->insertItem("Undo", this, SLOT(editUndo()), CTRL+Key_Z);
	int redoID = editMenu->insertItem("Redo", this, SLOT(editRedo()), CTRL+Key_R);
	int clearID = editMenu->insertItem("Clear Map...", this, SLOT(editClear()));

	editMenu->setItemEnabled(undoID, false);
	editMenu->setItemEnabled(redoID, false);
	if (canvas() == NULL)
		editMenu->setItemEnabled(clearID, false);
	else
		editMenu->setItemEnabled(clearID, true);

	QVButtonGroup *mode = new QVButtonGroup("Editting Mode", editMenu);
	QRadioButton *drag = new QRadioButton("Drag", mode);
	QRadioButton *paint = new QRadioButton("Paint", mode);
	
	if (dragOn)
		drag->setChecked(true);
	else
		paint->setChecked(true);

	connect(drag, SIGNAL(toggled(bool)), this, SLOT(editMode()));
	editMenu->insertSeparator();
	editMenu->insertItem(mode);
} // editMenuSetup()

void MapGrid::viewMenuSetup()
{
	viewMenu->clear();

	QCheckBox *grid = new QCheckBox("Toggle &Grid", viewMenu);

	if (canvas() == NULL)
	{
		grid->setChecked(false);
		grid->setEnabled(false);
	} // can't have a grid without a canvas
	else
	{
		if (gridOn)
			grid->setChecked(true);
		else
			grid->setChecked(false);
		grid->setEnabled(true);
	} // we have a canvas so all is good

	viewMenu->insertItem(grid);
	connect(grid, SIGNAL(toggled(bool)), this, SLOT(viewToggleGrid()));

	QVButtonGroup *properties = new QVButtonGroup("Tile Properties", viewMenu);
	viewNone = new QRadioButton("None", properties);
	viewTreasure = new QRadioButton("Treasure", properties);
	viewEvent = new QRadioButton("Event", properties);
	viewOccupied = new QRadioButton("Occupied", properties);
	viewNoWalk = new QRadioButton("Not walkable", properties);

	// mutually exclusive radio buttons, hence else if structure
	if ((viewProperty & TREASURE) == TREASURE)
		viewTreasure->setChecked(true);
	else if ((viewProperty & EVENT) == EVENT)
		viewEvent->setChecked(true);
	else if ((viewProperty & OCCUPIED) == OCCUPIED)
		viewOccupied->setChecked(true);
	else if ((viewProperty & NOT_WALKABLE) == NOT_WALKABLE)
		viewNoWalk->setChecked(true);
	else
		viewNone->setChecked(true);
	
	viewMenu->insertSeparator();
	viewMenu->insertItem(properties);
} // viewMenuSetup()

void MapGrid::tileMenuSetup()
{
	tileMenu->clear();
	
	int horFlipID = tileMenu->insertItem("Flip Horizontally",this, SLOT(tileFlipHorizontal()));
	int vertFlipID = tileMenu->insertItem("Flip Vertically", this, SLOT(tileFlipVertical()));
	int rotClkID = tileMenu->insertItem("Rotate Clockwise",this, SLOT(tileRotateClockwise()));
	int rotCntClkID = tileMenu->insertItem("Rotate Counterclockwise", this, SLOT(tileRotateCounterClockwise()));
	
	tileMenu->setItemEnabled(horFlipID, false);
	tileMenu->setItemEnabled(vertFlipID, false);
	tileMenu->setItemEnabled(rotClkID, false);
	tileMenu->setItemEnabled(rotCntClkID, false);

	QVButtonGroup *mode = new QVButtonGroup("Properties", tileMenu);
	QRadioButton *noWalk = new QRadioButton("Not walkable", mode);
	QRadioButton *walk = new QRadioButton("Walkable", mode);
	properties = new QVButtonGroup(mode);
	tileTreasure = new QCheckBox("Treasure", properties);
	tileEvent = new QCheckBox("Event", properties);
	tileOccupied = new QCheckBox("Occupied", properties);

	if (walkOn)
	{
		walk->setChecked(true);
		properties->setEnabled(true);

		if ((tileProperties & TREASURE) == TREASURE)
			tileTreasure->setChecked(true);

		if ((tileProperties & EVENT) == EVENT)
			tileEvent->setChecked(true);

		if ((tileProperties & OCCUPIED) == OCCUPIED)
			tileOccupied->setChecked(true);
	} // enable desired options
	else
	{
		noWalk->setChecked(true);
		properties->setEnabled(false);
	} // disable unwanted options

	connect(walk, SIGNAL(toggled(bool)), this, SLOT(tileMode()));
	tileMenu->insertSeparator();
	tileMenu->insertItem(mode);
} // tileMenuSetup()

void MapGrid::viewMenuEvaluate()
{
	if (viewTreasure->isChecked())
		viewProperty = TREASURE;
	else if (viewEvent->isChecked())
		viewProperty = EVENT;
	else if (viewOccupied->isChecked())
		viewProperty = OCCUPIED;
	else if (viewNoWalk->isChecked())
		viewProperty = NOT_WALKABLE;
	else
		viewProperty = 0;

	if (canvas() != NULL)
	{
		QCanvasItemList list = canvas()->allItems();

		for (QCanvasItemList::Iterator it = list.begin(); it != list.end();++it)
		{
			if (((*it)->rtti() == TILE_RTTI) && (viewProperty != 0))
			{
				Tile *item = (Tile*)(*it);
				
				// set color masks appropriately
				if ((item->tileInfo.event_mask & viewProperty) == TREASURE)
				{
					QCanvasRectangle *rect= new QCanvasRectangle(int(item->x()),
						int (item->y()), TILE_WIDTH, TILE_HEIGHT, canvas());
					rect->setPen(Qt::NoPen);
					rect->setBrush(QBrush(QColor("gold"), Qt::Dense4Pattern));
					rect->setZ(2);		// FIXME: TEMPORARY!!!
					rect->show();
				}
				else if ((item->tileInfo.event_mask & viewProperty) == EVENT)
				{
					QCanvasRectangle *rect= new QCanvasRectangle(int(item->x()),
						int (item->y()), TILE_WIDTH, TILE_HEIGHT, canvas());
					rect->setPen(Qt::NoPen);
					rect->setBrush(QBrush(QColor("blue"), Qt::Dense4Pattern));
					rect->setZ(2);		// FIXME: TEMPORARY!!!
					rect->show();
				}
				else if ((item->tileInfo.event_mask & viewProperty) == OCCUPIED)
				{
					QCanvasRectangle *rect= new QCanvasRectangle(int(item->x()),
						int (item->y()), TILE_WIDTH, TILE_HEIGHT, canvas());
					rect->setPen(Qt::NoPen);
					rect->setBrush(QBrush(QColor("orange"), Qt::Dense4Pattern));
					rect->setZ(2);		// FIXME: TEMPORARY!!!
					rect->show();
				}
				else if ((item->tileInfo.event_mask & viewProperty) == NOT_WALKABLE)
				{
					QCanvasRectangle *rect= new QCanvasRectangle(int(item->x()),
						int (item->y()), TILE_WIDTH, TILE_HEIGHT, canvas());
					rect->setPen(Qt::NoPen);
					rect->setBrush(QBrush(QColor("red"), Qt::Dense4Pattern));
					rect->setZ(2);		// FIXME: TEMPORARY!!!
					rect->show();
				}
			} // only want to tint tiles
			else if ((*it)->rtti() == QCanvasItem::Rtti_Rectangle)
				delete *it;		// always delete a tint so we can tint over it
		} // iterate through all the items on the canvas

		canvas()->update();
	} // need a canvas for these operations!
} // viewMenuEvaluate()

void MapGrid::tileMenuEvaluate()
{
	if (!walkOn)
		tileProperties = hoa_map::local_map::NOT_WALKABLE;
	else
	{
		tileProperties = 0;
		if (tileTreasure->isChecked())
			tileProperties |= hoa_map::local_map::TREASURE;		// bitwise OR
		
		if (tileEvent->isChecked())
			tileProperties |= hoa_map::local_map::EVENT;
		
		if (tileOccupied->isChecked())
			tileProperties |= hoa_map::local_map::OCCUPIED;
	} // tile is walkable and can have a mixture of these properties
	
	// figure out which tile was right-clicked on to change its properties
	QCanvasItemList l = canvas()->collisions(inverseWorldMatrix().map(mapFromGlobal(menuPosition)));
	for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it)
	{
		if ((*it)->rtti() == TILE_RTTI)
		{
			Tile *item= (Tile*)(*it);
			if (!item->hit(mapFromGlobal(menuPosition)))
				continue;

			item->tileInfo.event_mask = tileProperties;
			return;		// only apply changes to one tile
		} // only apply changes to a tile, nothing else
	} // go through the list of possible items to change their properties
} // tileMenuEvaluate()

void MapGrid::editUndo()
{
} // editUndo()

void MapGrid::editRedo()
{
} // editRedo()

void MapGrid::editClear()
{
	QCanvasItemList list = canvas()->allItems();
	
	for (QCanvasItemList::Iterator it = list.begin(); it != list.end(); ++it)
		if ((*it)->rtti() != QCanvasItem::Rtti_Line)
			delete *it;

	canvas()->update();
} // editClear()

void MapGrid::editMode()
{
	std::cerr << "changing the editting mode... but not really since painting is not yet implemented ;)" << std::endl;
	if (dragOn)
		dragOn = false;
	else
		dragOn = true;
} // editMode()

void MapGrid::viewToggleGrid()
{
	// toggles the grid on or off
	if (gridOn)
	{
		QCanvasItemList list = canvas()->allItems();
	    for (QCanvasItemList::Iterator it=list.begin(); it != list.end(); ++it)
		{
        	if ((*it)->rtti() == QCanvasItem::Rtti_Line)
			{
            	QCanvasLine *line = (QCanvasLine*)(*it);
				line->hide();
        	} // hide the lines
    	} // go through the list of items on the canvas

		gridOn = false;
	} // the grid was on
	else
	{
		QCanvasItemList list = canvas()->allItems();
	    for (QCanvasItemList::Iterator it=list.begin(); it != list.end(); ++it)
		{
        	if ((*it)->rtti() == QCanvasItem::Rtti_Line)
			{
            	QCanvasLine *line = (QCanvasLine*)(*it);
				line->show();
        	} // show the lines
    	} // go through the list of items on the canvas

		gridOn = true;
	} // the grid was off

	canvas()->update();
} // viewToggleGrid

void MapGrid::tileFlipHorizontal()
{
	if (!mapChanged)
		mapChanged = true;
} // tileFlipHorizontal()

void MapGrid::tileFlipVertical()
{
	if (!mapChanged)
		mapChanged = true;
} // tileFlipVertical()

void MapGrid::tileRotateClockwise()
{
	if (!mapChanged)
		mapChanged = true;
} // tileRotateClockwise()

void MapGrid::tileRotateCounterClockwise()
{
	if (!mapChanged)
		mapChanged = true;
} // tileRotateCounterClockwise()

void MapGrid::tileMode()
{
	if (walkOn)
	{
		properties->setEnabled(false);
		walkOn = false;
	} // gray out impossible combinations
	else
	{
		properties->setEnabled(true);
		walkOn = true;

		if ((tileProperties & hoa_map::local_map::TREASURE) == hoa_map::local_map::TREASURE)
			tileTreasure->setChecked(true);

		if ((tileProperties & hoa_map::local_map::EVENT) == hoa_map::local_map::EVENT)
			tileEvent->setChecked(true);

		if ((tileProperties & hoa_map::local_map::OCCUPIED) == hoa_map::local_map::OCCUPIED)
			tileOccupied->setChecked(true);
	} // ungray them out
} // tileMode()

void MapGrid::saveMap(QFile &file)
{
	mapChanged = false;
} // saveMap()
