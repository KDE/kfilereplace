//
//
// C++ Implementation: kfilereplacepart
//
// Description:
//
//
// Author: Andras Mantia <amantia@kde.org>, (C) 2003
// Maintainer: Emiliano Gulmini <emi_barbarossa@yahoo.it>, (C) 2004
//
// Copyright: GPL v2. See COPYING file that comes with this distribution
//
//

#undef APP

//qt includes
#include <qdir.h>
#include <qdatastream.h>
#include <qlistview.h>
#include <qregexp.h>

//kde includes
#include <kapplication.h>
#include <kaction.h>
#include <kbugreport.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kparts/genericfactory.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kaboutapplication.h>
#include <kcombobox.h>

//own includes
#include "kfilereplacelib.h"
#include "resource.h"
#include "kfilereplacepart.h"
#include "kfilereplaceview.h"
#include "koptionsdlg.h"
#include "knewprojectdlg.h"

#include "whatthis.h"

using namespace whatthisNameSpace;

// Factory code for KDE 3
typedef KParts::GenericFactory<KFileReplacePart> FileReplaceFactory;
K_EXPORT_COMPONENT_FACTORY( libkfilereplacepart, FileReplaceFactory )

KFileReplacePart::KFileReplacePart(QWidget* parentWidget, const char* , QObject* parent, const char* name, const QStringList & )
  : KParts::ReadOnlyPart(parent,name)
{
  setInstance(FileReplaceFactory::instance());

  m_parentWidget = parentWidget;
  QString configName = locateLocal("config", "kfilereplacerc");

  m_config = new KConfig(configName);
  m_dlgAbout = 0L;
  m_stop = false;
  m_optionMask = QDir::Files;

  initView();
  initGUI();
  setWhatsThis();
}

KFileReplacePart::~KFileReplacePart()
{
  m_view = 0L; //it's already deleted, so set it to NULL
  saveOptions();
  slotFileStop();

  if(m_dlgAbout != 0)
    delete m_dlgAbout;

  delete m_config;
}

void KFileReplacePart::launchNewProjectDialog(const KURL & startURL)
{
  KNewProjectDlg dlg(0L, m_config);
  if (!startURL.isEmpty())
    dlg.cbLocation->setCurrentText(startURL.path());
  if(!dlg.exec())
    return;

  m_info.setDirectory(dlg.currentDir());
  m_info.setFilter(dlg.currentFilter());
  slotStringsAddFromProjectDlg(dlg.stringsMap());
  readOptions();
  resetActions();

}

void KFileReplacePart::setOptionMask()
{
  /*if(m_info.recursive())*/ m_optionMask |= QDir::Dirs;
  if(!m_info.ignoreHidden()) m_optionMask |= QDir::Hidden;
  if(!m_info.followSymLinks()) m_optionMask |= QDir::NoSymLinks;
}

void KFileReplacePart::normalFileReplace()
{
  QString dirName = m_info.directory();
  QDir d(dirName);

  d.setFilter(m_optionMask);

  QStringList filesList = d.entryList(m_info.filter());
  QStringList::iterator filesIt;

  for (filesIt = filesList.begin(); filesIt != filesList.end() ; ++filesIt)
    { // We pushed stop button
      if(m_stop)
        return;

      QString fileName = (*filesIt);
      // Avoid files that not match requirements
      if(!verifyFileRequirements(d.canonicalPath(),fileName))
	continue;
      bool wildcards = m_info.wildcards();
      if(m_info.backup())
        {
	  if(wildcards)
	    replaceAndBackupExpression(d.canonicalPath(),fileName);
	  else
	    replaceAndBackupLiteral(d.canonicalPath(),fileName);
	}
      else
        {
	  if(wildcards)
	    replaceAndOverwriteExpression(d.canonicalPath(),fileName);
	  else
	    replaceAndOverwriteLiteral(d.canonicalPath(),fileName);
	}
    }
}

void KFileReplacePart::recursiveFileReplace(const QString& dirName)
{
  // avoid recursion if we want to stop
  if(m_stop)
    return;
  else
    {
      QDir d(dirName);

      d.setMatchAllDirs(true);
      d.setFilter(m_optionMask);

      QStringList filesList = d.entryList(m_info.filter());
      QStringList::iterator filesIt;

      for (filesIt = filesList.begin(); filesIt != filesList.end(); ++filesIt)
        {
          QString fileName = (*filesIt);
          // Avoid files that not match requirements
          if(!verifyFileRequirements(d.canonicalPath(),fileName))
	    continue;

          QString filePath = d.canonicalPath()+"/"+fileName;

          QFileInfo qi(filePath);
          //if filePath is a directory then recursion
          if(qi.isDir())
            recursiveFileReplace(filePath);
          else
            {
              bool wildcards = m_info.wildcards();
              if(m_info.backup())
                {
	          if(wildcards)
	            replaceAndBackupExpression(d.canonicalPath(),fileName);
	          else
	            replaceAndBackupLiteral(d.canonicalPath(),fileName);
	        }
              else
                {
	          if(wildcards)
	            replaceAndOverwriteExpression(d.canonicalPath(),fileName);
	          else
	            replaceAndOverwriteLiteral(d.canonicalPath(),fileName);
	        }
            }
          // avoid unnecessary for-loop if we want to stop
          if(m_stop)
            break;
        }
    }

}

void KFileReplacePart::replaceAndBackupExpression(const QString& currentDir, const QString& oldFileName)
{
  QWidget* w = new QWidget();

  QFile oldFile(currentDir+"/"+oldFileName);
  if (!oldFile.open(IO_ReadOnly))
    {
      KMessageBox::error(w,i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(oldFileName));
      return ;
    }

  QString backupExtension = m_info.backupExtension();

  QFile newFile(currentDir+"/"+oldFileName+backupExtension);
  if (!newFile.open(IO_WriteOnly))
    {
      KMessageBox::error(w,i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(oldFileName+backupExtension));
      return ;
    }

  QTextStream oldStream( &oldFile ),
              newStream( &newFile );
  QString line;
  int lineNumber = 1,
      occur = 0;

  QFileInfo fi(currentDir+"/"+oldFileName);
  QString fileSizeBeforeReplacing =  KFileReplaceLib::formatFileSize(fi.size());

  kapp->processEvents();

  QListViewItem *item = new QListViewItem(m_view->resultView());
  QMap<QString,QString> tempMap = m_replacementMap;
  bool caseSensitive = m_info.caseSensitive();
  while ( !oldStream.atEnd() )
    {
      line = oldStream.readLine()+"\n";
      QMap<QString,QString>::Iterator it;
      QMap<QString,QString> tempMap = m_replacementMap;
       kdDebug(23000)<<"\n\n\nr1\n";
      for ( it = tempMap.begin(); it != tempMap.end(); ++it )
        { kdDebug(23000)<<"\n\n\nr2\n";
	  QString key = it.key(),
	          data = it.data();
          QRegExp rxKey("("+key+")", caseSensitive, false);
          int pos = rxKey.search(line, 0);
	  kdDebug(23000)<<"\n\n\nr3\n";
          while(pos != -1)
            { kdDebug(23000)<<"\n\n\nr4\n";
              QString msg = i18n(" captured text \"%1\" replaced with \"%2\" at line:%3, column:%4").arg(rxKey.cap( 1 )).arg(data).arg(lineNumber).arg(pos+1);
              QListViewItem* tempItem = new QListViewItem(item);
	      tempItem->setText(0,msg);
	      occur++;
              pos = rxKey.search(line, pos+rxKey.matchedLength());
            } kdDebug(23000)<<"\n\n\nr5\n";
          line.replace(rxKey,data);
        }
      newStream << line;
      lineNumber++;
     }

   newFile.close();
   oldFile.close();

   item->setText(0,oldFileName);
   item->setText(1,currentDir);
   item->setText(2,fileSizeBeforeReplacing);
   QFileInfo nf(currentDir+"/"+oldFileName+backupExtension);
   item->setText(3,KFileReplaceLib::formatFileSize(nf.size()));

   item->setText(4,QString::number(occur,10));
   item->setText(5,i18n("%1[%2]").arg(fi.owner()).arg(QString::number(fi.ownerId(),10)));
   item->setText(6,i18n("%1[%2]").arg(fi.group()).arg(QString::number(fi.groupId(),10)));

}

