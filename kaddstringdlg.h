/***************************************************************************
                          KAddStringDlg.h  -  description
                             -------------------
    begin                : Sat Oct 16 1999
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

#ifndef KADDSTRINGDLG_H
#define KADDSTRINGDLG_H

#include <kconfig.h>
#include "kaddstringdlgs.h"
#include <qmap.h>
#include "configurationclasses.h"

class KAddStringDlg : public KAddStringDlgS
{
  Q_OBJECT
  private:
    QMap<QString,QString> m_map;
    KConfig* m_config;
    
  public: 
    KAddStringDlg(QWidget *parent=0, const char *name=0);
    ~KAddStringDlg();
  
  public:
    QMap<QString,QString> stringList();
    void loadDataFromStringsView(QMap<QString,QString> map);
    void empty(){ stringView->clear(); }
    void setConfig(KConfig* c) { m_config = c; }
  
  protected slots:
    void slotOK();
    void slotSearchOnly(bool b);
    void slotSearchReplace(bool b);
    void slotAdd();
    void slotDel();
  
  private:
    bool contains(QListView* lv,const QString& s, int column);
    void setMap();
};

#endif // KADDSTRINGDLG_H

