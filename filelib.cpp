/***************************************************************************
                           filelib.cpp  -  File library
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

#include <sys/vfs.h>
#include <errno.h>

#include "filelib.h"

// ========================================================================================
// Return the name of the file, not the path, then only file name + extension
// Parameters::.....* szName: pointer (char *) to the destiation filename
//                  * szFullPath: pointer (char *) to the full path of the file
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int getFileName(char *szName, const char *szFullPath)
{
  int i, j;
  
  szName[0] = 0; // End of the string
  
  if ( strstr(szFullPath, "/") == NULL) // If the is no '/' in the path: only copy the string
    {
      strcpy(szName, szFullPath);
      return 0;
    }
  
  int len = strlen(szFullPath);
  
  for(i=len; szFullPath[i] != '/' && i != 0; i--); // Begin of name = char[i]
  i++; // Go after the '/'	
  
  for(j=i; szFullPath[j] ; j++) // Copy the file name to szName
    szName[j-i] = szFullPath[j];
  
  szName[j-i] = 0;
  
  return 0;
}


// ========================================================================================
// Return the path of the file, not the name, then for example: "/root/file.txt" --> "/root"
// Parameters::.....* szPath: pointer (char *) to the destiation filepath
//                  * szFullPath: pointer (char *) to the full path of the file
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int getFilePath(char *szPath, const char *szFullPath)
{	
  int i, j;
  
  szPath[0] = 0; // End of the string
  
  if ( strstr(szFullPath, "/") == NULL) // If the is no '/' in the path: only copy the string
    {
      return -1;
    }
  
  int len = strlen(szFullPath);
  
  for(i=len; szFullPath[i] != '/' && i != 0; i--); // Begin of name = char[i]
  
  for(j=0; j < i ; j++) // Copy the file name to szName
    szPath[j] = szFullPath[j];
  
  szPath[j] = 0; // End of the string
  
  return 0;
}

// ========================================================================================
// Return the CRC of a file
// Parameters::.....* szFilename: Path of the file we need the CRC
//                  * nFileCrc: Adresse of the Int where the CRC must be written
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int calculateCRCOfFile(const char *szFilename, QWORD *qwFileCRC)
{
  QFile f;
  bool bRes;
  
  f.setName(szFilename);
  
  bRes = f.open(IO_ReadOnly);
  if (bRes == false)
    return -1; // Error
  
  QWORD qwCRC = 0LL;
  uint i;
  int car;
  
  for (i = 0; i < f.size(); i++)
    {
      car = f.getch();
      qwCRC += (QWORD) car;
    }
  
  *qwFileCRC = qwCRC;
  return 0; // Success
}

// ========================================================================================
// Create all directories of the path szfullPath
// Parameters::.....* szDirectory: pointer to the filepath of the directory to create
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int createAllDirsOfPath(const char *szDirectory)
{
  char *ptr;
  char szPath[MAXPATHLEN];
  QDir qDir;
  
  strcpy(szPath, szDirectory);
  ptr = szPath; // points to start of szDirectory
  
  while (*ptr)
    {
      if (*ptr == '/')
	{
	  *ptr = 0;
	  
	  // CreateDirectory if it doesn't exists
	  if (*szPath && !qDir.exists(szPath, true))
	    qDir.mkdir (szPath, true);
	  
	  *ptr = '/';  // Restore the string
	}
      
      ptr++; // Caractère suivant
    }
  
  // CreateDirectory if it doesn't exists
  if (!qDir.exists(szDirectory, true))
    {
      qDir.mkdir (szDirectory, true);
    }
  
  return 0;
}

// ========================================================================================
// Convert a binary permissions number into a string
// Parameters::.....* perm: permissions in binary format to convert
// Return values:...* string of the permissions
// ========================================================================================
QString parsePermissions(mode_t perm)// taken from "kdelibs/kfile/kfileviewitem.cpp"
{
  char szPermissions[] = "----------";
  
  if (perm & QFileInfo::ReadUser)
    szPermissions[1]='r';
  if (perm & QFileInfo::WriteUser)
    szPermissions[2]='w';
  if (perm & QFileInfo::ExeUser)
    szPermissions[3]='x';
  
  if (perm & QFileInfo::ReadGroup)
    szPermissions[4]='r';
  if (perm & QFileInfo::WriteGroup)
    szPermissions[5]='w';
  if (perm & QFileInfo::ExeGroup)
    szPermissions[6]='x';
  
  if (perm & QFileInfo::ReadOther)
    szPermissions[7]='r';
  if (perm & QFileInfo::WriteOther)
    szPermissions[8]='w';
  if (perm & QFileInfo::ExeOther)
    szPermissions[9]='x';
  
  return QString::fromLatin1(szPermissions);
}

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
      strSize.sprintf(i18n("%ld bytes"), qwSize);
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
QString formatFullPath(const char *szBasePath, const char *szFilename)
{
  QString strFullPath;
  
  if (*szFilename == '/') // skip beginning '/'
    szFilename++;
  
  if (QString(szBasePath).right(1) == "/")
    strFullPath.sprintf("%s%s", szBasePath, szFilename);
  else
    strFullPath.sprintf("%s/%s", szBasePath, szFilename);
  
  return strFullPath;
}

// ========================================================================================
// Add an extension to a filename, or a filepath
// Parameters::.....* strFilename: filename or filepath (it can have already the extension)
// .................* szExtension: extension to add without "." (ex: "html", "kfr")
// Return values:...* Filename / Filepath with the extension
// ========================================================================================
QString addFilenameExtension(QString strFilename, const char *szExtension)
{
  QString strFullExtension;
  
  strFullExtension = QString(".") + QString(szExtension);
  
  // filename cannot contain ".ext" ==> Add it
  if(strFilename.length() <= strFullExtension.length())
    {
      strFilename += strFullExtension;
    }
  else // filename can contain ".ext"
    {	
      if (strFilename.right(strFullExtension.length()) != strFullExtension)
	strFilename += strFullExtension;
    }
  
  return strFilename;	
}

// ========================================================================================
// retrun the free space on disk where the file szFilename is written
// Parameters::.....* szFilename: path of the file
// .................* qwAvailDiskSpace: pointer to the 64 bits number where the free disk space must be written
// Return values:...*  0 if success
//                  * -1 if error
// ========================================================================================
int getDiskFreeSpaceForFile(QWORD *qwAvailDiskSpace, const char *szFilename)
{
  int nRes;
  struct statfs fsInfo;
  
  *qwAvailDiskSpace = 0LL;
  
  errno = 0;
  nRes = statfs(szFilename, &fsInfo);
  if (nRes == -1) // error
    {
      if (errno == ENOENT) // file doesn't exists
	{	
	  // create an empty file
	  FILE *fFile;
	  fFile = fopen(szFilename, "w");
	  if (fFile == NULL)
	    return -1;
	  fclose(fFile);
	  nRes = getDiskFreeSpaceForFile(qwAvailDiskSpace, szFilename);
	  unlink(szFilename);
	  return nRes;
	}
      else // another error
	{
	  return -1;
	}
      
    }	
  
  *qwAvailDiskSpace = ((QWORD) fsInfo.f_bavail) * ((QWORD)fsInfo.f_bsize);
  
  return 0;
}


