/***************************************************************************
                          klistviewstring.h  -  description
                             -------------------
    begin                : Wed Jun 14 2000
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

#ifndef KLISTVIEWSTRING_H
#define KLISTVIEWSTRING_H

#include <qlistview.h>
#include <qpixmap.h>
#include <qstring.h>

/**
  *@author François Dupoux
  */

class KListViewString : public QListViewItem
{
public:
        KListViewString(QListViewItem *lvi);
        KListViewString(QListView *parent);
        KListViewString(QListView *parent, QString, QString = QString::null, QString = QString::null, QString = QString::null, QString = QString::null, QString = QString::null, QString = QString::null, QString = QString::null);
        KListViewString(QListViewItem *lvi, QString, QString = QString::null, QString = QString::null, QString = QString::null, QString = QString::null, QString = QString::null, QString = QString::null, QString = QString::null);
        ~KListViewString();

        void setup();

};

#endif
