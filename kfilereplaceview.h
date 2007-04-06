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
#include <qlcdnumber.h>
#include <q3widgetstack.h>
class QPixMap;

//KDE
class KMenu;
class K3ListView;

//local
#include "kfilereplaceviewwdg.h"
#include "configurationclasses.h"

class coord
{
  public:
    int line,
        column;
  public:
    coord(){ line = 1;
             column = 1;}
    coord(const coord& c) { line = c.line;
                            column = c.column;}
    coord operator=(const coord& c) { line = c.line;
                                      column = c.column;
                                      return (*this);}
};


/**
 * The view of KFilereplace.
 */
class KFileReplaceView : public KFileReplaceViewWdg
{
  Q_OBJECT
  private:
    KMenu* m_menuResult;
    RCOptions* m_option;
    K3ListViewItem* m_lviCurrent;
    K3ListView* m_rv,
             * m_sv;

  public://Constructors
    KFileReplaceView(RCOptions* info, QWidget *parent,const char *name);

  public:
    QString currentPath();
    void showSemaphore(const QString &s);
    void displayScannedFiles(int filesNumber) { m_lcdFilesNumber->display(QString::number(filesNumber,10)); }
    void stringsInvert(bool invertAll);
    void changeView(bool searchingOnlyMode);
    K3ListView* getResultsView();
    K3ListView* getStringsView();
    void updateOptions(RCOptions* info) { m_option = info; }
    void loadMap(KeyValueMap extMap){ loadMapIntoView(extMap); }
    KeyValueMap getStringsViewMap()const { return m_option->m_mapStringsView;}
    void setCurrentStringsViewMap(){ setMap(); }
    //void emitSearchingOnlyMode(bool b) { emit searchingOnlyMode(b); }

  public slots:
    void slotMouseButtonClicked (int button, Q3ListViewItem *lvi, const QPoint &pos);
    void slotResultProperties();
    void slotResultOpen();
    void slotResultOpenWith();
    void slotResultDirOpen();
    void slotResultEdit();
    void slotResultDelete();
    void slotResultTreeExpand();
    void slotResultTreeReduce();
    void slotStringsAdd();
    void slotQuickStringsAdd(const QString& quickSearch, const QString& quickReplace);
    void slotStringsDeleteItem();
    void slotStringsEmpty();
    void slotStringsEdit();
    void slotStringsSave();

  private:
    void initGUI();
    void raiseStringsView();
    void raiseResultsView();
    coord extractWordCoordinates(Q3ListViewItem* lvi);
    void expand(Q3ListViewItem *lviCurrent, bool b);
    void setMap();
    void loadMapIntoView(KeyValueMap map);
    void whatsThis();

  /*signals:
    void resetActions();
    void searchingOnlyMode(bool);*/
};

#endif // KFILEREPLACEVIEW_H
