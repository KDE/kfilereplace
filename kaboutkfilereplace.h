/***************************************************************************
                          kaboutkfilereplace.h  -  description
                             -------------------
    begin                : Sat Jul 22 2000
    copyright            : (C) 2000 by François Dupoux
    email                : dupoux@dupoux.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KABOUTKFILEREPLACE_H
#define KABOUTKFILEREPLACE_H

#include <kaboutdialog.h>

// ===========================================================================================================================
class KAboutKFileReplace : public KAboutDialog
{
public: 
	KAboutKFileReplace(QWidget *parent, const char *name, bool modal);
	~KAboutKFileReplace();
};

#endif // KABOUTKFILEREPLACE_H
