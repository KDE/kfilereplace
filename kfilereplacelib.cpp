/***************************************************************************
                           kfilereplacelib.cpp  -  File library
                                      -------------------
    begin                : lun mai  3 20:19:52 CEST 1999

    copyright            : (C) 1999 by François Dupoux
                           (C) 2003 Andras Mantia <amantia@kde.org>
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

//QT
#include <qstringlist.h>
#include <qwidget.h>
#include <qlistview.h>
#include <qfileinfo.h>

//KDE
#include <kdebug.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <kiconloader.h>

// local
#include "kfilereplacelib.h"

const double kilo = 1024.0;
const double mega = 1048576.0;//1024^2
const double giga = 1073741824.0;//1024^3
const double tera = 1099511627776.0;//1024^4

/**
 Format a path, from a path and a filename, or another sub-path (avoid double '/' risks)
 Parameters::.....* basePath: fist path (can be "/" if root, or "/usr/bin/" or "/usr/bin" for example)
 .................* fileName: second path (can be "/doc/html/", or "doc/html/" or "doc/html/index.html" for example)
 Return values:...* Full valid path (without double "/")
*/
QString KFileReplaceLib::formatFullPath(const QString& basePath, const QString &fileName)
{
  QString fullPath = basePath;
  QString fname = fileName;

  if (fname.startsWith("/")) // skip beginning '/'
    fname = fname.remove(0,1);

  if (fullPath.endsWith("/"))
    fullPath.append(fname);
  else
    fullPath.append("/"+fname);

  return fullPath;
}

/**
 Add an extension to a filename, or a filepath
 Parameters::.....* fileName: filename or filepath (it can have already the extension)
 .................* extension: extension to add without "." (ex: "html", "kfr")
 Return values:...* Filename / Filepath with the extension
*/
QString KFileReplaceLib::addExtension(const QString& fileName, const QString& extension)
{
  QString fullExtension = ".";
  QString fname = fileName;

  fullExtension.append(extension);

  // filename cannot contain ".ext" ==> Add it
  if(fname.length() <= fullExtension.length())
   fname.append(fullExtension);
  else // filename can contain ".ext"
  {
   if (fname.right(fullExtension.length()) != fullExtension)
    fname.append(fullExtension);
  }

  return fname;
}

QString KFileReplaceLib::formatFileSize(double size)
{
  QString stringSize;

  if(size < kilo)
    stringSize = i18n("%1 bytes").arg(QString::number(size,'f',0));
  else
  if(size >= kilo && size < mega)
    {
      double d = size / kilo;
      stringSize = i18n("%1 KB").arg(QString::number(d,'f',2));
    }
  else
  if(size >= mega && size < giga)
    {
      double d = size / mega;
      stringSize = i18n("%1 MB").arg(QString::number(d,'f',2));
    }
  else
  if(size >= giga)
    {
      double d = size / giga;
      stringSize =  i18n("%1 GB").arg(QString::number(d,'f',2));
    }
  return stringSize;
}

