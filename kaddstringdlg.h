/***************************************************************************
                          KAddStringDlg.h  -  description
                             -------------------
    begin                : Sat Oct 16 1999
    copyright            : (C) 1999 by Fran�ois Dupoux
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

// KDE
class KConfig;

// local
#include "kaddstringdlgs.h"
#include "configurationclasses.h"

class KAddStringDlg : public KAddStringDlgS
{
  Q_OBJECT
  private:
    ConfigurationInformation m_info;
    KConfig* m_config;
    
  public: 
    KAddStringDlg(QWidget *parent=0, const char *name=0);
    ~KAddStringDlg();
  
  public:
    /**
    Returns a QMap with the list of strings
    */
    KeyValueMap stringsMap();
    /**
    Loads the content of 'map' in a qlistview
    */
    void loadViewContent(KeyValueMap map);
    /**
    Cleans the 'stringView' up
    */
    void clearView(){ m_stringView->clear(); }
    void setConfig(KConfig* c) { m_config = c; }
    bool searchOnly()const { return m_info.searchMode();}
     
  protected slots:
    void slotOK();
    void slotSearchOnly(bool b);
    void slotSearchReplace(bool b);
    void slotAdd();
    void slotDel();
    void slotHelp();
  
  private:
    /**
    Verifies whether 'lv' contains 's'
    */
    bool contains(QListView* lv,const QString& s, int column);
    void setMap();
    void setMap(KeyValueMap map);
    void whatsThis();
};

#endif // KADDSTRINGDLG_H