void KFileReplacePart::replaceAndOverwriteExpression(const QString& currentDir, const QString& oldFileName, bool simulation)
{
  QWidget* w = new QWidget();

  QFile oldFile(currentDir+"/"+oldFileName);
  if (!oldFile.open(IO_ReadOnly))
    {
      KMessageBox::error(w,i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(oldFileName));
      return ;
    }

  QFileInfo fi(currentDir+"/"+oldFileName);
  if(!fi.isWritable())
    {
      KMessageBox::error(w,i18n("<qt>Cannot overwrite file <b>%1</b>.</qt>").arg(oldFileName));
      return ;
    }

  QString fileSizeBeforeReplacing =  KFileReplaceLib::formatFileSize(fi.size());

  QString newFileBuffer;
  QTextStream oldStream(&oldFile);
  QString line;
  int lineNumber = 1,
      occur = 0;

  kapp->processEvents();

  QListViewItem *item = new QListViewItem(m_view->resultView());
  bool caseSensitive = m_info.caseSensitive();
  while(!oldStream.atEnd())
    {
      line = oldStream.readLine()+"\n";
      QMap<QString,QString>::Iterator it;
      QMap<QString,QString> tempMap = m_replacementMap;
       kdDebug(23000)<<"\n\n\nr1\n";
      for ( it = tempMap.begin(); it != tempMap.end(); ++it )
        { kdDebug(23000)<<"\n\n\nr2\n";
	  QString key = it.key(),
	          data = it.data();
          QRegExp rxKey("("+key+")", caseSensitive, false);
          int pos = rxKey.search(line, 0);
	  kdDebug(23000)<<"\n\n\nr3\n";
          while(pos != -1)
            { kdDebug(23000)<<"\n\n\nr4\n";
              QString msg = i18n(" captured text \"%1\" replaced with \"%2\" at line:%3, column:%4").arg(rxKey.cap( 1 )).arg(data).arg(lineNumber).arg(pos+1);
              QListViewItem* tempItem = new QListViewItem(item);
	      tempItem->setText(0,msg);
	      occur++;
              pos = rxKey.search(line, pos+rxKey.matchedLength());
            } kdDebug(23000)<<"\n\n\nr5\n";
          line.replace(rxKey,data);
        }
      if(!simulation)
        newFileBuffer += line;
      lineNumber++;
     }

   oldFile.close();
   if(!simulation)
     {
       QFile newFile(currentDir+"/"+oldFileName);
       if(!newFile.open(IO_WriteOnly))
         {
           KMessageBox::error(w,i18n("<qt>Cannot overwrite file <b>%1</b>.</qt>").arg(oldFileName));
           return ;
         }
       QTextStream newStream( &newFile );

       newStream << newFileBuffer;

       newFile.close();
     }

   QFileInfo nf(currentDir+"/"+oldFileName);
   QString fileSizeAfterReplacing = KFileReplaceLib::formatFileSize(nf.size());

   item->setText(0,oldFileName);
   item->setText(1,currentDir);
   item->setText(2,fileSizeBeforeReplacing);

   item->setText(3,fileSizeAfterReplacing);

   item->setText(4,QString::number(occur,10));
   item->setText(5,i18n("%1[%2]").arg(fi.owner()).arg(QString::number(fi.ownerId(),10)));
   item->setText(6,i18n("%1[%2]").arg(fi.group()).arg(QString::number(fi.groupId(),10)));

}

void KFileReplacePart::replaceAndBackupLiteral(const QString& currentDir, const QString& oldFileName)
{
  QWidget* w = new QWidget();

  QFile oldFile(currentDir+"/"+oldFileName);
  if (!oldFile.open(IO_ReadOnly))
    {
      KMessageBox::error(w,i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(oldFileName));
      return ;
    }

  QString backupExtension = m_info.backupExtension();
  bool caseSensitive = m_info.caseSensitive();

  QFile newFile(currentDir+"/"+oldFileName+backupExtension);
  if (!newFile.open(IO_WriteOnly))
    {
      KMessageBox::error(w,i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(oldFileName+backupExtension));
      return ;
    }

  QTextStream oldStream( &oldFile ),
              newStream( &newFile );
  QString line;
  int lineNumber = 1,
      occur = 0;

  QFileInfo fi(currentDir+"/"+oldFileName);
  QString fileSizeBeforeReplacing =  KFileReplaceLib::formatFileSize(fi.size());

  kapp->processEvents();

  QListViewItem *item = new QListViewItem(m_view->resultView());

  while (!oldStream.atEnd())
    {
      line = oldStream.readLine()+"\n";
      QMap<QString,QString>::Iterator it;
      QMap<QString,QString> tempMap = m_replacementMap;
      for ( it = tempMap.begin(); it != tempMap.end(); ++it )
        {
	  QString key = it.key(),
	          data = it.data();
          QString rxKey(key);
          QStringList l;
          int pos = line.find(rxKey, 0, caseSensitive);
          while (pos != -1)
            {
	      QString msg = i18n(" string \"%1\" replaced with \"%2\" at line:%3, column:%4 ").arg(key).arg(data).arg(lineNumber).arg(pos+1);
              QListViewItem* tempItem = new QListViewItem(item);
	      tempItem->setText(0,msg);
	      occur++;
	      pos = line.find(rxKey, pos+rxKey.length(), caseSensitive);
            }
          line.replace(rxKey,data);
        }
      newStream << line;
      lineNumber++;
     }

   newFile.close();
   oldFile.close();

   item->setText(0,oldFileName);
   item->setText(1,currentDir);
   item->setText(2,fileSizeBeforeReplacing);
   QFileInfo nf(currentDir+"/"+oldFileName+backupExtension);
   item->setText(3,KFileReplaceLib::formatFileSize(nf.size()));

   item->setText(4,QString::number(occur,10));
   item->setText(5,i18n("%1[%2]").arg(fi.owner()).arg(QString::number(fi.ownerId(),10)));
   item->setText(6,i18n("%1[%2]").arg(fi.group()).arg(QString::number(fi.groupId(),10)));

}

void KFileReplacePart::replaceAndOverwriteLiteral(const QString& currentDir, const QString& oldFileName, bool simulation)
{
  QWidget* w = new QWidget();

  QFile oldFile(currentDir+"/"+oldFileName);
  if (!oldFile.open(IO_ReadOnly))
    {
      KMessageBox::error(w,i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(oldFileName));
      return ;
    }

  QFileInfo fi(currentDir+"/"+oldFileName);
  if(!fi.isWritable())
    {
      KMessageBox::error(w,i18n("<qt>Cannot overwrite file <b>%1</b>.</qt>").arg(oldFileName));
      return ;
    }

  QString fileSizeBeforeReplacing =  KFileReplaceLib::formatFileSize(fi.size());

  QString newFileBuffer;
  QTextStream oldStream( &oldFile );
  QString line;
  int lineNumber = 1,
      occur = 0;

  kapp->processEvents();

  QListViewItem *item = new QListViewItem(m_view->resultView());
  bool caseSensitive = m_info.caseSensitive();
  while ( !oldStream.atEnd() )
    {
      line = oldStream.readLine()+"\n";
      QMap<QString,QString>::Iterator it;
      QMap<QString,QString> tempMap = m_replacementMap;
      for ( it = tempMap.begin(); it != tempMap.end(); ++it )
        {
	  QString key = it.key(),
	          data = it.data();
          QString rxKey(key);
          int pos = line.find(rxKey, 0, caseSensitive);
          while ( pos != -1 )
            {
	      QString msg = i18n(" string \"%1\" replaced with \"%2\" at line:%3, column:%4").arg(key).arg(data).arg(lineNumber).arg(pos+1);
              QListViewItem* tempItem = new QListViewItem(item);
	      tempItem->setText(0,msg);
	      occur++;
              pos = line.find(rxKey, pos+rxKey.length(), caseSensitive);
            }
          line.replace(rxKey,data);
        }
      if(!simulation)
        newFileBuffer += line;
      lineNumber++;
     }

   oldFile.close();
   if(!simulation)
     {
       QFile newFile(currentDir+"/"+oldFileName);
       if(!newFile.open(IO_WriteOnly))
         {
           KMessageBox::error(w,i18n("<qt>Cannot overwrite file <b>%1</b>.</qt>").arg(oldFileName));
           return ;
         }
       QTextStream newStream( &newFile );

       newStream << newFileBuffer;

       newFile.close();
     }

   QFileInfo nf(currentDir+"/"+oldFileName);
   QString fileSizeAfterReplacing = KFileReplaceLib::formatFileSize(nf.size());

   item->setText(0,oldFileName);
   item->setText(1,currentDir);
   item->setText(2,fileSizeBeforeReplacing);

   item->setText(3,fileSizeAfterReplacing);

   item->setText(4,QString::number(occur,10));
   item->setText(5,i18n("%1[%2]").arg(fi.owner()).arg(QString::number(fi.ownerId(),10)));
   item->setText(6,i18n("%1[%2]").arg(fi.group()).arg(QString::number(fi.groupId(),10)));
}

