///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tile_model.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for the tile data model classes
***
*** This file contains classes that together build the data model for all tile
*** data for a given map. This data is contained by layers, contexts, and a data
*** managing class that provides an interface for the TileView class to request
*** changes to the data.
*** **************************************************************************/

#ifndef __TILE_MODEL_HEADER__
#define __TILE_MODEL_HEADER__

#include <QString>
#include <QStringList>

#include "script_read.h"
#include "script_write.h"
#include "editor_utils.h"

namespace hoa_editor {

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
	friend class TileDataModel;

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
*** separate collision data. The editor user interface refers to contexts as map contexts. For the most
*** part, map contexts and tile contexts can be thought of the same thing when it comes to the editor
*** and map data files that the editor writes.
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
	friend class TileDataModel;

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
*** \brief Manages all tile layer and context data for the map grid
***
*** This class serves as a protective interface for the modification of any map context or tile
*** layer data. As such, the class is the custodian for all TileLayer and TileContext objects for
*** the currently opened map. The Grid class maintains an instance of this class and calls the
*** appropriate methods in response to events generated by user input. In that sense, this class
*** is the model component in a MV architecture, where Grid serves as the view component.
***
*** The most important roll of this class is to maintain the integrity of the map tile data. For
*** example, when the user adds a new tile layer, this class will make sure that the layer is added
*** to every tile context. When the user requests a context to be removed, the class ensures that
*** there are no other contexts that inherit from the context being removed.
*** ***************************************************************************/
class TileDataModel {
public:
	TileDataModel();

	~TileDataModel()
		{ DestroyData(); }

	//! \brief Returns true if any initialized map data is being stored
	bool IsInitialized() const
		{ return (_tile_context_count > 0); }

	/** \brief Call when creating a new map to initialize the first TileContext object
	*** \param map_length The length of the new map data, in number of tiles
	*** \param map_height The height of the new map data, in number of tiles
	*** \return True only if initialization was successful
	***
	*** If this class currently holds any TileContext data, it will refuse to destroy it and
	*** return false. Call DestroyData() first to safely remove any TileContext data.
	**/
	bool CreateData(uint32 map_length, uint32 map_height);

	/** \brief Call whenever closing an open map to destroy all layers, contexts, and other data
	*** \note Any calls to GetContext or other functions that returned a TileContext or TileLayer
	*** pointer prior to this function being called will have invalid pointers if they were retained.
	*** Make sure to remove any locally stored TileContext object pointers after calling this function.
	**/
	void DestroyData();

	/** \brief Loads all the map contexts from an open map file
	*** \param data_file A reference to the open and initialized map data file
	*** \return True if all data was loaded successfully
	***
	*** This function presumes that the file is open to the correct table where the relevant data is stored.
	*** The function will do nothing if it detects that there is already map data loaded. Call DestroyData()
	*** prior to calling this function
	**/
	bool LoadData(hoa_script::ReadScriptDescriptor& data_file);

	/** \brief Saves all the map context data to an open map file
	*** \param data_file A reference to the open map data file to save to
	*** \return True if all data was waved successfully
	***
	*** This function presumes that the file is open to the location where the relevant data
	*** should be saved to.
	**/
	bool SaveData(hoa_script::WriteScriptDescriptor& data_file);

	//! \name Class member accessor functions
	//@{
	uint32 GetTileLayerCount() const
		{ return _tile_layer_count; }

	uint32 GetTileContextCount() const
		{ return _tile_context_count; }

	TileLayer* GetSelectedTileLayer() const
		{ return _selected_tile_layer; }

	TileContext* GetSelectedTileContext() const
		{ return _selected_tile_context; }

	//! \brief Gets the most recent error message generated by a call and clears that error
	QString GetErrorMessage()
		{ QString error = _error_message; _error_message.clear(); return error; }
	//@}

	//! \name Tile Layer Manipulation Methods
	//@{
	//! \brief Return the ordered list of names for all tile layers
	QStringList GetTileLayerNames() const;

	/** \brief Makes a tile layer visible in the editor
	*** \param layer_index The index of the layer to show
	**/
	void ShowTileLayer(uint32 layer_index)
		{ if (layer_index <= _tile_layer_count) _tile_layer_properties[layer_index].SetVisible(true); }

	/** \brief Removes visibility of a tile layer in the editor
	*** \param layer_index The index of the layer to hide
	**/
	void HideTileLayer(uint32 layer_index)
		{ if (layer_index <= _tile_layer_count) _tile_layer_properties[layer_index].SetVisible(false); }

