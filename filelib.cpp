/***************************************************************************
                           filelib.cpp  -  File library
                                      -------------------
    begin                : lun mai  3 20:19:52 CEST 1999

    copyright            : (C) 1999 by François Dupoux
                           (C) 20003 Andras Mantia <amantia@freemail.hu>
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

#include <sys/vfs.h>
#include <errno.h>

#include "filelib.h"


// ========================================================================================
// Create the text with a size in Bytes, KiloBytes, MegaBytes, GigaBytes, TeraBytes from a 64 bits number
// Parameters::.....* qwSize: 64 bits number of the size in bytes
// Return values:...* formatted text size
// ========================================================================================
QString formatSize(QWORD qwSize)
{
  QString strSize;
  double dSize;

  QWORD llKiloB = ((QWORD) 1024);
  QWORD llMegaB = ((QWORD) 1024) * ((QWORD) 1024);
  QWORD llGigaB = ((QWORD) 1024) * ((QWORD) 1024) * ((QWORD) 1024);
  QWORD llTeraB = ((QWORD) 1024) * ((QWORD) 1024) * ((QWORD) 1024) * ((QWORD) 1024);

  if (qwSize < llKiloB) // In Bytes
    {
      strSize = i18n("1 byte", "%1 bytes").arg(qwSize);
    }
  else if (qwSize < llMegaB) // In KiloBytes
    {
      dSize = ((double) qwSize) / ((double) 1024.0);
      strSize.sprintf(i18n("%.2f KB"), (float) dSize);
    }
  else if (qwSize < llGigaB) // In MegaBytes
    {
      dSize = ((double) qwSize) / ((double) (1024.0 * 1024.0));
      strSize.sprintf(i18n("%.2f MB"), (float) dSize);
    }
  else if (qwSize < llTeraB)// In GigaBytes
    {
      dSize = ((double) qwSize) / ((double) (1024.0 * 1024.0 * 1024.0));
      strSize.sprintf(i18n("%.2f GB"), (float) dSize);
    }
  else // In TeraBytes
    {
      dSize = ((double) qwSize) / ((double) (1024.0 * 1024.0 * 1024.0 * 1024.0));
      strSize.sprintf(i18n("%.2f TB"), (float) dSize);
    }

  return strSize;
}

// ========================================================================================
// Format a path, from a path and a filename, or another sub-path (avoid double '/' risks)
// Parameters::.....* szBasePath: fist path (can be "/" if root, or "/usr/bin/" or "/usr/bin" for example)
// .................* szFilename: second path (can be "/doc/html/", or "doc/html/" or "doc/html/index.html" for example)
// Return values:...* Full valid path (without double "/")
// ========================================================================================
QString formatFullPath(const QString& szBasePath, const QString &szFilename)
{
  QString strFullPath = szBasePath;
  QString fileName = szFilename;

  if (fileName[0] == '/') // skip beginning '/'
    fileName = fileName.mid(1);

  if (szBasePath.right(1) == "/")
    strFullPath.append(fileName);
  else
  {
    strFullPath.append('/');
    strFullPath.append(fileName);
  }
  return strFullPath;
}

// ========================================================================================
// Add an extension to a filename, or a filepath
// Parameters::.....* strFilename: filename or filepath (it can have already the extension)
// .................* szExtension: extension to add without "." (ex: "html", "kfr")
// Return values:...* Filename / Filepath with the extension
// ========================================================================================
QString addFilenameExtension(const QString& strFilename, const QString& szExtension)
{
  QString strFullExtension;
  QString fileName = strFilename;

  strFullExtension = ".";
  strFullExtension.append(szExtension);

  // filename cannot contain ".ext" ==> Add it
  if(fileName.length() <= strFullExtension.length())
    {
      fileName.append(strFullExtension);
    }
  else // filename can contain ".ext"
    {
      if (fileName.right(strFullExtension.length()) != strFullExtension)
        fileName.append(strFullExtension);
    }

  return fileName;
}


