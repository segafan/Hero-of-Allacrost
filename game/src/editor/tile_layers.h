///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tile_layers.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for tile layer data and view classes
***
*** This file contains classes that represent components of the data model for
*** a map. This data includes layers of tiles and map contexts, and a data
*** managing class that provides an interface for the TileView class to request
*** changes to the data.
*** **************************************************************************/

#ifndef __TILE_LAYERS_HEADER__
#define __TILE_LAYERS_HEADER__

#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "editor_utils.h"
#include "tileset.h"

namespace hoa_editor {

class MapData;

/** ****************************************************************************
*** \brief Represents a layer of tiles on the map
***
*** A tile layer is simply a 2D array of indeces that map to a specific tile among the
*** selected tilesets. Each tile context maintains its own set of tile layers that it
*** manages. This class provides public methods for setting the values of tiles within
*** the layer, but any operations that change the size of the layer are kept private and
*** are only able to be modified by the layer's containing TileContext.
***
*** \note There are additional properties about a tile layer that are not stored here. For
*** example, the layer's visibility, whether or not collision data is active, and the tileset's
*** name. This is because every map context shares the same layers, and these properties would
*** need to be duplicated for every TileLayer object. Instead, look to the class TileLayerProperties,
*** which contains a single set of these properties for every tile layer shared across all contexts.
*** ***************************************************************************/
class TileLayer {
	friend class MapData;

public:
	TileLayer()
		{}

	TileLayer(uint32 length, uint32 height)
		{ _ResizeLayer(length, height); }

	//! \brief Returns the number of tiles high that the layer is
	uint32 GetHeight() const
		{ return _tiles.size(); }

	//! \brief Returns the number of tiles long that the layer is
	uint32 GetLength() const
		{ if (_tiles.empty() == true) return 0; else return _tiles[0].size(); }

	/** \brief Retrieves the tile at a specific location
	*** \param x The x (column) location of the tile
	*** \param y The y (row) location of the tile
	*** \return The index value of the tile. Returns NO_TILE if the coordinate arguments are invalid.
	**/
	int32 GetTile(uint32 x, uint32 y) const;

	/** \brief Sets the value of a tile at a specific location
	*** \param x The x (column) location of the tile
	*** \param y The y (row) location of the tile
	*** \param value The value to set for the title
	***
	*** If the coordinates are invalid, no change will take place.
	**/
	void SetTile(uint32 x, uint32 y, int32 value);

	/** \brief Clears the tile at a specific location by setting it's value to NO_TILE
	*** \param x The x (column) location of the tile
	*** \param y The y (row) location of the tile
	***
	*** If the coordinates are invalid, no change will take place.
	**/
	void ClearTile(uint32 x, uint32 y)
		{ SetTile(x, y, NO_TILE); }

	/** \brief Sets every tile location in the layer to the same value
	*** \param value The value to set each tile to
	**/
	void FillLayer(int32 value);

	//! \brief Clears the layer of all data, setting each tile in the layer to NO_TILE
	void ClearLayer()
		{ FillLayer(NO_TILE); }

	/** \brief Returns a reference to the tile container
	*** \note This method is provided for convenience for operations such as drawing functions.
	*** The caller should not attempt to resize the tiles container, as that would lead to erroneous
	*** behavior.
	**/
	std::vector<std::vector<int32> >& GetTiles()
		{ return _tiles; }

private:
    //! \brief Represents the tile indeces, where a tile at (x,y) is accessed as _tiles[y][x]
    std::vector<std::vector<int32> > _tiles;

	/** \brief Adds a new row of tiles to a specified index in the table
	*** \param row_index The index where the row should be added
	*** \param value The value to set each tile in the newly added row
	***
	*** Specifying the row_index as the height of the layer results in appending the row to the end
	*** of the existing rows. Any value beyond this range is considered invalid and no operation will
	*** take place.
	**/
	void _AddLayerRow(uint32 row_index, int32 value);

