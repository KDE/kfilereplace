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
class KConfig;
class KPopupMenu;

//local
#include "kaddstringdlg.h"
#include "kfilereplaceviewwdg.h"
#include "configurationclasses.h"

class KFileReplaceView : public KFileReplaceViewWdg
{
  Q_OBJECT
  private:
    QString m_path;
    KPopupMenu* m_menuResult;
    ConfigurationInformation m_info;
    KListViewItem* m_lviCurrent;
    KAddStringDlg m_addStringdlg;
    KConfig* m_config;
        
  public:
    KFileReplaceView(QWidget *parent,const char *name);
    ~KFileReplaceView();
    
  public:
    KListView *stringView();
    KListView *resultView();
    QString currentItem();
    void setConfig(KConfig* c) { m_config = c;}
    void loadMap(KeyValueMap extMap);
    KeyValueMap stringsViewMap()const { return m_info.mapStringsView();}
    void currentStringsViewMap(){ setMap();}
    bool searchOnly()const { return m_info.searchMode();}
          
  public slots:
    void slotStringsAdd();
    void slotQuickStringsAdd(const QString& quickSearch, const QString& quickReplace);
    void slotStringsDeleteItem();
    void slotStringsEmpty();
    void slotStringsEdit(QListViewItem* lv);
    void slotResultProperties();
    void slotResultOpen();
    void slotResultOpenWith();
    void slotResultEdit();
    void slotResultDirOpen();
    void slotResultDelete();
    void slotResultTreeExpand();
    void slotResultTreeReduce();
    void slotMouseButtonClicked (int button, QListViewItem *lvi, const QPoint &pos, int column);
  
  private:
    void expand(QListViewItem *lviCurrent, bool b);
    void setMap();
    void loadMapIntoView(KeyValueMap map);
    void whatsThis();
};

#endif // KFILEREPLACEVIEW_H
