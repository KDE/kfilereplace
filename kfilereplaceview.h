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

//QT
class QPixMap;
class KListView;

//KDE
class KPopupMenu;

//local
#include "kfilereplaceviewwdg.h"
#include "configurationclasses.h"

class KFileReplaceView : public KFileReplaceViewWdg
{
  Q_OBJECT
  private:
    KPopupMenu* m_menuResult;
    RCOptions m_option;
    KListViewItem* m_lviCurrent;

  public:
    KFileReplaceView(QWidget *parent,const char *name);
    ~KFileReplaceView();

  public:
    void readOptions(const RCOptions& info) { m_option = info; }
    RCOptions writeOptions() { return m_option; }
    KListView* stringView(){ return m_lvStrings; }
    KListView* resultView(){ return m_lvResults; }
    QString currentItem();
    void loadMap(KeyValueMap extMap){ loadMapIntoView(extMap); }
    KeyValueMap stringsViewMap()const { return m_option.mapStringsView();}
    void setCurrentStringsViewMap(){ setMap();}

  public slots:
    void slotStringsAdd();
    void slotQuickStringsAdd(const QString& quickSearch, const QString& quickReplace);
    void slotStringsDeleteItem();
    void slotStringsEmpty();
    void slotStringsEdit();
    void slotResultProperties();
    void slotResultOpen();
    void slotResultOpenWith();
    void slotResultEdit();
    void slotResultDirOpen();
    void slotResultDelete();
    void slotResultTreeExpand();
    void slotResultTreeReduce();
    void slotMouseButtonClicked (int button, QListViewItem *lvi, const QPoint &pos);

  private:
    void expand(QListViewItem *lviCurrent, bool b);
    void setMap();
    void loadMapIntoView(KeyValueMap map);
    void whatsThis();

  signals:
    void resetActions();
    void searchMode(bool);
};

#endif // KFILEREPLACEVIEW_H
