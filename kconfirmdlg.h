/***************************************************************************
                          kconfirmdlg.h  -  description
                             -------------------
    begin                : Fri Jun 23 2000
    copyright            : (C) 2000 by François Dupoux
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

#ifndef KCONFIRMDLG_H
#define KCONFIRMDLG_H

#include "kconfirmdlgs.h"

class KConfirmDlg : public KConfirmDlgS
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
    void slotSkipFolder();
    void slotCancel();

  public:
    void setData(const QString& strFile, const QString& strDir, const QString& strSearch, const QString& strReplace);
    QString replaceString() const { return m_strReplace; }

 };

#endif
