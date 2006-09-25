///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    grid.cpp
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Source file for editor's grid, used for the OpenGL map portion
 *          where tiles are painted, edited, etc.
 *****************************************************************************/
			   
#include <iostream>
#include "grid.h"

using namespace hoa_data;
using namespace hoa_editor;
using namespace hoa_video;
using namespace std;

namespace hoa_editor
{
	LAYER_TYPE& operator++(LAYER_TYPE& value, int dummy) 
	{
		value=static_cast<LAYER_TYPE>(static_cast<int>(value)+1);
		return value;
	}
}

Grid::Grid(QWidget* parent, const QString& name, int width, int height)
	: QGLWidget(parent, (const char*) name)
{	
//	_walk_on = true;		// default value
//	_tile_properties = 0;	// default value (tiles are walkable)
//	_view_property = 0;		// default value (viewing mode off)
	_file_name = name;      // gives the map a name

	// Initial size
	_height = height;
	_width  = width;
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);

	// Default properties
	_changed = false;       // map has not yet been modified
	_grid_on = true;        // grid lines default to on
	_ll_on = true;          // lower layer default to on
	_ml_on = false;         // middle layer default to off
	_ul_on = false;         // upper layer default to off

	// Initialize layers
	for (int i = 0; i < _width * _height; i++)
	{
		_lower_layer.push_back(-1);
		_middle_layer.push_back(-1);
		_upper_layer.push_back(-1);
		tiles_walkable.push_back(-1);
		indiv_walkable.push_back(-1);
	} // -1 is used for no tiles

	// Create the video engine's singleton
	VideoManager = GameVideo::SingletonCreate();
} // Grid constructor

Grid::~Grid()
{
	// FIXME
//	VideoManager->DeleteImage();
	GameVideo::SingletonDestroy();
} // Grid destructor



// ********** Public functions **********

void Grid::SetChanged(bool value)
{
	_changed = value;
} // SetChanged(...)

void Grid::SetFileName(QString filename)
{
	_file_name = filename;
} // SetFileName(...)

void Grid::SetHeight(int height)
{
	_height = height;
	_changed = true;
} // SetHeight(...)

void Grid::SetWidth(int width)
{
	_width = width;
	_changed = true;
} // SetWidth(...)

void Grid::SetLLOn(bool value)
{
	_ll_on = value;
	updateGL();
} // SetLLOn(...)

void Grid::SetMLOn(bool value)
{
	_ml_on = value;
	updateGL();
} // SetMLOn(...)

void Grid::SetULOn(bool value)
{
	_ul_on = value;
	updateGL();
} // SetULOn(...)

void Grid::SetGridOn(bool value)
{
	_grid_on = value;
	updateGL();
} // SetGridOn(...)

std::vector<int32>& Grid::GetLayer(LAYER_TYPE layer) 
{
	switch(layer) {
		case LOWER_LAYER:
			return _lower_layer;
		case MIDDLE_LAYER:
			return _middle_layer;
		case UPPER_LAYER:
			return _upper_layer;
	}
	return _lower_layer;
}

void Grid::SetMusic(const std::string& music_file) 
{
	_music_file = music_file;
}

const std::string& Grid::GetMusic() const 
{
	return _music_file;
}