void KFileReplacePart::normalFileSearch(const QString& dirName, const QString& filters)
{
  QDir d(dirName);
  d.setMatchAllDirs(true);
  d.setFilter(m_optionMask);

  QStringList filesList = d.entryList(filters);
  QString filePath = d.canonicalPath();
  QStringList::iterator filesIt;

  for (filesIt = filesList.begin(); filesIt != filesList.end() ; ++filesIt)
    { // We pushed stop button
      if(m_stop)
        return;

      QString fileName = (*filesIt);

      // Avoid files that not match
      if(!verifyFileRequirements(filePath,fileName))
	continue;
      QFileInfo fi(filePath+"/"+fileName);
      if(fi.isDir())
        continue;
      if(m_info.wildcards())
        searchExpression(filePath,fileName);
      else
        searchLiteral(filePath,fileName);
    }
}

void KFileReplacePart::recursiveFileSearch(const QString& dirName, const QString& filters)
{
  // avoid recursion if we want to stop
  if(m_stop)
    return;
  else
    {
      QDir d(dirName);

      d.setMatchAllDirs(true);
      d.setFilter(m_optionMask);

      QStringList filesList = d.entryList(filters);
      QString filePath = d.canonicalPath();
      QStringList::iterator filesIt;

      for (filesIt = filesList.begin(); filesIt != filesList.end(); ++filesIt)
        {
	  QString fileName = (*filesIt);
	  // Avoid files that not match
          if(!verifyFileRequirements(filePath,fileName))
	    continue;

	  // Compose file path string
          QFileInfo fi(filePath+"/"+fileName);
          // Recursive search if "filePath" is a directory
          if(fi.isDir())
            recursiveFileSearch(filePath+"/"+fileName,filters);
          else
            {
	      if(m_info.wildcards())
	        searchExpression(filePath,fileName);
	      else
	        searchLiteral(filePath,fileName);
	    }
              // avoid unnecessary for-loop if we want to stop
              if(m_stop)
                break;
            }
   }
}

void KFileReplacePart::searchExpression(const QString& currentDir, const QString& fileName)
{
 // kdDebug(23000)<<"searchExpression" << end;;
  QWidget* w = new QWidget();

  QFile file(currentDir+"/"+fileName);

  if(!file.open(IO_ReadOnly))
    {
      KMessageBox::error(w,i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(fileName));
      return ;
    }
  // Create a stream with the file
  QTextStream stream( &file );
  QFileInfo fi(currentDir+"/"+fileName);

  kapp->processEvents();

  QListViewItem *item = 0L;
  bool caseSensitive = m_info.caseSensitive(),
       haltOnFirstOccur = m_info.haltOnFirstOccur();
  //Count the line number
  int lineNumber = 1;
  //Count occurrences
  int occur = 0;

  //This map contains search strings
  QMap<QString,QString> tempMap = m_replacementMap;
  while (!stream.atEnd())
    {
      QString line = stream.readLine()+"\n";
      QMap<QString,QString>::Iterator it = tempMap.begin();

      while(it != tempMap.end())
        {
	  QString key = it.key(),
	          data = it.data();
          //QString tmp = line;
          QRegExp rxKey("("+key+")", caseSensitive, false);
          //If this option is true then for any string in
	  //the map we search for the first occurrence of that string
          if(haltOnFirstOccur)
            {
	      int pos = line.find(rxKey);
              if(pos != -1)
                {
                  QString msg = i18n(" first captured text \"%1\" at line:%2, column:%3").arg(rxKey.cap(1)).arg(lineNumber).arg(pos+1);
                  if (!item)
                    item = new QListViewItem(m_view->resultView());
                  QListViewItem* tempItem = new QListViewItem(item);
		  tempItem->setText(0,msg);
		  tempMap.remove(it);
                  //if map is empty and if we must stop on first occurrence
		  //then we exit
                  if(tempMap.isEmpty())
                    return;
                  it = tempMap.begin();
                  continue;
                }

            }
          //This point of the code is reached when we must search for all
	  //occurrences of all the strings
	  kdDebug(23000)<<"1\n\n";
          int pos = rxKey.search(line, 0);
          while(pos != -1)
	    {
              QString msg = i18n(" captured text \"%1\" at line:%2, column:%3").arg(rxKey.cap(1)).arg(lineNumber).arg(pos+1);
              if (!item)
                 item = new QListViewItem(m_view->resultView());
	      QListViewItem* tempItem = new QListViewItem(item);
              tempItem->setText(0,msg);
	      occur++;
              pos = rxKey.search(line, pos+rxKey.matchedLength());
            } kdDebug(23000)<<"3 "<<key<<"\n\n";
	  //Advance in the map of the strings
          ++it;
         }
      //Count line number
      lineNumber++;
     }

   file.close();

   if (item)
   {
      item->setText(0,fileName);
      item->setText(1,currentDir);
      item->setText(2,KFileReplaceLib::formatFileSize(fi.size()));
      item->setText(4,QString::number(occur,10));
      item->setText(5,i18n("%1[%2]").arg(fi.owner()).arg(fi.ownerId()));
      item->setText(6,i18n("%1[%2]").arg(fi.group()).arg(fi.groupId()));
   }

}

void KFileReplacePart::searchLiteral(const QString& currentDir, const QString& fileName)
{
 // kdDebug(23000)<< "searchLiteral" << endl;
  QWidget* w = new QWidget();

  QFile file;

  file.setName(currentDir+"/"+fileName);

  if(!file.open(IO_ReadOnly))
    {
      KMessageBox::error(w,i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(fileName));
      return ;
    }
  // Create a stream with the file
  QTextStream stream( &file );

  QFileInfo fi(currentDir+"/"+fileName);

  kapp->processEvents();

  QListViewItem *item = 0L;

  bool caseSensitive = m_info.caseSensitive(),
       haltOnFirstOccur = m_info.haltOnFirstOccur();
  //Count the line number
  int lineNumber = 1;
  //Count occurrences
  int occur = 0;
  //This map contains search strings
  QMap<QString,QString> tempMap = m_replacementMap;

  while ( !stream.atEnd() )
    {
      QString line = stream.readLine()+"\n";
      QMap<QString,QString>::Iterator it = tempMap.begin();

      while(it != tempMap.end())
        {
	  QString key = it.key(),
	          data = it.data();
          //QString tmp = line;
          QString rxKey(key);

          //If this option is true then for any string in
	  //the map we search for the first occurrence of that string
          if(haltOnFirstOccur)
            {
	      int pos = line.find(rxKey, 0, caseSensitive);
              if(pos != -1)
                {
                  if (!item)
                     item = new QListViewItem(m_view->resultView());
                  QListViewItem* tempItem = new QListViewItem(item);
		  QString capturedText = line.mid(pos,rxKey.length()),
		          msg = i18n(" first occurence of string \"%1\" found at line:%2, column:%3").arg(capturedText).arg(lineNumber).arg(pos+1);
                  tempItem->setText(0,msg);
		  tempMap.remove(it);
                  //if map is empty and if we must stop on first occurrence
		  //then exit
                  if(tempMap.isEmpty())
                    return;
                  it = tempMap.begin();
                  continue;
                }
            }
          //This point of the code is reached when we must search for all
	  //the occurrences of all the strings
          int pos = line.find(rxKey, 0, caseSensitive);

          while(pos != -1)
	    {
              QString capturedText = line.mid(pos,rxKey.length()),
	              msg = i18n(" string \"%1\" found at line:%2, column:%3 ").arg(capturedText).arg(lineNumber).arg(pos+1);
              if (!item)
                  item = new QListViewItem(m_view->resultView());
	      QListViewItem* tempItem = new QListViewItem(item);
              tempItem->setText(0,msg);
	      occur++;
              pos = line.find(rxKey,pos+rxKey.length()+1);
            }
	  //Advance in the map of the strings
	  ++it;
	}
      //Count line number
      lineNumber++;
    }
  file.close();

  if (item)
  {
      item->setText(0,fileName);
      item->setText(1,currentDir);
      item->setText(2,KFileReplaceLib::formatFileSize(fi.size()));
      item->setText(4,QString::number(occur,10));
      item->setText(5,i18n("%1[%2]").arg(fi.owner()).arg(fi.ownerId()));
      item->setText(6,i18n("%1[%2]").arg(fi.group()).arg(fi.groupId()));
  }
}

