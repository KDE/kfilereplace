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

#include "kaddstringdlgs.h"

class KAddStringDlg : public KAddStringDlgS
{
   Q_OBJECT
public: 
       KAddStringDlg(QWidget *parent=0, const char *name=0);
        ~KAddStringDlg();
        
private:
        QString m_strSearch;
        QString m_strReplace;

protected slots:
        void slotOK();
        void changeSearchText();
        void changeReplaceText();
public:
        void setSearchText(const QString &strText);
        QString searchText() const;
        void setReplaceText(const QString &strText);
        QString replaceText() const;

};

#endif //KADDSTRINGDLG_H

