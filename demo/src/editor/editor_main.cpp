///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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

#include "editor.h"

using namespace hoa_editor;

hoa_script::GameScript *ScriptManager = NULL;

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	Editor* editor = new Editor();
	editor->setCaption("Hero of Allacrost Level Editor");
	ScriptManager = hoa_script::GameScript::SingletonCreate();
	ScriptManager->SingletonInitialize();
	app.setMainWidget(editor);
	editor->show();
	return app.exec();
}