KAboutData* KFileReplacePart::createAboutData()
{
  KAboutData * aboutData = new KAboutData( "kfilereplacepart",
                                              I18N_NOOP("KFileReplacePart"),
                                              KFR_VERSION,
                                              I18N_NOOP( "Batch search and replace tool."),
                                              KAboutData::License_GPL_V2,
                                              "(C) 1999-2002 Francois Dupoux\n(C) 2003-2004 Andras Mantia\n(C) 2004 Emiliano Gulmini", I18N_NOOP("Part of the KDEWebDev module."), "http://www.kdewebdev.org");
  aboutData->addAuthor("Francois Dupou",
                          I18N_NOOP("Original author of the KFileReplace tool"),
                          "dupoux@dupoux.com");
  aboutData->addAuthor("Emiliano Gulmini", I18N_NOOP("Current maintainer, code cleaner and rewriter"),"emi_barbarossa@yahoo.it");
  aboutData->addAuthor("Andras Mantia",
                          I18N_NOOP("Co-maintainer, KPart creator"),
                          "amantia@kde.org");
  aboutData->addCredit("Heiko Goller", I18N_NOOP("Original german translator"), "heiko.goller@tuebingen.mpg.de");
  return aboutData;
}

bool KFileReplacePart::openURL(const KURL &url)
{
  if (url.protocol() != "file")
  {
    KMessageBox::sorry(0, i18n("Sorry, currently the KFileReplace part works only for local files."), i18n("Non Local File"));
    emit canceled("");
    return false;
  }
  launchNewProjectDialog(url);
  return true;
}

void KFileReplacePart::initGUI()
{
   setXMLFile("kfilereplacepartui.rc");

   // File
   (void)new KAction(i18n("New Search Project..."), "newproject", 0, this, SLOT(slotFileNew()), actionCollection(), "new_project");
   (void)new KAction(i18n("&Search"), "find", 0, this, SLOT(slotFileSearch()), actionCollection(), "search");
   (void)new KAction(i18n("S&imulate"), "filesimulate", 0, this, SLOT(slotFileSimulate()), actionCollection(), "file_simulate");
   (void)new KAction(i18n("&Replace"), "filereplace", 0, this, SLOT(slotFileReplace()), actionCollection(), "replace");
   (void)new KAction(i18n("Sto&p"), "stop", 0, this, SLOT(slotFileStop()), actionCollection(), "stop");
   (void)new KAction(i18n("S&ave Results As..."), "filesave", 0, this, SLOT(slotFileSave()), actionCollection(), "save_results");

   // Strings
   (void)new KAction(i18n("&Add String..."), "edit_add", 0, this, SLOT(slotStringsAdd()), actionCollection(), "strings_add");
   (void)new KAction(i18n("&Delete String"), "edit_remove", 0, this, SLOT(slotStringsDel()), actionCollection(), "strings_del");
   (void)new KAction(i18n("&Empty Strings List"), "editdelete", 0, this, SLOT(slotStringsEmpty()), actionCollection(), "strings_empty");
   (void)new KAction(i18n("Edit Selected String..."), "editclear", 0, this, SLOT(slotStringsEdit()), actionCollection(), "strings_edit");
   (void)new KAction(i18n("&Save Strings List to File..."), "filesave", 0, this, SLOT(slotStringsSave()), actionCollection(), "strings_save");
   (void)new KAction(i18n("&Load Strings List From File..."), "unsortedList", 0, this, SLOT(slotStringsLoad()), actionCollection(), "strings_load");
   (void)new KRecentFilesAction(i18n("&Load Recent Strings Files..."), "fileopen", 0, this, SLOT(slotOpenRecentStringFile(const KURL&)), actionCollection(),"strings_load_recent");
   (void)new KAction(i18n("&Invert Current String (search <--> replace)"), "invert", 0, this, SLOT(slotStringsInvertCur()), actionCollection(), "strings_invert");
   (void)new KAction(i18n("&Invert All Strings (search <--> replace)"), "invert", 0, this, SLOT(slotStringsInvertAll()), actionCollection(), "strings_invert_all");

   // Options
   (void)new KToggleAction(i18n("&Include Sub-Folders"), "recursive", 0, this, SLOT(slotOptionsRecursive()), actionCollection(), "options_recursive");
   (void)new KToggleAction(i18n("Create &Backup"), "optbackup", 0, this, SLOT(slotOptionsBackup()), actionCollection(), "options_backup");
   (void)new KToggleAction(i18n("Case &Sensitive"), "casesensitive", 0, this, SLOT(slotOptionsCaseSensitive()), actionCollection(), "options_case");
   (void)new KToggleAction(i18n("Enable &Variables in Replace String: [$command:option$]"), "optvar", 0, this, SLOT(slotOptionsVariables()), actionCollection(), "options_var");
   (void)new KToggleAction(i18n("Enable &Wildcards"), "optwildcards", 0, this, SLOT(slotOptionsWildcards()), actionCollection(), "options_wildcards");
   (void) new KAction(i18n("Configure &KFileReplace..."), "configure", 0, this, SLOT(slotOptionsPreferences()), actionCollection(), "configure_kfilereplace");

   // Results
   (void)new KAction(i18n("&Properties"), "resfileinfo", 0, m_view, SLOT(slotResultProperties()), actionCollection(), "results_infos");
   (void)new KAction(i18n("&Open"), "filenew", 0, m_view, SLOT(slotResultOpen()), actionCollection(), "results_openfile");
   (void)new KAction(i18n("&Open in Quanta"), "edit", 0, m_view, SLOT(slotResultEdit()), actionCollection(), "results_editfile");
   (void)new KAction(i18n("Open Parent &Folder"), "fileopen", 0, m_view, SLOT(slotResultDirOpen()), actionCollection(), "results_opendir");
   (void)new KAction(i18n("&Delete"), "editdelete", 0, m_view, SLOT(slotResultDelete()), actionCollection(), "results_delete");
   (void)new KAction(i18n("E&xpand Tree"), 0, m_view, SLOT(slotResultTreeExpand()), actionCollection(), "results_treeexpand");
   (void)new KAction(i18n("&Reduce Tree"), 0, m_view, SLOT(slotResultTreeReduce()), actionCollection(), "results_treereduce");
   (void)new KAction(i18n("&About KFileReplace"), "kfilereplace", 0, this, SLOT(slotAboutApplication()), actionCollection(), "help_about_kfilereplace");
   (void)new KAction(i18n("KFileReplace &Handbook"), "help", 0, this, SLOT(appHelpActivated()), actionCollection(), "help_kfilereplace");
   (void)new KAction(i18n("&Report Bug..."), 0, 0, this, SLOT(reportBug()), actionCollection(), "report_bug");
}

void KFileReplacePart::initView()
{
  m_view = new KFileReplaceView(m_parentWidget, "view");

  setWidget(m_view);

  m_view->setAcceptDrops(false);

  m_view->setConfig(m_config);
}

KConfig* KFileReplacePart::config()
{
  return m_config;
}