void KFileReplaceLib::convertOldToNewKFRFormat(const QString& fileName, QListView* stringView)
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
 int err = fread(&head, sizeof(KFRHeader), 1, f);
 QString pgm(head.pgm);

 if(!f || (err != 1) || (pgm != "KFileReplace"))
 {
   KMessageBox::error(0, i18n("<qt>Cannot open the file <b>%1</b> and load the string list. This file seems not to be a valid old kfr file or it is broken.</qt>").arg(fileName));
   return ;
 }

  stringView->clear();

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
        KMessageBox::error(0, i18n("<qt>Cannot read data.</qt>"));
      else
        {
          stringSize = ((oldTextSize > newTextSize) ? oldTextSize : newTextSize) + 2;
          char* oldString = (char*) malloc(stringSize+10),
              * newString = (char*) malloc(stringSize+10);
          memset(oldString, 0, stringSize);
          memset(newString,0, stringSize);
          if (oldString == 0 || newString == 0)
            KMessageBox::error(0, i18n("<qt>Out of memory.</qt>"));
          else
            {
              if (fread(oldString, oldTextSize, 1, f) != 1)
                KMessageBox::error(0, i18n("<qt>Cannot read data.</qt>"));
              else
                {
                  if (newTextSize > 0) // If there is a Replace text
                    {
                      if (fread(newString, newTextSize, 1, f) != 1)
                        KMessageBox::error(0, i18n("<qt>Cannot read data.</qt>"));
                      else
                        {
                          QListViewItem* lvi = new QListViewItem(stringView);
                          lvi->setText(0,oldString);
                          lvi->setText(1,newString);

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

bool KFileReplaceLib::isAnAccessibleFile(const QString& filePath, const QString& fileName, const ConfigurationInformation& info)
{
  QString bkExt = info.backupExtension();
  if(fileName == ".." || fileName == "." || fileName.right(bkExt.length()) == bkExt)
    return false;

  QFileInfo fi;
  if(filePath.isEmpty())
    fi.setFile(fileName);
  else
    fi.setFile(filePath+"/"+fileName);

  if(fi.isDir())
    return true;

   int minSize = info.minSize(),
       maxSize = info.maxSize();
   QString minDate = info.minDate(),
           maxDate = info.maxDate(),
           dateAccess = info.dateAccess();

  // Avoid files that not match access date requirements
  QString last = "unknown";
  if(dateAccess == "Last Writing Access")
    last = fi.lastModified().toString(Qt::ISODate);
  if(dateAccess == "Last Reading Access")
    last = fi.lastRead().toString(Qt::ISODate);

  if(last != "unknown")
    {
      if(minDate != "unknown" && maxDate != "unknown")
        { //If out of range then exit
          if((minDate > last) || (maxDate < last))
            return false;
        }
      else
        {
          if(minDate != "unknown")
            { //If out of range then exit
              if(minDate > last)
                return false;
            }
          else
            {
              if(maxDate != "unknown")
              //If out of range then exit
              if(maxDate < last)
                return false;
            }
        }
    }
  // Avoid files that not match size requirements
  int size = fi.size();
  if(maxSize != FileSizeOption && minSize != FileSizeOption)
    if(size > (maxSize*1024) || size < (minSize*1024))
      return false;

  // Avoid files that not match ownership requirements
  if(info.ownerUserIsChecked())
    {
      QString fileOwnerUser;
      if(info.ownerUserType() == "Name")
        fileOwnerUser = fi.owner();
      else
        fileOwnerUser = QString::number(fi.ownerId(),10);

      if(info.ownerUserBool() == "Equals To")
        {
          if(info.ownerUserValue() != fileOwnerUser)
            return false;
        }
      else
        {
          if(info.ownerUserValue() == fileOwnerUser)
            return false;
        }
    }

  if(info.ownerGroupIsChecked())
    {
      QString fileOwnerGroup;
      if(info.ownerGroupType() == "Name")
        fileOwnerGroup = fi.group();
      else
        fileOwnerGroup = QString::number(fi.groupId(),10);
      if(info.ownerGroupBool() == "Equals To")
        {
          if(info.ownerGroupValue() != fileOwnerGroup)
            return false;
        }
      else
        {
          if(info.ownerGroupValue() == fileOwnerGroup)
            return false;
        }
    }

  //If we are here then all requirements have been verified
  return true;
}

void KFileReplaceLib::setIconForFileEntry(QListViewItem* item, QString path)
{
  QFileInfo fi(path);
  QString extension = fi.extension(),
          baseName = fi.baseName();

  if(extension == "cpp")
    item->setPixmap(0,SmallIcon("source_cpp"));
  else
  if(extension == "h")
    item->setPixmap(0,SmallIcon("source_h"));
  else
  if(extension == "o")
    item->setPixmap(0,SmallIcon("source_o"));
  else
  if((extension == "png") || (extension == "jpg") || (extension == "xpm"))
    item->setPixmap(0,SmallIcon("image"));
  else
  if((extension.contains("htm",false) != 0) || (extension.contains("xml",false) != 0))
    item->setPixmap(0,SmallIcon("html"));
  else
  if(extension.contains("pdf",false) != 0)
    item->setPixmap(0,SmallIcon("pdf"));
  else
  if((extension.contains("wav",false) != 0) || (extension.contains("mp3",false) != 0))
    item->setPixmap(0,SmallIcon("sound"));
  else
  if((extension.contains("txt",false) != 0))
    item->setPixmap(0,SmallIcon("txt"));
  else
  if((extension.contains("sh",false) != 0))
    item->setPixmap(0,SmallIcon("shellscript"));
  else
  if((extension.contains("eml",false) != 0))
    item->setPixmap(0,SmallIcon("message"));
  else
  if((extension.contains("php",false) != 0))
    item->setPixmap(0,SmallIcon("source_php"));
  else
  if((extension.contains("pl",false) != 0))
    item->setPixmap(0,SmallIcon("source_pl"));
  else
  if((extension.contains("tex",false) != 0))
    item->setPixmap(0,SmallIcon("tex"));
  else
  if((extension.contains("moc",false) != 0))
    item->setPixmap(0,SmallIcon("source_moc"));
  else
  if((extension.contains("log",false) != 0))
    item->setPixmap(0,SmallIcon("log"));
  else
    {
      if(baseName.contains("makefile",false) != 0)
        item->setPixmap(0,SmallIcon("make"));
      else
      if(baseName.contains("configure",false) != 0)
        item->setPixmap(0,SmallIcon("shellscript"));
      else
      if(baseName.contains("readme",false) != 0)
        item->setPixmap(0,SmallIcon("txt"));
    }
}
