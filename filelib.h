/***************************************************************************
                           filelib.h  -  File library
                                      -------------------                     
    begin                : lun mai  3 20:19:52 CEST 1999
                                           
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

#ifndef FILELIB_H
#define FILELIB_H

typedef unsigned long long int QWORD;	// 64 bits
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
// Return the name of the file, not the path, then only file name + extension
// Parameters::.....* szName: pointer (char *) to the destiation filename
//                  * szFullPath: pointer (char *) to the full path of the file
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int getFileName(char *szName, const char *szFullPath);

// ========================================================================================
// Return the path of the file, not the name, then for example: "/root/file.txt" --> "/root"
// Parameters::.....* szPath: pointer (char *) to the destiation filepath
//                  * szFullPath: pointer (char *) to the full path of the file
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int getFilePath(char *szPath, const char *szFullPath);

// ========================================================================================
// Return the CRC of a file
// Parameters::.....* szFilename: Path of the file we need the CRC
//                  * nFileCrc: Adresse of the Int where the CRC must be written
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int calculateCRCOfFile(const char *szFilename, QWORD *qwFileCRC);

// ========================================================================================
// Create all directories of the path szfullPath
// Parameters::.....* szDirectory: pointer to the filepath of the directory to create
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int createAllDirsOfPath(const char *szDirectory);

// ========================================================================================
// Convert a binary permissions number into a string
// Parameters::.....* perm: permissions in binary format to convert
// Return values:...* string of the permissions
// ========================================================================================
QString parsePermissions(mode_t perm);

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
QString formatFullPath(const char *szBasePath, const char *szFilename);

// ========================================================================================
// Add an extension to a filename, or a filepath
// Parameters::.....* strFilename: filename or filepath (it can have already the extension)
// .................* szExtension: extension to add without "." (ex: "html", "kfr")
// Return values:...* Filename / Filepath with the extension
// ========================================================================================
QString addFilenameExtension(QString strFilename, const char *szExtension);

// ========================================================================================
// retrun the free space on disk where the file szFilename is written
// Parameters::.....* szFilename: path of the file
// .................* qwAvailDiskSpace: pointer to the 64 bits number where the free disk space must be written
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int getDiskFreeSpaceForFile(QWORD *qwAvailDiskSpace, const char *szFilename);

#endif // FILELIB_H



