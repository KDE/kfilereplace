/***************************************************************************
                           kfilereplacelib.h  -  File library, derived from filelib.h
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

#ifndef KFILEREPLACELIB_H
#define KFILEREPLACELIB_H
/*
// Standard includes
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h> */

// KDE
#include <klocale.h>
class KlistView;

// local
#include "configurationclasses.h"

class KFileReplaceLib
{
  public:
    KFileReplaceLib() {}

  public:
 
    /**
    Format a path, from a path and a filename, or another sub-path (avoid double '/' risks)
    Parameters::.....* basePath: fist path (can be "/" if root, or "/usr/bin/" or "/usr/bin" for example)
    .................* filename: second path (can be "/doc/html/", or "doc/html/" or "doc/html/index.html" for example)
    Return values:...* Full valid path (without double "/")
    */ 
    static QString formatFullPath(const QString& basePath, const QString& fileName);

    /**
    Add an extension to a filename, or a filepath
    Parameters::.....* filename: filename or filepath (it can have already the extension)
    .................* extension: extension to add without "." (ex: "html", "kfr")
    Return values:...* Filename / Filepath with the extension
    */
    static QString addExtension(const QString& fileName, const QString& extension);

    static QString formatFileSize(double size);

    /**
    converts the old kfr format file in the new xml-based format.
    */
    static void convertOldToNewKFRFormat(const QString& fileName, QListView* stringView);
    /**
    Verifies that files, which we are scanning, respect some
    conditions
    */
    static bool isAnAccessibleFile(const QString& filePath, const QString& fileName, const RCOptions& info);

    static void setIconForFileEntry(QListViewItem* item, QString path);
};
#endif // KFILEREPLACEFILELIB_H
