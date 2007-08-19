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
#include <q3listview.h>
#include <qfileinfo.h>
#include <qpixmap.h>

//KDE
#include <kdebug.h>
#include <kmessagebox.h>
#include <k3listview.h>
#include <kiconloader.h>

//needed for malloc/free
#include <stdlib.h>

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

  if (fname.startsWith('/')) // skip beginning '/'
    fname = fname.remove(0,1);

  if (fullPath.endsWith('/'))
    fullPath.append(fname);
  else
    fullPath.append('/'+fname);

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
    {
      const int asInt = (int) size;
      stringSize = i18np("1 byte", "%1 bytes", asInt);
    }
  else
  if(size >= kilo && size < mega)
    {
      double d = size / kilo;
      stringSize = ki18n("%1 KB").subs(d,0,'f',2).toString();
    }
  else
  if(size >= mega && size < giga)
    {
      double d = size / mega;
      stringSize = ki18n("%1 MB").subs(d,0,'f',2).toString();
    }
  else
  if(size >= giga)
    {
      double d = size / giga;
      stringSize =  ki18n("%1 GB").subs(d,0,'f',2).toString();
    }
  return stringSize;
}

void KFileReplaceLib::convertOldToNewKFRFormat(const QString& fileName, K3ListView* stringView)
{
 //this method convert old format in new XML-based format
 typedef struct
 {
   char pgm[13]; // Must be "KFileReplace" : like MZ for EXE files
   int stringNumber; // Number of strings in file
   char reserved[64]; // Reserved for future use
 } KFRHeader;

 KFRHeader head;

 FILE* f = fopen(QFile::encodeName(fileName),"rb");
 int err = fread(&head, sizeof(KFRHeader), 1, f);
 QString pgm(head.pgm);

 if(!f || (err != 1) || (pgm != "KFileReplace"))
 {
   KMessageBox::error(0, i18n("<qt>Cannot open the file <b>%1</b> and load the string list. This file seems not to be a valid old kfr file or it is broken.</qt>", fileName));
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
        KMessageBox::error(0, i18n("Cannot read data."));
      else
        {
          stringSize = ((oldTextSize > newTextSize) ? oldTextSize : newTextSize) + 2;
          char* oldString = (char*) malloc(stringSize+10),
              * newString = (char*) malloc(stringSize+10);
          if (oldString == 0 || newString == 0)
            KMessageBox::error(0, i18n("Out of memory."));
          else
            {
              memset(oldString, 0, stringSize);
              memset(newString,0, stringSize);

              if (fread(oldString, oldTextSize, 1, f) != 1)
                KMessageBox::error(0, i18n("Cannot read data."));
              else
                {
                  if (newTextSize > 0) // If there is a Replace text
                    {
                      if (fread(newString, newTextSize, 1, f) != 1)
                        KMessageBox::error(0, i18n("Cannot read data."));
                      else
                        {
                          Q3ListViewItem* lvi = new Q3ListViewItem(stringView);
                          lvi->setText(0,oldString);
                          lvi->setText(1,newString);

                       }
                    }
                }
              }
          if(newString)
            free(newString);
          if(oldString)
            free(oldString);
        }
    }
    fclose(f);
    return ;
 }

