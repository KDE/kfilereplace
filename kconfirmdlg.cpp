/***************************************************************************
                          kconfirmdlg.cpp  -  description
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

#include <qlayout.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <klocale.h>

#include "kconfirmdlg.h"

KConfirmDlg::KConfirmDlg(QWidget *parent, const char *name) : KConfirmDlgS(parent,name,true)
{
  initDialog();
  
  connect(pbYes,SIGNAL(clicked()),this,SLOT(slotYes()));
  connect(pbNo,SIGNAL(clicked()),this,SLOT(slotNo()));
  connect(pbSkipFile,SIGNAL(clicked()),this,SLOT(slotSkipFile()));
  connect(pbSkipFolder,SIGNAL(clicked()),this,SLOT(slotSkipFolder()));
  connect(pbCancel,SIGNAL(clicked()),this,SLOT(slotCancel()));
}


KConfirmDlg::~KConfirmDlg()
{
}

void KConfirmDlg::initDialog()
{
  // set normal cursor
  QApplication::setOverrideCursor(Qt::arrowCursor);
}

void KConfirmDlg::setData(const QString& strFile, const QString& strDir, const QString& strSearch, const QString& strReplace)
{
  m_strFile = strFile;
  m_strDir = strDir;
  m_strSearch = strSearch;
  m_strReplace = strReplace;

  tlFile->setText(i18n("<qt>File: <b>%1</b></qt>").arg(strFile));

  tlFolder->setText(i18n("<qt>Folder: <b>%1</b></qt>").arg(strDir));

  txtedSearch->setText(strSearch);

  txtedReplace->setText(strReplace);
}

void KConfirmDlg::slotYes()
{
  // get new replace string if edited
  if (txtedReplace->isModified())
    m_strReplace = txtedReplace->text();

  QApplication::restoreOverrideCursor();
  done(Yes);
}

void KConfirmDlg::slotNo()
{
  QApplication::restoreOverrideCursor();
  done(No);
}

void KConfirmDlg::slotSkipFile()
{
  QApplication::restoreOverrideCursor();
  done(SkipFile);
}

void KConfirmDlg::slotSkipFolder()
{
  QApplication::restoreOverrideCursor();
  done(SkipDir);
}

void KConfirmDlg::slotCancel()
{
  QApplication::restoreOverrideCursor();
  done(Cancel);
}

#include "kconfirmdlg.moc"