	/** \brief Toggles whether or not a tile layer is visible in the editor
	*** \param layer_index The index of the tile layer to toggle visibility for
	**/
	void ToggleTileLayerVisibility(uint32 layer_index);

	/** \brief Activates a tile layer's collision data
	*** \param layer_index The index of the tile layer to activate collisions for
	**/
	void EnableTileLayerCollision(uint32 layer_index)
		{ if (layer_index <= _tile_layer_count) _tile_layer_properties[layer_index].SetCollisionEnabled(true); }

	/** \brief Deactivates a tile layer's collision data
	*** \param layer_index The index of the tile layer to deactivate collisions for
	**/
	void DisableTileLayerCollision(uint32 layer_index)
		{ if (layer_index <= _tile_layer_count) _tile_layer_properties[layer_index].SetCollisionEnabled(false); }

	/** \brief Toggles the activation of a tile layer's collision data
	*** \param layer_index The index of the tile layer to toggle collision data for
	**/
	void ToggleTileLayerCollision(uint32 layer_index);

	/** \brief Adds a new tile layer to all active contexts
	*** \param name The name of the layer to add, as will be see in the editor
	*** \param collision_enabled If true, the tile layer's collision data will be active in the map
	*** \return True if the layer was added successfully
	*** \note The layer name should be unique amongst all existing tile layers
	**/
	bool AddTileLayer(QString name, bool collision_enabled);

	/** \brief Removes a tile later from all active contexts
	*** \param layer_index The index of the layer to remove
	*** \return True if the layer was deleted successfully
	**/
	bool DeleteTileLayer(uint32 layer_index);

	/** \brief Renames an existing tile layer
	*** \param layer_index The index of the layer to rename
	*** \param new_name The new name for the tile layer
	*** \return True if the layer was renamed successfully
	*** \note The layer name should be unique amongst all existing tile layers
	**/
	bool RenameTileLayer(uint32 layer_index, QString new_name);

	/** \brief Moves a tile layer one position up in the layer list
	*** \param layer_index The index of the layer that should be moved upward
	*** \return True if the layer was moved successfully
	**/
	bool MoveTileLayerUp(uint32 layer_index);

	/** \brief Moves a tile layer one position down in the layer list
	*** \param layer_index The index of the layer that should be moved downward
	*** \return True if the layer was moved successfully
	**/
	bool MoveTileLayerDown(uint32 layer_index);
	//@}

	//! \name Tile Context Manipulation Methods
	//@{
	//! \brief Returns the ordered list of names for all tile contexts
	QStringList GetTileContextNames() const;

	/** \brief Returns an ordered list of all names of the contexts that each context inherits from
	*** \note Contexts which do not inherit from another context will be represented with an empty string
	**/
	QStringList GetInheritedTileContextNames() const;

	/** \brief Creates a new TileContext object and add it to the end of the context list
	*** \param name The name to assign to the context (must be a non-empty string)
	*** \param inheriting_context_id The ID of the context that this context should inherit from.
	*** Leaving this argument empty will use a default value designating the context as a base context
	*** \return A pointer to the newly created TileContext, or NULL if an error prevented the context from being created
	***
	*** Possible errors that could prevent context creation include exceeding the maximum number of
	*** contexts allowed (MAX_CONTEXTS), an existing context with the same name, or an invalid context ID argument.
	**/
	TileContext* AddTileContext(QString name, int32 inheriting_context_id = NO_CONTEXT);

	/** \brief Deletes an existing TileContext object
	*** \param context A pointer to the context to delete
	*** \return True only if the context was deleted successfully
	***
	*** A context may fail to be deleted if it's the final base context in the context list or one or more
	*** contexts inherit from the context.
	**/
	bool DeleteTileContext(TileContext* context);

	/** \brief Renames an existing TileContext object
	*** \param context_index The index of the context to rename
	*** \param new_name The name to set for the context
	*** \return True if the context was renamed successfully
	***
	*** The name should be unique among all existing TileContext names. Note that any previous calls to
	*** GetTileContextNames() or GetInheritedTileContextNames() that retained the QStringList from those
	*** calls will be outdated if this function completes successfully. You should always remember to
	*** update any external context name lists after a rename operation.
	**/
	bool RenameTileContext(uint32 context_index, QString new_name);