bool KFileReplaceLib::isAnAccessibleFile(const QString& filePath, const QString& fileName, RCOptions* info)
{
  QString bkExt = info->m_backupExtension;
  if(fileName == ".." || fileName == "." || (!bkExt.isEmpty() && fileName.right(bkExt.length()) == bkExt))
    return false;

  QFileInfo fi;
  if(filePath.isEmpty())
    fi.setFile(fileName);
  else
    fi.setFile(filePath+'/'+fileName);

  if(fi.isDir())
    return true;

   int minSize = info->m_minSize,
       maxSize = info->m_maxSize;
   QString minDate = info->m_minDate,
           maxDate = info->m_maxDate,
           dateAccess = info->m_dateAccess;

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
  if(info->m_ownerUserIsChecked)
    {
      QString fileOwnerUser;
      if(info->m_ownerUserType == "Name")
        fileOwnerUser = fi.owner();
      else
        fileOwnerUser = QString::number(fi.ownerId(),10);

      if(info->m_ownerUserBool == "Equals To")
        {
          if(info->m_ownerUserValue != fileOwnerUser)
            return false;
        }
      else
        {
          if(info->m_ownerUserValue == fileOwnerUser)
            return false;
        }
    }

  if(info->m_ownerGroupIsChecked)
    {
      QString fileOwnerGroup;
      if(info->m_ownerGroupType == "Name")
        fileOwnerGroup = fi.group();
      else
        fileOwnerGroup = QString::number(fi.groupId(),10);
      if(info->m_ownerGroupBool == "Equals To")
        {
          if(info->m_ownerGroupValue != fileOwnerGroup)
            return false;
        }
      else
        {
          if(info->m_ownerGroupValue == fileOwnerGroup)
            return false;
        }
    }

  //If we are here then all requirements have been verified
  return true;
}

void KFileReplaceLib::setIconForFileEntry(Q3ListViewItem* item, const QString &path)
{
  QFileInfo fi(path);
  QString extension = fi.extension(),
          baseName = fi.baseName();

  KeyValueMap extensionMap;

  extensionMap["a"] = "binary";
  extensionMap["am"] = "shellscript";
  extensionMap["bz"] = "zip";
  extensionMap["bz2"] = "zip";
  extensionMap["c"] = "source_c";
  extensionMap["cc"] = "source_cpp";
  extensionMap["cpp"] = "source_cpp";
  extensionMap["eml"] = "message";
  extensionMap["exe"] = "exec_wine";
  extensionMap["gz"] = "zip";
  extensionMap["h"] = "source_h";
  extensionMap["htm"] = "html";
  extensionMap["html"] = "html";
  extensionMap["in"] = "shellscript";
  extensionMap["java"] = "source_java";
  extensionMap["jpg"] = "image";
  extensionMap["kfr"] = "html";
  extensionMap["kmdr"] = "widget_doc";
  extensionMap["kwd"] = "kword_kwd";
  extensionMap["log"] = "log";
  extensionMap["moc"] = "source_moc";
  extensionMap["mp3"] = "sound";
  extensionMap["o"] = "source_o";
  extensionMap["pdf"] = "pdf";
  extensionMap["php"] = "source_php";
  extensionMap["py"] = "source_py";
  extensionMap["pl"] = "source_pl";
  extensionMap["p"] = "source_p";
  extensionMap["ps"] = "postscript";
  extensionMap["png"] = "image";
  extensionMap["sa"] = "binary";
  extensionMap["sh"] = "shellscript";
  extensionMap["so"] = "binary";
  extensionMap["tar"] = "tar";
  extensionMap["tex"] = "tex";
  extensionMap["tgz"] = "tgz";
  extensionMap["txt"] = "txt";
  extensionMap["ui"] = "widget_doc";
  extensionMap["uml"] = "umbrellofile";
  extensionMap["wav"] = "sound";
  extensionMap["xml"] = "html";
  extensionMap["xpm"] = "image";

  KeyValueMap::Iterator itExtensionMap;

  for(itExtensionMap = extensionMap.begin(); itExtensionMap != extensionMap.end(); ++itExtensionMap)
    {
      if(extension == itExtensionMap.key())
        {
          item->setPixmap(0,SmallIcon(itExtensionMap.data()));
          return;
        }
    }

  KeyValueMap baseNameMap;

  baseNameMap["configure"] = "shellscript";
  baseNameMap["core"] = "core";
  baseNameMap["makefile"] = "make";
  baseNameMap["readme"] = "readme";
  baseNameMap["README"] = "readme";
  baseNameMap["Readme"] = "readme";
  baseNameMap["TODO"] = "txt";

  KeyValueMap::Iterator itBaseNameMap;

  for(itBaseNameMap = baseNameMap.begin(); itBaseNameMap != baseNameMap.end(); ++itBaseNameMap)
    {
      if(baseName == itBaseNameMap.key())
        {
          item->setPixmap(0,SmallIcon(itBaseNameMap.data()));
          return;
        }
    }
}
