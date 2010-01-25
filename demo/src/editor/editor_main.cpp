///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    editor_main.cpp
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Source file for editor's main() function: from here the editor is
 *          started and exited.
 *****************************************************************************/

#ifdef __MACH__
	#include <unistd.h>
	#include <string>
#endif

#include "editor.h"
#include "global.h"

using namespace std;
using namespace hoa_editor;

#if defined(main) && !defined(_WIN32)
	#undef main
#endif

int main(int argc, char **argv)
{
#ifndef _WIN32
#ifndef __MACH__
	// Look for data files in DATADIR only if they are not available in the
	// current directory.
	if (ifstream("./dat/config/settings.lua") == NULL)
		chdir(DATADIR);
#endif
#endif

#ifdef __MACH__
	string path;
	path = argv[0];
	// remove the binary name
	path.erase(path.find_last_of('/'));
	// remove the MacOS directory
	path.erase(path.find_last_of('/'));
	// remove the Contents directory
	path.erase(path.find_last_of('/'));
	// remove the Editor.app directory
	path.erase(path.find_last_of('/'));
	// we are now in a common directory containing both Allacrost and the Editor
	path.append ( "/Allacrost.app/Contents/Resources/" );
	chdir ( path.c_str() );
#endif

	QApplication app(argc, argv);
	hoa_script::ScriptManager = hoa_script::ScriptEngine::SingletonCreate();
	hoa_script::ScriptManager->SingletonInitialize();
	hoa_global::GlobalManager = hoa_global::GameGlobal::SingletonCreate();
	hoa_defs::BindGlobalsToLua();
	hoa_global::GlobalManager->SingletonInitialize();

	Editor* editor = new Editor();
	editor->setCaption("Hero of Allacrost Level Editor");
	app.setMainWidget(editor);
	editor->show();
	return app.exec();
}
