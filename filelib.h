/***************************************************************************
                           filelib.h  -  File library
                                      -------------------
    begin                : lun mai  3 20:19:52 CEST 1999

    copyright            : (C) 1999 by François Dupoux                                              (C) 2003 Andras Mantia <amantia@freemail.hu>
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

#ifndef FILELIB_H
#define FILELIB_H

typedef unsigned long long int QWORD;        // 64 bits
typedef unsigned long int DWORD; // 32 bits
typedef unsigned short UINT; // 16 bits

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>

#include <qdir.h>
#include <qfileinfo.h>

#include <klocale.h>

// ========================================================================================
// Create the text with a size in Bytes, KiloBytes, MegaBytes, GigaBytes, TeraBytes from a 64 bits number
// Parameters::.....* qwSize: 64 bits number of the size in bytes
// Return values:...* formatted text size
// ========================================================================================
QString formatSize(QWORD qwSize);

// ========================================================================================
// Format a path, from a path and a filename, or another sub-path (avoid double '/' risks)
// Parameters::.....* szBasePath: fist path (can be "/" if root, or "/usr/bin/" or "/usr/bin" for example)
// .................* szFilename: second path (can be "/doc/html/", or "doc/html/" or "doc/html/index.html" for example)
// Return values:...* Full valid path (without double "/")
// ========================================================================================
QString formatFullPath(const QString& szBasePath, const QString& szFilename);

// ========================================================================================
// Add an extension to a filename, or a filepath
// Parameters::.....* strFilename: filename or filepath (it can have already the extension)
// .................* szExtension: extension to add without "." (ex: "html", "kfr")
// Return values:...* Filename / Filepath with the extension
// ========================================================================================
QString addFilenameExtension(const QString& strFilename, const QString& szExtension);

#endif // FILELIB_H