	/** \brief Adds a new row of empty tiles to a specified index in the table
	*** \param row_index The index where the row should be added
	***
	*** Specifying the row_index as the height of the layer results in appending the row to the end
	*** of the existing rows. Any value beyond this range is considered invalid and no operation will
	*** take place.
	**/
	void _AddLayerRow(uint32 row_index)
		{ _AddLayerRow(row_index, NO_TILE); }

	/** \brief Adds a new column of tiles to a specified index in the table
	*** \param col_index The index where the column should be added
	*** \param value The value to set each tile in the newly added column
	***
	*** Specifying the col_index as the length of the layer results in appending the column to the end
	*** of the existing columns. Any value beyond this range is considered invalid and no operation will
	*** take place.
	**/
	void _AddLayerCol(uint32 col_index, int32 value);

	/** \brief Adds a new column of empty tiles to a specified index in the table
	*** \param col_index The index where the column should be added
	***
	*** Specifying the col_index as the length of the layer results in appending the column to the end
	*** of the existing columns. Any value beyond this range is considered invalid and no operation will
	*** take place.
	**/
	void _AddLayerCol(uint32 col_index)
		{ _AddLayerCol(col_index, NO_TILE); }

	/** \brief Delets a row from the tile layer at a specific index
	*** \param row_index The index of the row to delete
	***
	*** All tile rows will be shifted over to accomodate the deleted row. A row_index that exceeds the
	*** height of the map will result in no operation.
	**/
	void _DeleteLayerRow(uint32 row_index);

	/** \brief Delets a column from the tile layer at a specific index
	*** \param col_index The index of the column to delete
	***
	*** All tile columns will be shifted over to accomodate the deleted column. A col_index that exceeds the
	*** length of the map will result in no operation.
	**/
	void _DeleteLayerCol(uint32 col_index);

	/** \brief Resizes the layer to the dimensions specified
	*** \param length The new length of the layer, in number of tiles
	*** \param height The new height of the layer, in number of tiles
	***
	*** If the resize operation makes the layer smaller in either dimension, the appropriate
	*** number of rows and/or columns will be removed from the rows and columns on the ends.
	*** If the resize operations causes the layer to grow in size, the rows and columns will be
	*** added to the end with NO_TILE.
	**/
	void _ResizeLayer(uint32 length, uint height);
}; // class TileLayer


/** ****************************************************************************
*** \brief A container class holding properties of tile layers that are shared across contexts
***
*** This simple class retains properties of a tile layer that must remain the same for the layer
*** across all map contexts. This includes the layer's name, whether or not it is visible, and whether
*** or not it's collision data is active.
*** ***************************************************************************/
class TileLayerProperties {
public:
	TileLayerProperties() :
		_name(QString("")), _visible(true), _collision_enabled(true) {}

	TileLayerProperties(QString name) :
		_name(name), _visible(true), _collision_enabled(true) {}

	TileLayerProperties(QString name, bool visible, bool collisions) :
		_name(name), _visible(visible), _collision_enabled(collisions) {}

	//! \name Class member accessor functions
	//@{
	QString GetName() const
		{ return _name; }

	bool IsVisible() const
		{ return _visible; }

	bool IsCollisionEnabled() const
		{ return _collision_enabled; }

	void SetName(QString name)
		{ _name = name; }

	void SetVisible(bool visible)
		{ _visible = visible; }

	void SetCollisionEnabled(bool collisions)
		{ _collision_enabled = collisions; }

	void ToggleVisible()
		{ _visible = !_visible; }

	void ToggleCollisionEnabled()
		{ _collision_enabled = !_collision_enabled; }
	//@}

private:
	/** \brief The name of the layer as it will be seen by the user of the editor
	*** \note Although this data is saved to the map file, it is used only by the editor and not the game.
	**/
	QString _name;

	/** \brief Indicates whether or not the layer is visible in the editor
	*** \note This data is not saved to the map file. Any newly created or loaded tile layer will be visible by default.
	**/
	bool _visible;

