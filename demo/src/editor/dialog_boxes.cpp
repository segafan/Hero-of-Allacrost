///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    dialog_boxes.cpp
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Source file for all of editor's dialog boxes.
 *****************************************************************************/

#include "dialog_boxes.h"

using namespace hoa_editor;

/************************
  MapPropertiesDialog class functions follow
************************/

MapPropertiesDialog::MapPropertiesDialog(QWidget* parent, const QString& name,
	bool prop)
	: QDialog(parent, (const char*) name)
{
	setCaption("Map Properties...");

	// Set up the height spinbox
	_height_label = new QLabel("Height (in tiles):", this);
	_height_sbox  = new QSpinBox(this);
	_height_sbox->setMinimum(24);
	_height_sbox->setMaximum(1000);
	
	// Set up the width spinbox
	_width_label  = new QLabel(" Width (in tiles):", this);
	_width_sbox   = new QSpinBox(this);
	_width_sbox->setMinimum(32);
	_width_sbox->setMaximum(1000);
	
	// Set up the cancel and okay push buttons
	_cancel_pbut  = new QPushButton("Cancel", this);
	_ok_pbut      = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);
	// at construction no tilesets are checked, disable ok button
	_ok_pbut->setEnabled(false);
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	// Set up the list of selectable tilesets
	QDir tileset_dir("img/tilesets");
	_tileset_tree = new QTreeWidget(this);
	_tileset_tree->setColumnCount(1);
	_tileset_tree->setHeaderLabels(QStringList("Tilesets"));
	connect(_tileset_tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this,
		SLOT(_EnableOKButton()));
	QList<QTreeWidgetItem*> tilesets;

	// Start the loop at 2 to skip over . (present working directory)
	// and .. (parent directory).
	for (uint32 i = 2; i < tileset_dir.count(); i++)
	{
		tilesets.append(new QTreeWidgetItem((QTreeWidget*)0,
			QStringList(tileset_dir[i].remove(".png"))));
		tilesets.back()->setCheckState(0, Qt::Unchecked);  // enables checkboxes
		
		if (prop)
		{
			// get reference to the Editor
			Editor* editor = static_cast<Editor*> (parent);

			// Iterate through the names of the tabs to see which ones in the
			// list are already present and set their checkbox appropriately.
			for (int j = 0; j < editor->_ed_tabs->count(); j++)
				if (tilesets.back()->text(0) == editor->_ed_tabs->tabText(j))
				{
					tilesets.back()->setCheckState(0, Qt::Checked);
					_ok_pbut->setEnabled(true);
					break;
				} // the two tileset names must match in order to set the checkbox
		} // user wants to edit the map's properties
	} // loop through all files in the tileset directory
	_tileset_tree->insertTopLevelItems(0, tilesets);

	if (prop)
	{
		// get reference to the Editor
		Editor* editor = static_cast<Editor*> (parent);

		_height_sbox->setValue(editor->_ed_scrollview->_map->GetHeight());
		_width_sbox->setValue(editor->_ed_scrollview->_map->GetWidth());
	} // user wants to edit the map's properties
	
	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	_dia_layout->addWidget(_height_label, 0, 0);
	_dia_layout->addWidget(_height_sbox, 1, 0);
	_dia_layout->addWidget(_width_label, 2, 0);
	_dia_layout->addWidget(_width_sbox, 3, 0);
	_dia_layout->addWidget(_tileset_tree, 0, 1, 5, -1);
	_dia_layout->addWidget(_cancel_pbut, 6, 0);
	_dia_layout->addWidget(_ok_pbut, 6, 1);
} // MapPropertiesDialog constructor

MapPropertiesDialog::~MapPropertiesDialog()
{
	delete _height_label;
	delete _height_sbox;
	delete _width_label;
	delete _width_sbox;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _tileset_tree;
	delete _dia_layout;
} // MapPropertiesDialog destructor



// ********** Private slot **********

void MapPropertiesDialog::_EnableOKButton()
{
	// Disable the ok button if no tilesets are checked, otherwise enable it.
	int num_items = _tileset_tree->topLevelItemCount();
	for (int i = 0; i < num_items; i++)
	{
		if (_tileset_tree->topLevelItem(i)->checkState(0) == Qt::Checked)
		{
			_ok_pbut->setEnabled(true);
			return;
		} // at least one tileset must be checked in order to enable push button
	} // iterate through all items in the _tileset_tree

	// If this point is reached, no tilesets are checked.
	_ok_pbut->setEnabled(false);
} // _EnableOKButton()



