/***************************************************************************
                          kfilereplaceview.cpp  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by Fran�is Dupoux <dupoux@dupoux.com>
                           (C) 2004 Emiliano Gulmini <emi_barbarossa@yahoo.it>
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
#include <qprinter.h>
#include <qpainter.h>
#include <qwhatsthis.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qlistview.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <qvaluelist.h>

// include files for KDE
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kurl.h>
#include <kpropertiesdialog.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <dcopref.h>
// application specific includes
#include "kfilereplaceview.h"
#include "kfilereplacedoc.h"
#include "kfilereplacelib.h"

#ifdef APP
#include "kfilereplace.h"
#else
#include "kfilereplacepart.h"
#endif

#include "kaddstringdlg.h"

#include "resource.h"
#include "whatthis.h"

using namespace whatthisNameSpace;

KFileReplaceView::KFileReplaceView(QWidget *parent,const char *name):KFileReplaceViewWdg(parent,name)
{
  m_path = KGlobal::instance()->dirs()->saveLocation("data", "kfilereplace/");

  // Create popup menus
  m_kpmResult = new KPopupMenu(this, "ResultPopup");

  m_kpmResult->insertItem(QPixmap("resfileopen"),
                          i18n("&Open"),
                          this,
                          SLOT(slotResultOpen()));
  m_kpmResult->insertItem(i18n("Open &With..."),
                          this,
                          SLOT(slotResultOpenWith()));
  /*DCOPClient *client = kapp->dcopClient();
  if(QString(client->appId()).contains("quanta"))*/
  //if(QString(kapp->startupId()).contains("quanta")==0)
  m_kpmResult->insertItem(QPixmap("resfileedit"),
                          i18n("&Edit with Quanta"),
                          this,
                          SLOT(slotResultEdit()));
  m_kpmResult->insertItem(QPixmap("resdiropen"),
                          i18n("Open Parent &Folder"),
                          this,
                          SLOT(slotResultDirOpen()));
  m_kpmResult->insertItem(QPixmap("resfiledel"),
                          i18n("&Delete"),
                          this,
                          SLOT(slotResultDelete()));
  m_kpmResult->insertSeparator();
  m_kpmResult->insertItem(QPixmap("resfileinfo"),
                          i18n("&Properties"),
                          this,
                          SLOT(slotResultProperties()));

  // Load icons
  m_pmIconString.load( locate("data", "kfilereplace/pics/string.png"));
  m_pmIconSuccess.load( locate("data", "kfilereplace/pics/success.png"));
  m_pmIconError.load( locate("data", "kfilereplace/pics/error.png"));
  m_pmIconSubString.load( locate("data", "kfilereplace/pics/substring.png"));

  // connect events
  connect(lwResult,
          SIGNAL(mouseButtonClicked(int, QListViewItem *, const QPoint &, int)), this,
          SLOT(slotMouseButtonClicked(int, QListViewItem *, const QPoint &, int)));
  connect(lwStrings,
          SIGNAL(doubleClicked(QListViewItem *)),
          this,
          SLOT(slotStringsEdit(QListViewItem *)));

  dlg = new KAddStringDlg(parentWidget());

  QWhatsThis::add(lwResult, i18n(lwResultWhatthis));
  QWhatsThis::add(lwStrings, i18n(lwStringsWhatthis));
}

KFileReplaceView::~KFileReplaceView()
{
 //delete m_kpmResult;
 delete dlg;
}

QListView* KFileReplaceView::stringView()
{
 return lwStrings;
}

QListView* KFileReplaceView::resultView()
{
 return lwResult;
}

QPixmap KFileReplaceView::iconString()
{
 return m_pmIconString;
}

KFileReplaceDoc* KFileReplaceView::document() const
{
#ifdef APP
  KFileReplaceApp* theApp=(KFileReplaceApp*)parentWidget();
  return theApp->document();
#else
  KFileReplacePart* theApp=(KFileReplacePart*)parentWidget();
  return theApp->document();
#endif
}

