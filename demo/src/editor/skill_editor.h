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

#ifndef __SKILL_EDITOR_HEADER__
#define __SKILL_EDITOR_HEADER__

#include "script.h"
#include "global_skills.h"

#include <QDialog>
#include <QTabWidget>

namespace hoa_editor
{

class SkillEditor : public QDialog
{
	//! Macro needed to use Qt's slots and signals
	Q_OBJECT

public:
	SkillEditor(QWidget *parent, const QString &name);
	~SkillEditor();	

private:
	//! \brief a tab control to hold the tabs for attack skills, support skills, and defense skills
	QTabWidget *_tab_control;
};

} // namespace hoa_editor

#endif
// __SKILL_EDITOR_HEADER__