/************************
  MusicDialog class functions follow
************************/

MusicDialog::MusicDialog(QWidget* parent, const QString& name,
	const QString& selected_music)
	: QDialog(parent, name)
{
	setCaption("Select map music");
	_dia_layout   = new QGridLayout(this);

	_cancel_pbut  = new QPushButton("Cancel", this);
	_ok_pbut      = new QPushButton("OK", this);
	_select_label = new QLabel("Select the music for this map:",this);
	_music_list   = new QTreeWidget(this);

	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	_dia_layout->addWidget(_select_label, 0, 0);
	_dia_layout->addWidget(_music_list, 1, 0);
	_dia_layout->addWidget(_ok_pbut, 2, 0);
	_dia_layout->addWidget(_cancel_pbut, 2, 1);

	_PopulateMusicList(selected_music);
} // MusicDialog constructor

MusicDialog::~MusicDialog()
{
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _select_label;
	delete _music_list;
	delete _dia_layout;
} // MusicDialog destructor

QString MusicDialog::GetSelectedFile()
{
	if (_music_list->currentItem() == 0)
		return QString("None");

	return QString("mus/" + _music_list->currentItem()->text(0));
} // GetSelectedFile()



// ********** Private function **********

void MusicDialog::_PopulateMusicList(const QString& selected_str)
{
	QDir music_dir("mus");
	_music_list->setColumnCount(1);
	_music_list->setHeaderLabels(QStringList("Filename"));
	
	// Add music files
	QList<QTreeWidgetItem*> music;
	music.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList("None")));
	for (unsigned int i = 0; i < music_dir.count(); i++) 
	{
		if (music_dir[i].contains(".ogg"))
		{
			music.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(music_dir[i])));
			if (selected_str.endsWith(music_dir[i]) && !selected_str.isEmpty())
				music.back()->setSelected(true);
		} // only look for .ogg files
	} // iterate through all files in the music directory

	if (selected_str.isEmpty() || selected_str == "None")
		music.first()->setSelected(true);

	_music_list->insertTopLevelItems(0, music);
} // _PopulateMusicList(...)



/************************
  ContextPropertiesDialog class functions follow
************************/

ContextPropertiesDialog::ContextPropertiesDialog(QWidget* parent, const QString& name)
	: QDialog(parent, (const char*) name)
{
	setCaption("Context Properties...");
	_name_label = new QLabel("Context name", this);
	_name_ledit = new QLineEdit(this);
	connect(_name_ledit, SIGNAL(textEdited(const QString&)), this,
		SLOT(_EnableOKButton()));

	// Set up the cancel and okay push buttons
	_cancel_pbut  = new QPushButton("Cancel", this);
	_ok_pbut      = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);
	// at construction nothing has been entered, disable ok button
	_ok_pbut->setEnabled(false);
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	// Set up the list of inheritable contexts
	_context_tree = new QTreeWidget(this);
	_context_tree->setColumnCount(1);
	_context_tree->setHeaderLabels(QStringList("Inherit from context:"));
	QList<QTreeWidgetItem*> contexts;

	// get reference to the Editor
	Editor* editor = static_cast<Editor*> (parent);

	QStringList context_names = editor->_ed_scrollview->_map->context_names;
	for (QStringList::Iterator qit = context_names.begin();
	     qit != context_names.end(); ++qit)
	{
		contexts.append(new QTreeWidgetItem((QTreeWidget*)0,
			QStringList(*qit)));
	} // loop through all files in the tileset directory
	_context_tree->insertTopLevelItems(0, contexts);

	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	_dia_layout->addWidget(_name_label, 0, 0);
	_dia_layout->addWidget(_name_ledit, 0, 1);
	_dia_layout->addWidget(_context_tree, 1, 1, 5, -1);
	_dia_layout->addWidget(_cancel_pbut, 6, 0);
	_dia_layout->addWidget(_ok_pbut, 6, 1);
} // ContextPropertiesDialog constructor

ContextPropertiesDialog::~ContextPropertiesDialog()
{
	delete _name_label;
	delete _name_ledit;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _context_tree;
	delete _dia_layout;
} // ContextPropertiesDialog destructor



// ********** Private slot **********

void ContextPropertiesDialog::_EnableOKButton()
{
	// Disable the ok button if the line edit is empty.
	// The default inheritable context is the base context.
	if (_name_ledit->text() == "")
		_ok_pbut->setEnabled(false);
	else
		_ok_pbut->setEnabled(true);
} // _EnableOKButton()

