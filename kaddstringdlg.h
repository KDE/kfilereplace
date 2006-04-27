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
/**
 * This is the dialog used to add a list of searching/replacing strings
 */
class KAddStringDlg : public KAddStringDlgS
{
  Q_OBJECT
  private:
    RCOptions* m_option;
    Q3ListView* m_sv;
    KeyValueMap m_currentMap;
    bool m_wantEdit;

  public: //Constructors
    KAddStringDlg(RCOptions* info, bool wantEdit, QWidget *parent=0, const char *name=0);

  private slots:
    void slotOK();
    void slotSearchOnly();
    void slotSearchReplace();
    void slotAddStringToView();
    void slotDeleteStringFromView();
    void slotHelp();

  private:
   /**
    * inits... the GUI :-)
    */
    void initGUI(); 
   /**
    * Deletes all the items of the view
    */
    void eraseViewItems();
    /**
     * Deletes all the items of the map
     */
    void clearMap() { KeyValueMap m; m_option->m_mapStringsView = m; }
    /**
     * The view is actually a stack widget that contains two different views widget
     */
    void raiseView();
    /**
     * Verifies whether 'lv' contains 's'
     */
    bool columnContains(Q3ListView* lv,const QString& s, int column);
    void saveViewContentIntoMap();
    void loadMapIntoView();
    void whatsThis();
};

#endif // KADDSTRINGDLG_H

