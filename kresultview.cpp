/***************************************************************************
                          kresultview.cpp  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
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

// include files for Qt
#include <qfileinfo.h>

// include files for KDE
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kopenwith.h>
#include <kurl.h>
#include <kpropsdlg.h>
#include <krun.h>
#include <kdebug.h>
#include <kiconloader.h>

// application specific includes
#include "kfilereplacepart.h"
#include "kresultview.h"
#include "kaddstringdlg.h"
#include "kpropertiesdialog.h"
#include "resource.h"
#include "kfilereplacelib.h"

KResultView::KResultView( QWidget *parent, const char *name): QListView( parent, name )
{
  int nRes;

  // Main application class
  m_app = (KFileReplaceApp *) parentWidget()->parentWidget();

  m_lviCurrent = 0L;

  // Initialize ResultList
  addColumn(i18n("Name"), -1 );
  setColumnWidthMode( 0, QListView::Maximum );
  setColumnAlignment( 0, 1 );
  addColumn(i18n("Folder"), -1 );
  setColumnWidthMode( 1, QListView::Maximum );
  setColumnAlignment( 1, 1 );
  addColumn(i18n("Old Size"), -1 );
  setColumnWidthMode( 2, QListView::Maximum );
  setColumnAlignment( 2, 2 );
  addColumn(i18n("New Size"), -1 );
  setColumnWidthMode( 3, QListView::Maximum );
  setColumnAlignment( 3, 2 );
  addColumn(i18n("Replaced Items"), -1 );
  setColumnWidthMode( 4, QListView::Maximum );
  setColumnAlignment( 4, 2 );
  addColumn(i18n("Result"), -1 );
  setColumnWidthMode( 5, QListView::Maximum );
  setColumnAlignment( 5, 1 );
  addColumn(i18n("Owner User"), -1 );
  setColumnWidthMode( 6, QListView::Maximum );
  setColumnAlignment( 6, 1 );
  addColumn(i18n("Owner Group"), -1 );
  setColumnWidthMode( 7, QListView::Maximum );
  setColumnAlignment( 7, 1 );
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setLineWidth(2);
  setRootIsDecorated (true);
  setResizeMode(QListView::LastColumn);
  setAllColumnsShowFocus(true);

  // Create popup menus
  m_kpmResult = new KPopupMenu(this, "ResultPopup");
  m_kpmResult->insertItem(SmallIcon("resfileopen"),i18n("&Open"), this, SLOT(slotResultOpen()));
  m_kpmResult->insertItem(i18n("Open &With..."), this, SLOT(slotResultOpenWith()));
  m_kpmResult->insertItem(SmallIcon("resfileedit"),i18n("&Edit with Kate"), this, SLOT(slotResultEdit()));
  m_kpmResult->insertItem(SmallIcon("resdiropen"),i18n("Open Parent &Folder"), this, SLOT(slotResultDirOpen()));
  m_kpmResult->insertSeparator();
  m_kpmResult->insertItem(SmallIcon("resfiledel"),i18n("&Delete"), this, SLOT(slotResultDelete()));
  m_kpmResult->insertSeparator();
  m_kpmResult->insertItem(SmallIcon("resfileinfo"),i18n("&Properties"), this, SLOT(slotResultProperties()));

  // Load icons
  nRes = m_pmIconSuccess.load( locate("data", "kfilereplace/pics/success.png"));
  nRes = m_pmIconError.load( locate("data", "kfilereplace/pics/error.png"));
  nRes = m_pmIconString.load( locate("data", "kfilereplace/pics/string.png"));
  nRes = m_pmIconSubString.load( locate("data", "kfilereplace/pics/substring.png"));

  connect(this, SIGNAL(mouseButtonClicked (int, QListViewItem *, const QPoint &, int)), this, SLOT(slotMouseButtonClicked (int, QListViewItem *, const QPoint &, int)));
}


KResultView::~KResultView()
{
 if(m_kpmResult)
   delete m_kpmResult;
 m_kpmResult = 0L;
 m_lviCurrent = 0L;
 m_app = 0L;
}


QListViewItem* KResultView::addFullItem(bool bSuccess, const QString &szName, const QString &szDirectory, uint nOldSize, uint nNewSize, int nNbRepl, const QString &szErrMsg)
{
  QString strOldSize,
          strNewSize,
          strNbRepl,
          strTemp;
  QListViewItem *lvi;
  QFileInfo fi; 

  // Prepare text to add
  strOldSize = KFileReplaceLib::instance()->formatSize(nOldSize);
  strNewSize = KFileReplaceLib::instance()->formatSize(nNewSize);
  if (nNbRepl > 0)
    strNbRepl.setNum(nNbRepl);

  // Add item to list
  lvi = new QListViewItem(this, szName, szDirectory, strOldSize);

  // set owners infos
  strTemp = szDirectory + "/" + szDirectory, szName;
  fi.setFile(strTemp);
  strTemp = QString("%1 (%2)").arg(fi.owner()).arg(fi.ownerId());
  lvi->setText(6, strTemp);
  strTemp = QString("%1 (%2)").arg(fi.group()).arg(fi.groupId());
  lvi->setText(7, strTemp);

  if (bSuccess) // Success
    {
      lvi->setPixmap(0, m_pmIconSuccess);
      lvi->setText(3, strNewSize);
      lvi->setText(4, strNbRepl);
      lvi->setText(5, i18n("Success"));
    }
  else // Error
    {
      lvi->setPixmap(0, m_pmIconError);
      if (szErrMsg)
        lvi->setText(5, szErrMsg);
    }

  return lvi;
}


// BUG IN THIS FUNCTION WITH LISTVIEW WHEN USING THREADS
int KResultView::updateItem(QListViewItem *lvi, bool bSuccess, uint nNewSize, int nNbRepl, const QString& szErrMsg)
{
  QString strNewSize;
  QString strNbRepl;
  QString strTemp;
  QFileInfo fi;

  // Prepare text to add
  strNewSize = KFileReplaceLib::instance()->formatSize(nNewSize);
  if (nNbRepl > 0)
    strNbRepl.setNum(nNbRepl);

  // set owners infos
  strTemp = QString("%1/%2").arg(lvi->text(1)).arg(lvi->text(0));
  fi.setFile(strTemp);
  strTemp = QString("%1 (%2)").arg(fi.owner()).arg(fi.ownerId());
  lvi->setText(6, strTemp);
  strTemp = QString("%1 (%2)").arg(fi.group()).arg(fi.groupId());
  lvi->setText(7, strTemp);

  if (bSuccess) // Success
    {
      lvi->setPixmap(0, m_pmIconSuccess);
      lvi->setText(3, strNewSize);
      lvi->setText(4, strNbRepl);
      lvi->setText(5, i18n("Success"));
    }
  else // Error
    {
      lvi->setPixmap(0, m_pmIconError);
      if (szErrMsg)
        lvi->setText(5, szErrMsg);
    }

  return 0;
}


bool KResultView::increaseStringCount(QListViewItem *lvi, QString strTextOld, QString strTextNew, QString strTextReplace,const char *szSearch, int nSearchLen, bool bShowDetails)
{
  // Add item to list
  QString strNum;
  QListViewItem *lviCurItem,
                *lviFirst;
  QListViewItem *lviNew,
                *lviParent = 0L;
  bool bPresent = false;
  QString strTextSearch;
  char szTemp[8192];

  memset(szTemp, 0, sizeof(szTemp));
  strncpy(szTemp, szSearch, MIN((int)nSearchLen,(int)sizeof(szTemp)));
  strTextSearch = QString(szTemp);

  // 1. ---------- Add the parent string in list view if not already present
  bPresent = false;
  lviCurItem = lviFirst = lvi->firstChild();
  if (lviCurItem != 0)
    {
      do
        {
          if (lviCurItem->text(0) == strTextOld)
            {
              bPresent = true;
              lviParent = lviCurItem;

                                // Increase number of strings found
              if (bShowDetails) // if need to show how many strings
                {
                  strNum = lviCurItem->text(4);
                  strNum.setNum( strNum.toInt()+1 );
                  lviCurItem->setText(4, strNum);
                }
            }

          lviCurItem        = lviCurItem->nextSibling();
        } while(lviCurItem && lviCurItem != lviFirst);
    }

  // If parent not present, add it
  if (!bPresent)
    {
      lviParent = new QListViewItem(lvi, strTextOld, strTextNew, "", "", (bShowDetails ? QString("1") : QString("")));
      if (!lviParent)
        return false;
      lviParent->setPixmap(0, m_pmIconString);
    }

  // 2. ----------- Add the detailed string to list view

  // Check if the string is already in the list --> look for all child items
  bPresent = false;
  lviCurItem = lviFirst = lviParent->firstChild();

  if (lviCurItem != 0)
    {
      do
        {
          if (lviCurItem->text(0) == strTextSearch)
            {
              bPresent = true;

              if (bShowDetails) // if need to show how many strings
                {
                  // Increase number
                  strNum = lviCurItem->text(4);
                  strNum.setNum( strNum.toInt()+1 );
                  lviCurItem->setText(4, strNum);
                }
              return true;
            }

          lviCurItem        = lviCurItem->nextSibling();
        } while(lviCurItem && lviCurItem != lviFirst);
    }


  if (!bPresent)
    {
      lviNew = new QListViewItem(lviParent, strTextSearch, strTextReplace, "", "", (bShowDetails ? QString("1") : QString("")));
      if (lviNew == 0)
        return false;
      lviNew->setPixmap(0, m_pmIconSubString);
    }

  return true;
}

QPixmap KResultView::iconString() 
{
 return m_pmIconString;
}

void KResultView::slotMouseButtonClicked (int nButton, QListViewItem *lvi, const QPoint &pos, int /*column*/)
{
  // Don't look at events while working
  if (g_bThreadRunning)
    return;

  if (lvi == 0) // No item selected
    return;

  // LEFT BUTTON
  /*if (event->button() == QMouseEvent::LeftButton)
    {
    }*/

  // RIGHT BUTTON
  if (nButton == QMouseEvent::RightButton)
    {
      m_lviCurrent = lvi;
      m_kpmResult->popup(pos);
    }
}