	/** \brief Moves a context up in the list
	*** \param context A pointer to the context to move
	*** \return True if the move operation was successful (fails if the context is already at the top of the list)
	**/
	bool MoveTileContextUp(TileContext* context);

	/** \brief Moves a context down in the list
	*** \param context A pointer to the context to move
	*** \return True if the move operation was successful (fails if the context is already at the bottom of the list)
	**/
	bool MoveTileContextDown(TileContext* context);

	/** \brief Returns a pointer to a TileContext with a specified id
	*** \param context_id The ID of the context to retrieve
	*** \return A pointer to the TileContext, or NULL if no context with the given ID was found
	**/
	TileContext* FindTileContextByID(int32 context_id) const;

	/** \brief Returns a pointer to a TileContext with a specified name
	*** \param context_name The name of the context to retrieve
	*** \return A pointer to the TileContext, or NULL if no context with the given name was found
	*** \note Context names are guaranteed to be unique, so a name will never map to more than one context
	**/
	TileContext* FindTileContextByName(QString context_name) const;

	/** \brief Returns a pointer to a TileContext at the given index in the context list
	*** \param context_index The index of the context to retrieve that should map to the _all_map_contexts container
	*** \return A pointer to the TileContext, or NULL if no context with the given name was found
	*** \note Context names are guaranteed to be unique, so a name will never map to more than one context
	**/
	TileContext* FindTileContextByIndex(uint32 context_index) const;
	//@}

private:
	//! \brief The number of tile layers that the open map contains
	uint32 _tile_layer_count;

	//! \brief The number of map (tile) contexts that the open map contains
	uint32 _tile_context_count;

	/** \brief A pointer to the tile layer currently selected by the user
	*** \note This tile layer exists in the _active_tile_context
	**/
	TileLayer* _selected_tile_layer;

	//! \brief A pointer to the map context currently selected by the user
	TileContext* _selected_tile_context;

	/** \brief Stores all TileContext objects for the given map
	***
	*** This container always has a size of MAX_CONTEXTS. The value at index 0 is always non-NULL
	*** (except when no map is loaded) while other locations may or may not be NULL depending on
	*** the number of contexts that have been created. All non-NULL entries are always contained
	*** within the front of the container, so you wouldn't have a situation where you'd have NULL
	*** values inbetween valid context objects. The context at index i will always have an ID value of i+1.
	**/
	std::vector<TileContext*> _all_tile_contexts;

	//! \brief An ordered container of the shared properties for each tile layer across all contexts
	std::vector<TileLayerProperties> _tile_layer_properties;

	/** \brief A tile layer that contains nothing but empty tiles, used for TileContext construction
	***
	*** This structure is maintained to the current height and length of the open map so that when a new
	*** context is created or tile layer is added, this member can be used to create a new empty layer
	*** of the correct size.
	**/
	TileLayer _empty_tile_layer;

	//! \brief Contains the error message generated by the most recently called method that failed
	QString _error_message;

	//! \brief Returns the index where the context object is stored
	uint32 _GetTileContextIndex(TileContext* context)
		{ return static_cast<uint32>(context->GetContextID() - 1); }

	/** \brief Swaps the postion of two contexts in the _all_map_contexts list
	*** \param first A pointer to the first context to swap
	*** \param second A pointer to the second context to swap
	***
	*** This function does not do any error checking on the TileContext arguments, so the caller is
	*** responsible for ensuring that they point to valid entries in _all_map_contexts. This operation will
	*** potentially modify more contexts than just the two being swapped, since inherited context IDs need
	*** to be checked to maintain consistency.
	**/
	void _SwapTileContexts(TileContext* first, TileContext* second);

	/** \brief Computes the collision grid data from the available map information
	*** \param data A reference to a 2D vector where the resulting data should be written to
	***
	*** The collision grid is four times the size of the tile grid (twice as long, and twice as high).
	*** The tileset data contains the collision information for every quadrant of its map tiles. The
	*** data is computed by looking at the collsion data for each tile in every position of the map
	*** grid on every layer that has the layer collision property enabled. This is done for each tile
	*** context, and the results are bitmasked together so that the collision data for all potential
	*** 32 contexts can fit within a single 32-bit integer.
	***
	**/
	void _ComputeCollisionData(std::vector<std::vector<uint32> >& data);
}; // class TileDataModel

} // namespace hoa_editor

#endif // __TILE_MODEL_HEADER__
