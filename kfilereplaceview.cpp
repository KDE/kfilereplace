/***************************************************************************
                          kfilereplaceview.cpp  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by François Dupoux <dupoux@dupoux.com>
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
#include <qwhatsthis.h>
#include <qlistview.h>
#include <qmap.h>

// include files for KDE
#include <kmessagebox.h>
#include <kstandarddirs.h>
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
#include <kdebug.h>

// application specific includes
#include "kfilereplaceview.h"
#include "kfilereplacelib.h"

#ifdef APP
#include "kfilereplace.h"
#else
#include "kfilereplacepart.h"
#endif

#include "resource.h"
#include "whatthis.h"

using namespace whatthisNameSpace;

KFileReplaceView::KFileReplaceView(QWidget *parent,const char *name):KFileReplaceViewWdg(parent,name)
{
  m_path = KGlobal::instance()->dirs()->saveLocation("data", "kfilereplace/");

  //lwResult->setSorting(-1);
  // Create popup menus
  m_kpmResult = new KPopupMenu(this, "ResultPopup");

  m_kpmResult->insertItem(QPixmap("resfileopen"),
                          i18n("&Open"),
                          this,
                          SLOT(slotResultOpen()));
  m_kpmResult->insertItem(i18n("Open &With..."),
                          this,
                          SLOT(slotResultOpenWith()));
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
          SIGNAL(mouseButtonClicked(int, QListViewItem *, const QPoint &, int)),
	  this,
          SLOT(slotMouseButtonClicked(int, QListViewItem *, const QPoint &, int)));
  connect(lwStrings,
          SIGNAL(doubleClicked(QListViewItem *)),
          this,
          SLOT(slotStringsEdit(QListViewItem *)));

  QWhatsThis::add(lwResult, i18n(lwResultWhatthis));
  QWhatsThis::add(lwStrings, i18n(lwStringsWhatthis));

}

KFileReplaceView::~KFileReplaceView()
{
  kdDebug(23000) << "KFileReplaceView::~KFileReplaceView()" << endl;
  delete m_kpmResult;
  m_config = 0L;
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

QString KFileReplaceView::currentItem()
{
  QListViewItem *lvi;

  if(!m_lviCurrent)
    if(!(m_lviCurrent = lwResult->currentItem()))
      return QString::null;

  lvi = m_lviCurrent;
  while (lvi->parent())
    lvi = lvi->parent();

  return QString(lvi->text(1)+"/"+lvi->text(0));
}

void KFileReplaceView::slotMouseButtonClicked (int nButton, QListViewItem *lvi, const QPoint &pos, int column)
{
  Q_UNUSED(column);

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
  QString filePath = currentItem();
  DCOPClient *client = kapp->dcopClient();

  DCOPRef quanta(client->appId(),"WindowManagerIf");

  bool success = quanta.send("openFile", filePath, 0, 0);

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

  if(lviRoot)
    expand(lviRoot, true);
}

void KFileReplaceView::slotResultTreeReduce()
{
  QListViewItem *lviRoot = lwResult->firstChild();

  if(lviRoot)
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

void KFileReplaceView::setMap()
{
  m_map.clear();
  QListViewItem* i = lwStrings->firstChild();
  while (i != 0)
    {
      m_map[i->text(0)] = i->text(1);
      i = i->nextSibling();
    }
}

void KFileReplaceView::slotStringsAdd()
{
  dlg.setConfig(m_config);

  dlg.empty();

  if(!dlg.exec())
     return;

  m_map = dlg.stringList();

  QMap<QString,QString>::Iterator itMap;

  for(itMap = m_map.begin(); itMap != m_map.end(); ++itMap)
    {
      QListViewItem* lvi = new QListViewItem(lwStrings);
      lvi->setText(0,itMap.key());
      lvi->setText(1,itMap.data());
    }
}

void KFileReplaceView::slotStringsAddFromProjectDlg(const QMap<QString,QString>& replacementMap)
{
  m_map = replacementMap;

  QMap<QString,QString>::Iterator itMap;

  for(itMap = m_map.begin(); itMap != m_map.end(); ++itMap)
    {
      QListViewItem* lvi = new QListViewItem(lwStrings);
      lvi->setText(0,itMap.key());
      lvi->setText(1,itMap.data());
    }
}

void KFileReplaceView::slotStringsEdit(QListViewItem* lvi)
{
  Q_UNUSED(lvi);

  dlg.setConfig(m_config);

  dlg.loadDataFromStringsView(m_map);

  if(!dlg.exec())
    return;

  m_map = dlg.stringList();

  lwStrings->clear();

  QMap<QString,QString>::Iterator itMap;

  for(itMap = m_map.begin(); itMap != m_map.end(); ++itMap)
    {
      QListViewItem* lvi = new QListViewItem(lwStrings);
      lvi->setText(0,itMap.key());
      lvi->setText(1,itMap.data());
    }
}

void KFileReplaceView::slotStringsDel()
{
  delete lwStrings->currentItem();
}

void KFileReplaceView::slotStringsClear()
{
  lwStrings->clear();
}

#include "kfilereplaceview.moc"
