/*****************************************************************************
                          kfilereplaceview.cpp  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by Fran�is Dupoux <dupoux@dupoux.com>
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
#include <kled.h>
#include <kfiledialog.h>

// local
#include "kfilereplaceview.h"
#include "kfilereplacelib.h"
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

QString KFileReplaceView::currentPath()
{
  QListViewItem *lvi;

  if(! m_lviCurrent) lvi = m_rv->currentItem();
  else lvi = (QListViewItem*) m_lviCurrent;

  while (lvi->parent())
    lvi = lvi->parent();

  return QString(lvi->text(1)+"/"+lvi->text(0));
}

void KFileReplaceView::showSemaphore(QString s)
{
  if(s == "green")
  {
    m_ledGo->setState(KLed::On);
    m_ledWait->setState(KLed::Off);
    m_ledStop->setState(KLed::Off);
  }
  else
    if(s == "yellow")
  {
    m_ledGo->setState(KLed::Off);
    m_ledWait->setState(KLed::On);
    m_ledStop->setState(KLed::Off);
  }
  else
    if(s == "red")
  {
    m_ledGo->setState(KLed::Off);
    m_ledWait->setState(KLed::Off);
    m_ledStop->setState(KLed::On);
  }
}

void KFileReplaceView::stringsInvert(bool invertAll)
{
  QListViewItem* lviCurItem,
  * lviFirst;
  KListView* sv = getStringsView();

  if(invertAll)
    lviCurItem = lviFirst = sv->firstChild();
  else
    lviCurItem = lviFirst = sv->currentItem();

  if(lviCurItem == 0)
    return ;

  do
  {
    QString searchText = lviCurItem->text(0),
            replaceText = lviCurItem->text(1);

      // Cannot invert the string when search string is empty
    if (replaceText.isEmpty())
    {
      KMessageBox::error(0, i18n("<qt>Cannot invert string <b>%1</b>, because the search string would be empty.</qt>").arg(searchText));
      return;
    }

    lviCurItem->setText(0, replaceText);
    lviCurItem->setText(1, searchText);

    lviCurItem = lviCurItem->nextSibling();
    if(!invertAll)
      break;
  } while(lviCurItem && lviCurItem != lviFirst);
  setCurrentStringsViewMap();
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

//PUBLIC SLOTS
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
  QString currItem = currentPath();
  if(! currItem.isEmpty())
    {
      KURL url(currItem);
      (void) new KPropertiesDialog(url);
      m_lviCurrent = 0;
    }
}

void KFileReplaceView::slotResultOpen()
{
  QString currItem = currentPath();
  if(!currItem.isEmpty())
    {
      (void) new KRun(KURL(currItem), 0, true, true);
      m_lviCurrent = 0;
    }
}

void KFileReplaceView::slotResultOpenWith()
{
  QString currItem = currentPath();
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
  QString currItem = currentPath();
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
  QListViewItem *lvi = m_rv->firstChild();

  while (lvi)
    {
      DCOPClient *client = kapp->dcopClient();
      DCOPRef quanta(client->appId(),"WindowManagerIf");
      QString path = QString(lvi->text(1)+"/"+lvi->text(0));
      QListViewItem *lviChild = lvi;

      while(lviChild)
        {
          if(lviChild->isSelected())
            {
              coord c;
              if(lviChild == lvi)
                {
                  c.line = 0;
                  c.column = 0;
                }
              else
                {
                  c= extractWordCoordinates(lviChild);
                }
              bool success = quanta.send("openFile", path, c.line, c.column);

              if(!success)
                {
                  QString message = i18n("File %1 cannot be opened. Might be a DCOP problem.").arg(path);
                  KMessageBox::error(parentWidget(), message);
                }
            }
            if (lviChild == lvi)
              lviChild = lviChild->firstChild();
            else
              lviChild = lviChild->nextSibling();
        }

      lvi = lvi->nextSibling();
    }

  m_lviCurrent = 0;
}

void KFileReplaceView::slotResultDelete()
{
  QString currItem = currentPath();
  if (!currItem.isEmpty())
    {
      QFile fi;
      int answer = KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete %1?").arg(currItem),
                                                      QString::null,KStdGuiItem::del());

      if(answer == KMessageBox::Continue)
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
  if(oldMap.isEmpty()) return;
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

void KFileReplaceView::slotStringsSave()
{
  // Check there are strings in the list
  KListView* sv = getStringsView();

  if (sv->firstChild() == 0)
  {
    KMessageBox::error(0, i18n("No strings to save as the list is empty."));
    return ;
  }

  QString header("<?xml version=\"1.0\" ?>\n<kfr>"),
  footer("\n</kfr>"),
  body;
  if(m_option->m_searchingOnlyMode)
    header += "\n\t<mode search=\"true\"/>";
  else
    header += "\n\t<mode search=\"false\"/>";

  QListViewItem*  lvi = sv->firstChild();

  while( lvi )
  {
    body += QString("\n\t<replacement>"
	"\n\t\t<oldstring><![CDATA[%1]]></oldstring>"
	"\n\t\t<newstring><![CDATA[%2]]></newstring>"
	"\n\t</replacement>").arg(lvi->text(0)).arg(lvi->text(1));
    lvi = lvi->nextSibling();
  }

   // Selects the file where strings will be saved
  QString menu = "*.kfr|" + i18n("KFileReplace Strings") + " (*.kfr)\n*|" + i18n("All Files") + " (*)";
  QString fileName = KFileDialog::getSaveFileName(QString::null, menu, 0, i18n("Save Strings to File"));
  if (fileName.isEmpty())
    return;

   // Forces the extension to be "kfr" == KFileReplace extension

  fileName = KFileReplaceLib::addExtension(fileName, "kfr");

  QFile file( fileName );
  if(!file.open( IO_WriteOnly ))
  {
    KMessageBox::error(0, i18n("File %1 cannot be saved.").arg(fileName));
    return ;
  }
  QTextStream oTStream( &file );
  oTStream.setEncoding(QTextStream::UnicodeUTF8);
  oTStream << header
      << body
      << footer;
  file.close();
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

//PRIVATE
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
                               SLOT(slotResultEdit()));
    }

  m_menuResult->insertItem(SmallIconSet(QString::fromLatin1("up")),
                           i18n("Open Parent &Folder"),
                           this,
                           SLOT(slotResultDirOpen()));
  m_menuResult->insertItem(SmallIconSet(QString::fromLatin1("editdelete")),
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
  c.line = 0;
  c.column = 0;
  QString s = lvi->text(0);
  //qWarning("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW=%s",s.latin1());
 /* if(lvi->parent()) s = lvi->text(0);
  else return c;*/

  QString temp;
  int i = 0;

  //extracts line and column from lvi->text(0)
  //FIXME: Don't get the line and column number from the text as it's translated and it will
  //fail for non-English languages!

  //EMILIANO: This is not a good fixing but for now it should reduce the problems
  while(true)
    {
      if(s[i] < '0' || s[i] > '9')
	i++;
      else
	break;
    }
  while(true)
    {
      if(s[i] >= '0' && s[i] <= '9')
        {
	  temp += s[i];
	  i++;
	}
      else
	break;
    }
  c.line = temp.toInt();
  temp = QString::null;

  while(true)
    {
      if(s[i] < '0' || s[i] > '9')
	i++;
      else
	break;
    }
  while(true)
    {
      if(s[i] >= '0' && s[i] <= '9')
      {
	temp += s[i];
	i++;
      }
      else
	break;
    }
  c.column = temp.toInt();

  if(c.line > 0) c.line--;
  if(c.column > 0) c.column--;

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

void KFileReplaceView::whatsThis()
{
  QWhatsThis::add(getResultsView(), lvResultWhatthis);
  QWhatsThis::add(getStringsView(), lvStringsWhatthis);
}
#include "kfilereplaceview.moc"
