/***************************************************************************
                          kfilereplaceview.h  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by François Dupoux
                                  (C) 2004 Emiliano Gulmini <emi_barbarossa@yahoo.it>
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
#include <qsplitter.h>

// includes for the app
#include "kresultview.h"
class KAddStringDlg;
class QPixMap;
class QListView;
class KFileReplaceDoc;
class KConfig;

class KFileReplaceView : public QSplitter
{
  Q_OBJECT

public:
  /** Constructor for the main view */
  KFileReplaceView(QWidget *parent,const char *name);
  /** Destructor for the main view */
  virtual ~KFileReplaceView();

public slots:
  void slotStringsAdd();
  void slotStringsEdit(QListViewItem*);

private:
  KConfig* m_config;
  KResultView *m_resultView;
  QListView *m_stringView;
  QPixmap m_pmIconString;
  QString m_path;
  KAddStringDlg* dlg;

public:
  QListView *stringView();
  KResultView *resultView();
  QPixmap iconString();
  /** Initialize lists */
  void init();
  bool addString(QListViewItem *lviCurrent);
  bool editString(QListViewItem *lviCurrent);
  /** returns a pointer to the document connected to the view instance. Mind that this method requires a KFileReplaceApp instance as a parent
    * widget to get to the window document pointer by calling the KFileReplaceApp::getDocument() method.
    *
    * @see KFileReplaceApp#getDocument */
  KFileReplaceDoc* document() const;
};

#endif // KFILEREPLACEVIEW_H














