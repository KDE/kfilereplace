/***************************************************************************
                          kfilereplacepref.h  -  description
                             -------------------
    copyright            : (C) 1999 by François Dupoux
                           (C) 2003 Andras Mantia
    email                : dupoux@dupoux.com, amantia@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KFILEREPLACEPREF_H
#define KFILEREPLACEPREF_H

#include <kdialogbase.h>
#include <qframe.h>

class KFileReplacePrefPageOne;
class KFileReplacePrefPageTwo;

class KFileReplacePreferences : public KDialogBase
{
    Q_OBJECT
public:
    KFileReplacePreferences();

private:
    KFileReplacePrefPageOne *m_pageOne;
    KFileReplacePrefPageTwo *m_pageTwo;
};

class KFileReplacePrefPageOne : public QFrame
{
    Q_OBJECT
public:
    KFileReplacePrefPageOne(QWidget *parent = 0);
};

class KFileReplacePrefPageTwo : public QFrame
{
    Q_OBJECT
public:
    KFileReplacePrefPageTwo(QWidget *parent = 0);
};

#endif // KFILEREPLACEPREF_H