bool KFileReplaceView::addString( QListViewItem *lviCurrent, const QString &searchStr, const QString &replaceStr)
{
  QListViewItem *lviCurItem,
                *lviFirst;
  QString searchText, replaceText;
   if (searchStr.isEmpty())
   {
       searchText = dlg->searchText(),
       replaceText = dlg->replaceText();
   } else
   {
       searchText = searchStr;
       replaceText = replaceStr;
   }
  // Check item is not already in the TextList
  lviCurItem = lviFirst = lwStrings->firstChild();
  if (lviCurItem != 0)
    {
      do // For all strings there are in the TextList
        {
          if ((lviCurrent != lviCurItem) && (searchText == lviCurItem->text(0))) // Item is already in the TextList
            {
              QString strMess = QString(i18n("<qt>The <b>%1</b> item is already present in the list.</qt>")).arg(searchText);
              KMessageBox::error(parentWidget(), strMess);
              return false;
            }

          lviCurItem = lviCurItem->nextSibling();
        } while(lviCurItem && lviCurItem != lviFirst);
    }

  // Check there is not too items to replace
  if (lwStrings->childCount() >= MaxStringToSearch)
    {
      QString strMess = QString(i18n("Unable to have more than %1 items to search or replace.")).arg(MaxStringToSearch);
      KMessageBox::error(parentWidget(), strMess);
      return false;
    }

  // Add string to string list
  QListViewItem* lvi = new QListViewItem(lwStrings);
  Q_CHECK_PTR( lvi );
  lvi->setPixmap(0, m_pmIconString);
  lvi->setText(0, searchText);
  lvi->setText(1, replaceText);

  lvi->setup();

  return true;
}

bool KFileReplaceView::editString(QListViewItem *lviCurrent)
{
 QListViewItem *lviCurItem,
               *lviFirst;
  QString strMess,
          searchText = dlg->searchText(),
          replaceText = dlg->replaceText();

  // Check item is not already in the TextList
  lviCurItem = lviFirst = lwStrings->firstChild();
  if (lviCurItem != 0)
    {
      do // For all strings there are in the TextList
        {
          if ((lviCurrent != lviCurItem) && (searchText == lviCurItem->text(0))) // Item is already in the TextList
            {
              strMess = QString(i18n("<qt>The <b>%1</b> item is already present in the list.</qt>")).arg(searchText);
              KMessageBox::error(parentWidget(), strMess);
              return false;
            }

          lviCurItem = lviCurItem->nextSibling();
        } while(lviCurItem && lviCurItem != lviFirst);
    }

  // Check there is not too items to replace
  if (lwStrings->childCount() >= MaxStringToSearch)
    {
      strMess = QString(i18n("Unable to have more than %1 items to search or replace.")).arg(MaxStringToSearch);
      KMessageBox::error(parentWidget(), strMess);
      return false;
    }

  // replace string in the string list

  lviCurrent->setPixmap(0, m_pmIconString);
  lviCurrent->setText(0, searchText);
  lviCurrent->setText(1, replaceText);

  lviCurrent->setup();

  return true;
}

QListViewItem* KFileReplaceView::addFullItem(bool bSuccess, const QString &szName, const QString &szDirectory, uint nOldSize, uint nNewSize, int nNbRepl, const QString &szErrMsg)
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
  lvi = new QListViewItem(lwResult, szName, szDirectory, strOldSize);

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
int KFileReplaceView::updateItem(QListViewItem *lvi, bool bSuccess, uint nNewSize, int nNbRepl, const QString& szErrMsg)
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