QString KResultView::currentItem()
{
  QString strFilename;
  QListViewItem *lvi;

  if(!m_lviCurrent)
    if(!(m_lviCurrent = QListView::currentItem()))
      return QString::null;
/*  if (!m_lviCurrent)
    m_lviCurrent = QListView::currentItem();

  if (!m_lviCurrent) // If no selected item
    return QString::null;*/

  lvi = m_lviCurrent;
  while (lvi->parent())
    lvi = lvi->parent();

  strFilename = QString("%1/%2").arg(lvi->text(1)).arg(lvi->text(0));

  return strFilename;
}


void KResultView::slotResultProperties()
{
  if (!currentItem().isEmpty())
  {
    KURL url(currentItem());
    (void) new KPropertiesDialog(url);
    m_lviCurrent = 0L;
  }  
}


void KResultView::slotResultOpen()
{
  if (!currentItem().isEmpty())
  {
    (void) new KRun(currentItem(), 0, true, true);
    m_lviCurrent = 0L;
  }  
}


void KResultView::slotResultOpenWith()
{
  if (!currentItem().isEmpty())
  {
    KURL::List kurls;
    kurls.append(currentItem());
    KRun::displayOpenWithDialog(kurls);
    m_lviCurrent = 0L;
  } 
}


void KResultView::slotResultDirOpen()
{
  if (!currentItem().isEmpty())
  {
    QFileInfo fiFile;
    fiFile.setFile(currentItem());
    (void) new KRun (fiFile.dirPath(), 0, true, true);
    m_lviCurrent = 0L;
  }
}

