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

KFileReplaceView::KFileReplaceView(RCOptions* info, QWidget *parent,const char *name):KFileReplaceViewWdg(parent,name)
{
  m_option = info;

  initGUI();

  // connect events
  connect(m_lvResults, SIGNAL(mouseButtonClicked(int, QListViewItem *, const QPoint &, int)), this, SLOT(slotMouseButtonClicked(int, QListViewItem *, const QPoint &)));
  connect(m_lvResults_2, SIGNAL(mouseButtonClicked(int, QListViewItem *, const QPoint &, int)), this, SLOT(slotMouseButtonClicked(int, QListViewItem *, const QPoint &)));
  connect(m_lvStrings, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slotStringsEdit()));
  connect(m_lvStrings_2, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slotStringsEdit()));

  whatsThis();
}

QString KFileReplaceView::currentItem()
{
  QListViewItem *lvi;

  if(! m_lviCurrent) lvi = m_rv->currentItem();
  else lvi = (QListViewItem*) m_lviCurrent;

  while (lvi->parent())
    lvi = lvi->parent();

  return QString(lvi->text(1)+"/"+lvi->text(0));
}

void KFileReplaceView::changeView(bool searchingOnlyMode)
{
  if(searchingOnlyMode)
    {
      m_stackResults->raiseWidget(m_lvResults_2);
      m_stackStrings->raiseWidget(m_lvStrings_2);
      m_rv = m_lvResults_2;
      m_sv = m_lvStrings_2;
    }
  else
    {
      m_stackResults->raiseWidget(m_lvResults);
      m_stackStrings->raiseWidget(m_lvStrings);
      m_rv = m_lvResults;
      m_sv = m_lvStrings;
    }
}

KListView* KFileReplaceView::getResultsView()
{
  if(m_option->m_searchingOnlyMode)
    m_rv = m_lvResults_2;
  else
    m_rv = m_lvResults;

  return m_rv;
}

