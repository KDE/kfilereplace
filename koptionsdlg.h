/***************************************************************************
                          koptionsdlg.h  -  description
                             -------------------
    begin                : Tue Dec 28 1999
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

#ifndef KOPTIONSDLG_H
#define KOPTIONSDLG_H

#include "configurationclasses.h"
#include "koptionsdlgs.h"

/**
  *@author François Dupoux
  */

class KOptionsDlg : public KOptionsDlgS
{
  Q_OBJECT
  public:
    KOptionsDlg(QWidget *parent, const char *name);
    ~KOptionsDlg();
      
  private: 
    KConfig* m_config;   
      
  protected slots:
    void slotOK();
    void slotDefaults();
    void slotChbBackup(bool b);
    
  private:
    void loadOptions();
    void saveOptions();
    void whatsThisPage();
};

#endif // KOPTIONSDLG_H
