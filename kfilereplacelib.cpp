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


#include "kfilereplacelib.h"
//KDE
#include <errno.h>
#include <kdebug.h>
#include <kuser.h>
#include <krandomsequence.h>

/**
 Create the text with a size in Bytes, KiloBytes, MegaBytes, GigaBytes, TeraBytes from a 64 bits number
 Parameters::.....* qwSize: 64 bits number of the size in bytes
 Return values:...* formatted text size
*/
QString KFileReplaceLib::formatSize(QWORD qwSize)
{
  QString size;
  double d;

  QWORD llKiloB = ((QWORD) 1024);
  QWORD llMegaB = ((QWORD) 1024) * ((QWORD) 1024);
  QWORD llGigaB = ((QWORD) 1024) * ((QWORD) 1024) * ((QWORD) 1024);
  QWORD llTeraB = ((QWORD) 1024) * ((QWORD) 1024) * ((QWORD) 1024) * ((QWORD) 1024);

  if (qwSize < llKiloB) // In Bytes
    {
      size = i18n("%n byte", "%n bytes", (unsigned long)qwSize);
    }
  else if (qwSize < llMegaB) // In KiloBytes
    {
      d = ((double) qwSize) / ((double) 1024.0);
      size.sprintf(i18n("%.2f KB"), (float) d);
    }
  else if (qwSize < llGigaB) // In MegaBytes
    {
      d = ((double) qwSize) / ((double) (1024.0 * 1024.0));
      size.sprintf(i18n("%.2f MB"), (float) d);
    }
  else if (qwSize < llTeraB)// In GigaBytes
    {
      d = ((double) qwSize) / ((double) (1024.0 * 1024.0 * 1024.0));
      size.sprintf(i18n("%.2f GB"), (float) d);
    }
  else // In TeraBytes
    {
      d = ((double) qwSize) / ((double) (1024.0 * 1024.0 * 1024.0 * 1024.0));
      size.sprintf(i18n("%.2f TB"), (float) d);
    }

  return size;
}

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
QString KFileReplaceLib::addFilenameExtension(const QString& fileName, const QString& extension)
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


int KFileReplaceLib::diskSpace()
{kdDebug(23000)<<"---KFILEREPLACE::DISKSPACE\n";
  return m_ds.diskSpace();  
}

QString KFileReplaceLib::formatFileSize(uint size)
{
  QString stringSize;
    
  if(size < 1000)
    stringSize = QString::number(size,10) + " bytes";
  else
  if(size >= 1000 and size < 999999)
    {
      uint d = size / 1024;
      stringSize = QString::number(d,10) + " KB";
    }
  else
  if(size >= 1000000)
    {
      uint d = size / 1048576;
      stringSize = QString::number(d,10) + " MB";
    } 
    
  return stringSize; 
}

int DiskSpace::diskSpace()
{
  kdDebug(23000)<<"---DISKSPACE\n";
  KDiskFreeSp* fs = new KDiskFreeSp();
  connect(fs, 
          SIGNAL(foundMountPoint (const QString &, unsigned long, unsigned long , unsigned long)),
          this,
          SLOT(slotDiskUsage(const QString &, unsigned long, unsigned long, unsigned long)));
          
  int e = fs->readDF(".");
  delete fs;
  if( e == 1 ) return 0;
  else return e; 
}

void DiskSpace::slotDiskUsage(const QString &mountPoint, unsigned long kBSize, unsigned long kBUsed, unsigned long kBAvail)
{kdDebug(23000)<<"---SLOTDISKUSAGE\n";
  m_kBSize = kBSize;
  m_kBUsed = kBUsed;
  m_kBAvail = kBAvail;
}

QString KFileReplaceLib::datetime(const QString& value) 
{ 
  if(value == "iso")
    return QDateTime::currentDateTime(Qt::LocalTime).toString(Qt::ISODate);
  if(value == "local")
    return QDateTime::currentDateTime(Qt::LocalTime).toString(Qt::LocalDate);
  return QString::null;
}

QString KFileReplaceLib::user(const QString& value) 
{ 
  KUser u;
  if(value == "uid")
    return QString::number(u.uid(),10);
  if(value == "gid")
    return QString::number(u.gid(),10);
  if(value == "loginname")
    return u.loginName();
  if(value == "fullname")
    return u.fullName();
  if(value == "homedir")
    return u.homeDir();
  if(value == "shell")
    return u.shell();
  return QString::null;
}

QString KFileReplaceLib::loadfile(const QString& value) 
{ 
  QFile f(value);
  if(!f.open(IO_ReadOnly)) return QString::null;
  
  QTextStream t(&f);
  
  QString s = t.read();
  
  f.close();
  
  return s;
  
}

QString KFileReplaceLib::empty(const QString& value) 
{ 
  Q_UNUSED(value);
  return "";  
}

QString KFileReplaceLib::mathexp(const QString& value)
{ 
  return "";  
}

QString KFileReplaceLib::random(const QString& value)
{ 
  QDateTime dt;
  KRandomSequence seq(dt.toTime_t());
  return QString::number(seq.getLong(1000000),10);  
}
#include "kfilereplacelib.moc"