void KFileReplacePart::resetActions()
{
  bool hasChildren = false,
       searchOnly = m_info.searchMode();
  kdDebug(23000)<<"\n\nSEARCHMODE="<<searchOnly<<endl;
  if(m_view && m_view->stringView()->childCount() > 0)
    hasChildren = true;

  // File
  actionCollection()->action("new_project")->setEnabled(true);
  actionCollection()->action("search")->setEnabled(searchOnly);
  actionCollection()->action("file_simulate")->setEnabled(hasChildren && !searchOnly);
  actionCollection()->action("replace")->setEnabled(hasChildren && !searchOnly);
  actionCollection()->action("save_results")->setEnabled(hasChildren);
  actionCollection()->action("stop")->setEnabled(false);

  // Strings
  actionCollection()->action("strings_add")->setEnabled(true);
  actionCollection()->action("strings_del")->setEnabled(hasChildren );
  actionCollection()->action("strings_empty")->setEnabled(hasChildren);
  actionCollection()->action("strings_edit")->setEnabled(hasChildren);
  actionCollection()->action("strings_save")->setEnabled(hasChildren);
  actionCollection()->action("strings_load")->setEnabled(true);
  actionCollection()->action("strings_invert")->setEnabled(hasChildren);
  actionCollection()->action("strings_invert_all")->setEnabled(hasChildren);

  // Options
  actionCollection()->action("options_recursive")->setEnabled(true);
  actionCollection()->action("options_backup")->setEnabled(true);
  actionCollection()->action("options_case")->setEnabled(true);
  actionCollection()->action("options_var")->setEnabled(true);
  actionCollection()->action("options_wildcards")->setEnabled(true);
  actionCollection()->action("configure_kfilereplace")->setEnabled(true);

  if(m_view && m_view->resultView()->childCount() > 0)
    hasChildren = true;
  else hasChildren = false;

  // Results
  actionCollection()->action("results_infos")->setEnabled(hasChildren);
  actionCollection()->action("results_openfile")->setEnabled(hasChildren);
  actionCollection()->action("results_editfile")->setEnabled(hasChildren);
  actionCollection()->action("results_opendir")->setEnabled(hasChildren);
  actionCollection()->action("results_delete")->setEnabled(hasChildren);
  actionCollection()->action("results_treeexpand")->setEnabled(hasChildren);
  actionCollection()->action("results_treereduce")->setEnabled(hasChildren);

  // Update menus and toolbar

  ((KToggleAction* ) actionCollection()->action("options_recursive"))->setChecked(m_info.recursive());
  ((KToggleAction* ) actionCollection()->action("options_backup"))->setChecked(m_info.backup());
  ((KToggleAction* ) actionCollection()->action("options_case"))->setChecked(m_info.caseSensitive());
  ((KToggleAction* ) actionCollection()->action("options_var"))->setChecked(m_info.variables());
  ((KToggleAction* ) actionCollection()->action("options_wildcards"))->setChecked(m_info.wildcards());

}

void KFileReplacePart::freezeActions()
{
  //Disable action during search/replace operation
  actionCollection()->action("new_project")->setEnabled(false);
  actionCollection()->action("stop")->setEnabled(true);
  actionCollection()->action("file_simulate")->setEnabled(false);
  actionCollection()->action("replace")->setEnabled(false);
  actionCollection()->action("search")->setEnabled(false);
  actionCollection()->action("strings_add")->setEnabled(false);
  actionCollection()->action("strings_del")->setEnabled(false);
  actionCollection()->action("strings_empty")->setEnabled(false);
  actionCollection()->action("strings_edit")->setEnabled(false);
  actionCollection()->action("strings_save")->setEnabled(false);
  actionCollection()->action("strings_load")->setEnabled(false);
  actionCollection()->action("strings_invert")->setEnabled(false);
  actionCollection()->action("strings_invert_all")->setEnabled(false);
  actionCollection()->action("options_recursive")->setEnabled(false);
  actionCollection()->action("options_backup")->setEnabled(false);
  actionCollection()->action("options_case")->setEnabled(false);
  actionCollection()->action("options_var")->setEnabled(false);
  actionCollection()->action("options_wildcards")->setEnabled(false);
  actionCollection()->action("configure_kfilereplace")->setEnabled(false);
  actionCollection()->action("results_infos")->setEnabled(false);
  actionCollection()->action("results_openfile")->setEnabled(false);
  actionCollection()->action("results_editfile")->setEnabled(false);
  actionCollection()->action("results_opendir")->setEnabled(false);
  actionCollection()->action("results_delete")->setEnabled(false);
  actionCollection()->action("results_treeexpand")->setEnabled(false);
  actionCollection()->action("results_treereduce")->setEnabled(false);
  ((KToggleAction* ) actionCollection()->action("options_recursive"))->setChecked(false);
  ((KToggleAction* ) actionCollection()->action("options_backup"))->setChecked(false);
  ((KToggleAction* ) actionCollection()->action("options_case"))->setChecked(false);
  ((KToggleAction* ) actionCollection()->action("options_var"))->setChecked(false);
  ((KToggleAction* ) actionCollection()->action("options_wildcards"))->setChecked(false);

}

void KFileReplacePart::readOptions()
{

  m_config->setGroup("General Options");

  // Recent files
  m_recentStringFileList = m_config->readListEntry(rcRecentFiles);
  ((KRecentFilesAction* ) actionCollection()->action("strings_load_recent"))->setItems(m_recentStringFileList);

  // options seetings (recursivity, backup, case sensitive)
  m_config->setGroup("Options");

  m_info.setRecursive(m_config->readBoolEntry(rcRecursive, RecursiveOption));
  QStringList bkList = QStringList::split(",",
                                          m_config->readEntry(rcBackupExtension, BackupExtension),
					  true);
  if(bkList[0] == "true")
    m_info.setBackup(true);
  else
    m_info.setBackup(false);

  m_info.setBackupExtension(bkList[1]);

  m_info.setCaseSensitive(m_config->readBoolEntry(rcCaseSensitive, CaseSensitiveOption));
  m_info.setVariables(m_config->readBoolEntry(rcVariables, VariablesOption));
  m_info.setWildcards(m_config->readBoolEntry(rcWildcards, WildcardsOption));
  m_info.setFollowSymLinks(m_config->readBoolEntry(rcFollowSymLinks, FollowSymbolicLinksOption));

  m_info.setConfirmFiles(m_config->readBoolEntry(rcConfirmFiles, ConfirmFilesOption));
  m_info.setConfirmStrings(m_config->readBoolEntry(rcConfirmStrings, ConfirmStringsOption));
  m_info.setConfirmDirs(m_config->readBoolEntry(rcConfirmDirs, ConfirmDirectoriesOption));
  m_info.setHaltOnFirstOccur(m_config->readBoolEntry(rcHaltOnFirstOccur, StopWhenFirstOccurenceOption));
  m_info.setIgnoreWhitespaces(m_config->readBoolEntry(rcIgnoreWhitespaces, IgnoreWhiteSpacesOption));
  m_info.setIgnoreHidden(m_config->readBoolEntry(rcIgnoreHidden, IgnoreHiddenOption));
  m_info.setSearchMode(m_config->readBoolEntry(rcSearchMode,SearchMode));

  m_info.setMinSize(m_config->readNumEntry(rcMinFileSize, FileSizeOption));
  m_info.setMaxSize(m_config->readNumEntry(rcMaxFileSize, FileSizeOption));

  m_info.setDateAccess(m_config->readEntry(rcValidAccessDate, ValidAccessDateOption));
  m_info.setMinDate(m_config->readEntry(rcMinDate, AccessDateOption));
  m_info.setMaxDate(m_config->readEntry(rcMaxDate, AccessDateOption));

  QStringList ownerList = QStringList::split(",",m_config->readEntry(rcOwnerUser, OwnerOption),true);
  if(ownerList[0] == "true")
    m_info.setOwnerUserIsChecked(true);
  else
    m_info.setOwnerUserIsChecked(false);

  m_info.setOwnerUserType(ownerList[1]);
  m_info.setOwnerUserBool(ownerList[2]);
  if(ownerList[3] == "???")
    m_info.setOwnerUserValue("");
  else
    m_info.setOwnerUserValue(ownerList[3]);

  ownerList = QStringList::split(",",m_config->readEntry(rcOwnerGroup, OwnerOption),true);

  if(ownerList[0] == "true")
    m_info.setOwnerGroupIsChecked(true);
  else
    m_info.setOwnerGroupIsChecked(false);

  m_info.setOwnerGroupType(ownerList[1]);
  m_info.setOwnerGroupBool(ownerList[2]);
  if(ownerList[3] == "???")
    m_info.setOwnerGroupValue("");
  else
    m_info.setOwnerGroupValue(ownerList[3]);

}

void KFileReplacePart::saveOptions()
{
  m_config->setGroup("General Options");
  // Recent file list
  m_config->writeEntry(rcRecentFiles, m_recentStringFileList);

  // options seetings (recursivity, backup, case sensitive)
  m_config->setGroup("Options");

  m_config->writeEntry(rcRecursive, m_info.recursive());
  QString bkOptions;

  if(m_info.backup())
    bkOptions = "true";
  else
    bkOptions = "false";

  bkOptions += ","+m_info.backupExtension();

  m_config->writeEntry(rcBackupExtension, bkOptions);
  m_config->writeEntry(rcCaseSensitive, m_info.caseSensitive());
  m_config->writeEntry(rcVariables, m_info.variables());
  m_config->writeEntry(rcWildcards, m_info.wildcards());

  m_config->writeEntry(rcFollowSymLinks, m_info.followSymLinks());

  m_config->writeEntry(rcConfirmFiles, m_info.confirmFiles());
  m_config->writeEntry(rcConfirmStrings, m_info.confirmStrings());
  m_config->writeEntry(rcConfirmDirs, m_info.confirmDirs());
  m_config->writeEntry(rcHaltOnFirstOccur, m_info.haltOnFirstOccur());
  m_config->writeEntry(rcIgnoreWhitespaces, m_info.ignoreWhitespaces());
  m_config->writeEntry(rcIgnoreHidden, m_info.ignoreHidden());

  m_config->sync();
}