	/** \brief Indicates whether the collision properties of the tile in this layer should take effect
	***
	*** This member is best set to true for layers that comprise the ground or floor of a tileset. Layers which constitute
	*** the higher part of ceilings, the tops of trees, and other unwalkable locations usually should have this property disabled.
	**/
	bool _collision_enabled;
}; // class TileLayerProperties


/** ****************************************************************************
*** \brief A collection of tile layers
***
*** A tile context is a group of TileLayer objects that together compose the makeup of a map view.
*** Every tile context corresponds to a map context, the difference between the two being that the
*** tile context only handles the tile data whereas a map context has tiles, objects, sprites, and
*** separate collision data. The editor user interface, however, does not mention the word "tile context"
*** and only uses the term "map context" to avoid confusing the user with this difference.
***
*** Every map must contain at least one TileContext, and can contain a maximum of MAX_CONTEXTS. Every
*** context has an ID that should be unique amongst any and all other contexts. Contexts can also inherit
*** from one (and only only one) other context. When a context inherits from another context, what happens is
*** that the context that was inherited from is drawn first and the inherting context is drawn on top of that.
*** The effect of this is that sections of the map can be easily replaced with other tiles without having to
*** load an entirely different map. For example, consider a small map with a single building. One context would
*** represent the outside of the building, while a second context inherits from the first and places tiles over
*** the building to show it's interior for when the player moves inside.
***
*** Due to the nature of inheriting contexts, TileContext objects must be constructed with care. Deleting a context
*** can potentially break the map data if it is not handled properly. Therefore, the constructors and several other
*** methods for this class are made private and can only be accessed by the the TileDataModel class. This class
*** manages all instances of TileContext for the open map and ensures that there is no violation of context data.
***
*** \note All contexts are named, but the name data is not stored within this class. This is because storing all
*** context names in a single container is more efficient for returning the list of context names and performing name
*** lookups. The name data is stored in the TileDataModel class.
***
*** \note TileContext, like TileLayer, does not store any collision data information.
*** ***************************************************************************/
class TileContext {
	friend class MapData;

public:
	//! \name Class member accessor functions
	//@{
	int32 GetContextID() const
		{ return _context_id; }

	QString GetContextName() const
		{ return _context_name; }

	bool IsInheritingContext() const
		{ return (_inherited_context_id != NO_CONTEXT); }

	int32 GetInheritedContextID() const
		{ return _inherited_context_id; }

	std::vector<TileLayer>& GetTileLayers()
		{ return _tile_layers; }

	//! \note Returns NULL if layer_index is invalid
	TileLayer* GetTileLayer(uint32 layer_index)
		{ if (layer_index < _tile_layers.size()) return &(_tile_layers[layer_index]); else return NULL; }

	void SetContextName(QString name)
		{ _context_name = name; }
	//@}

private:
	/** \param id The ID to set the newly created context
	***
	*** This constructor is used when not inheriting from another context
	**/
	TileContext(int32 id, QString name) :
		_context_id(id), _context_name(name), _inherited_context_id(NO_CONTEXT) {}

	/** \param id The ID to set the newly created context (should be unique among all TileContext objects)
	*** \param inherited_context_id The ID of the context that this newly created context should inherit from
	***
	*** It is the caller's responsibility to ensure that the inherited_context_id is valid (ie, another TileContext
	*** exists with the provided ID). The constructor has no means to determine if there is a valid context with this
	*** ID, other than ensuring that the value provided lies within the range 1-MAX_CONTEXTS.
	**/
	TileContext(int32 id, QString name, int32 inherited_context_id) :
		_context_id(id), _context_name(name), _inherited_context_id(inherited_context_id) {}

	//! \brief The ID number of the context which has an acceptable range of [1 - MAX_CONTEXTS]
	int32 _context_id;

	//! \brief The name of the context as it will be seen by the user in the editor
	QString _context_name;

	/** \brief The ID of the context that this context inherits from
	*** If this context does not inherit from another, then this member is set to NO_CONTEXT.
	**/
	int32 _inherited_context_id;

	//! \brief All tile layers that belong to the context
	std::vector<TileLayer> _tile_layers;

	void _SetContextID(int32 id)
		{ _context_id = id; }

