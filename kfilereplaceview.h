/***************************************************************************
                          kfilereplaceview.h  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by François Dupoux
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

#ifndef KFILEREPLACEVIEW_H
#define KFILEREPLACEVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for Qt
#include <qpixmap.h>
#include <qsplitter.h>

// includes for the app
#include "kresultview.h"

class KFileReplaceDoc;
class QListView;

class KFileReplaceView : public QSplitter
{
  Q_OBJECT

public:
  /** Constructor for the main view */
  KFileReplaceView(QWidget *parent,const char *name);
  /** Destructor for the main view */
  ~KFileReplaceView();

        /** returns a pointer to the document connected to the view instance. Mind that this method requires a KFileReplaceApp instance as a parent
                * widget to get to the window document pointer by calling the KFileReplaceApp::getDocument() method.
                *
                * @see KFileReplaceApp#getDocument
                */
        KFileReplaceDoc* getDocument() const;

  /** contains the implementation for printing functionality */
        void print(QPrinter* m_pPrinter);
  /** Initialize lists
 */
  void init();

        QPixmap getIconString() {return m_pmIconString;}
        int addString(QString strSearch, QString strReplace, QListViewItem *lviCurrent);

public slots:
        void slotStringsAdd();
        void slotStringsEdit(QListViewItem*);

private:
         KResultView *m_ResultView;
        QListView *m_StringView;
        QPixmap m_pmIconString;

public:
        QListView *getStringView() {return m_StringView;}
        KResultView *getResultView() {return m_ResultView;}

};

#endif // KFILEREPLACEVIEW_H