bool KFileReplacePart::checkBeforeOperation()
{
  readOptions();
  loadInformationFromView();
  QWidget* w = widget();

  // Check there are strings to replace (not need in search operation)
  if (m_view->stringView()->childCount() == 0)
    {
      KMessageBox::error(w, i18n("There are no strings to search and replace."));
      return false;
    }

  // Check the main directory can be accessed
  QString directory = m_info.directory();
  QDir dir;

  dir.setPath(directory);
  directory = dir.absPath();

  if (!dir.exists())
    {
      KMessageBox::error(w, i18n("<qt>The main folder of the project <b>%1</b> does not exist.</qt>").arg(directory));
      return false;
    }

  if (::access(directory.local8Bit(), R_OK | X_OK) == -1)
    {
      KMessageBox::error(w, i18n("<qt>Access denied in the main folder of the project:<br><b>%1</b></qt>").arg(directory));
      return false;
    }

  // Clear the list view
  m_view->resultView()->clear();

  return true;
}

bool KFileReplacePart::verifyFileRequirements(const QString& filePath, const QString& fileName)
{
  if(fileName == ".." or
     fileName == "." or
     fileName.right(4) == ".OLD")
    //continue;
    return false;

  QFileInfo fi;
  if(filePath.isEmpty())
    fi.setFile(fileName);
  else
    fi.setFile(filePath+"/"+fileName);

  if(fi.isDir())
    return true;

   int minSize = m_info.minSize(),
       maxSize = m_info.maxSize();
   QString minDate = m_info.minDate(),
           maxDate = m_info.maxDate(),
           dateAccess = m_info.dateAccess();

  // Avoid files that not match access date requirements
  QString last = "unknown";
  if(dateAccess == "Last Writing Access")
    last = fi.lastModified().toString(Qt::ISODate);
  if(dateAccess == "Last Reading Access")
    last = fi.lastRead().toString(Qt::ISODate);

  if(last != "unknown")
    {
      if(minDate != "unknown" and maxDate != "unknown")
        { //If out of range then exit
          if((minDate > last) or (maxDate < last))
	    //continue;
	    return false;
        }
      else
        {
          if(minDate != "unknown")
	    { //If out of range then exit
	      if(minDate > last)
	        //continue;
		return false;
            }
          else
            {
	      if(maxDate != "unknown")
	      //If out of range then exit
              if(maxDate < last)
	        //continue;
		return false;
            }
        }
    }
  // Avoid files that not match size requirements
  int size = fi.size();
  if(maxSize != -1 and minSize != -1)
    if(size > (maxSize*1024) or
       size < (minSize*1024))
      //continue;
      return false;
  // Avoid files that not match ownership requirements
  if(m_info.ownerUserIsChecked())
    {
      QString fileOwnerUser;
      if(m_info.ownerUserType() == "Name")
        fileOwnerUser = fi.owner();
      else
        fileOwnerUser = QString::number(fi.ownerId(),10);

      if(m_info.ownerUserBool() == "Equals To")
        {
	  if(m_info.ownerUserValue() != fileOwnerUser)
	    return false;
	}
      else
        {
	  if(m_info.ownerUserValue() == fileOwnerUser)
	    return false;
	}
    }

  if(m_info.ownerGroupIsChecked())
    {
      QString fileOwnerGroup;
      if(m_info.ownerGroupType() == "Name")
        fileOwnerGroup = fi.group();
      else
        fileOwnerGroup = QString::number(fi.groupId(),10);
      if(m_info.ownerGroupBool() == "Equals To")
        {
	  if(m_info.ownerGroupValue() != fileOwnerGroup)
	    return false;
	}
      else
        {
	  if(m_info.ownerGroupValue() == fileOwnerGroup)
	    return false;
	}
    }
  //All requirements verified
  return true;
}

void KFileReplacePart::slotFileNew()
{
  m_view->resultView()->clear();
  m_config->reparseConfiguration();
  launchNewProjectDialog(KURL());
  emit setStatusBarText(i18n("Ready."));
}

void KFileReplacePart::slotFileSearch()
{
   if (!checkBeforeOperation())
      return;

   emit setStatusBarText(i18n("Searching files..."));
   m_view->resultView()->setColumnText(4, i18n("Total number occurrences"));

   // show wait cursor
   QApplication::setOverrideCursor( Qt::waitCursor );

   freezeActions();

   setOptionMask();

   m_view->resultView()->setSorting(-1);

   QString dirName = m_info.directory(),
           filters = m_info.filter();

   if(m_info.recursive())
     recursiveFileSearch(dirName,filters);
   else
     normalFileSearch(dirName,filters);

   m_view->resultView()->setSorting(0);
   m_view->resultView()->sort();
   m_view->resultView()->setSorting(-1);

   // restore false status for stop button
   m_stop = false;

   QApplication::restoreOverrideCursor();

   emit setStatusBarText(i18n("Search completed."));

   resetActions();
}

void KFileReplacePart::slotFileSimulate()
{
  m_info.setSimulation(true);
  //slotFileReplace();
}

void KFileReplacePart::slotFileReplace()
{
  if(!checkBeforeOperation())
    return;

  emit setStatusBarText(i18n("Replacing files..."));
  m_view->resultView()->setColumnText(4, i18n("Replaced strings"));

  // show wait cursor
  QApplication::setOverrideCursor( Qt::waitCursor );

  freezeActions();

  setOptionMask();

  m_view->resultView()->setSorting(-1);

  if(m_info.recursive())
    recursiveFileReplace(m_info.directory());
  else
    normalFileReplace();

  m_view->resultView()->setSorting(0);
  m_view->resultView()->sort();
  m_view->resultView()->setSorting(-1);

  // restore false status for stop button
  m_stop = false;

  QApplication::restoreOverrideCursor();

  resetActions();
}

void KFileReplacePart::slotFileStop()
{
  emit setStatusBarText(i18n("Stop."));
  m_stop = true;
  resetActions();
  QApplication::restoreOverrideCursor();
}

