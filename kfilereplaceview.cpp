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
#include <qfileinfo.h>

// KDE
#include <klistview.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kpropertiesdialog.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <dcopref.h>
//#include <kdebug.h>
#include <kiconloader.h>

// local
#include "kfilereplaceview.h"
#include "kaddstringdlg.h"
#include "whatthis.h"

using namespace whatthisNameSpace;

KFileReplaceView::KFileReplaceView(QWidget *parent,const char *name):KFileReplaceViewWdg(parent,name)
{
  // Create popup menus
  m_menuResult = new KPopupMenu(this, "ResultPopup");

  m_menuResult->insertItem(SmallIconSet(QString::fromLatin1("fileopen")),
                           i18n("&Open"),
                           this,
                           SLOT(slotResultOpen()));
  m_menuResult->insertItem(i18n("Open &With..."),
                           this,
                           SLOT(slotResultOpenWith()));

  DCOPClient *client = kapp->dcopClient();
  QCStringList appList = client->registeredApplications();
  bool quantaFound = false;

  for (QCStringList::Iterator it = appList.begin(); it != appList.end(); ++it)
    {
      if ((*it).left(6) == "quanta")
        {
          quantaFound = true;
          break;
        }
    }

  if (quantaFound)
    {
      m_menuResult->insertItem(SmallIconSet("quanta"),
                               i18n("&Edit in Quanta"),
                               this,
                               SLOT(slotResultEdit()));
    }

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
          SLOT(slotMouseButtonClicked(int, QListViewItem *, const QPoint &)));
  connect(m_lvStrings,
          SIGNAL(doubleClicked(QListViewItem *)),
          this,
          SLOT(slotStringsEdit()));
  whatsThis();
}

KFileReplaceView::~KFileReplaceView()
{
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

void KFileReplaceView::slotMouseButtonClicked (int button, QListViewItem *lvi, const QPoint &pos)
{
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
//FIXME: Don't get the line and column number from the text as it's translated and it will
//fail for non-English languages!

//EMILIANO: This is not a good fixing but for now it should reduce the problems
  QString s = lvi->text(0),
          temp;
  int i = 0,
      j = s.length();
  while(i < 2)
    {
      if(s[j-1] >= '0' && s[j-1] <= '9')
        temp = s[j-1] + temp;
      if(s[j-1] == ':')
         {
          if(i == 0)
            column = temp.toInt();
          else
            line = temp.toInt();
          temp = QString::null;
          i++;
        }
      j--;
    }

  if(line != 0) line--;
  if(column != 0) column--;

  QString filePath = currentItem();
  DCOPClient *client = kapp->dcopClient();

  DCOPRef quanta(client->appId(),"WindowManagerIf");

  bool success = quanta.send("openFile", filePath, line, column);

  if(!success)
    {
      QString message = i18n("File %1 cannot be opened. Might be a DCOP problem.").arg(filePath);
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
      int answer = KMessageBox::questionYesNo(this, i18n("Do you really want to delete %1?").arg(currItem));

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
  m_option.setMapStringsView(map);
}

void KFileReplaceView::loadMapIntoView(KeyValueMap map)
{
  m_lvStrings->clear();
  KeyValueMap::Iterator itMap;
  bool searchOnlyMode = true;
  for(itMap = map.begin(); itMap != map.end(); ++itMap)
    {
      QListViewItem* lvi = new QListViewItem(m_lvStrings);
      lvi->setMultiLinesEnabled(true);
      lvi->setText(0,itMap.key());
      lvi->setText(1,itMap.data());
      searchOnlyMode = itMap.data().isEmpty();
    }
  m_option.setSearchMode(searchOnlyMode);
  m_option.setMapStringsView(map);
}

void KFileReplaceView::slotStringsAdd()
{
  KeyValueMap oldMap(m_option.mapStringsView());

  KAddStringDlg addStringDlg;

  addStringDlg.readOptions(m_option);

  if(!addStringDlg.exec())
     return;

  m_option = addStringDlg.writeOptions();

  KeyValueMap addedStringsMap(m_option.mapStringsView());
  KeyValueMap::Iterator itMap;

  for(itMap = oldMap.begin(); itMap != oldMap.end(); ++itMap)
    addedStringsMap.insert(itMap.key(),itMap.data());

  m_option.setMapStringsView(addedStringsMap);
  loadMapIntoView(addedStringsMap);
  emit searchMode(m_option.searchMode());
}

void KFileReplaceView::slotQuickStringsAdd(const QString& quickSearch, const QString& quickReplace)
{
  if(!quickSearch.isEmpty())
    {
      KeyValueMap pair;
      pair[quickSearch] = quickReplace;
      m_option.setMapStringsView(pair);
      loadMapIntoView(pair);
    }
  emit searchMode(m_option.searchMode());
}

void KFileReplaceView::slotStringsEdit()
{
  KAddStringDlg addStringDlg;

  addStringDlg.readOptions(m_option);

  if(!addStringDlg.exec())
    return;
  m_option = addStringDlg.writeOptions();

  loadMapIntoView(m_option.mapStringsView());
  emit resetActions();
}

void KFileReplaceView::slotStringsDeleteItem()
{
  QListViewItem* item = m_lvStrings->currentItem();
  if(item != 0)
    {
      KeyValueMap m = m_option.mapStringsView();
      m.remove(item->text(0));
      m_option.setMapStringsView(m);
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
  m_option.setMapStringsView(m);
}

void KFileReplaceView::whatsThis()
{
  QWhatsThis::add(m_lvResults, lvResultWhatthis);
  QWhatsThis::add(m_lvStrings, lvStringsWhatthis);
}
#include "kfilereplaceview.moc"
