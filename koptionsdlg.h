/***************************************************************************
                          koptionsdlg.h  -  description
                             -------------------
    begin                : Tue Dec 28 1999
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

#ifndef KOPTIONSDLG_H
#define KOPTIONSDLG_H

// KDE
class KConfig;

//local
#include "configurationclasses.h"
#include "koptionsdlgs.h"

/**
  *@author Fran�ois Dupoux
  */

class KOptionsDlg : public KOptionsDlgS
{
  Q_OBJECT
  public:
    KOptionsDlg(RCOptions* info, QWidget *parent, const char *name);
    ~KOptionsDlg();

  private:
    RCOptions* m_option;
    KConfig *m_config;

  private slots:
    void slotOK();
    void slotDefaults();
    void slotChbBackup(bool b);
    void slotChbConfirmStrings(bool b);
    void slotChbShowConfirmDialog(bool b);
    void slotHelp(){ kapp->invokeHelp(QString::null, "kfilereplace"); }

  private:
    void initGUI();
    void saveRCOptions();
    void whatsThis();
};

#endif // KOPTIONSDLG_H
