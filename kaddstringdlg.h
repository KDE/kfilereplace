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

// local
#include "kaddstringdlgs.h"
#include "configurationclasses.h"

class KAddStringDlg : public KAddStringDlgS
{
  Q_OBJECT
  private:
    RCOptions m_option;

  public:
    KAddStringDlg(QWidget *parent=0, const char *name=0);
    ~KAddStringDlg();

  public:
    void readOptions(const RCOptions& info) { m_option = info;
                                              updateStringsViewContent(); }
    RCOptions writeOptions() { return m_option; }
    /**
     *Loads the content of 'map' in a qlistview
     */
    void loadViewContent(KeyValueMap map);
    /**
     *Cleans the 'stringView' up
     */
    void clearStringsView();

  private slots:
    void slotOK();
    void slotSearchOnly(bool b);
    void slotSearchReplace(bool b);
    void slotAdd();
    void slotDel();
    void slotHelp();

  private:
    /**
     *Verifies whether 'lv' contains 's'
     */
    bool contains(QListView* lv,const QString& s, int column);
    void updateStringsMapContent();
    void updateStringsViewContent();
    void whatsThis();
};

#endif // KADDSTRINGDLG_H

