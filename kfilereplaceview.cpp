/***************************************************************************
                          kfilereplaceview.cpp  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by François Dupoux
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

#include <kmessagebox.h>
#include <kstandarddirs.h>

#include <stdlib.h>
#include <stdio.h>

// application specific includes
#include "kfilereplaceview.h"
#include "kfilereplacedoc.h"
#include "klistviewstring.h"

#ifdef APP
#include "kfilereplace.h"
#else
#include "kfilereplacepart.h"
#endif

#include "kaddstringdlg.h"
#include "kresultview.h"
#include "resource.h"

// ===========================================================================================================================
KFileReplaceView::KFileReplaceView(QWidget *parent,const char *name):QSplitter(Vertical, parent,name)
{
  int nRes;

  m_ResultView = new KResultView(this, "ResultView");
  m_StringView = new QListView(this, "StringView");

  // Load icons
  nRes = m_pmIconString.load( locate("data", "kfilereplace/pics/string.png"));

  // connect events
  connect(getStringView(), SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slotStringsEdit(QListViewItem *)));
}

// ===========================================================================================================================
KFileReplaceView::~KFileReplaceView()
{


}

// ===========================================================================================================================
KFileReplaceDoc* KFileReplaceView::getDocument() const
{
#ifdef APP
  KFileReplaceApp* theApp=(KFileReplaceApp*)parentWidget();
  return theApp->getDocument();
#else
  KFileReplacePart* theApp=(KFileReplacePart*)parentWidget();
  return theApp->getDocument();
#endif

}

// ===========================================================================================================================
void KFileReplaceView::print(QPrinter* m_pPrinter)
{
  QPainter printpainter;
  printpainter.begin(m_pPrinter);

  // TODO: add your printing code here

  printpainter.end();

}

// ===========================================================================================================================
void KFileReplaceView::init()
{
  // activate(m_StringsList, m_ResultList); // TopView = Strings, Bottom = Result

  QWhatsThis::add(m_ResultView, i18n("The list which shows all the results of the last operation. You can "
                                     "see all the files which were successfully found/replaced in green, and files where there was an "
                                     "error in red. You can see in the <b>Result</b> column, the number of strings found/replaced for "
                                     "each file. In blue, there is the general string which was found, and in yellow, if you are using "
                                     "wildcards or variables, you can see the real text which was found from a string with wildcards"));

  // Initialize StringsList
  m_StringView->addColumn(i18n("Search For"), -1 );
  m_StringView->setColumnWidthMode( 0, QListView::Maximum );
  m_StringView->setColumnAlignment( 0, 1 );
  m_StringView->addColumn(i18n("Replace With"), -1 );
  m_StringView->setColumnWidthMode( 1, QListView::Maximum );
  m_StringView->setColumnAlignment( 1, 1 );
  m_StringView->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  m_StringView->setLineWidth(2);
  QWhatsThis::add(m_StringView, i18n("In the strings list, the user must add all the strings which must "
                                     "be searched and replaced inside the files. If you are searching, you can add no string in this list. Then "
                                     "all files of the directory, will be shown if the filter is valid (*.htm* for example). This list is "
                                     "managed by the <b>Strings</b> menu. Before making any replace, and sometimes before the search, you need to "
                                     "add string in this list. You can use the <b>+</b> icon of the toolbar to do it. You can delete a string, "
                                     "empty the list, save the strings of the list into a file, and load them from a file. This last option is "
                                     "very useful when you makes the same search/replace operation all the time, and you need to reuse the same "
                                     "strings. You can use wildcards and variables options in the strings. Please, read help for more details."));
}

// ===========================================================================================================================
int KFileReplaceView::addString(QString strSearch, QString strReplace, QListViewItem *lviCurrent)
{
  KListViewString *lvi;
  QListViewItem *lviCurItem;
  QListViewItem *lviFirst;
  QString strMess;

  // Check item is not already in the TextList
  lviCurItem = lviFirst = getStringView() -> firstChild();
  if (lviCurItem != NULL)
    {
      do // For all strings there are in the TextList
        {
          if ((lviCurrent != lviCurItem) && (strcmp(strSearch.ascii(), lviCurItem -> text(0).ascii()) == 0)) // Item is already in the TextList
            {
              strMess.sprintf(i18n("The <b>%s</b> item is already present in the list."), strSearch.ascii());
              KMessageBox::error(parentWidget(), strMess);
              return -1;
            }

          lviCurItem = lviCurItem -> nextSibling();
        } while(lviCurItem && lviCurItem != lviFirst);
    }

  // Check there is not too items to replace
  if (getStringView() -> childCount() >= MAX_STRINGSTOSEARCHREP)
    {
      strMess.sprintf(i18n("Unable to have more than %ld items to search or replace."), MAX_STRINGSTOSEARCHREP);
      KMessageBox::error(parentWidget(), strMess);
      return -1;
    }

  // Add string to string list
  lvi = new KListViewString(getStringView());
  Q_CHECK_PTR( lvi );
  lvi -> setPixmap(0, m_pmIconString);
  lvi -> setText(0, strSearch);
  lvi -> setText(1, strReplace);
  lvi -> setup();

  return 0;
}

// ===========================================================================================================================
void KFileReplaceView::slotStringsAdd()
{
  KAddStringDlg dlg(parentWidget());
  int nRes;

  do
    {
      if (dlg.exec() == QDialog::Rejected) // If Cancel
        return ;
      nRes = addString(dlg.m_strSearch, dlg.m_strReplace, NULL);

    }
  while(nRes == -1);
}

//===========================================================================================================================
void KFileReplaceView::slotStringsEdit(QListViewItem  */*lviCurrent*/)
{
  KListViewString *lviCurItem;
  int nRes;

  lviCurItem = (KListViewString *) getStringView() -> currentItem();
  if (lviCurItem == 0)
    return;
  KAddStringDlg dlg(parentWidget());

  dlg.setSearchText( lviCurItem -> text(0) );
  dlg.setReplaceText( lviCurItem -> text(1) );

  do
    {
      if (dlg.exec() == QDialog::Rejected) // If Cancel
        return ;
      nRes = addString(dlg.m_strSearch, dlg.m_strReplace, lviCurItem);
    }
  while(nRes == -1);

  delete lviCurItem;
}


#include "kfilereplaceview.moc"

