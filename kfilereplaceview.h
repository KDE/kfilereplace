/***************************************************************************
                          kfilereplaceview.h  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by Fran�is Dupoux <dupoux@dupoux.com>
                           (C) 2004 Emiliano Gulmini <emi_barbarossa@yahoo.it>
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


class QPixMap;
class QListView;

class KConfig;
class KPopupMenu;
//class KProcess;

class KAddStringDlg;
class KFileReplaceDoc;


#include <qpixmap.h>

#include "kfilereplaceviewWdg.h"
class KFileReplaceView : public KFileReplaceViewWdg
{
  Q_OBJECT

public:
  /** Constructor for the main view */
  KFileReplaceView(QWidget *parent,const char *name);
  /** Destructor for the main view */
  virtual ~KFileReplaceView();

  QListView *stringView();
  QListView *resultView();
  QPixmap iconString();

  QString currentItem();
  bool addString(QListViewItem *lviCurrent, const QString &searchStr = QString::null, const QString &replaceStr = QString::null);
  bool editString(QListViewItem *lviCurrent);
  KFileReplaceDoc* document() const;
  QListViewItem* addFullItem(bool bSuccess, const QString& szName, const QString& szDirectory, uint nOldSize, uint nNewSize, int nNbRepl, const QString& szErrMsg=QString::null);
  int updateItem(QListViewItem *lvi, bool bSuccess, uint nNewSize, int nNbRepl, const QString& szErrMsg=QString::null);
  bool increaseStringCount(QListViewItem *lvi, QString strOld, QString strNew, QString strReplace, const char *szSearch, int nSearchLen, bool bShowDetails);
  void execShellCommand(const QString& cmd);
  /** Sets the search mode to "Search" if search==true. */
  void setSearchMode(bool search);

public slots:
  void slotStringsAdd();
  void slotStringsEdit(QListViewItem*);
  void slotResultProperties();
  void slotResultOpen();
  void slotResultOpenWith();
  void slotResultEdit();
  void slotResultDirOpen();
  void slotResultDelete();
  void slotResultTreeExpand();
  void slotResultTreeReduce();
  void slotMouseButtonClicked (int nButton, QListViewItem *lvi, const QPoint &pos, int column);

private:
  void expand(QListViewItem *lviCurrent, bool bExpand);

private:
  KConfig* m_config;
  QListView *m_stringView;
  QString m_path;
  KAddStringDlg* dlg;
  //KFileReplaceApp *m_app;
  KPopupMenu *m_kpmResult;
  QListViewItem *m_lviCurrent;
  QPixmap m_pmIconSuccess;
  QPixmap m_pmIconError;
  QPixmap m_pmIconString;
  QPixmap m_pmIconSubString;
};

#endif // KFILEREPLACEVIEW_H
