/******************************************************************************
 *
 *	Hero of Allacrost MapEditor
 *	Copyright (c) 2004
 *	Licensed under the GPL
 *
 *	Created by: Philip Vorsilak
 *	Filename: hoa_map_editor.cpp
 *
 *	$Id$
 *
 *	Description: Main HoA map editor file, main() runs the program.
 *
 *****************************************************************************/

#include "map_editor.h"

using namespace hoa_mapEd;

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	MapEditor *mapEd = new MapEditor();
	mapEd->setCaption("Hero of Allacrost Map Editor");
	app.setMainWidget(mapEd);
	mapEd->show();
	return app.exec();
}
