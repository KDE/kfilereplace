/***************************************************************************
                          kconfirmdlg.cpp  -  description
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

#include "kconfirmdlg.h"
#include <kapplication.h>
#include <klocale.h>

#include <qlayout.h>

// ===========================================================================================================================
KConfirmDlg::KConfirmDlg(QWidget *parent, const char *name) : QDialog(parent,name,true)
{
  initDialog();

  setCaption(i18n("Do you want to replace the first string with the second ?"));

  connect(m_btnYes,SIGNAL(clicked()),this,SLOT(slotYes()));
  connect(m_btnNo,SIGNAL(clicked()),this,SLOT(slotNo()));
  connect(m_btnSkipFile,SIGNAL(clicked()),this,SLOT(slotSkipFile()));
  connect(m_btnSkipDir,SIGNAL(clicked()),this,SLOT(slotSkipDir()));
  connect(m_btnCancel,SIGNAL(clicked()),this,SLOT(slotCancel()));
}

// ===========================================================================================================================
KConfirmDlg::~KConfirmDlg()
{
}

// ===========================================================================================================================
void KConfirmDlg::initDialog()
{
  // resize dialog
  setMinimumSize(300, 200);
  resize(500, 300);

  // set normal cursor
  QApplication::setOverrideCursor(Qt::arrowCursor);

  QGridLayout *gbox;
  gbox = new QGridLayout (this, 7, 5, 0, 10);
  gbox -> setMargin(15);

  // widgets
  m_labelFile = new QLabel(this,"m_labelFile");
  m_labelFile->setText(i18n("File:"));
  gbox -> addMultiCellWidget (m_labelFile, 0, 0, 0, 4);

  m_labelDir = new QLabel(this,"m_labelDir");
  m_labelDir->setText(i18n("Directory:"));
  gbox -> addMultiCellWidget (m_labelDir, 1, 1, 0, 4);

  m_labelSearch = new QLabel(this,"m_labelSearch");
  m_labelSearch->setText(i18n("Search for:"));
  gbox -> addMultiCellWidget (m_labelSearch, 2, 2, 0, 4);

  m_editSearch = new QMultiLineEdit(this,"m_editSearch");
  m_editSearch -> setReadOnly(true);
  gbox -> addMultiCellWidget (m_editSearch, 3, 3, 0, 4);

  m_labelReplace = new QLabel(this,"m_labelReplace");
  m_labelReplace->setText(i18n("Replace with:"));
  gbox -> addMultiCellWidget (m_labelReplace, 4, 4, 0, 4);

  m_editReplace = new QMultiLineEdit(this,"m_editReplace");
  gbox -> addMultiCellWidget (m_editReplace, 5, 5, 0, 4);

  // buttons
  m_btnYes = new QPushButton(this,"m_btnYes");
  m_btnYes->setText(i18n("&Yes"));
  gbox -> addWidget (m_btnYes, 6, 0);

  m_btnNo = new QPushButton(this,"m_btnNo");
  m_btnNo->setText(i18n("&No"));
  gbox -> addWidget (m_btnNo, 6, 1);

  m_btnSkipFile = new QPushButton(this,"m_btnSkipFile");
  m_btnSkipFile->setText(i18n("Skip &File"));
  gbox -> addWidget (m_btnSkipFile, 6, 2);

  m_btnSkipDir = new QPushButton(this,"m_btnSkipDir");
  m_btnSkipDir->setText(i18n("Skip &Directory"));
  gbox -> addWidget (m_btnSkipDir, 6, 3);

  m_btnCancel = new QPushButton(this,"m_btnCancel");
  m_btnCancel->setText(i18n("Cancel"));
  gbox -> addWidget (m_btnCancel, 6, 4);

}

// ===========================================================================================================================
void KConfirmDlg::setData(const QString& strFile, const QString& strDir, const QString& strSearch, const QString& strReplace)
{
  m_strFile = strFile;
  m_strDir = strDir;
  m_strSearch = strSearch;
  m_strReplace = strReplace;

  m_labelFile -> setText(i18n("<qt>File: <b>%1</b></qt>").arg(strFile));

  m_labelDir -> setText(i18n("<qt>Directory: <b>%1</b></qt>").arg(strDir));

  m_editSearch -> setText(strSearch);

  m_editReplace -> setText(strReplace);
}

// ===========================================================================================================================
void KConfirmDlg::slotYes()
{
  // get new replace string if edited
  if (m_editReplace->edited())
    m_strReplace = m_editReplace->text();

  QApplication::restoreOverrideCursor();
  done(Yes);
}

// ===========================================================================================================================
void KConfirmDlg::slotNo()
{
  QApplication::restoreOverrideCursor();
  done(No);
}

// ===========================================================================================================================
void KConfirmDlg::slotSkipFile()
{
  QApplication::restoreOverrideCursor();
  done(SkipFile);
}

// ===========================================================================================================================
void KConfirmDlg::slotSkipDir()
{
  QApplication::restoreOverrideCursor();
  done(SkipDir);
}

// ===========================================================================================================================
void KConfirmDlg::slotCancel()
{
  QApplication::restoreOverrideCursor();
  done(Cancel);
}




#include "kconfirmdlg.moc"

