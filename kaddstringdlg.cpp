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
#include <qwidgetstack.h>

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

KAddStringDlg::KAddStringDlg(RCOptions* info, bool wantEdit, QWidget *parent, const char *name) : KAddStringDlgS(parent,name,true)
{
  m_option = info;
  m_wantEdit = wantEdit;
  m_currentMap = m_option->m_mapStringsView;

  initGUI();

  connect(m_pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(m_rbSearchOnly, SIGNAL(pressed()), this, SLOT(slotSearchOnly()));
  connect(m_rbSearchReplace, SIGNAL(pressed()), this, SLOT(slotSearchReplace()));
  connect(m_pbAdd, SIGNAL(clicked()), this, SLOT(slotAddStringToView()));
  connect(m_pbDel, SIGNAL(clicked()), this, SLOT(slotDeleteStringFromView()));
  connect(m_pbHelp, SIGNAL(clicked()), this ,SLOT(slotHelp()));

  whatsThis();
}

void KAddStringDlg::raiseView()
{
  if(m_option->m_searchingOnlyMode)
    m_sv = m_stringView_2;
  else
    m_sv = m_stringView;

  m_stack->raiseWidget(m_sv);
}

bool KAddStringDlg::columnContains(QListView* lv,const QString& s, int column)
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

void KAddStringDlg::saveViewContentIntoMap()
{
  QListViewItem* i = m_sv->firstChild();
  while(i != 0)
    {
      if(m_option->m_searchingOnlyMode)
        m_currentMap[i->text(0)] = QString::null;
      else
        m_currentMap[i->text(0)] = i->text(1);
      i = i->nextSibling();
    }
}

void KAddStringDlg::loadMapIntoView()
{
  KeyValueMap::Iterator itMap;

  for (itMap = m_currentMap.begin(); itMap != m_currentMap.end(); ++itMap)
    {
      QListViewItem* temp = new QListViewItem(m_sv);
      temp->setText(0,itMap.key());
      if(!m_option->m_searchingOnlyMode)
        temp->setText(1,itMap.data());
    }
}

void KAddStringDlg::slotOK()
{
  m_option->m_mapStringsView = m_currentMap;

  accept();
}

void KAddStringDlg::slotSearchOnly()
{
  m_option->m_searchingOnlyMode = true;

  m_rbSearchOnly->setChecked(true);
  m_edSearch->setEnabled(true);
  m_edReplace->setEnabled(false);
  m_tlSearch->setEnabled(true);
  m_tlReplace->setEnabled(false);

  //sets the right view appearance
  raiseView();
  //empties the view content
  eraseViewItems();
}

void KAddStringDlg::slotSearchReplace()
{
  m_option->m_searchingOnlyMode = false;

  m_rbSearchReplace->setChecked(true);
  m_edSearch->setEnabled(true);
  m_edReplace->setEnabled(true);
  m_tlSearch->setEnabled(true);
  m_tlReplace->setEnabled(true);

  //sets the right view appearance
  raiseView();
  //empties the view content
  eraseViewItems();
}

void KAddStringDlg::slotAddStringToView()
{
  if(m_option->m_searchingOnlyMode)
    {
      QString text = m_edSearch->text();
      if(!(text.isEmpty() || columnContains(m_sv, text, 0)))
        {
          QListViewItem* lvi = new QListViewItem(m_sv);
          lvi->setMultiLinesEnabled(true);
          lvi->setText(0,text);
          m_currentMap[text] = QString::null;
          m_edSearch->clear();
        }
    }
  else
    {
      QString searchText = m_edSearch->text(),
              replaceText = m_edReplace->text();

      if(!(searchText.isEmpty() || replaceText.isEmpty() || columnContains(m_sv,searchText,0) || columnContains(m_sv,replaceText,1)))
        {
          QListViewItem* lvi = new QListViewItem(m_sv);
          lvi->setMultiLinesEnabled(true);
          lvi->setText(0,searchText);
          m_edSearch->clear();
          lvi->setText(1,replaceText);
          m_currentMap[searchText] = replaceText;
          m_edReplace->clear();
        }
    }
}

void KAddStringDlg::slotDeleteStringFromView()
{
  // Choose current item or selected item
  QListViewItem* currentItem = m_sv->currentItem();

  // Do nothing if list is empty
  if(currentItem == 0)
    return;

  m_currentMap.remove(currentItem->text(0));

  if(m_option->m_searchingOnlyMode)
    {
      m_edSearch->setText(currentItem->text(0));
      m_edReplace->clear();
      //currentItem->setText(1,m_edReplace->text());
    }
  else
    {
      m_edSearch->setText(currentItem->text(0));
      m_edReplace->setText(currentItem->text(1));
    }

  delete currentItem;

  currentItem = 0;
}

void KAddStringDlg::slotHelp()
{
  kapp->invokeHelp(QString::null, "kfilereplace");
}

void KAddStringDlg::eraseViewItems()
{
  QListViewItem* item = m_sv->firstChild();
  if(item == 0)
    return;
  else
    {
      while(item)
        {
          QListViewItem* tempItem = item;
          item = item->nextSibling();
          delete tempItem;
        }
    }
}

void KAddStringDlg::initGUI()
{
  m_pbAdd->setIconSet(SmallIconSet(QString::fromLatin1("forward")));
  m_pbDel->setIconSet(SmallIconSet(QString::fromLatin1("back")));

  m_stack->addWidget(m_stringView);
  m_stack->addWidget(m_stringView_2);


 if(m_option->m_searchingOnlyMode)
    {
      if(m_wantEdit)
        m_rbSearchReplace->setEnabled(false);
      m_rbSearchOnly->setChecked(true);
      m_edSearch->setEnabled(true);
      m_edReplace->setEnabled(false);
      m_tlSearch->setEnabled(true);
      m_tlReplace->setEnabled(false);
    }
  else
   {
     if(m_wantEdit)
       m_rbSearchOnly->setEnabled(false);
     m_rbSearchReplace->setChecked(true);
     m_edSearch->setEnabled(true);
     m_edReplace->setEnabled(true);
     m_tlSearch->setEnabled(true);
     m_tlReplace->setEnabled(true);
   }

  raiseView();

  if(m_wantEdit)
    loadMapIntoView();
}

void KAddStringDlg::whatsThis()
{
  QWhatsThis::add(m_rbSearchOnly, rbSearchOnlyWhatthis);
  QWhatsThis::add(m_rbSearchReplace, rbSearchReplaceWhatthis);
  QWhatsThis::add(m_edSearch, edSearchWhatthis);
  QWhatsThis::add(m_edReplace, edReplaceWhatthis);
}
#include "kaddstringdlg.moc"

