/*****************************************************************************
                          kfilereplaceview.cpp  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by François Dupoux <dupoux@dupoux.com>
                           (C) 2004 Emiliano Gulmini <emi_barbarossa@yahoo.it>
*****************************************************************************/

/*****************************************************************************
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *****************************************************************************/

// Qt
#include <qwhatsthis.h>
#include <qmap.h>
#include <qregexp.h>
#include <qfileinfo.h>

// KDE
#include <klistview.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kpropertiesdialog.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kdebug.h>
#include <kiconloader.h>

// local
#include "kfilereplaceview.h"
#include "kfilereplacelib.h"
#include "whatthis.h"

using namespace whatthisNameSpace;

KFileReplaceView::KFileReplaceView(QWidget *parent,const char *name):KFileReplaceViewWdg(parent,name)
{
  //m_path = KGlobal::instance()->dirs()->saveLocation("data", "kfilereplace/");

  // Create popup menus
  m_menuResult = new KPopupMenu(this, "ResultPopup");

  m_menuResult->insertItem(SmallIconSet(QString::fromLatin1("fileopen")),
                           i18n("&Open"),
                           this,
                           SLOT(slotResultOpen()));
  m_menuResult->insertItem(i18n("Open &With..."),
                           this,
                           SLOT(slotResultOpenWith()));
  m_menuResult->insertItem(SmallIconSet(QString::fromLatin1("edit")),
                           i18n("&Edit in Quanta"),
                           this,
                           SLOT(slotResultEdit()));
  m_menuResult->insertItem(SmallIconSet(QString::fromLatin1("up")),
                           i18n("Open Parent &Folder"),
                           this,
                           SLOT(slotResultDirOpen()));
  m_menuResult->insertItem(SmallIconSet(QString::fromLatin1("eraser")),
                           i18n("&Delete"),
                           this,
                           SLOT(slotResultDelete()));
  m_menuResult->insertSeparator();
  m_menuResult->insertItem(SmallIconSet(QString::fromLatin1("info")),
                           i18n("&Properties"),
                           this,
                           SLOT(slotResultProperties()));

  // connect events
  connect(m_lvResults,
          SIGNAL(mouseButtonClicked(int, QListViewItem *, const QPoint &, int)),
          this,
          SLOT(slotMouseButtonClicked(int, QListViewItem *, const QPoint &, int)));
  connect(m_lvStrings,
          SIGNAL(doubleClicked(QListViewItem *)),
          this,
          SLOT(slotStringsEdit(QListViewItem *)));
  whatsThis();
}

KFileReplaceView::~KFileReplaceView()
{
  m_config = 0;
}

KListView* KFileReplaceView::stringView()
{
  return m_lvStrings;
}

KListView* KFileReplaceView::resultView()
{
  return m_lvResults;
}

QString KFileReplaceView::currentItem()
{
  QListViewItem *lvi;

  if(not m_lviCurrent){
    lvi = m_lvResults->currentItem();
  }
  else lvi = (QListViewItem*) m_lviCurrent;

  while (lvi->parent())
    lvi = lvi->parent();

  return QString(lvi->text(1)+"/"+lvi->text(0));
}

void KFileReplaceView::slotMouseButtonClicked (int button, QListViewItem *lvi, const QPoint &pos, int column)
{
  Q_UNUSED(column);

  if (lvi == 0) // No item selected
    return;

  // RIGHT BUTTON
  if (button == QMouseEvent::RightButton)
    {
      m_lviCurrent = static_cast<KListViewItem*>(lvi);
      m_menuResult->popup(pos);
    }
}

void KFileReplaceView::slotResultProperties()
{
  QString currItem = currentItem();
  if(not currItem.isEmpty())
    {
      KURL url(currItem);
      (void) new KPropertiesDialog(url);
      m_lviCurrent = 0;
    }
}

void KFileReplaceView::slotResultOpen()
{
  QString currItem = currentItem();
  if(!currItem.isEmpty())
    {
      (void) new KRun(KURL(currItem), 0, true, true);
      m_lviCurrent = 0;
    }
}

void KFileReplaceView::slotResultOpenWith()
{
  QString currItem = currentItem();
  if(!currItem.isEmpty())
    {
      KURL::List kurls;
      kurls.append(KURL(currItem));
      KRun::displayOpenWithDialog(kurls);
      m_lviCurrent = 0;
    }
}

void KFileReplaceView::slotResultDirOpen()
{
  QString currItem = currentItem();
  if(!currItem.isEmpty())
    {
      QFileInfo fi;
      fi.setFile(currItem);
      (void) new KRun (KURL::fromPathOrURL(fi.dirPath()), 0, true, true);
      m_lviCurrent = 0;
    }
}

void KFileReplaceView::slotResultEdit()
{
  QListViewItem* lvi = m_lvResults->currentItem();

  int line = 1,
      column = 1;

  line = lvi->text(0).section(":",1,1).remove(", column").toInt();
  column = lvi->text(0).section(":",2,2).toInt();

  if(line != 0) line--;
  if(column != 0) column--;

  QString filePath = currentItem();
  DCOPClient *client = kapp->dcopClient();

  DCOPRef quanta(client->appId(),"WindowManagerIf");

  bool success = quanta.send("openFile", filePath, line, column);

  if(!success)
    {
      QString message = i18n("<qt>File <b>%1</b> cannot be opened. Might be a DCOP problem.</qt>").arg(filePath);
      KMessageBox::error(parentWidget(), message);
    }
  m_lviCurrent = 0;
}