bool KFileReplaceView::increaseStringCount(QListViewItem *lvi, QString strTextOld, QString strTextNew, QString strTextReplace,const char *szSearch, int nSearchLen, bool bShowDetails)
{
  // Add item to list
  QString strNum;
  QListViewItem *lviCurItem,
                *lviFirst,
                *lviNew,
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

          lviCurItem = lviCurItem->nextSibling();
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

          lviCurItem = lviCurItem->nextSibling();
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

QString KFileReplaceView::currentItem()
{
  QString strFilename;
  QListViewItem *lvi;

  if(!m_lviCurrent)
    if(!(m_lviCurrent = lwResult->currentItem()))
      return QString::null;

  lvi = m_lviCurrent;
  while (lvi->parent())
    lvi = lvi->parent();

  //strFilename = QString("%1/%2").arg(lvi->text(1)).arg(lvi->text(0));
  strFilename = QString(lvi->text(1)+"/"+lvi->text(0));
  return strFilename;
}

void KFileReplaceView::slotMouseButtonClicked (int nButton, QListViewItem *lvi, const QPoint &pos, int column)
{
  Q_UNUSED(column);
  // Don't look at events while working
  if (g_bThreadRunning)
    return;

  if (lvi == 0) // No item selected
    return;

  // RIGHT BUTTON
  if (nButton == QMouseEvent::RightButton)
    {
      m_lviCurrent = lvi;
      m_kpmResult->popup(pos);
    }
}

void KFileReplaceView::slotResultProperties()
{
  if (!currentItem().isEmpty())
  {
    KURL url(currentItem());
    (void) new KPropertiesDialog(url);
    m_lviCurrent = 0L;
  }

}

void KFileReplaceView::slotResultOpen()
{
  if (!currentItem().isEmpty())
  {
    (void) new KRun(KURL(currentItem()), 0, true, true);
    m_lviCurrent = 0L;
  }
}

void KFileReplaceView::slotResultOpenWith()
{
  if (!currentItem().isEmpty())
  {
    KURL::List kurls;
    kurls.append(KURL(currentItem()));
    KRun::displayOpenWithDialog(kurls);
    m_lviCurrent = 0L;
  }
}

void KFileReplaceView::slotResultDirOpen()
{
  if (!currentItem().isEmpty())
  {
    QFileInfo fiFile;
    fiFile.setFile(currentItem());
    (void) new KRun (KURL::fromPathOrURL(fiFile.dirPath()), 0, true, true);
    m_lviCurrent = 0L;
  }
}

void KFileReplaceView::slotResultEdit()
{
  /*if (!currentItem().isEmpty())
  {
    QString strCommand = QString("kate %1 &").arg(currentItem());
    KRun::runCommand(strCommand);
    m_lviCurrent = 0L;
  }*/

  QString filePath = currentItem();
  DCOPClient *client = kapp->dcopClient();

  DCOPRef quanta(client->appId(),"WindowManagerIf");

  bool success = quanta.send("openFile",filePath,1,1);

  if(!success)
    {
      QString message = QString(i18n("<qt>File <b>%1</b> cannot be opened. Might be a DCOP problem.</qt>")).arg(filePath);
      KMessageBox::error(parentWidget(), message);
    }
  m_lviCurrent = 0L;
}

void KFileReplaceView::slotResultDelete()
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
      //takeItem(m_lviCurrent); // Remove item from ListView
      delete m_lviCurrent;
    }

   m_lviCurrent = 0L;
  }
}

void KFileReplaceView::slotResultTreeExpand()
{
  QListViewItem *lviRoot = lwResult->firstChild();

  if (lviRoot)
    expand(lviRoot, true);
}

void KFileReplaceView::slotResultTreeReduce()
{
  QListViewItem *lviRoot = lwResult->firstChild();

  if (lviRoot)
    expand(lviRoot, false);
}

void KFileReplaceView::expand(QListViewItem *lviCurrent, bool bExpand)
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

void KFileReplaceView::slotStringsAdd()
{
   do
   {
    if (!dlg->exec()) // If Cancel
     return ;
   }
   while(!addString(0L));

}

void KFileReplaceView::slotStringsEdit(QListViewItem  *lvi)
{
  QListViewItem* lviCurItem = lvi;

  if (lviCurItem != 0)
  {

   dlg->setSearchText( lviCurItem->text(0) );
   dlg->setReplaceText( lviCurItem->text(1) );

    do
    {
     if (!dlg->exec()) // If Cancel
      return ;

    }
    while(!editString(lviCurItem));

  }
}

#include "kfilereplaceview.moc"
