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

// Standard includes
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>

#include <qobject.h>
#include <qdir.h>
#include <qfileinfo.h>

#include <klocale.h>
#include <kdiskfreesp.h> 

typedef unsigned long long int QWORD; // 64 bits
typedef unsigned long int DWORD; // 32 bits
typedef unsigned short UINT; // 16 bits

class DiskSpace: public QObject
{
  Q_OBJECT
  public slots:
    void slotDiskUsage(const QString &mountPoint, unsigned long kBSize, unsigned long kBUsed, unsigned long kBAvail);
    
  public:
    unsigned long kBSize() const { return m_kBSize; }
    unsigned long kBUsed() const { return m_kBUsed; }
    unsigned long kBAvail() const { return m_kBAvail; }
    int diskSpace();
        
  private:
    unsigned long m_kBSize,
                  m_kBUsed,
                  m_kBAvail;
};

class KFileReplaceLib
{
  static KFileReplaceLib frl;
  private:
    KFileReplaceLib() {};
    KFileReplaceLib(const KFileReplaceLib&) {}
    KFileReplaceLib& operator=(const KFileReplaceLib& f) { return (*this); }
   
  public:
    static KFileReplaceLib& instance() { return frl; }
    /**
    Create the text with a size in Bytes, KiloBytes, MegaBytes, GigaBytes, TeraBytes from a 64 bits number
    Parameters::.....* qwSize: 64 bits number of the size in bytes
    Return values:...* formatted text size
    */
    static QString formatSize(QWORD qwSize);

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
    static QString addFilenameExtension(const QString& fileName, const QString& extension);
    unsigned long kBSize() { return m_kBSize; }
    unsigned long kBUsed() { return m_kBUsed; }
    unsigned long kBAvail() { return m_kBAvail; }
    int diskSpace();
    
    static QString formatFileSize(uint size);
    
    static QString datetime(const QString& value) ;
    static QString user(const QString& value) ;
    static QString loadfile(const QString& value) ;
    static QString empty(const QString& value);
    static QString mathexp(const QString& value);
    static QString random(const QString& value);
  private:
    unsigned long m_kBSize,
                  m_kBUsed,
                  m_kBAvail;
    DiskSpace m_ds;
};

#endif // KFILEREPLACEFILELIB_H



