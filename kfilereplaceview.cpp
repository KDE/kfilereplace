/***************************************************************************
                          kfilereplaceview.cpp  -  description
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
#include <qprinter.h>
#include <qpainter.h>
#include <qwhatsthis.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>

// application specific includes
#include "kfilereplaceview.h"
#include "kfilereplacedoc.h"

#ifdef APP
#include "kfilereplace.h"
#else
#include "kfilereplacepart.h"
#endif

#include "kaddstringdlg.h"
#include "kresultview.h"
#include "resource.h"

KFileReplaceView::KFileReplaceView(QWidget *parent,const char *name):QSplitter(Vertical, parent,name)
{
  m_resultView = new KResultView(this, "ResultView");
  m_stringView = new QListView(this, "StringView");

  m_path = KGlobal::instance()->dirs()->saveLocation("data", "kfilereplace/");

  // Load icons
  int nRes;
  nRes = m_pmIconString.load( locate("data", "kfilereplace/pics/string.png"));

   // connect events
  connect(m_stringView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slotStringsEdit(QListViewItem *)));
  dlg = new KAddStringDlg(parentWidget());
}

KFileReplaceView::~KFileReplaceView()
{
 delete m_resultView;
 delete m_stringView;
 delete dlg;
}

QListView* KFileReplaceView::stringView()
{
 return m_stringView;
}

KResultView* KFileReplaceView::resultView()
{
 return m_resultView;
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

void KFileReplaceView::init()
{
  // activate(m_StringsList, m_ResultList); // TopView = Strings, Bottom = Result

  QWhatsThis::add(m_resultView, i18n("The list which shows all the results of the last operation. You can "
                                     "see all the files which were successfully found/replaced in green, and files where there was an "
                                     "error in red. You can see in the <b>Result</b> column, the number of strings found/replaced for "
                                     "each file. In blue, there is the general string which was found, and in yellow, if you are using "
                                     "wildcards or variables, you can see the real text which was found from a string with wildcards"));

  // Initialize StringsList
  m_stringView->addColumn(i18n("Search For"), -1 );
  m_stringView->setColumnWidthMode( 0, QListView::Maximum );
  m_stringView->setColumnAlignment( 0, 1 );
  m_stringView->addColumn(i18n("Replace With"), -1 );
  m_stringView->setColumnWidthMode( 1, QListView::Maximum );
  m_stringView->setColumnAlignment( 1, 1 );
  m_stringView->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  m_stringView->setLineWidth(2);
  m_stringView->setResizeMode(QListView::LastColumn);
  m_stringView->setAllColumnsShowFocus(true);
  QWhatsThis::add(m_stringView, i18n("In the strings list, the user must add all the strings which must "
                                     "be searched and replaced inside the files. If you are searching, you can add no string in this list. Then "
                                     "all files of the directory, will be shown if the filter is valid (*.htm* for example). This list is "
                                     "managed by the <b>Strings</b> menu. Before making any replace, and sometimes before the search, you need to "
                                     "add string in this list. You can use the <b>+</b> icon of the toolbar to do it. You can delete a string, "
                                     "empty the list, save the strings of the list into a file, and load them from a file. This last option is "
                                     "very useful when you makes the same search/replace operation all the time, and you need to reuse the same "
                                     "strings. You can use wildcards and variables options in the strings. Please, read help for more details."));

}

bool KFileReplaceView::addString( QListViewItem *lviCurrent)
{
  QListViewItem *lviCurItem,
                *lviFirst;
  QString searchText = dlg->searchText(),
          replaceText = dlg->replaceText();
  // Check item is not already in the TextList
  lviCurItem = lviFirst = m_stringView-> firstChild();
  if (lviCurItem != 0)
    {
      do // For all strings there are in the TextList
        {
          if ((lviCurrent != lviCurItem) && (searchText == lviCurItem->text(0))) // Item is already in the TextList
            {
              QString strMess = QString(i18n("The <b>%1</b> item is already present in the list.")).arg(searchText);
              KMessageBox::error(parentWidget(), strMess);
              return false;
            }

          lviCurItem = lviCurItem->nextSibling();
        } while(lviCurItem && lviCurItem != lviFirst);
    }

  // Check there is not too items to replace
  if (m_stringView-> childCount() >= MaxStringToSearch)
    {
      QString strMess = QString(i18n("Unable to have more than %1 items to search or replace.")).arg(MaxStringToSearch);
      KMessageBox::error(parentWidget(), strMess);
      return false;
    }

  // Add string to string list
  QListViewItem* lvi = new QListViewItem(m_stringView);
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
  lviCurItem = lviFirst = m_stringView-> firstChild();
  if (lviCurItem != 0)
    {
      do // For all strings there are in the TextList
        {
          if ((lviCurrent != lviCurItem) && (searchText == lviCurItem->text(0))) // Item is already in the TextList
            {
              strMess = QString(i18n("The <b>%1</b> item is already present in the list.")).arg(searchText);
              KMessageBox::error(parentWidget(), strMess);
              return false;
            }

          lviCurItem = lviCurItem->nextSibling();
        } while(lviCurItem && lviCurItem != lviFirst);
    }

  // Check there is not too items to replace
  if (m_stringView-> childCount() >= MaxStringToSearch)
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