void Grid::LoadMap()
{
	DataDescriptor read_data;
	vector<int32> vect;             // used to read in vectors from the file

	if (!read_data.OpenFile(_file_name, READ))
		QMessageBox::warning(this, "Loading File...", QString("ERROR: could not open %1 for reading!").arg(_file_name));

	file_name_list.clear();
	_lower_layer.clear();
	_middle_layer.clear();
	_upper_layer.clear();

	_random_encounters=read_data.ReadBool("random_encounters");
	_encounter_rate=read_data.ReadInt("encounter_rate");
	
	_height = read_data.ReadInt("row_count");
	_width  = read_data.ReadInt("col_count");
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);

	read_data.OpenTable("tileset_names");
	uint32 table_size = read_data.GetTableSize();
	for (uint32 i = 1; i <= table_size; i++)
		tileset_list.append(read_data.ReadString(i));
	read_data.CloseTable();

	read_data.OpenTable("tile_filenames");
	table_size = read_data.GetTableSize();
	for (uint32 i = 1; i <= table_size; i++)
		file_name_list.append(QString(read_data.ReadString(i)).prepend("img/tiles/").append(".png"));
	read_data.CloseTable();

	read_data.OpenTable("lower_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.FillIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_lower_layer.push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.CloseTable();

	read_data.OpenTable("middle_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.FillIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_middle_layer.push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.CloseTable();

	read_data.OpenTable("upper_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.FillIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_upper_layer.push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.CloseTable();
	
	read_data.OpenTable("tile_walkable");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.FillIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			tiles_walkable.push_back(*it);
		vect.clear();
	} // iterate through the rows of the walkability table
	read_data.CloseTable();
	
	// Initialize individual tile walkability.
	for (int i = 0; i < _width * _height; i++)
		indiv_walkable.push_back(-1);

	// Load music
	read_data.OpenTable("music_filenames");
	if(read_data.GetErrorCode() == DATA_NO_ERRORS)
	{
		int Size=read_data.GetTableSize();
		if(Size==0)
			_music_file="None";
		else
			_music_file=read_data.ReadString(1);
		read_data.CloseTable();
	}

	_grid_on = true;        // grid lines default to on
	_ll_on = true;          // lower layer default to on
	_ml_on = true;          // middle layer default to off
	_ul_on = true;          // upper layer default to off
	_changed = false;       // map has not been changed yet
	updateGL();
} // LoadMap()

void Grid::SaveMap()
{
	char buffer[2]; // used for converting an int to a string with sprintf
	int i;      // Lua table index / Loop counter variable
	int j;      // Loop counter variable FIXME: temporary!
	vector<int32>::iterator it;  // used to iterate through the layers
	vector<int32> layer_row;     // one row of a layer
	DataDescriptor write_data;
	
	if (!write_data.OpenFile(_file_name, WRITE))
		QMessageBox::warning(this, "Saving File...", QString("ERROR: could not open %1 for writing!").arg(_file_name));
	else
	{
		write_data.WriteComment(_file_name);
		write_data.InsertNewLine();

		write_data.WriteComment("Whether or not we have random encounters, and if so the rate of encounter");
		write_data.WriteBool("random_encounters", _random_encounters);
		write_data.WriteInt("encounter_rate", _encounter_rate);
		write_data.InsertNewLine();

		write_data.WriteComment("The music files used as background music on this map.");
		write_data.BeginTable("music_filenames");
		if(_music_file!="None")
			write_data.WriteString(1,_music_file);
		write_data.EndTable();
		write_data.InsertNewLine();

		write_data.WriteComment("The number of rows and columns of tiles that compose the map");
		write_data.WriteInt("row_count", _height);
		write_data.WriteInt("col_count", _width);
		write_data.InsertNewLine();

		write_data.WriteComment("The names of the tilesets used, with the path and file extension omitted (note that the indeces begin with 1, not 0)");
		write_data.BeginTable("tileset_names");
		i = 0;
		for (QStringList::Iterator qit = tileset_list.begin();
		     qit != tileset_list.end(); ++qit)
		{
			i++;
			write_data.WriteString(i, (*qit).ascii());
		} // iterate through tileset_list writing each element
		write_data.EndTable();
		write_data.InsertNewLine();

		write_data.WriteComment("The names of the tile image files used, with the path and file extension omitted (note that the indeces begin with 1, not 0)");
		write_data.BeginTable("tile_filenames");
		i = 0;
		for (QStringList::Iterator qit = file_name_list.begin();
		     qit != file_name_list.end(); ++qit)
		{
			i++;
			QString temp = *qit;
			temp.remove(".png").remove("img/tiles/");
			write_data.WriteString(i, temp.ascii());
		} // iterate through file_name_list writing each element
		write_data.EndTable();
		write_data.InsertNewLine();

		// FIXME: hard-coded for now
		write_data.WriteComment("This structure forms still or animate tile images. In this case, all of our tiles are stills, but note that each element must still be a table.");
		write_data.BeginTable("tile_mappings");
		vector<int32> vect_single;
		for (j = 0; j < i; j++)
		{
			vect_single.push_back(j);
			sprintf(buffer, "%d", j);
			write_data.WriteIntVector(buffer, vect_single);
			vect_single.clear();
		}
		write_data.EndTable();
		write_data.InsertNewLine();

		write_data.WriteComment("The lower tile layer. The numbers are indeces to the tile_mappings table.");
		write_data.BeginTable("lower_layer");
		it = _lower_layer.begin();
		for (int row = 0; row < _height; row++)
		{
			for (int col = 0; col < _width; col++)
			{
				layer_row.push_back(*it);
				it++;
			} // iterate through the columns of the lower layer
			sprintf(buffer, "%d", row);
			write_data.WriteIntVector(buffer, layer_row);
			layer_row.clear();
		} // iterate through the rows of the lower layer
		write_data.EndTable();
		write_data.InsertNewLine();

		write_data.WriteComment("The middle tile layer. The numbers are indeces to the tile_mappings table.");
		write_data.BeginTable("middle_layer");
		it = _middle_layer.begin();
		for (int row = 0; row < _height; row++)
		{
			for (int col = 0; col < _width; col++)
			{
				layer_row.push_back(*it);
				it++;
			} // iterate through the columns of the middle layer
			sprintf(buffer, "%d", row);
			write_data.WriteIntVector(buffer, layer_row);
			layer_row.clear();
		} // iterate through the rows of the middle layer
		write_data.EndTable();
		write_data.InsertNewLine();

		write_data.WriteComment("The upper tile layer. The numbers are indeces to the tile_mappings table.");
		write_data.BeginTable("upper_layer");
		it = _upper_layer.begin();
		for (int row = 0; row < _height; row++)
		{
			for (int col = 0; col < _width; col++)
			{
				layer_row.push_back(*it);
				it++;
			} // iterate through the columns of the upper layer
			sprintf(buffer, "%d", row);
			write_data.WriteIntVector(buffer, layer_row);
			layer_row.clear();
		} // iterate through the rows of the upper layer
		write_data.EndTable();
		write_data.InsertNewLine();

		// Create vector of 0s.
		vector<int32> vect_0s(_width, 0);

		// FIXME: hard-coded for now
		write_data.WriteComment("Tile properties, not including walkability status. Valid range: [0-255]");
		write_data.BeginTable("tile_properties");
		for (i = 0; i < _height; i++)
		{
			sprintf(buffer, "%d", i);
			write_data.WriteIntVector(buffer, vect_0s);
		}
		write_data.EndTable();
		write_data.InsertNewLine();

		write_data.WriteComment("Walkability status of tiles for 8 height levels. Non-zero indicates walkable. Valid range: [0-255]");
		write_data.BeginTable("tile_walkable");
		DataDescriptor read_data;
		if (!read_data.OpenFile("dat/tilesets/tiles_database.lua", READ))
			QMessageBox::warning(this, "Tiles Database",
				QString("ERROR: could not open dat/tilesets/tiles_database.lua for reading!"));
		else
		{
			for (int row = 0; row < _height; row++)
			{
				for (int col = 0; col < _width; col++)
				{
					// Individual tile property supersedes anything else.
					if (indiv_walkable[row * _width + col] != -1)
						layer_row.push_back(indiv_walkable[row * _width + col]);
					else if (tiles_walkable[row * _width + col] != -1)
						layer_row.push_back(tiles_walkable[row * _width + col]);
					else
					{
						QString temp = file_name_list[_lower_layer[row * _width + col]];
						temp.remove(".png").remove("img/tiles/");
						read_data.OpenTable("tile_filenames");
						uint32 table_size = read_data.GetTableSize();
						uint32 index = 0;
						QString filename = "";
						while (filename != temp && index < table_size)
						{
							index++;
							filename = read_data.ReadString(index);
						} // find index of current tile in the database
						read_data.CloseTable();
					
						if (filename == temp)
						{
							read_data.OpenTable("tile_properties");
							layer_row.push_back(read_data.ReadInt(index));
							read_data.CloseTable();
						} // tile exists in the database
					} // falls back to global property in tile database
				} // iterate through the columns of the lower layer
				sprintf(buffer, "%d", row);
				write_data.WriteIntVector(buffer, layer_row);
				layer_row.clear();
			} // iterate through the rows of the lower layer
			read_data.CloseFile();
		} // file was successfully opened
		write_data.EndTable();
		write_data.InsertNewLine();

		// Create vector of -1s.
		vector<int32> vect(_width, -1);

		// FIXME: hard-coded for now
		write_data.WriteComment("Events associated with each tile. -1 indicates no event.");
		write_data.BeginTable("tile_events");
		for (i = 0; i < _height; i++)
		{
			sprintf(buffer, "%d", i);
			write_data.WriteIntVector(buffer, vect);
		}
		write_data.EndTable();

		write_data.InsertNewLine();
		write_data.CloseFile();
	
		_changed = false;
	} // successfully opened file for writing
} // SaveMap()



// ********** Protected slots **********

void Grid::initializeGL()
{
	VideoManager->SetTarget(VIDEO_TARGET_QT_WIDGET);
	VideoManager->SingletonInitialize();
	VideoManager->ToggleFPS();
} // initializeGL()

void Grid::paintGL()
{
	int32 col;                       // tile array loop index
	vector<int32>::iterator it;      // used to iterate through tile arrays
	hoa_video::StillImage tile;      // tile to draw to screen

	// Setup drawing parameters
	VideoManager->SetCoordSys(0.0f, VideoManager->GetWidth() / TILE_WIDTH,
		VideoManager->GetHeight() / TILE_HEIGHT, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	VideoManager->Clear(Color::white);
	tile.SetDimensions(1.0f, 1.0f);  // all tiles are same size (for now)

	// Draw lower layer
	if (_ll_on)
	{
		VideoManager->Move(0.0f, 0.0f);
		col = 0;
		for (it = _lower_layer.begin(); it != _lower_layer.end(); it++)
		{
			if (*it != -1)
			{
				tile.SetFilename(file_name_list[*it]);
				VideoManager->LoadImage(tile);
				VideoManager->DrawImage(tile);
			} // a tile exists to draw
			col = ++col % _width;
			if (col == 0)
				VideoManager->MoveRelative(-_width + 1, 1.0f);
			else
				VideoManager->MoveRelative(1.0f, 0.0f);
		} // iterate through lower layer
	} // lower layer must be viewable

	// Draw middle layer
	if (_ml_on)
	{
		VideoManager->Move(0.0f, 0.0f);
		col = 0;
		for (it = _middle_layer.begin(); it != _middle_layer.end(); it++)
		{
			if (*it != -1)
			{
				tile.SetFilename(file_name_list[*it]);
				VideoManager->LoadImage(tile);
				VideoManager->DrawImage(tile);
			} // a tile exists to draw
			col = ++col % _width;
			if (col == 0)
				VideoManager->MoveRelative(-_width + 1, 1.0f);
			else
				VideoManager->MoveRelative(1.0f, 0.0f);
		} // iterate through middle layer
	} // middle layer must be viewable

	// Draw upper layer
	if (_ul_on)
	{
		VideoManager->Move(0.0f, 0.0f);
		col = 0;
		for (it = _upper_layer.begin(); it != _upper_layer.end(); it++)
		{
			if (*it != -1)
			{
				tile.SetFilename(file_name_list[*it]);
				VideoManager->LoadImage(tile);
				VideoManager->DrawImage(tile);
			} // a tile exists to draw
			col = ++col % _width;
			if (col == 0)
				VideoManager->MoveRelative(-_width + 1, 1.0f);
			else
				VideoManager->MoveRelative(1.0f, 0.0f);
		} // iterate through upper layer
	} // upper layer must be viewable

	// If grid is toggled on, draw it
	if (_grid_on)
		VideoManager->DrawGrid(0.0f, 0.0f, 1.0f, 1.0f, Color::black);
} // paintGL()

void Grid::resizeGL(int w, int h)
{
	VideoManager->SetResolution(w, h);
	VideoManager->ApplySettings();
} // resizeGL(...)



/*
// what about contentsDragEnterEvent???
void Grid::dragEnterEvent(QDragEnterEvent *evt)
{
    if (QImageDrag::canDecode(evt))
        evt->accept();
} // dragEnterEvent(...)

void Grid::dropEvent(QDropEvent *evt)
{
    QImage img;		// image to insert into the map
    
    if (QImageDrag::decode(evt, img) && (canvas() != NULL))
	{
		//QString name = temp->currentItem()->text().remove(".png");
		//int index = _file_name_list.findIndex(name);
		// Check if tile name is new.
		//if (index == -1)
		//	_file_name_list.append(name);
		
		//Tile *tile = new Tile(name, img, canvas());
		//QPoint point = inverseWorldMatrix().map(evt->pos());
		
		// the division here will effectively snap the tile to the grid
		// yes, it looks dumb. is there another way to round down to the
		// nearest multiple of 32?
		//tile->move((point.x() + horizontalScrollBar()->value())
		//			/ TILE_WIDTH * TILE_WIDTH,
		//		   (point.y() + verticalScrollBar()->value())
		//			/ TILE_HEIGHT * TILE_HEIGHT);
		//tile->setZ(0);		// sets height of tile TODO: create a menu option
		// tile->tileInfo.lower_layer = 1; FIXME: perhaps not needed here
		//tile->tileInfo.upper_layer = -1; // TEMPORARY!!! FIXME: this neither
		//tile->tileInfo.event_mask = tileProperties;
		//tile->show();
		//canvas()->update();
		_changed = TRUE;
	} // must be able to decode the drag in order to place the image
} // dropEvent(...)

void Grid::mousePressEvent(QMouseEvent *)
{
	QTable::mousePressEvent(evt);
	dragging = TRUE;
} // mousePressEvent(...)

void Grid::mouseMoveEvent(QMouseEvent *)
{
	if (dragging) {
		QDragObject *d = new QImageDrag(currentItem()->pixmap(), this );
		d->dragCopy(); // do NOT delete d.
		dragging = FALSE;
	}
} // mouseMoveEvent(...)

void Grid::contentsMousePressEvent(QMouseEvent *evt)
{
	if (evt->button() == Qt::LeftButton)
	{
//		if (dragOn)
//		{
			QPoint p = inverseWorldMatrix().map(evt->pos());
			QCanvasItemList l = canvas()->collisions(p);
			for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it)
			{
				if ((*it)->rtti() == TILE_RTTI)
				{
					Tile *item= (Tile*)(*it);
					if (!item->Hit(p))
						continue;

					_moving = *it;
					_moving_start = p;
					return;
				} // only want to move a tile, nothing else
			} // go through the list of possible items to move
//		} // not painting tiles means we want to start moving them instead
	} // only make the left mouse button useful/active

	_moving = 0;
} // contentsMousePressEvent(...)

void Grid::contentsMouseMoveEvent(QMouseEvent *evt)
{
	if (_moving)
	{
		QPoint p = inverseWorldMatrix().map(evt->pos());
		_moving->moveBy(p.x() - _moving_start.x(), p.y() - _moving_start.y());
		_moving_start = p;
		canvas()->update();
	} // only if the user is moving an object
} // contentsMouseMoveEvent(...)

void Grid::contentsMouseReleaseEvent(QMouseEvent *evt)
{
	if (_moving)
	{
		QPoint point = inverseWorldMatrix().map(evt->pos());
		
		// the division here will effectively snap the tile to the grid
		// yes, it looks dumb. is there another way to round down to the
		// nearest multiple of 32?
		_moving->move(point.x() / TILE_WIDTH * TILE_WIDTH,
					  point.y() / TILE_HEIGHT * TILE_HEIGHT);
		canvas()->update();
		_changed = TRUE;
	} // only if the user is moving an object
} // contentsMouseReleaseEvent(...)

void Grid::contentsMouseDoubleClickEvent(QMouseEvent *evt)
{
    if (canvas() != NULL)
	{
// FIXME FIXME FIXME OMG what a hack this is FIXME FIXME FIXME
//		std::cerr << "Blah" << std::endl;
//		if (((MapEditor*) parent()) == NULL)
//			std::cerr << "parent() is NULL" << std::endl;
		std::cerr << "1" << std::endl;
//		if (((MapEditor*) parent())->tiles == NULL)
//			std::cerr << "tiles is NULL" << std::endl;
		std::cerr << "2" << std::endl;
		//((MapEditor*) parent())->tiles->printBlah();
		if (temp->count() != 0)
			std::cerr << "count() isn't 0" << std::endl;
		std::cerr << "3" << std::endl;
		if (temp->currentItem()->pixmap() == NULL)
			std::cerr << "pixmap() is NULL" << std::endl;
		std::cerr << "4" << std::endl;
		if (temp->currentItem()->pixmap()->convertToImage() == NULL)
			std::cerr << "convertToImage() is NULL" << std::endl;
		std::cerr << "5" << std::endl;
//		QImage img = temp->currentItem()->pixmap()->convertToImage();
		//QString name = temp->currentItem()->text().remove(".png");
		//int index = _file_name_list.findIndex(name);
		// Check if tile name is new.
		//if (index == -1)
		//	_file_name_list.append(name);
		
		//Tile *tile = new Tile(name, img, canvas());
		//QPoint point = inverseWorldMatrix().map(evt->pos());
		
		// the division here will effectively snap the tile to the grid
		// yes, it looks dumb. is there another way to round down to the
		// nearest multiple of 32?
		//tile->move((point.x() + horizontalScrollBar()->value())
		//			/ TILE_WIDTH * TILE_WIDTH,
		//		   (point.y() + verticalScrollBar()->value())
		//			/ TILE_HEIGHT * TILE_HEIGHT);
		//tile->setZ(0);		// sets height of tile TODO: create a menu option
		// tile->tileInfo.lower_layer = 1; FIXME: perhaps not needed here
		//tile->tileInfo.upper_layer = -1; // TEMPORARY!!! FIXME: this neither
		//tile->tileInfo.event_mask = tileProperties;
		//tile->show();
		//canvas()->update();
		_changed = TRUE;
	} // better have a canvas first
} // contentsMouseDoubleClickEvent(...)

void Grid::contentsContextMenuEvent(QContextMenuEvent *)
{
	_menu_position = QCursor::pos();
	_the_menu->exec(_menu_position);
} // contentsContextMenuEvent(...)

void Grid::CreateGrid()
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

	_grid_on = true;
} //CreateGrid()

void Grid::_CreateMenus()
{
	// top menu creation
	_the_menu = new QPopupMenu(this);
	
	// edit menu creation
	_edit_menu = new QPopupMenu(_the_menu);
	connect(_edit_menu, SIGNAL(aboutToShow()), this, SLOT(_EditMenuSetup()));

	// view menu creation
	_view_menu = new QPopupMenu(_the_menu);
	connect(_view_menu, SIGNAL(aboutToShow()), this, SLOT(_ViewMenuSetup()));
	connect(_view_menu, SIGNAL(aboutToHide()), this, SLOT(_ViewMenuEvaluate()));
	
	// tile menu creation
	_tile_menu = new QPopupMenu(_the_menu);
	connect(_tile_menu, SIGNAL(aboutToShow()), this, SLOT(_TileMenuSetup()));
	connect(_tile_menu, SIGNAL(aboutToHide()), this, SLOT(_TileMenuEvaluate()));
	
	_the_menu->insertItem("Edit", _edit_menu);
	_the_menu->insertItem("View", _view_menu);
	_the_menu->insertItem("Tile", _tile_menu);
} // _CreateMenus()

void Grid::_EditMenuSetup()
{
	_edit_menu->clear();
	
	int undo_ID = _edit_menu->insertItem("Undo", this, SLOT(_EditUndo()), CTRL+Key_Z);
	int redo_ID = _edit_menu->insertItem("Redo", this, SLOT(_EditRedo()), CTRL+Key_R);
	int clear_ID = _edit_menu->insertItem("Clear Map...", this, SLOT(_EditClear()));

	_edit_menu->setItemEnabled(undo_ID, false);
	_edit_menu->setItemEnabled(redo_ID, false);
	if (canvas() == NULL)
		_edit_menu->setItemEnabled(clear_ID, false);
	else
		_edit_menu->setItemEnabled(clear_ID, true);

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
} // _EditMenuSetup()

void Grid::_ViewMenuSetup()
{
	_view_menu->clear();

	QCheckBox* grid = new QCheckBox("Toggle &Grid", _view_menu);

	if (canvas() == NULL)
	{
		grid->setChecked(false);
		grid->setEnabled(false);
	} // can't have a grid without a canvas
	else
	{
		if (_grid_on)
			grid->setChecked(true);
		else
			grid->setChecked(false);
		grid->setEnabled(true);
	} // we have a canvas so all is good

	_view_menu->insertItem(grid);
	connect(grid, SIGNAL(toggled(bool)), this, SLOT(_ViewToggleGrid()));

	QVButtonGroup* properties = new QVButtonGroup("Tile Properties", _view_menu);
	_view_none = new QRadioButton("None", properties);
	_view_treasure = new QRadioButton("Treasure", properties);
	_view_event = new QRadioButton("Event", properties);
	_view_occupied = new QRadioButton("Occupied", properties);
	_view_no_walk = new QRadioButton("Not walkable", properties);

	// mutually exclusive radio buttons, hence else if structure
	if ((_view_property & TREASURE) == TREASURE)
		_view_treasure->setChecked(true);
	else if ((_view_property & EVENT) == EVENT)
		_view_event->setChecked(true);
	else if ((_view_property & OCCUPIED) == OCCUPIED)
		_view_occupied->setChecked(true);
	else if ((_view_property & NOT_WALKABLE) == NOT_WALKABLE)
		_view_no_walk->setChecked(true);
	else
		viewNone->setChecked(true);

	_view_menu->insertSeparator();
	_view_menu->insertItem(properties);
} // viewMenuSetup()

void Grid::_TileMenuSetup()
{
	_tile_menu->clear();
	
	int horFlipID = tileMenu->insertItem("Flip Horizontally",this, SLOT(tileFlipHorizontal()));
	int vertFlipID = tileMenu->insertItem("Flip Vertically", this, SLOT(tileFlipVertical()));
	int rotClkID = tileMenu->insertItem("Rotate Clockwise",this, SLOT(tileRotateClockwise()));
	int rotCntClkID = tileMenu->insertItem("Rotate Counterclockwise", this, SLOT(tileRotateCounterClockwise()));
	
	tileMenu->setItemEnabled(horFlipID, false);
	tileMenu->setItemEnabled(vertFlipID, false);
	tileMenu->setItemEnabled(rotClkID, false);
	tileMenu->setItemEnabled(rotCntClkID, false);

	QVButtonGroup* mode = new QVButtonGroup("Properties", _tile_menu);
	QRadioButton* no_walk = new QRadioButton("Not walkable", mode);
	QRadioButton* walk = new QRadioButton("Walkable", mode);
	_properties = new QVButtonGroup(mode);
	_tile_treasure = new QCheckBox("Treasure", _properties);
	_tile_event = new QCheckBox("Event", _properties);
	_tile_occupied = new QCheckBox("Occupied", _properties);

	if (_walk_on)
	{
		walk->setChecked(true);
		_properties->setEnabled(true);

		if ((_tile_properties & TREASURE) == TREASURE)
			_tile_treasure->setChecked(true);

		if ((_tile_properties & EVENT) == EVENT)
			_tile_event->setChecked(true);

		if ((_tile_properties & OCCUPIED) == OCCUPIED)
			_tile_occupied->setChecked(true);
	} // enable desired options
	else
	{
		no_walk->setChecked(true);
		_properties->setEnabled(false);
	} // disable unwanted options

	connect(walk, SIGNAL(toggled(bool)), this, SLOT(_TileMode()));
	_tile_menu->insertSeparator();
	_tile_menu->insertItem(mode);
} // _TileMenuSetup()

void Grid::_ViewMenuEvaluate()
{
	if (_view_treasure->isChecked())
		_view_property = TREASURE;
	else if (_view_event->isChecked())
		_view_property = EVENT;
	else if (_view_occupied->isChecked())
		_view_property = OCCUPIED;
	else if (_view_no_walk->isChecked())
		_view_property = NOT_WALKABLE;
	else
		_view_property = 0;

	if (canvas() != NULL)
	{
		QCanvasItemList list = canvas()->allItems();

		for (QCanvasItemList::Iterator it = list.begin(); it != list.end();++it)
		{
			if (((*it)->rtti() == TILE_RTTI) && (_view_property != 0))
			{
				Tile *item = (Tile*)(*it);
				
				// set color masks appropriately
				if ((item->tileInfo.event_mask & _view_property) == TREASURE)
				{
					QCanvasRectangle *rect= new QCanvasRectangle(int(item->x()),
						int (item->y()), TILE_WIDTH, TILE_HEIGHT, canvas());
					rect->setPen(Qt::NoPen);
					rect->setBrush(QBrush(QColor("gold"), Qt::Dense4Pattern));
					rect->setZ(2);		// FIXME: TEMPORARY!!!
					rect->show();
				}
				else if ((item->tileInfo.event_mask & _view_property) == EVENT)
				{
					QCanvasRectangle *rect= new QCanvasRectangle(int(item->x()),
						int (item->y()), TILE_WIDTH, TILE_HEIGHT, canvas());
					rect->setPen(Qt::NoPen);
					rect->setBrush(QBrush(QColor("blue"), Qt::Dense4Pattern));
					rect->setZ(2);		// FIXME: TEMPORARY!!!
					rect->show();
				}
				else if ((item->tileInfo.event_mask & _view_property) == OCCUPIED)
				{
					QCanvasRectangle *rect= new QCanvasRectangle(int(item->x()),
						int (item->y()), TILE_WIDTH, TILE_HEIGHT, canvas());
					rect->setPen(Qt::NoPen);
					rect->setBrush(QBrush(QColor("orange"), Qt::Dense4Pattern));
					rect->setZ(2);		// FIXME: TEMPORARY!!!
					rect->show();
				}
				else if ((item->tileInfo.event_mask & _view_property) == NOT_WALKABLE)
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
} // _ViewMenuEvaluate()

void Grid::_TileMenuEvaluate()
{
	if (!_walk_on)
		_tile_properties = hoa_map::local_map::NOT_WALKABLE;
	else
	{
		_tile_properties = 0;
		if (_tile_treasure->isChecked())
			_tile_properties |= hoa_map::local_map::TREASURE;	// bitwise OR
		
		if (_tileEvent->isChecked())
			_tile_properties |= hoa_map::local_map::EVENT;
		
		if (_tile_occupied->isChecked())
			_tile_properties |= hoa_map::local_map::OCCUPIED;
	} // tile is walkable and can have a mixture of these properties
	
	// figure out which tile was right-clicked on to change its properties
	QCanvasItemList l = canvas()->collisions(inverseWorldMatrix().
		map(mapFromGlobal(_menu_position)));
	for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it)
	{
		if ((*it)->rtti() == TILE_RTTI)
		{
			Tile* item = (Tile*)(*it);
			if (!item->Hit(mapFromGlobal(_menu_position)))
				continue;

//			item->tileInfo.event_mask = _tile_properties;
			return;		// only apply changes to one tile
		} // only apply changes to a tile, nothing else
	} // go through the list of possible items to change their properties
} // _TileMenuEvaluate()

void Grid::_EditUndo()
{
} // _EditUndo()

void Grid::_EditRedo()
{
} // _EditRedo()

void Grid::_EditClear()
{
	QCanvasItemList list = canvas()->allItems();
	
	for (QCanvasItemList::Iterator it = list.begin(); it != list.end(); ++it)
		if ((*it)->rtti() != QCanvasItem::Rtti_Line)
			delete *it;

	canvas()->update();
} // _EditClear()

void Grid::_EditMode()
{
	std::cerr << "changing the editting mode... but not really since painting is not yet implemented ;)" << std::endl;
	if (_drag_on)
		_drag_on = false;
	else
		_drag_on = true;
} // _EditMode()

void Grid::_ViewToggleGrid()
{
	// toggles the grid on or off
	if (_grid_on)
	{
		QCanvasItemList list = canvas()->allItems();
	    for (QCanvasItemList::Iterator it=list.begin(); it != list.end(); ++it)
		{
        	if ((*it)->rtti() == QCanvasItem::Rtti_Line)
			{
            	QCanvasLine* line = (QCanvasLine*)(*it);
				line->hide();
        	} // hide the lines
    	} // go through the list of items on the canvas

		_grid_on = false;
	} // the grid was on
	else
	{
		QCanvasItemList list = canvas()->allItems();
	    for (QCanvasItemList::Iterator it=list.begin(); it != list.end(); ++it)
		{
        	if ((*it)->rtti() == QCanvasItem::Rtti_Line)
			{
            	QCanvasLine* line = (QCanvasLine*)(*it);
				line->show();
        	} // show the lines
    	} // go through the list of items on the canvas

		_grid_on = true;
	} // the grid was off

	canvas()->update();
} // viewToggleGrid

void Grid::tileFlipHorizontal()
{
	if (!mapChanged)
		mapChanged = true;
} // tileFlipHorizontal()

void Grid::tileFlipVertical()
{
	if (!mapChanged)
		mapChanged = true;
} // tileFlipVertical()

void Grid::tileRotateClockwise()
{
	if (!mapChanged)
		mapChanged = true;
} // tileRotateClockwise()

void Grid::tileRotateCounterClockwise()
{
	if (!mapChanged)
		mapChanged = true;
} // tileRotateCounterClockwise()

void Grid::_TileMode()
{
	if (_walk_on)
	{
		_properties->setEnabled(false);
		_walk_on = false;
	} // gray out impossible combinations
	else
	{
		_properties->setEnabled(true);
		_walk_on = true;

		if ((_tile_properties & hoa_map::local_map::TREASURE) == hoa_map::local_map::TREASURE)
			_tile_treasure->setChecked(true);

		if ((_tile_properties & hoa_map::local_map::EVENT) == hoa_map::local_map::EVENT)
			_tile_event->setChecked(true);

		if ((_tile_properties & hoa_map::local_map::OCCUPIED) == hoa_map::local_map::OCCUPIED)
			_tile_occupied->setChecked(true);
	} // ungray them out
} // _TileMode()*/