	//! \brief Removes inheriting context data, if any exists
	void _ClearInheritingContext()
		{ _inherited_context_id = NO_CONTEXT; }

	/** \brief Transforms the context into an inheriting context
	*** \param inherited_context_id The ID of the context that this context should inherit from
	**/
	void _SetInheritingContext(int32 inherited_context_id)
		{ _inherited_context_id = inherited_context_id; }

	/** \brief Adds a new tile layer to the end of the layer container
	*** \param layer A reference to the pre-created TileLayer to add
	**/
	void _AddTileLayer(TileLayer& layer);

	/** \brief Removes an existing tile layer from the context
	*** \param layer_index The index of the layer to remove
	**/
	void _RemoveTileLayer(uint32 layer_index);

	/** \brief Swaps the position of two tile layers
	*** \param first_index The index of the first layer to swap
	*** \param second_index The index of the second layer to swap
	**/
	void _SwapTileLayers(uint32 first_index, uint32 second_index);
}; // class TileContext


/** ****************************************************************************
*** \brief Displays the sortable list of tile layers on the map
***
*** This widget is located in the top right section of the main editor window.
*** The user can see the order of tile layers and some of the properties of those
*** layers. The user interacts with this widget to query information about a layer,
*** change the order of the layer, or change the active property of a layer.
*** ***************************************************************************/
class LayerView : public QTreeWidget {
private:
	Q_OBJECT // Macro needed to use QT's slots and signals

	//! \name Widget Column Identifiers
	//@{
	static const uint32 ID_COLUMN = 0;
	static const uint32 VISIBLE_COLUMN = 1;
	static const uint32 NAME_COLUMN = 2;
	static const uint32 COLLISION_COLUMN = 3;
	//@}

public:
	LayerView(MapData* data);

	~LayerView()
		{}

	/** \brief Reimplemented from QTreeWidget to process left and right clicks separately
	*** \param event A pointer to the mouse event which occurred
	**/
	void mousePressEvent(QMouseEvent* event);

	// TODO: look into cusing ontext menu events for doing right click menus

	//! \brief Refreshes the viewable contents of the widget. Should be called whenever any of the map layer data changes
	void RefreshView();

private slots:
	/** \brief Updates the selected layer for editing in the map view widget
	***
	*** This function is called whenever the user single-clicks one of the layer items in the widget
	**/
	void _ChangeSelectedLayer();

	/** \brief Modifies one of the properties of a tile layer
	*** \param item A pointer to the layer where the property change will happen
	*** \param column The column number of the property which should be changed
	***
	*** This function is called whenever the user double-clicks one of the layer items in the widget
	**/
	void _ChangeLayerProperties(QTreeWidgetItem* item, int column);

private:
	//! \brief A pointer to the active map data that contains the tile layers
	MapData* _map_data;

	//! \brief An icon used to indicate the visibility property of a tile layer
	QIcon _visibility_icon;
}; // class LayerView : public QTreeWidget


/** ****************************************************************************
*** \brief Displays the sortable list of tile contexts for a map
***
*** This widget is located below the layer view widget in the right section of the main editor window.
*** The active map context is highlighted and shows each context's ID, name, and inheriting context
*** if any is active. These properties can be modified except for the ID, which is automatically
*** set according to the order of each context in the context list.
***
*** TODO: this is a skeleton class that will be fleshed out later
*** ***************************************************************************/
class ContextView : public QTreeWidget {
private:
	Q_OBJECT // Macro needed to use QT's slots and signals

	//! \name Widget Column Identifiers
	//@{
	static const uint32 ID_COLUMN = 0;
	static const uint32 NAME_COLUMN = 1;
	static const uint32 INHERITS_COLUMN = 2;
	//@}
public:
	ContextView(MapData* data);

	~ContextView()
		{}

private:
	//! \brief A pointer to the active map data that contains the tile contexts
	MapData* _map_data;
}; // class ContextView : public QTreeWidget

} // namespace hoa_editor

#endif // __TILE_LAYERS_HEADER__