void KFileReplacePart::slotFileSave()
{
  QString fileName;
  QWidget* w = widget();
  QListView* lvResult = m_view->resultView();

  // Check there are results
  if (lvResult->childCount() == 0)
    {
      KMessageBox::error(w, i18n("There are no results to save: the result list is empty."));
      return ;
    }
  // Select the file where results will be saved
  fileName = KFileDialog::getSaveFileName(QString::null, i18n("*.xhtml|XHTML Files (*.xhtml)\n*|All Files (*)"), w, i18n("Save Results"));
  if (fileName.isEmpty())
    return ;

  // Force the extension to be "xhtml"
  fileName = KFileReplaceLib::addFilenameExtension(fileName, "xhtml");

  // Save results to file
  // a) Open the file
  QFile fResults(fileName);
  if ( !fResults.open( IO_WriteOnly ) )
    {
      KMessageBox::error(w, i18n("<qt>Cannot open the file <b>%1</b> for writing the save results.</qt>").arg(fileName));
      return ;
    }
   // b) Write header of the XHTML file
   QDateTime datetime = QDateTime::currentDateTime(Qt::LocalTime);
   QString dateString = datetime.toString(Qt::LocalDate);
   QString XHTML = "<?xml version=\"1.0\" ?>\n"
                   "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
                   "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                   "\t<head>\n"
                   "\t\t<title>";
           XHTML += i18n("KFileReplace Results File")+
	            "</title>\n"
                    "\t\t\t<style type=\"text/css\">\n"
                    "\t\t\t\t.a {background-color : lightblue;}\n"
                    "\t\t\t\t.b {background-color : paleturquoise;}\n"
                    "\t\t\t\tbody { border: solid teal;}\n"
                    "\t\t\t\t.date { text-align:right; color:darkcyan;}\n"
                    "\t\t\t</style>\n"
                    "\t</head>\n"
                    "\t<body>\n"
                    "\t\t<div style=\"background-color:ivory;padding :10px;\">\n"
                    "\t\t\t<table width=\"100%\">\n"
                    "\t\t\t\t<tr>\n"
                    "\t\t\t\t\t<td><h1>";
           XHTML += i18n("KFileReplace Results File")+"</h1></td>\n"
                    "\t\t\t\t\t<td><div class=\"date\">";
           XHTML += i18n("Creation date : ")+dateString+"</div></td>\n"
                    "\t\t\t\t</tr>\n"
                    "\t\t\t</table>\n"
                    "\t\t<div>\n"
                    "\t\t\t<dl>\n";
   QTextStream oTStream( &fResults );
   oTStream << XHTML;

  // c) Write the file list
  QListViewItem* lviCurItem;
  QListViewItem* lviFirst;
  QString strPath;

  lviCurItem = lviFirst = lvResult->firstChild();

  if(lviCurItem == 0)
    return ;

  unsigned int replacedFileNumber = 0;

  QString classValue="a";
  do
    {
      strPath = KFileReplaceLib::formatFullPath(lviCurItem->text(1), lviCurItem->text(0));
      QString divclassString ="\t\t\t\t<div class=\""+classValue+"\">\n"
                              "\t\t\t\t\t<dt><a href=\"file:"+strPath+"\">file:"+strPath+"</a>\n"
                              "\t\t\t\t\t</dt>\n";
              divclassString += "\t\t\t\t\t<dd>";
	      if(lvResult->columnText(4) == "Found occurrences")
	        {
		  divclassString += i18n("Size: ")
		                 + lviCurItem->text(2)
				 +i18n("--> Found strings:")
				 + lviCurItem->text(4);
		}
              else
	        {
		  divclassString += i18n("Old size: ")
		                 + lviCurItem->text(2)
				 + i18n("--> New size: ")
				 + lviCurItem->text(3)
				 + i18n("--> Replaced strings: ")
				 + lviCurItem->text(4);
		}

              divclassString += "</dd>\n"
                                "\t\t\t\t</div>\n";

      oTStream << divclassString;

      if(classValue == "a")
        classValue = "b";
      else
        classValue = "a";

      replacedFileNumber += lviCurItem->text(4).toInt();

      lviCurItem = lviCurItem->nextSibling();
    } while(lviCurItem and lviCurItem != lviFirst);


  // d) Write the end of the file

   oTStream<<"\t\t\t\t</dl>\n"
             "\t\t\t</div>\n"
             "\t\t\t<div style=\"text-align:right;color:darkcyan\">";
   if(lvResult->columnText(4) == "Found occurrences")
     oTStream<<i18n("Total number of found occurrences: ");
   else
     oTStream<<i18n("Total number of replaced strings: ");

   oTStream<<replacedFileNumber
           <<"</div>\n"
             "\t\t</div>\n"
             "\t</body>\n"
             "</html>\n";
   fResults.close();
   resetActions();
}

void KFileReplacePart::slotStringsAdd()
{
  m_view->slotStringsAdd();
  m_config->setGroup("General Options");
  m_info.setSearchMode(m_config->readBoolEntry(rcSearchMode,SearchMode));

  resetActions();
}

void KFileReplacePart::slotStringsAddFromProjectDlg(const QMap<QString,QString>& replacementMap)
{
  m_view->slotStringsAddFromProjectDlg(replacementMap);
  m_config->setGroup("General Options");
  m_info.setSearchMode(m_config->readBoolEntry(rcSearchMode,SearchMode));

  resetActions();
}

void KFileReplacePart::slotStringsDel()
{
  m_view->slotStringsDel();
  resetActions();
}

void KFileReplacePart::slotStringsEmpty()
{
  m_view->slotStringsClear();
  resetActions();
}

void KFileReplacePart::slotStringsEdit()
{
  m_view->slotStringsEdit(0L);
  m_config->setGroup("General Options");
  m_info.setSearchMode(m_config->readBoolEntry(rcSearchMode,SearchMode));

  resetActions();
}

void KFileReplacePart::slotStringsSave()
{
  QWidget* w = widget();
  // Check there are strings in the list
  if (m_view->stringView()->childCount() == 0)
    {
      KMessageBox::error(w, i18n("There are no strings to save in the list."));
      return ;
    }

   QString header("<?xml version=\"1.0\" ?>\n<kfr>"),
           footer("\n</kfr>"),
           body;
   QListViewItem*  lvi = m_view->stringView()->firstChild();

   while( lvi )
     {
       body += QString("\n\t<replacement>"
                       "\n\t\t<oldstring><![CDATA[%1]]></oldstring>"
		       "\n\t\t<newstring><![CDATA[%2]]></newstring>"
		       "\n\t</replacement>").arg(lvi->text(0)).arg(lvi->text(1));
       lvi = lvi->nextSibling();
     }

   // Select the file where strings will be saved
   QString fileName = KFileDialog::getSaveFileName(QString::null, i18n("*.kfr|KFileReplace Strings (*.kfr)\n*|All Files (*)"), w, i18n("Save Strings to File"));
   if (fileName.isEmpty())
     return;

   // Force the extension to be "kfr" == KFileReplace extension
   fileName = KFileReplaceLib::addFilenameExtension(fileName, "kfr");

   QFile file( fileName );
   if(!file.open( IO_WriteOnly ))
     {
       KMessageBox::error(w, i18n("File %1 cannot be saved.").arg(fileName));
       return ;
     }
   QTextStream oTStream( &file );
   oTStream << header
            << body
            << footer;
   file.close();
}

void KFileReplacePart::loadStringFile(const QString& fileName)
{
  //load kfr file
  QDomDocument doc("mydocument");
  QFile file(fileName);
  if(!file.open(IO_ReadOnly))
    {
      KMessageBox::error(widget(), i18n("<qt>Cannot open the file <b>%1</b> and load the string list.</qt>").arg(fileName));
      return ;
    }
  if(!doc.setContent(&file))
    {
      file.close();
      KMessageBox::information(widget(), i18n("<qt>File <b>%1</b> seems not to be written in new kfr format. Remember that old kfr format will be soon abandoned! You can convert your old rules files by simply saving them with kfilereplace.</qt>").arg(fileName),i18n("Warning"));
      convertOldToNewKFRFormat(fileName);
      return;
    }
  file.close();

  //clear view
  m_view->stringView()->clear();

  QDomElement docElem = doc.documentElement();
  QDomNode n = docElem.firstChild();
  while(!n.isNull())
    {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if(!e.isNull())
        {
          QListViewItem* lvi = new QListViewItem(m_view->stringView());
          QString oldString = e.firstChild().toElement().text();
          lvi->setText(0,oldString);
          QString newString = e.lastChild().toElement().text();
          lvi->setText(1,newString);
          lvi->setPixmap(0, m_view->iconString());
        }
      n = n.nextSibling();
    }

     // Add file to "load strings form file" menu
  if (!m_recentStringFileList.contains(fileName))
    {
      m_recentStringFileList.append(fileName);
      ((KRecentFilesAction* ) actionCollection()->action("strings_load_recent"))->setItems(m_recentStringFileList);
    }
  resetActions();
}

void KFileReplacePart::slotStringsLoad()
{
  // Select the file to load from
  QString fileName = KFileDialog::getOpenFileName(QString::null, i18n("*.kfr|KFileReplace strings (*.kfr)\n*|All Files (*)"), widget(), i18n("Load Strings From File"));

  if(!fileName.isEmpty())
    loadStringFile(fileName);

  resetActions();
}

void KFileReplacePart::slotStringsInvertCur()
{
  QListViewItem* lviCurItem;
  lviCurItem = m_view->stringView()->currentItem();
  if (lviCurItem == 0)
    return;

  QString searchText = lviCurItem->text(0),
          replaceText = lviCurItem->text(1);

  // Cannot invert the string if search string will be empty
  if (replaceText.isEmpty())
    {
      KMessageBox::error(widget(), i18n("<qt>Cannot invert string <b>%1</b>, because the search string would be empty.</qt>").arg(searchText));
      return;
    }

  lviCurItem->setText(0, replaceText);
  lviCurItem->setText(1, searchText);
}

