/***************************************************************************
                          kfilereplacedoc.cpp  -  description
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
#include <qdir.h>
#include <qfileinfo.h>
#include <qwidget.h>
#include <qdatetime.h>
#include <qstring.h>
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

KFileReplaceDoc::~KFileReplaceDoc()
{
}

void KFileReplaceDoc::addView(KFileReplaceView* m_pView)
{
  viewList->append(m_pView);
}

void KFileReplaceDoc::removeView(KFileReplaceView* m_pView)
{
  viewList->remove(m_pView);
}

void KFileReplaceDoc::setModified(bool modified)
{ 
  b_modified=modified; 
}

bool KFileReplaceDoc::isModified()
{ 
  return b_modified;
}

const QString& KFileReplaceDoc::pathName() const
{
  return m_path;
}

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

void KFileReplaceDoc::setPathName( const char* path_name)
{
  m_path=path_name;
}

void KFileReplaceDoc::setTitle( const char* title)
{
  m_title=title;
}

const QString& KFileReplaceDoc::title() const
{
  return m_title;
}

void KFileReplaceDoc::closeDocument()
{
  deleteContents();
}

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
      KNewProjectDlg dlg(m_parent->widget(), m_parent->config());

      QString strAdding;

      // Add values to combo if they have been passed with the command line (see main.cpp)
      dlg.setDatas(strArguDir, strArguFilter);

      if (dlg.exec() ) // If Cancel
      {
        // Get the Directory and the Filter
      m_strProjectDirectory = dlg.location();
      m_strProjectFilter = dlg.filter();

      // criters (date & time)
      m_nTypeOfAccess = dlg.accessType();

      m_bMinDate = dlg.isMinDate();
      m_bMaxDate = dlg.isMaxDate();
      m_qdMinDate = dlg.minDate();
      m_qdMaxDate = dlg.maxDate();

      m_bMinSize = dlg.isMinSize();
      m_bMaxSize = dlg.isMaxSize();
      m_nMinSize = dlg.minSize();
      m_nMaxSize = dlg.maxSize();

      // owner
      m_bOwnerUserBool = dlg.isOwnerUser();
      m_bOwnerGroupBool = dlg.isOwnerGroup();
      m_bOwnerUserMustBe = dlg.ownerUserMustBe();
      m_bOwnerGroupMustBe = dlg.ownerGroupMustBe();
      m_strOwnerUserType = dlg.ownerUserType();
      m_strOwnerGroupType = dlg.ownerGroupType();
      m_strOwnerUserValue = dlg.ownerUserValue();
      m_strOwnerGroupValue = dlg.ownerGroupValue();
      }  
      else return false;  

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

void KFileReplaceDoc::deleteContents()
{
  /////////////////////////////////////////////////
  // TODO: Add implementation to delete the document contents
  /////////////////////////////////////////////////

}

#include "kfilereplacedoc.moc"
