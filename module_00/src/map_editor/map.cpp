/******************************************************************************
 *
 *	Hero of Allacrost Map class implementation
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Date: 8/29/04
 *	Filename: map.cpp
 *
 *	$Id$
 *
 *	Description: This class constructs and manipulates a map used in the map
 *               editor.
 *
 *****************************************************************************/

#include "map.h"

using namespace hoa_map;

Map::Map(QWidget *parent, const QString &name)
	: QTable(parent, (const char*)name)
{	
/*	if (name == QString("Untitled"))
	{
		bool ok_pressed;	// TRUE = user pressed OK, FALSE otherwise
		
		// get map width from user
		int width = QInputDialog::getInteger("New Map...",
			"Enter map width (in tiles):", 0, 0, 1000, 1, &ok_pressed, this);
		if (ok_pressed)
			mapWidth = width;
		else
			mapWidth = 0;
		
		// get map height from user
		int height = QInputDialog::getInteger("New Map...",
			"Enter map height (in tiles):", 0, 0, 1000, 1, &ok_pressed, this);
		if (ok_pressed)
			mapHeight = height;
		else
			mapHeight = 0;
	} // create a new map
	else
	{
		QFile file(name);						// file user chose to open
		
		if (!file.open(IO_ReadWrite))
		{
			//statusBar()->message(
			//	QString("\'%1\' not readwriteable").arg(name, 5000));
			return;
		} // make sure file is readwriteable
		
		file.close();
		mapFileName = name;		// sets the map file name
//		getMapData();
	} // open an existing map
*/	
	// table settings
	setAcceptDrops(TRUE);
	dragging = FALSE;
	setCaption(QString("%1").arg(QString(name)));
	horizontalHeader()->hide();
	verticalHeader()->hide();
	setTopMargin(0);
	setLeftMargin(0);
//	setNumRows(mapHeight);
//	setNumCols(mapWidth);
//	for (int col = 0; col < mapWidth; col++)	
//		setColumnWidth(col, 32);
//	for (int row = 0; row < mapHeight; row++)
//		setRowHeight(row, 32);
	
//	createMenus();
	mapChanged = FALSE;		// map has not yet been modified
	//paintMap();
	//show();
	//table->setShowGrid(FALSE);

	// top menu creation
	theMenu = new QPopupMenu(this);
	theMenu->insertItem("Toggle &Grid", this, SLOT(ToggleGrid()));
} // Map constructor

Map::~Map()
{
} // Map destructor

bool Map::getChanged()
{
	return mapChanged;
} // getChanged()

QString Map::getFileName()
{
	return mapFileName;
} // getFileName()

void Map::setWidth(int width)
{
	setNumCols(width);
	mapWidth = width;
	for (int col = 0; col < mapWidth; col++)	
		setColumnWidth(col, 32);
} //setWidth(...)

void Map::setHeight(int height)
{
	setNumRows(height);
	mapHeight = height;
	for (int row = 0; row < mapHeight; row++)
		setRowHeight(row, 32);
} //setHeight(...)

void Map::setFileName(QString filename)
{
	mapFileName = filename;
} //setFileName(...)

/*void Map::getMapData()
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

void Map::paintMap()
{
	if(!locationVector.isEmpty())
		for(int h = 0; h < mapHeight; h++)
			for(int w = 0; w < mapWidth; w++) {
				QPixmap tile("img/" + fileNameList[locationVector[h * mapWidth + w]]);
				setPixmap(h, w, tile);
			}
	//show();
} // paintMap()

void Map::paintCell(QPainter *p, int row, int col)
{
	QPixmap tile("img/" + fileNameList[locationVector[row * mapWidth + col]]);
	//p->drawPixmap(col * cellWidth(), row * cellHeight(), tile);
}// paintCell(...)

void Map::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	
	if(!locationVector.isEmpty())
		for(int h = 0; h < mapHeight; h++)
			for(int w = 0; w < mapWidth; w++)
				paintCell(&painter, h, w);
} // paintEvent(...)
*/

void Map::dragEnterEvent(QDragEnterEvent *evt)
{
    if (QImageDrag::canDecode(evt))
        evt->accept();
} // dragEnterEvent(...)

void Map::dropEvent(QDropEvent *evt)
{
    QPixmap pix;		// pixmap to insert into the map (table)
    
    if (QImageDrag::decode(evt, pix)) {
        this->setPixmap( this->rowAt(evt->pos().y()), this->columnAt(evt->pos().x()), pix );
		if (!mapChanged)
			mapChanged = TRUE;
//		cerr << "x = " << evt->pos().x() << endl;
//		cerr << "y = " << evt->pos().y() << endl;
//		cerr << "row = " << this->rowAt(evt->pos().y()) << endl;
//		cerr << "col = " << this->columnAt(evt->pos().x()) << endl;
	}
} // dropEvent(...)

void Map::mousePressEvent(QMouseEvent *evt)
{
//	QTable::mousePressEvent(evt);
//	dragging = TRUE;
} // mousePressEvent(...)

void Map::mouseMoveEvent(QMouseEvent *)
{
//	if (dragging) {
//		QDragObject *d = new QImageDrag(currentItem()->pixmap(), this );
//		d->dragCopy(); // do NOT delete d.
//		dragging = FALSE;
//	}
} // mouseMoveEvent(...)

/*void Map::closeEvent(QCloseEvent *)
{
    eraseOK();
} // closeEvent(...) */

void Map::contextMenuEvent(QContextMenuEvent *)
{
	theMenu->exec(QCursor::pos());
} // contextMenuEvent(...)