void KResultView::slotResultEdit()
{
  if (!currentItem().isEmpty())
  {
    QString strCommand = QString("kate %1 &").arg(currentItem());
    KRun::runCommand(strCommand);
    m_lviCurrent = 0L;
  }    
}

void KResultView::slotResultDelete()
{
  if (!currentItem().isEmpty())
  {
    QFile fiFile;
    int nRes = KMessageBox::questionYesNo(this, i18n("<qt>Do you really want to delete <b>%1</b>?</qt>").arg(currentItem()));

    if (nRes == KMessageBox::Yes)
    {
      fiFile.setName(currentItem());
      fiFile.remove();

      // Remove item from list if file was deleted
      takeItem(m_lviCurrent); // Remove item from ListView
    }

   m_lviCurrent = 0L;
  } 
}


void KResultView::slotResultTreeExpand()
{
  QListViewItem *lviRoot = firstChild();

  if (lviRoot)
    expand(lviRoot, true);
}


void KResultView::slotResultTreeReduce()
{
  QListViewItem *lviRoot = firstChild();

  if (lviRoot)
    expand(lviRoot, false);
}


void KResultView::expand(QListViewItem *lviCurrent, bool bExpand)
{
  // current item
  lviCurrent->setOpen(bExpand);

  // recursivity
  while((lviCurrent = lviCurrent->nextSibling()) != 0)
    {
      lviCurrent->setOpen(bExpand);

      if (lviCurrent->firstChild())
        expand(lviCurrent->firstChild(), bExpand);
    }
}

#include "kresultview.moc"