void KFileReplaceView::slotResultDelete()
{
  QString currItem = currentItem();
  if (!currItem.isEmpty())
    {
      QFile fi;
      int answer = KMessageBox::questionYesNo(this, i18n("<qt>Do you really want to delete <b>%1</b>?</qt>").arg(currItem));

      if(answer == KMessageBox::Yes)
        {
          fi.setName(currItem);
          fi.remove();

          delete m_lviCurrent;
          m_lviCurrent = 0;
        }
    }
}

void KFileReplaceView::slotResultTreeExpand()
{
  QListViewItem *lviRoot = m_lvResults->firstChild();

  if(lviRoot)
    expand(lviRoot, true);
}

void KFileReplaceView::slotResultTreeReduce()
{
  QListViewItem *lviRoot = m_lvResults->firstChild();

  if(lviRoot)
    expand(lviRoot, false);
}

void KFileReplaceView::expand(QListViewItem *lviCurrent, bool b)
{
  // current item
  lviCurrent->setOpen(b);

  // recursivity
  while((lviCurrent = lviCurrent->nextSibling()) != 0)
    {
      lviCurrent->setOpen(b);

      if (lviCurrent->firstChild())
        expand(lviCurrent->firstChild(), b);
    }
}

void KFileReplaceView::setMap()
{
  KeyValueMap map;
  QListViewItem* i = m_lvStrings->firstChild();
  while (i != 0)
    {
      map[i->text(0)] = i->text(1);
      i = i->nextSibling();
    }
  m_info.setMapStringsView(map);
}

void KFileReplaceView::loadMap(KeyValueMap extMap)
{
  loadMapIntoView(extMap);
}

void KFileReplaceView::loadMapIntoView(KeyValueMap map)
{
  m_lvStrings->clear();
  KeyValueMap::Iterator itMap;
  bool searchOnly = true;
  for(itMap = map.begin(); itMap != map.end(); ++itMap)
    {
      QListViewItem* lvi = new QListViewItem(m_lvStrings);
      lvi->setMultiLinesEnabled(true);
      lvi->setText(0,itMap.key());
      lvi->setText(1,itMap.data());
      if(!itMap.data().isEmpty())
        searchOnly = false;
    }
  m_info.setMapStringsView(map);
  m_info.setSearchMode(searchOnly);
  m_config->setGroup("General Options");
  m_config->writeEntry(rcSearchMode,searchOnly);
  m_config->sync();
}

void KFileReplaceView::slotStringsAdd()
{
  KeyValueMap oldMap(m_info.mapStringsView());
  m_addStringdlg.setConfig(m_config);
  m_addStringdlg.clearView();

  if(!m_addStringdlg.exec())
     return;

  KeyValueMap addedStringsMap(m_addStringdlg.stringsMap());
  KeyValueMap::Iterator itMap;

  for(itMap = oldMap.begin(); itMap != oldMap.end(); ++itMap)
    addedStringsMap.insert(itMap.key(),itMap.data());

  m_info.setMapStringsView(addedStringsMap);
  loadMapIntoView(addedStringsMap);
}

void KFileReplaceView::slotQuickStringsAdd(const QString& quickSearch, const QString& quickReplace)
{
  if(!quickSearch.isEmpty())
    {
      KeyValueMap pair;
      pair[quickSearch] = quickReplace;
      m_info.setMapStringsView(pair);
      loadMapIntoView(pair);
    }
}

void KFileReplaceView::slotStringsEdit(QListViewItem* lvi)
{
  Q_UNUSED(lvi);

  m_addStringdlg.setConfig(m_config);
  m_addStringdlg.loadViewContent(m_info.mapStringsView());

  if(!m_addStringdlg.exec())
    return;
  KeyValueMap map = m_addStringdlg.stringsMap();
  m_info.setMapStringsView(map);

  loadMapIntoView(map);
}

void KFileReplaceView::slotStringsDeleteItem()
{
  QListViewItem* item = m_lvStrings->currentItem();
  if(item != 0)
    {
      KeyValueMap m = m_info.mapStringsView();
      m.remove(item->text(0));
      m_info.setMapStringsView(m);
      delete item;
    }
}

void KFileReplaceView::slotStringsEmpty()
{
  QListViewItem * myChild = m_lvStrings->firstChild();
  while( myChild )
    {
      QListViewItem* temp = myChild;
      myChild = myChild->nextSibling();
      delete temp;
    }
  KeyValueMap m;
  m_info.setMapStringsView(m);
}

void KFileReplaceView::whatsThis()
{
  QWhatsThis::add(m_lvResults, lvResultWhatthis);
  QWhatsThis::add(m_lvStrings, lvStringsWhatthis);
}
#include "kfilereplaceview.moc"