void Map::ToggleGrid()
{
	// toggles the grid on or off
	if (showGrid())
		setShowGrid(false);
	else
		setShowGrid(true);
} // ToggleGrid

/*void Map::createMenus()
{
	// top menu creation
	theMenu = new QPopupMenu(this);
	
	// file menu creation
	fileMenu = new QPopupMenu(theMenu);
	theMenu->insertItem("File", fileMenu);
	fileMenu->insertItem("Save", this, SLOT(fileSave()), CTRL+Key_S);
	fileMenu->insertItem("Save As...", this, SLOT(fileSaveAs()));
	fileMenu->insertSeparator();
	fileMenu->insertItem("Close", this, SLOT(closeEvent(QCloseEvent *)),
		CTRL+Key_W);

	// edit menu creation
	editMenu = new QPopupMenu(theMenu);
	theMenu->insertItem("Edit", editMenu);
	editMenu->insertItem("Undo", this, SLOT(editUndo()), CTRL+Key_Z);
	editMenu->insertItem("Redo", this, SLOT(editRedo()), CTRL+Key_R);
	// show grid
	
	// tile menu creation
	tileMenu = new QPopupMenu(theMenu);
	theMenu->insertItem("Tile", tileMenu);
	tileMenu->insertItem("Flip Horizontally",this, SLOT(tileFlipHorizontal()));
	tileMenu->insertItem("Flip Vertically", this, SLOT(tileFlipVertical()));
	tileMenu->insertItem("Rotate Clockwise",this, SLOT(tileRotateClockwise()));
	tileMenu->insertItem("Rotate Counterclockwise", this,
		SLOT(tileRotateCounterClockwise()));
} // createMenus()

void Map::fileSaveAs()
{
	// get the file name from the user
	QString fileName = QFileDialog::getSaveFileName(
		"data/maps", "Maps (*.xml)", this, "file open",
		"HoA Map Editor -- File Save");
		
	if (!fileName.isEmpty())
	{
		int answer = 0;		// button pressed by user
		
		// ask to overwrite existing file
		if (QFile::exists(fileName))
			answer = QMessageBox::warning(
				this, "HoA Map Editor -- Overwrite File",
				QString("Overwrite\n\'%1\'?" ).arg(fileName),
				"&Yes", "&No", QString::null, 1, 1);
				
		if (answer == 0)
		{
			mapFileName = fileName;
//			updateRecentFiles(fileName);  <-- fix this ************************
			fileSave();
			return;
		} // save the file
    } // make sure the file name is not blank
	
    //statusBar()->message( "Save abandoned", 5000 );
} // saveAs()

void Map::fileSave()
{
	if (mapFileName.isEmpty())
	{
		fileSaveAs();
		return;
    } // gets a file name if it is blank

    QFile file(mapFileName);		// file to write to
	
    if (!file.open(IO_WriteOnly))
	{
		//statusBar()->message(
			//QString("Failed to save \'%1\'").arg(mapFileName), 5000);
		return;
    } // make sure file is openable
    
    saveMap(file);		// actually saves the map
    file.close();

    setCaption(QString("%1").arg(mapFileName));
    //statusBar()->message(QString("Saved \'%1\'").arg(mapFileName), 5000);
	mapChanged = FALSE;
} // fileSave()

void Map::editUndo()
{
	//statusBar()->message( "Hold your horses!"
		//" Undo will be implemented soon...", 5000 );
} // editUndo()

void Map::editRedo()
{
	//statusBar()->message( "Hold your horses!"
		//" Redo will be implemented soon...", 5000 );
} // editRedo()

void Map::tileFlipHorizontal()
{
	//statusBar()->message( "Hold your horses!"
		//" Flip Horizontally will be implemented soon...", 5000 );
	if (!mapChanged)
		mapChanged = TRUE;
} // tileFlipHorizontal()

void Map::tileFlipVertical()
{
	//statusBar()->message( "Hold your horses!"
		//" Flip Vertically will be implemented soon...", 5000 );
	if (!mapChanged)
		mapChanged = TRUE;
} // tileFlipVertical()

void Map::tileRotateClockwise()
{
	//statusBar()->message( "Hold your horses!"
		//" Rotate Clockwise will be implemented soon...", 5000 );
	if (!mapChanged)
		mapChanged = TRUE;
} // tileRotateClockwise()

void Map::tileRotateCounterClockwise()
{
	//statusBar()->message( "Hold your horses!"
		//" Rotate Counterclockwise will be implemented soon...", 5000 );
	if (!mapChanged)
		mapChanged = TRUE;
} // tileRotateCounterClockwise()

bool Map::eraseOK()
{
    if (mapChanged)
	{
		QString message;		// information to tell the user
		
		if (mapFileName.isEmpty())
			message = "Unnamed map ";
		else
			message = QString("Map '%1'\n").arg(mapFileName);
		message += "has been changed.";
	
		// ask user to save map
		int choice = QMessageBox::information(
			this, "HoA Map Editor -- Unsaved Changes",
			message, "&Save", "Cancel", "&Abandon", 0, 1);
			
		switch(choice)
		{
			case 0: // Save
				fileSave();
			break;
			case 2: // Abandon
				break;
			case 1: // Cancel
			default:
				return FALSE;
		} // do appropriate action on user's response
    } // map has been modified

    return TRUE;
} // eraseOK()
*/
void Map::saveMap(QFile &file)
{
	mapChanged = FALSE;
} // saveMap()
