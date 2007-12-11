///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
* \file    skill_editor.h
* \author  Daniel Steuernol, steu@allacrost.org
* \brief   Header file for editor's skill editor dialog
*****************************************************************************/

#include "skill_editor.h"

using namespace hoa_editor;
using namespace hoa_script;

SkillEditor::SkillEditor(QWidget *parent, const QString &name)
: QDialog(parent, static_cast<const char *>(name))
{
	setCaption("Skill Editor");
}

SkillEditor::~SkillEditor()
{}