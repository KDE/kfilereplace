/***************************************************************************
                          KAddStringDlg.cpp  -  description
                             -------------------
    begin                : Sat Oct 16 1999
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

#include "kaddstringdlg.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <qmultilineedit.h>
#include <qlabel.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include <qvbox.h>
#include <qframe.h>
#include <qlayout.h>

// ===========================================================================================================================
KAddStringDlg::KAddStringDlg(QWidget *parent, const char *name) : QDialog(parent,name,true)
{
  initDialog();

  connect(m_btnOk,SIGNAL(clicked()),this,SLOT(slotOk()));
  connect(m_btnCancel,SIGNAL(clicked()),this,SLOT(reject()));
}

// ===========================================================================================================================
KAddStringDlg::~KAddStringDlg()
{
}

// ===========================================================================================================================
void KAddStringDlg::slotOk()
{
  // Check the Search text is not empty
  if (m_editSearch -> text().isEmpty())
    {
      KMessageBox::error(this, i18n("You must fill the text to find (and Replace) before clicking on 'OK'."));
      return;
    }

  // Copy text into variables
  m_strSearch = m_editSearch -> text();
  m_strReplace = m_editReplace -> text();

  accept();
}

// ===========================================================================================================================
void KAddStringDlg::initDialog()
{
  // resize dialog
  setMinimumSize(300, 200);
  resize(500, 300);

  QGridLayout *gbox;
  gbox = new QGridLayout (this, 2, 2, 0, 10);
  gbox -> setMargin(15);

  // widgets
  m_labelSearch = new QLabel(this,"m_labelSearch");
  m_labelSearch->setText(i18n("Search for:"));
  gbox -> addWidget (m_labelSearch, 0, 0);

  m_editSearch = new QMultiLineEdit(this,"m_editSearch");
  gbox -> addMultiCellWidget (m_editSearch, 1, 1, 0, 1);

  m_labelReplace = new QLabel(this,"m_labelReplace");
  m_labelReplace->setText(i18n("Replace with:"));
  gbox -> addWidget (m_labelReplace, 3, 0);

  m_editReplace = new QMultiLineEdit(this,"m_editReplace");
  gbox -> addMultiCellWidget (m_editReplace, 4, 4, 0, 1);

  // ok / cancel
  m_btnOk = new KPushButton( KStdGuiItem::ok(), this,"m_btnOk");
  gbox -> addWidget (m_btnOk, 6, 0);

  m_btnCancel = new KPushButton( KStdGuiItem::cancel(), this,"m_btnCancel");
  gbox -> addWidget (m_btnCancel, 6, 1);
}
#include "kaddstringdlg.moc"

