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
// qt
#include <qtextedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlistview.h>
#include <qiconset.h>
#include <qpixmap.h>
// kde
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
// local
#include "kaddstringdlg.h"

KAddStringDlg::KAddStringDlg(QWidget *parent, const char *name) : KAddStringDlgS(parent,name,true)
{ 
  connect(pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(rbSearchOnly, SIGNAL(toggled(bool)), this, SLOT(slotSearchOnly(bool)));
  connect(rbSearchReplace, SIGNAL(toggled(bool)), this, SLOT(slotSearchReplace(bool)));
  connect(pbAdd, SIGNAL(clicked()), this, SLOT(slotAdd())); 
  connect(pbDel, SIGNAL(clicked()), this, SLOT(slotDel())); 
  pbAdd->setIconSet(QIconSet(SmallIconSet(QString::fromLatin1("next")))); 
  pbDel->setIconSet(QIconSet(SmallIconSet(QString::fromLatin1("back")))); 
}

KAddStringDlg::~KAddStringDlg()
{
  m_config = 0L;
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

void KAddStringDlg::setMap()
{
  m_map.clear();
  QListViewItem* i = stringView->firstChild();
  while (i != 0)
    {
      m_map[i->text(0)] = i->text(1);
      i = i->nextSibling();
    }
}

void KAddStringDlg::slotOK()
{
  m_config->setGroup("General Options");
  if(rbSearchOnly->isChecked())
    m_config->writeEntry(rcSearchMode,true);
  else
    m_config->writeEntry(rcSearchMode,false);
  m_config->sync();
  
  accept();
}

void KAddStringDlg::slotSearchOnly(bool b)
{
  m_editSearch->setEnabled(b);
  m_editReplace->setEnabled(false);
  m_labelSearch->setEnabled(b);
  m_labelReplace->setEnabled(false);
  
  stringView->clear();  
}
	
void KAddStringDlg::slotSearchReplace(bool b)
{
  m_editSearch->setEnabled(b);
  m_editReplace->setEnabled(b);
  m_labelSearch->setEnabled(b);
  m_labelReplace->setEnabled(b);
  
  stringView->clear();
}

void KAddStringDlg::slotAdd()
{
  if(rbSearchOnly->isChecked())
    {
      QString text = m_editSearch->text();
      if(!text.isEmpty() and !contains(stringView,text,0))
        {
	  QListViewItem* lvi = new QListViewItem(stringView);
	  lvi->setText(0,text);
          m_editSearch->clear();
	}     
    }
  else
    { 
      QString searchText = m_editSearch->text(),
              replaceText = m_editReplace->text();
      
      if(!searchText.isEmpty() and 
         !replaceText.isEmpty() and
         !contains(stringView,searchText,0) and 
	 !contains(stringView,replaceText,1))
        {
	  QListViewItem* lvi = new QListViewItem(stringView);
	  lvi->setText(0,searchText);
	  m_editSearch->clear();
	  lvi->setText(1,replaceText);
	  m_editReplace->clear();
	}    
    }
  setMap(); 
}

void KAddStringDlg::slotDel()
{
  // Choose current item or selected item
  QListViewItem* currItem = stringView->currentItem();
  
  // Do nothing if list is empty
  if(currItem == 0)
    return;
      
  if(rbSearchOnly->isChecked())
    {
      m_editSearch->setText(currItem->text(0));
      delete currItem;
    }
  else
    {
      m_editSearch->setText(currItem->text(0));
      m_editReplace->setText(currItem->text(1));
      delete currItem;
    }
  setMap();
}

QMap<QString,QString> KAddStringDlg::stringList()
{
  return m_map;
}

void KAddStringDlg::loadDataFromStringsView(QMap<QString,QString> map)
{ 
  stringView->clear();
 
  QMap<QString,QString>::Iterator itMap;
 
  for (itMap = map.begin(); itMap != map.end(); ++itMap)
    {  
      QListViewItem* i = new QListViewItem(stringView);
      i->setText(0,itMap.key());
      i->setText(1,itMap.data());    
    }
   
   QListViewItem* p=stringView->firstChild(); 
   
  if(p->text(1).isEmpty())
    {  
      m_editSearch->setEnabled(true);
      m_editReplace->setEnabled(false);
      m_labelSearch->setEnabled(true);
      m_labelReplace->setEnabled(false);
    }
  else
    {
      m_editSearch->setEnabled(true);
      m_editReplace->setEnabled(true);
      m_labelSearch->setEnabled(true);
      m_labelReplace->setEnabled(true);
    }
   
  setMap();
}

#include "kaddstringdlg.moc"

