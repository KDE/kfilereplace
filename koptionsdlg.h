/***************************************************************************
                          kpropertiesdlg.h  -  description
                             -------------------
    begin                : Tue Dec 28 1999
    copyright            : (C) 1999 by François Dupoux
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

#include <kmessagebox.h>
#include "apistruct.h"

#include <qtabdialog.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <kseparator.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qdialog.h>

/**
  *@author François Dupoux
  */

class KOptionsDlg : public QTabDialog
{
   Q_OBJECT
public:
        KOptionsDlg(QWidget *parent, const char *name, KSettings settings);
        ~KOptionsDlg();
        void initPage1();
        void initPage2();
        void initPage3();
        void setValues();

protected:
        QLabel *m_label1;
        KSeparator *m_sep1;
        QLineEdit *m_editLetter;
        QLineEdit *m_editWord;
        QCheckBox *m_checkWildcardsInReplaceStr;
        QLineEdit *m_editExpLength;

        QCheckBox *m_checkCaseSensitive;
        QCheckBox *m_checkRecursive;
        QCheckBox *m_checkBackup;
        QCheckBox *m_checkWildcards;
        QCheckBox *m_checkVariables;
        QCheckBox *m_checkConfirmDirs;
        QCheckBox *m_checkConfirmFiles;
        QCheckBox *m_checkConfirmStrings;
        QCheckBox *m_checkHaltOnFirstOccur;
        QCheckBox *m_checkIgnoreWhitespaces;
        QCheckBox *m_checkFollowSymLinks;
        QCheckBox *m_checkAllStringsMustBeFound;
        QCheckBox *m_checkIgnoreHidden;

private: // Data of configuration
        KSettings m_settings;


public: // Functions of access to datas
        KSettings getSettings() {return m_settings;}

protected slots:
        void slotOk();
        void slotDefaults();
};

#endif //KOPTIONSDLG_H