KListView* KFileReplaceView::getStringsView()
{
  if(m_option->m_searchingOnlyMode)
    m_sv = m_lvStrings_2;
  else
    m_sv = m_lvStrings;
  return m_sv;
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
  if(! currItem.isEmpty())
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

void KFileReplaceView::slotResultsEdit()
{
  QListViewItem *lvi = m_rv->firstChild();

  while (lvi)
    {
      DCOPClient *client = kapp->dcopClient();
      DCOPRef quanta(client->appId(),"WindowManagerIf");

      QListViewItem *lviChild = lvi->firstChild();

      while(lviChild)
        {
          if(lviChild->isSelected())
            {
              coord c = extractWordCoordinates(lviChild);
              QString path = QString(lvi->text(1)+"/"+lvi->text(0));
              bool success = quanta.send("openFile", path, c.line, c.column);

              if(!success)
                {
                  QString message = i18n("File %1 cannot be opened. Might be a DCOP problem.").arg(path);
                  KMessageBox::error(parentWidget(), message);
                }
            }
          lviChild = lviChild->nextSibling();
        }

      lvi = lvi->nextSibling();
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
  QListViewItem *lviRoot = getResultsView()->firstChild();

  if(lviRoot)
    expand(lviRoot, true);
}

void KFileReplaceView::slotResultTreeReduce()
{
  QListViewItem *lviRoot = getResultsView()->firstChild();

  if(lviRoot)
    expand(lviRoot, false);
}

void KFileReplaceView::initGUI()
{
  m_option->m_searchingOnlyMode = true;

  m_stackResults->addWidget(m_lvResults);
  m_stackResults->addWidget(m_lvResults_2);
  m_stackStrings->addWidget(m_lvStrings);
  m_stackStrings->addWidget(m_lvStrings_2);

  DCOPClient *client = kapp->dcopClient();
  QCStringList appList = client->registeredApplications();
  bool quantaFound = false;


  for(QCStringList::Iterator it = appList.begin(); it != appList.end(); ++it)
    {
      if((*it).left(6) == "quanta")
        {
          quantaFound = true;
          break;
        }
    }

  m_menuResult = new KPopupMenu(this, "ResultPopup");



  m_menuResult->insertItem(SmallIconSet(QString::fromLatin1("fileopen")),
                           i18n("&Open"),
                           this,
                           SLOT(slotResultOpen()));
  if(!quantaFound)
    {
      m_menuResult->insertItem(i18n("Open &With..."),
                               this,
                               SLOT(slotResultOpenWith()));
    }

  if(quantaFound)
    {
      m_menuResult->insertItem(SmallIconSet("quanta"),
                               i18n("&Edit in Quanta"),
                               this,
                               SLOT(slotResultsEdit()));
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
  raiseResultsView();
  raiseStringsView();
}

void KFileReplaceView::raiseStringsView()
{
  if(m_option->m_searchingOnlyMode)
    m_sv = m_lvStrings_2;
  else
    m_sv = m_lvStrings;

  m_stackStrings->raiseWidget(m_sv);
}

void KFileReplaceView::raiseResultsView()
{
  if(m_option->m_searchingOnlyMode)
    m_rv = m_lvResults_2;
  else
    m_rv = m_lvResults;

  m_stackResults->raiseWidget(m_rv);
}

coord KFileReplaceView::extractWordCoordinates(QListViewItem* lvi)
{
  //get coordinates of the first string of the current selected file
  coord c;

  QString s = lvi->text(0),
          temp;
  int i = 0,
      j = s.length();

  //extracts line and column from lvi->text(0)
  //FIXME: Don't get the line and column number from the text as it's translated and it will
  //fail for non-English languages!

  //EMILIANO: This is not a good fixing but for now it should reduce the problems
  while(i < 2)
    {
      if(s[j-1] >= '0' && s[j-1] <= '9')
        temp = s[j-1] + temp;

      if(s[j-1] == ':')
        {
          if(i == 0)
            c.column = temp.toInt();
          else
            c.line = temp.toInt();

          temp = QString::null;
          i++;
        }
      j--;
    }

  if(c.line != 0) c.line--;
  if(c.column != 0) c.column--;

  return c;
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
  QListViewItem* i = m_sv->firstChild();
  while(i != 0)
  {
    if(m_option->m_searchingOnlyMode)
      map[i->text(0)] = QString::null;
    else
      map[i->text(0)] = i->text(1);
    i = i->nextSibling();
  }
  m_option->m_mapStringsView = map;
}

void KFileReplaceView::loadMapIntoView(KeyValueMap map)
{
  m_sv->clear();
  KeyValueMap::Iterator itMap;

  for(itMap = map.begin(); itMap != map.end(); ++itMap)
    {
      QListViewItem* lvi = new QListViewItem(m_sv);
      lvi->setMultiLinesEnabled(true);
      lvi->setText(0,itMap.key());
      if(!m_option->m_searchingOnlyMode)
        lvi->setText(1,itMap.data());
    }

}

void KFileReplaceView::slotStringsAdd()
{
  KeyValueMap oldMap(m_option->m_mapStringsView);

  KAddStringDlg addStringDlg(m_option, false);

  if(!addStringDlg.exec())
     return;

  KeyValueMap addedStringsMap(m_option->m_mapStringsView);
  KeyValueMap::Iterator itMap;

  for(itMap = oldMap.begin(); itMap != oldMap.end(); ++itMap)
    addedStringsMap.insert(itMap.key(),itMap.data());

  m_option->m_mapStringsView = addedStringsMap;

  raiseResultsView();
  raiseStringsView();

  loadMapIntoView(addedStringsMap);
}

void KFileReplaceView::slotQuickStringsAdd(const QString& quickSearch, const QString& quickReplace)
{
  if(!quickSearch.isEmpty())
    {
      KeyValueMap map;
      if(quickReplace.isEmpty())
        {
          map[quickSearch] = QString::null;
          m_option->m_searchingOnlyMode = true;
        }
      else
        {
          map[quickSearch] = quickReplace;
          m_option->m_searchingOnlyMode = false;
        }

      m_option->m_mapStringsView = map;

      raiseResultsView();
      raiseStringsView();

      loadMapIntoView(map);
    }
}

void KFileReplaceView::slotStringsEdit()
{
  KeyValueMap oldMap(m_option->m_mapStringsView);
  bool oldSearchFlagValue = m_option->m_searchingOnlyMode;

  oldMap.remove(m_sv->currentItem()->text(0));

  m_option->m_mapStringsView.clear();

  m_option->m_mapStringsView.insert(m_sv->currentItem()->text(0), m_sv->currentItem()->text(1));

  KAddStringDlg addStringDlg(m_option, true);

  if(!addStringDlg.exec())
    return;

  KeyValueMap newMap(m_option->m_mapStringsView);
  if(oldSearchFlagValue == m_option->m_searchingOnlyMode)
    {
      KeyValueMap::Iterator itMap;

      //merges the two maps
      for(itMap = oldMap.begin(); itMap != oldMap.end(); ++itMap)
        newMap.insert(itMap.key(),itMap.data());
    }

  m_option->m_mapStringsView = newMap;

  raiseResultsView();
  raiseStringsView();

  loadMapIntoView(newMap);
}

void KFileReplaceView::slotStringsDeleteItem()
{
  QListViewItem* item = m_sv->currentItem();
  if(item != 0)
    {
      KeyValueMap m = m_option->m_mapStringsView;
      m.remove(item->text(0));
      m_option->m_mapStringsView = m;
      delete item;
    }
}

void KFileReplaceView::slotStringsEmpty()
{
  QListViewItem * myChild = m_sv->firstChild();
  while( myChild )
    {
      QListViewItem* item = myChild;
      myChild = myChild->nextSibling();
      delete item;
    }
  KeyValueMap m;
  m_option->m_mapStringsView = m;
}

void KFileReplaceView::whatsThis()
{
  QWhatsThis::add(getResultsView(), lvResultWhatthis);
  QWhatsThis::add(getStringsView(), lvStringsWhatthis);
}
#include "kfilereplaceview.moc"
