/***************************************************************************
                          kfilereplacedoc.cpp  -  description
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
#include <qdir.h>
#include <qfileinfo.h>
#include <qwidget.h>

// include files for KDE
#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>

// application specific includes
#include "kfilereplacedoc.h"
#include "kfilereplaceview.h"
#include "kfilereplacepart.h"
#include "knewprojectdlg.h"

QPtrList<KFileReplaceView>* KFileReplaceDoc::viewList = 0L;


// ===========================================================================================================================
KFileReplaceDoc::KFileReplaceDoc(QWidget *parentWidget, QObject *parent, const char* name):QObject(parentWidget, name)
{
  if( !viewList )
    viewList = new QPtrList<KFileReplaceView>;
  viewList->setAutoDelete(true);

  // Initialize project data
  m_bMinSize = false;
  m_bMaxSize = false;
  m_nMinSize = 0;
  m_nMaxSize = 0;
  m_nTypeOfAccess = 0;
  m_bMinDate = false;
  m_bMaxDate = false;
#ifdef APP
  m_parent = (KFileReplaceApp *) parent;
#else
  m_parent = (KFileReplacePart *) parent;
#endif
}

// ===========================================================================================================================
KFileReplaceDoc::~KFileReplaceDoc()
{
}

// ===========================================================================================================================
void KFileReplaceDoc::addView(KFileReplaceView* m_pView)
{
  viewList->append(m_pView);
}

// ===========================================================================================================================
void KFileReplaceDoc::removeView(KFileReplaceView* m_pView)
{
  viewList->remove(m_pView);
}

// ===========================================================================================================================
const QString& KFileReplaceDoc::getPathName() const
{
  return m_path;
}

// ===========================================================================================================================
void KFileReplaceDoc::slotUpdateAllViews(KFileReplaceView* pSender)
{
  KFileReplaceView* w;
  if(viewList)
    {
      for( w = viewList->first(); w; w = viewList->next() )
        {
          if( w != pSender)
            w->repaint();
        }
    }

}

// ===========================================================================================================================
void KFileReplaceDoc::pathName( const char* path_name)
{
  m_path=path_name;
}

// ===========================================================================================================================
void KFileReplaceDoc::title( const char* title)
{
  m_title=title;
}

// ===========================================================================================================================
const QString& KFileReplaceDoc::getTitle() const
{
  return m_title;
}


// ===========================================================================================================================
void KFileReplaceDoc::closeDocument()
{
  deleteContents();
}

// ===========================================================================================================================
bool KFileReplaceDoc::newDocument(const QString& strArguDir, const QString& strArguFilter, bool showNewProjectDlg)
{
  //kDebugInfo("doc::newDoc: argu=(%s) and filter=(%s)\n", strArguDir.data(), strArguFilter.data());

  if (!strArguDir.isEmpty() && (!strArguFilter.isEmpty() ) && !showNewProjectDlg)
    {
      m_strProjectDirectory = strArguDir;
      m_strProjectFilter = strArguFilter;
    }
  else // If a value or more is missing
    {
      KNewProjectDlg dlg(m_parent->widget(), m_parent->getConfig());

      QString strAdding;

      // Add values to combo if they have been passed with the command line (see main.cpp)
      dlg.setDatas(strArguDir, strArguFilter);

      if (dlg.exec() == QDialog::Rejected) // If Cancel
        return false;

      // Get the Directory and the Filter
      m_strProjectDirectory = dlg.getDirectory();
      m_strProjectFilter = dlg.getFilter();

      // criters (date & time)
      m_nTypeOfAccess = dlg.getTypeOfAccess();

      m_bMinDate = dlg.getBoolMinDate();
      m_bMaxDate = dlg.getBoolMaxDate();
      m_qdMinDate = dlg.getMinDate();
      m_qdMaxDate = dlg.getMaxDate();

      m_bMinSize = dlg.getBoolMinSize();
      m_bMaxSize = dlg.getBoolMaxSize();
      m_nMinSize = dlg.getMinSize();
      m_nMaxSize = dlg.getMaxSize();

      // owner
      m_bOwnerUserBool = dlg.getBoolOwnerUser();
      m_bOwnerGroupBool = dlg.getBoolOwnerGroup();
      m_bOwnerUserMustBe = dlg.getOwnerUserMustBe();
      m_bOwnerGroupMustBe = dlg.getOwnerGroupMustBe();
      m_strOwnerUserType = dlg.getOwnerUserType();
      m_strOwnerGroupType = dlg.getOwnerGroupType();
      m_strOwnerUserValue = dlg.getOwnerUserValue();
      m_strOwnerGroupValue = dlg.getOwnerGroupValue();

    }

  // Check m_strProjectDirectory is an absolute path
  QDir dir;
  dir.setPath(m_strProjectDirectory);
  m_strProjectDirectory = dir.absPath();

  b_modified=false;
  m_title = QString("[%1, %2]").arg(m_strProjectDirectory).arg(m_strProjectFilter);
  m_path=QDir::homeDirPath();
  return true;
}

// ===========================================================================================================================
void KFileReplaceDoc::deleteContents()
{
  /////////////////////////////////////////////////
  // TODO: Add implementation to delete the document contents
  /////////////////////////////////////////////////

}


































#include "kfilereplacedoc.moc"
