/***************************************************************************
                          klistviewstring.cpp  -  description
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

#include "klistviewstring.h"
#include <qpixmap.h>
#include <stdio.h>

// =================================================================================================================
KListViewString::KListViewString(QListViewItem *lvi): QListViewItem(lvi)
{
}

// =================================================================================================================
KListViewString::KListViewString(QListView *parent): QListViewItem(parent)
{
}

// =================================================================================================================
KListViewString::KListViewString(QListView *parent, QString str1, QString str2, QString str3, QString str4, QString str5, QString str6, QString str7, QString str8): QListViewItem(parent, str1, str2, str3, str4, str5, str6, str7, str8)
{
}

// =================================================================================================================
KListViewString::KListViewString(QListViewItem *lvi, QString str1, QString str2, QString str3, QString str4, QString str5, QString str6, QString str7, QString str8): QListViewItem(lvi, str1, str2, str3, str4, str5, str6, str7, str8)
{
}

// =================================================================================================================
KListViewString::~KListViewString()
{
}

// =================================================================================================================
void KListViewString::setup()
{
  //QListViewItem::setup();
  int nLines;
  int i;

  nLines = QMAX(text(0).contains('\n'), text(1).contains('\n'))+1;
  
  widthChanged();
  QListView * v = listView();
  int ph = 0;
  for (i=0; i < (int)v->columns(); ++i)
    {
      if ( pixmap( i ) )
	ph = QMAX( ph, pixmap( i )->height() );
    }
  
  int h = QMAX( v->fontMetrics().height()*nLines, ph ) + 2*v->itemMargin();
  if ( h % 2 > 0 )
    h++;
  
  setHeight(h);
}
