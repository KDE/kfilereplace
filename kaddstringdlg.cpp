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
// QT
#include <qtextedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlistview.h>
#include <qwhatsthis.h>

// KDE
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>

// local
#include "kaddstringdlg.h"
#include "whatthis.h"

using namespace whatthisNameSpace;

KAddStringDlg::KAddStringDlg(QWidget *parent, const char *name) : KAddStringDlgS(parent,name,true)
{
  m_pbAdd->setIconSet(SmallIconSet(QString::fromLatin1("next")));
  m_pbDel->setIconSet(SmallIconSet(QString::fromLatin1("back")));

  connect(m_pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(m_rbSearchOnly, SIGNAL(toggled(bool)), this, SLOT(slotSearchOnly(bool)));
  connect(m_rbSearchReplace, SIGNAL(toggled(bool)), this, SLOT(slotSearchReplace(bool)));
  connect(m_pbAdd, SIGNAL(clicked()), this, SLOT(slotAdd()));
  connect(m_pbDel, SIGNAL(clicked()), this, SLOT(slotDel()));
  connect( m_pbHelp, SIGNAL(clicked()), this ,SLOT(slotHelp()));

  clearStringsView();

  whatsThis();

}

KAddStringDlg::~KAddStringDlg()
{
}

bool KAddStringDlg::contains(QListView* lv,const QString& s, int column)
{
  QListViewItem* i = lv->firstChild();
  while (i != 0)
    {
      if(i->text(column) == s)
        return true;
      i = i->nextSibling();
    }
  return false;
}

void KAddStringDlg::updateStringsMapContent()
{
  KeyValueMap map;
  QListViewItem* i = m_stringView->firstChild();
  while(i != 0)
    {
      map[i->text(0)] = i->text(1);
      i = i->nextSibling();
    }
  m_option.setMapStringsView(map);
}

void KAddStringDlg::updateStringsViewContent()
{
  KeyValueMap::Iterator itMap;

  KeyValueMap map =  m_option.mapStringsView();
  for (itMap = map.begin(); itMap != map.end(); ++itMap)
    {
      QListViewItem* temp = new QListViewItem(m_stringView);
      temp->setText(0,itMap.key());
      temp->setText(1,itMap.data());
    }
}

void KAddStringDlg::slotOK()
{
  qWarning("KAddStringDlg::slotOK--->search=%d",m_option.searchMode());
  accept();
}

void KAddStringDlg::slotSearchOnly(bool b)
{
  m_edSearch->setEnabled(b);
  m_edReplace->clear();
  m_edReplace->setEnabled(false);
  m_tlSearch->setEnabled(b);
  m_tlReplace->setEnabled(false);

  clearStringsView();
  updateStringsMapContent();
}

void KAddStringDlg::slotSearchReplace(bool b)
{
  m_edSearch->setEnabled(b);
  m_edReplace->setEnabled(b);
  m_tlSearch->setEnabled(b);
  m_tlReplace->setEnabled(b);

  clearStringsView();
  updateStringsMapContent();
}

void KAddStringDlg::slotAdd()
{
  bool searchOnly = m_rbSearchOnly->isChecked();
  if(searchOnly)
    {
      QString text = m_edSearch->text();
      if(!text.isEmpty() && !contains(m_stringView, text, 0))
        {
          QListViewItem* lvi = new QListViewItem(m_stringView);
          lvi->setMultiLinesEnabled(true);
          lvi->setText(0,text);
          m_edSearch->clear();
        }
    }
  else
    {
      QString searchText = m_edSearch->text(),
              replaceText = m_edReplace->text();

      if(!searchText.isEmpty() &&
         !replaceText.isEmpty() &&
         !contains(m_stringView,searchText,0) &&
         !contains(m_stringView,replaceText,1))
        {
          QListViewItem* lvi = new QListViewItem(m_stringView);
          lvi->setMultiLinesEnabled(true);
          lvi->setText(0,searchText);
          m_edSearch->clear();
          lvi->setText(1,replaceText);
          m_edReplace->clear();
        }
    }
  m_option.setSearchMode(searchOnly);
  qWarning("KAddStringDlg::slotAdd--->search=%d",m_option.searchMode());
  updateStringsMapContent();
}

void KAddStringDlg::slotDel()
{
  // Choose current item or selected item
  QListViewItem* currentItem = m_stringView->currentItem();

  // Do nothing if list is empty
  if(currentItem == 0)
    return;
  bool searchOnly =  m_rbSearchOnly->isChecked();
  if(searchOnly)
    {
      m_edSearch->setText(currentItem->text(0));
      m_edReplace->clear();
      currentItem->setText(1,m_edReplace->text());
    }
  else
    {
      m_edSearch->setText(currentItem->text(0));
      m_edReplace->setText(currentItem->text(1));
    }
  delete currentItem;
  currentItem = 0;
  m_option.setSearchMode(searchOnly);
  qWarning("KAddStringDlg::slotDel--->search=%d",m_option.searchMode());
  updateStringsMapContent();
}

void KAddStringDlg::slotHelp()
{
  kapp->invokeHelp(QString::null, "kfilereplace");
}

void KAddStringDlg::loadViewContent(KeyValueMap map)
{
  m_stringView->clear();

  KeyValueMap::Iterator itMap;
  bool searchOnly = true;
  for (itMap = map.begin(); itMap != map.end(); ++itMap)
    {
      QListViewItem* i = new QListViewItem(m_stringView);
      i->setText(0,itMap.key());
      i->setText(1,itMap.data());

      if(!itMap.data().isEmpty())
        searchOnly = false;
    }

  m_rbSearchOnly->setChecked(searchOnly);
  m_rbSearchReplace->setChecked(!searchOnly);

  m_edSearch->setEnabled(true);
  m_edReplace->setEnabled(!searchOnly);
  m_tlSearch->setEnabled(true);
  m_tlReplace->setEnabled(!searchOnly);

  m_option.setSearchMode(searchOnly);
  qWarning("KAddStringDlg::loadViewContent--->search=%d",m_option.searchMode());

  updateStringsMapContent();
}

void KAddStringDlg::clearStringsView()
{
  QListViewItem* item = m_stringView->firstChild();
  if(item == 0)
    return;
  else
    {
      QListViewItem* tempItem = item;
      item = item->nextSibling();
      delete tempItem;
    }
}

void KAddStringDlg::whatsThis()
{
  QWhatsThis::add(m_rbSearchOnly, rbSearchOnlyWhatthis);
  QWhatsThis::add(m_rbSearchReplace, rbSearchReplaceWhatthis);
  QWhatsThis::add(m_edSearch, edSearchWhatthis);
  QWhatsThis::add(m_edReplace, edReplaceWhatthis);
}
#include "kaddstringdlg.moc"

