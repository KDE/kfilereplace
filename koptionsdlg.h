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

#include "apistruct.h"

#include <kseparator.h>

class QLabel;
class QLineEdit;
class QCheckBox;

#include "koptionsdlgs.h"

/**
  *@author François Dupoux
  */

class KOptionsDlg : public KOptionsDlgS
{
   Q_OBJECT
public:
        KOptionsDlg(QWidget *parent, const char *name, Settings settings);
        ~KOptionsDlg();
        void whatsThisPage1();
        void whatsThisPage2();
        void whatsThisPage3();
       // void setValues();
        /** Functions of access to datas */
        Settings settings();

private: // Data of configuration
        Settings m_settings;

protected slots:
        void slotOK();
        void slotDefaults();
};

#endif //KOPTIONSDLG_H