void KFileReplacePart::slotStringsInvertAll()
{
  QListViewItem* lviCurItem,
               * lviFirst;
  QString searchText,
          replaceText;

  lviCurItem = lviFirst = m_view->stringView()->firstChild();
  if (lviCurItem == 0)
    return ;

  do
    {
      searchText = lviCurItem->text(0);
      replaceText = lviCurItem->text(1);

      // Cannot invert the string if search string will be empty
      if (replaceText.isEmpty())
        {
          KMessageBox::error(widget(), i18n("<qt>Cannot invert string <b>%1</b>, because the search string would be empty.</qt>").arg(searchText));
          return;
        }

      lviCurItem->setText(0, replaceText);
      lviCurItem->setText(1, searchText);

      lviCurItem = lviCurItem->nextSibling();
    } while(lviCurItem and lviCurItem != lviFirst);
}

void KFileReplacePart::slotOpenRecentStringFile(const KURL& urlFile)
{
  QString fileName;

  // Download file if need (if url is "http://...")
  if (!(KIO::NetAccess::download(urlFile, fileName, 0L))) // Open the Archive
    return;

  // Check it's not a directory
  QFileInfo fi;
  fi.setFile(fileName);
  if(fi.isDir())
    {
      KMessageBox::error(widget(), i18n("Cannot open folders."));
      return;
    }

  loadStringFile(fileName);
  resetActions();
}

void KFileReplacePart::slotOptionsRecursive()
{
  m_info.setRecursive(!(m_info.recursive()));
  m_config->setGroup("Options");
  m_config->writeEntry(rcRecursive, m_info.recursive());
  m_config->sync();
  resetActions();
}

void KFileReplacePart::slotOptionsBackup()
{
  m_info.setBackup(!(m_info.backup()));
  m_config->setGroup("Options");
  QString bkOptions;
  if(m_info.backup())
    bkOptions = "true";
  else
    bkOptions = "false";

  bkOptions += ","+m_info.backupExtension();
  m_config->writeEntry(rcBackupExtension, bkOptions);

  m_config->sync();
  resetActions();
}

void KFileReplacePart::slotOptionsCaseSensitive()
{
  m_info.setCaseSensitive(!(m_info.caseSensitive()));
  m_config->setGroup("Options");
  m_config->writeEntry(rcCaseSensitive, m_info.caseSensitive());
  m_config->sync();
  resetActions();
}

void KFileReplacePart::slotOptionsVariables()
{
  m_info.setVariables(!(m_info.variables()));
  m_config->setGroup("Options");
  m_config->writeEntry(rcVariables, m_info.variables());
  m_config->sync();
  resetActions();
}

void KFileReplacePart::slotOptionsWildcards()
{
  m_info.setWildcards(!(m_info.wildcards()));
  m_config->setGroup("Options");
  m_config->writeEntry(rcWildcards, m_info.wildcards());
  m_config->sync();
  resetActions();
}

void KFileReplacePart::slotOptionsPreferences()
{
  KOptionsDlg dlg(widget(), 0);

  if(!dlg.exec())
    return;
  m_config->reparseConfiguration();
  readOptions();
  resetActions();
}

void KFileReplacePart::setWhatsThis()
{
  actionCollection()->action("file_simulate")->setWhatsThis(i18n(fileSimulateWhatthis));
  actionCollection()->action("options_backup")->setWhatsThis(i18n(optionsBackupWhatthis));
  actionCollection()->action("options_case")->setWhatsThis(i18n(optionsCaseWhatthis));
  actionCollection()->action("options_var")->setWhatsThis(i18n(optionsVarWhatthis));
  actionCollection()->action("options_recursive")->setWhatsThis(i18n(optionsRecursiveWhatthis));
  actionCollection()->action("options_wildcards")->setWhatsThis(i18n(optionsWildcardsWhatthis));
}

void KFileReplacePart::reportBug()
{
  KBugReport dlg(widget(), true, createAboutData());
  dlg.exec();
}

void KFileReplacePart::appHelpActivated()
{
  kapp->invokeHelp(QString::null, "kfilereplace");
}

void KFileReplacePart::slotAboutApplication()
{
  m_dlgAbout = new KAboutApplication(createAboutData(), 0, false);
  if(m_dlgAbout == 0)
    return;

  if(!m_dlgAbout->isVisible())
    m_dlgAbout->show();
  else
    m_dlgAbout->raise();

  resetActions();
}

void KFileReplacePart::loadInformationFromView()
{
  QMap<QString,QString> tempMap;
  QListViewItemIterator itlv(m_view->stringView());
  while(itlv.current())
    {
      QListViewItem *item = itlv.current();
      if(m_info.variables())
        tempMap[item->text(0)] = variableValue(item->text(1));
      else
        tempMap[item->text(0)] = item->text(1);
      ++itlv;
    }
  m_replacementMap = tempMap;
}

QString KFileReplacePart::variableValue(const QString &variable)
{
  QString s = variable;

  s.remove("[$").remove("$]").remove(" ");

  if(s.contains(":") == 0)
    return variable;
  else
    {
      QString leftValue = s.section(":",0,0),
              rightValue = s.section(":",1,1);

      if(leftValue == "datetime")
        return KFileReplaceLib::datetime(rightValue);
      if(leftValue == "user")
        return KFileReplaceLib::user(rightValue);
      if(leftValue == "loadfile")
        return KFileReplaceLib::loadfile(rightValue);
      if(leftValue == "empty")
        return KFileReplaceLib::empty(rightValue);
      if(leftValue == "mathexp")
        return KFileReplaceLib::mathexp(rightValue);
      if(leftValue == "random")
        return KFileReplaceLib::random(rightValue);
     /* if(leftValue.contains("filelastreadtime") != 0)
        return filelastreadtimeValue(rightValue);

      if(leftValue.contains("filelastreadtime") != 0)
        return filenameValue(rightValue);*/
    }
 return variable;
}

void KFileReplacePart::convertOldToNewKFRFormat(const QString& fileName)
{
 //this method convert old format in new XML-based format
 typedef struct
 {
   char pgm[13]; // Must be "KFileReplace" : like MZ for EXE files
   int stringNumber; // Number of strings in file
   char reserved[64]; // Reserved for future use
 } KFRHeader;

 KFRHeader head;

 FILE* f = fopen(fileName.ascii(),"rb");
 if(!f)
 {
   KMessageBox::error(widget(), i18n("<qt>Cannot open the file <b>%1</b> and load the string list.</qt>").arg(fileName));
   return ;
 }
 int err = fread(&head, sizeof(KFRHeader), 1, f);
 if(err != 1)
 {
   KMessageBox::error(widget(), i18n("<qt>Cannot open the file <b>%1</b> and load the string list.</qt>").arg(fileName));
   return ;
 }
 QString pgm(head.pgm);
 if (pgm != "KFileReplace")
 {
   KMessageBox::error(widget(), i18n("<qt>Cannot open the file <b>%1</b> and load the string list. This file seems not to be a valid old kfr file.</qt>").arg(fileName));
   return ;
 }

  m_view->stringView()->clear();

  int oldTextSize,
      newTextSize,
      errors = 0,
      stringSize;
  QStringList l;

  int i ;
  for (i=0; i < head.stringNumber; i++)
    {
      errors += (fread(&oldTextSize, sizeof(int), 1, f)) != 1;
      errors += (fread(&newTextSize, sizeof(int), 1, f)) != 1;
      if(errors > 0)
        KMessageBox::error(widget(), i18n("<qt>Cannot read data.</qt>"));
      else
        {
          stringSize = ((oldTextSize > newTextSize) ? oldTextSize : newTextSize) + 2;
          char* oldString = (char*) malloc(stringSize+10),
              * newString = (char*) malloc(stringSize+10);
          memset(oldString, 0, stringSize);
          memset(newString,0, stringSize);
          if (oldString == 0 or newString == 0)
            KMessageBox::error(widget(), i18n("<qt>Out of memory.</qt>"));
          else
            {
              if (fread(oldString, oldTextSize, 1, f) != 1)
                KMessageBox::error(widget(), i18n("<qt>Cannot read data.</qt>"));
              else
                {
                  if (newTextSize > 0) // If there is a Replace text
                    {
                      if (fread(newString, newTextSize, 1, f) != 1)
                        KMessageBox::error(widget(), i18n("<qt>Cannot read data.</qt>"));
                      else
                        {
                          QListViewItem* lvi = new QListViewItem(m_view->stringView());
                          lvi->setText(0,oldString);
                          lvi->setText(1,newString);
                          lvi->setPixmap(0, m_view->iconString());

                          if(newString)
                            free(newString);
                          if(oldString)
                            free(oldString);
                        }
                    }
                }
            }
        }
    }
    fclose(f);
    return ;
 }

#include "kfilereplacepart.moc"

