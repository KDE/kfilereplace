/***************************************************************************
                          kconfirmdlg.h  -  description
                             -------------------
    begin                : Fri Jun 23 2000
    copyright            : (C) 2000 by François Dupoux
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

#ifndef KCONFIRMDLG_H
#define KCONFIRMDLG_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmultilinedit.h>
#include <qdialog.h>


// ===========================================================================================================================
class KConfirmDlg : public QDialog
{
   Q_OBJECT

public:
        KConfirmDlg(QWidget *parent=0, const char *name=0);
        ~KConfirmDlg();

        enum {Yes=1, No=2, SkipFile=3, SkipDir=4, Cancel=5};

private:
        QString m_strSearch;
        QString m_strReplace;
        QString m_strFile;
        QString m_strDir;

        void initDialog();

protected slots:
        void slotYes();
        void slotNo();
        void slotSkipFile();
        void slotSkipDir();
        void slotCancel();

public:
        void setData(const QString& strFile, const QString& strDir, const QString& strSearch, const QString& strReplace);
        QString getReplaceString() {return m_strReplace;}

private:
        QLabel *m_labelFile;
        QLabel *m_labelDir;
        QLabel *m_labelSearch;
        QMultiLineEdit *m_editSearch;
        QLabel *m_labelReplace;
        QMultiLineEdit *m_editReplace;
        QButton *m_btnYes;
        QButton *m_btnNo;
        QButton *m_btnSkipFile;
        QButton *m_btnSkipDir;
        QButton *m_btnCancel;

};

#endif
