/***************************************************************************
                          kfilereplaceview.h  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by François Dupoux <dupoux@dupoux.com>
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

#include <kconfig.h>
#include <qmap.h>

#include "kaddstringdlg.h"
#include "kfilereplaceviewwdg.h"
#include "configurationclasses.h"

class KFileReplaceView : public KFileReplaceViewWdg
{
  Q_OBJECT
  private:
    QString m_path;
    KPopupMenu *m_kpmResult;
    QMap<QString,QString> m_map;
    QListViewItem *m_lviCurrent;
    QPixmap m_pmIconSuccess,
            m_pmIconError,
	    m_pmIconString,
	    m_pmIconSubString;
    KAddStringDlg dlg;
    KConfig* m_config;
        
  public:
    KFileReplaceView(QWidget *parent,const char *name);
    ~KFileReplaceView();
    
  public:
    QListView *stringView();
    QListView *resultView();
    QPixmap iconString();
    QString currentItem();
    void setConfig(KConfig* c) { m_config = c;}
      
  public slots:
    void slotStringsAdd();
    void slotStringsAddFromProjectDlg(const QMap<QString,QString>& replacementMap);
    void slotStringsDel();
    void slotStringsClear();
    void slotStringsEdit(QListViewItem* lv);
    //void slotStringsEditFromProjectDlg(const QMap<QString,QString>& replacementMap);
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
    void setMap();
  
  
};

#endif // KFILEREPLACEVIEW_H
