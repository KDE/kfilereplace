/***************************************************************************
                          KAddStringDlg.cpp  -  description
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

#include "kaddstringdlg.h"

#include <kmessagebox.h>
#include <klocale.h>

#include <qtextedit.h>
#include <qlabel.h>
#include <qpushbutton.h>

KAddStringDlg::KAddStringDlg(QWidget *parent, const char *name) : KAddStringDlgS(parent,name,true)
{
  connect(pbOK,SIGNAL(clicked()),this,SLOT(slotOK()));
}

KAddStringDlg::~KAddStringDlg()
{
}

void KAddStringDlg::slotOK()
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

void KAddStringDlg::setSearchText(const QString &strText) 
{
 m_editSearch -> setText(strText);
}
void KAddStringDlg::setReplaceText(const QString &strText) 
{
 m_editReplace -> setText(strText);
 }
 QString KAddStringDlg::searchText() const
 {
  return m_strSearch;
 }
 QString KAddStringDlg::replaceText() const
 {
  return m_strReplace;
 }
#include "kaddstringdlg.moc"

