/***************************************************************************
                          kernel.cpp  -  description
                             -------------------
    begin                : Sat Sep 25 1999
    copyright            : (C) 1999 by François Dupoux
                           (C) 2003 Andras Mantia <amantia@kde.org>
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

// app includes
#include "apistruct.h"
#include "kexpression.h"
#include "apistruct.h"
#include "kernel.h"
#include "resource.h"
#include "kfilereplacedoc.h"
#include "kfilereplaceview.h"
#include "kfilereplacepart.h"
#include "klistviewstring.h"
#include "kconfirmdlg.h"
#include "filelib.h"

// KDE includes
#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>

// Qt includes
#include <qdir.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qregexp.h>

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/param.h>
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#define STATFS statvfs
#else
#define STATFS statfs
#endif
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

#define min(A,B) (A < B ? A : B)

// ===========================================================================================================================
void *ReplaceThread(void *param)
{
  int nRes;
  RepDirArg *argu;
  argu = (RepDirArg *) param;

  g_bThreadRunning = true;

  kdDebug(23000) << "Starting ReplaceDirectory..." << endl;
  // Call another function to make easier to verify Thread Variables
  nRes = ReplaceDirectory(argu -> szDir, argu, true); // true --> replace
  kdDebug(23000) << "ReplaceDirectory exited..." << endl;

  // The thread always finished here: success or error
  g_nFilesRep = nRes; // Number of replaced files
  g_bThreadRunning = false;

  /*if (nRes == -1) // Error
    {        pthread_exit(-1);
    return 0;
    }
    else // Success
    {        pthread_exit(nRes);
    return 0;
    } */

  return 0;
}

// ===========================================================================================================================
void *SearchThread(void *param)
{

  int nRes;
  RepDirArg *argu;
  argu = (RepDirArg *) param;

  g_bThreadRunning = true;

  // Call another function to make easier to verify Thread Variables
  nRes = ReplaceDirectory(argu -> szDir, argu, false); // false --> search

  // The thread always finished here: success or error
  g_nFilesRep = nRes; // Number of replaced files
  g_bThreadRunning = false;

  /*if (nRes == -1) // Error
    {        pthread_exit(-1);
    return 0;
    }
    else // Success
    {        pthread_exit(nRes);
    return 0;
    } */

  return 0;
}

// ===========================================================================================================================
int ReplaceDirectory(const QString& szDir, RepDirArg* argu, bool bReplace)
{

  QString strFileReadpath;
  QString strFileWritepath;
  QString strDirpath;
  QString strBackup;
  QDir dir;
  QFileInfo fi, fiOld, fiNew;
  int nRes;
  int i;
  int nNbRepFiles = 0;
  int nNeedReplace; // Not NULL if the file need to be replaced
  int nNbReplacements; // Nb Rep made in a call to ReplaceFile
  uint nDiskFreeSpace;
  int nConfirm = 0;
  KListViewString *lvi;
  QString strTemp;
  QString strMess;
  bool bAllStringsFound;
  int nFlags;

  //KFileReplaceApp *appKFR;
  //appKFR = (KFileReplaceApp *) (argu -> mainwnd);

  nFlags = QDir::Files | QDir::Readable | QDir::NoSymLinks;
  if (argu -> bIgnoreHidden == false)
    nFlags |= QDir::Hidden;

  // What type of files we must lis
  dir.setFilter(nFlags);
  dir.setPath(szDir);
  dir.setNameFilter(argu -> szFilter);

  // 0. -*-*-*-*-*-*-*-*-*- Check it's a valid directory -*-*-*-*-*-*-*-*-*-
  if (!dir.isReadable() || !dir.exists())
    {
      g_szErrMsg = i18n("<qt>Can't access directory <b>%1</b>.").arg(szDir);
      return -1;
    }

  // 1. -*-*-*-*-*-*-*-*-*- First, list all files -*-*-*-*-*-*-*-*-*-

  // If directory doesn't exists
  if (!dir.exists())
    {
      //sprintf (g_szErrMsg, i18n("Directory %s doesn't exists"), szDir);
      return -1;
    }

  for (i=0; i < (int)dir.count(); i++)
    {
      // Process event to make the GUI updated
      kapp->processEvents();

      // Check the Thread needn't to stop
      if (g_bThreadMustStop == true)
        {
          kdDebug(23000) << "STOP THREAD!" << endl;
          return -1;
        }

      strFileWritepath = formatFullPath(szDir, dir[i]) + "new";
      strFileReadpath = formatFullPath(szDir, dir[i]);
      fiOld.setFile(strFileReadpath);

      // if the file dates & size are correct for options
      if ((dir[i].right(4) != ".old") && HasFileGoodOwners(strFileReadpath, argu) && IsFileGoodSizeProperties(strFileReadpath, argu -> bMinSize, argu -> bMaxSize, argu -> nMinSize, argu -> nMaxSize)
          && ((argu -> bMinDate == false && argu -> bMaxDate == false) || (IsFileGoodDateProperties(strFileReadpath, argu -> nTypeOfAccess, argu -> bMinDate, argu -> bMaxDate, argu -> qdMinDate, argu -> qdMaxDate))))
        {

          // Test read access for file
          QFileInfo fi(strFileReadpath);
          if (!fi.exists() || !fi.isReadable()) // We don't have access to the file
          {
            argu -> qlvResult -> addFullItem(false, dir[i], szDir, fiOld.size(), 0, 0, g_szErrMsg);
          } else
          {
                              // ***** IF SEARCHNIG *******
              if (bReplace == false)
                {
                  // If there are strings to search
                  if (argu -> qlvStrings -> childCount() > 0)
                    {
                      // Add the item in the list, but without details
                      strTemp = formatSize(fiOld.size());
                      lvi = new KListViewString(argu -> qlvResult, dir[i], szDir, strTemp);
                      // Run the search operation
                      kdDebug(23000) << "begin CALL for SearchFile()" << endl;
                      nRes = SearchFile(lvi, strFileReadpath, &nNbReplacements, &bAllStringsFound, argu, argu->bHaltOnFirstOccur);
                      kdDebug(23000) << "end CALL for SearchFile(). nNbReplacements = " << nNbReplacements << endl;
                      if (nRes == 0)
                        {
                          // if all strings must be found, and have not be found
                          if (argu -> bAllStringsMustBeFound && !bAllStringsFound)
                            {
                              delete lvi;
                            }
                          // Update Result ListView        if found
                          else
                            {
                              if (nNbReplacements > 0)
                                {
                                  nNbRepFiles++;
                                  // nNbReplacements must be 0 if options "Stop at first occurrence" is enabled
                                  argu -> qlvResult -> updateItem(lvi, true, fiOld.size(), nNbReplacements*(argu->bHaltOnFirstOccur==false));
                                }
                              else // No strings found
                                argu -> qlvResult -> takeItem(lvi); // Remove lvi from the list
                            }

                        }
                      else // (nRes == -1)
                        {
                          // Update Result ListView        if found
                          nNbRepFiles++;
                          lvi = argu -> qlvResult -> addFullItem(false, dir[i], szDir, fiOld.size(), 0, 0, g_szErrMsg);
                          argu -> qlvResult -> updateItem(lvi, true, fiOld.size(), 0);
                        }

                    }
                  else // if there are no strigns to search
                    {
                      nNbRepFiles++;
                      lvi = argu -> qlvResult -> addFullItem(true, dir[i], szDir, fiOld.size(), fiOld.size(), 0);
                      argu -> qlvResult -> updateItem(lvi, true, fiOld.size(), 0);
                    }

                }
              else // ******* IF REPLACING *******
                {
                  // Test read access for file
                  QFileInfo fInfo(strFileReadpath);
                  if (!fInfo.exists() || !fInfo.isReadable() || !fInfo.isWritable()) // We don't have access to the file
                  {
                    g_szErrMsg = i18n("<qt>Can't access file <b>%1</b> for writing.</qt>").arg(strFileReadpath);
                    argu -> qlvResult -> addFullItem(false, dir[i], szDir, fiOld.size(), 0, 0, g_szErrMsg);
                  } else
                  {
                    kdDebug(23000) << QString("In (SearchFile) to check the file need replace (%1)").arg( strFileReadpath) << endl;
                    nRes = SearchFile(0, strFileReadpath, &nNeedReplace, &bAllStringsFound, argu, true);
                    kdDebug(23000) << QString("Out (SearchFile) to check the file need replace (%1)").arg( strFileReadpath) << endl;
                    if (nRes == -1)
                      return -1;

                    if (nNeedReplace && (!argu -> bAllStringsMustBeFound || bAllStringsFound)) // Replace only if there are occurrences
                      {
                        if ((argu -> bConfirmFiles == true) && (argu->bSimulation == false))
                          {
                            strMess = i18n("<qt>Directory: %1<br>Path: %2<br>Do you want to replace strings inside <b>%3</b> ?</qt>").arg(szDir).arg(dir[i]).arg(strFileReadpath);
                            nConfirm = KMessageBox::questionYesNo(argu->mainwnd, strMess, i18n("Replace file confirmation"));
                          }

                        if ((argu -> bConfirmFiles == false || nConfirm == KMessageBox::Yes) || (argu->bSimulation == true)) // if we must replace in this file
                          {
                            // Check there is enough free disk space
                            if (argu->bSimulation == false) // if not a simulation
                              {
                                nRes = GetDiskFreeSpaceForFile(&nDiskFreeSpace, strFileReadpath);
                                if (nRes != -1 && nDiskFreeSpace < fiOld.size())
                                  {
                                    g_szErrMsg = i18n("<qt>There is not enough disk free space to replace in the file <b>%1</b>.</qt>").arg(strFileReadpath);
                                    return -1;
                                  }
                              }

                            // Add the item in the list, but without details
                            strTemp = formatSize(fiOld.size());
                            lvi = new KListViewString(argu -> qlvResult, dir[i], szDir, strTemp);
                            if (lvi == 0) // Not enough memory
                              return -1;

                            // Run the replace operation
                            kdDebug(23000) << "In ReplaceFile " << strFileReadpath << endl;
                            nRes = ReplaceFile(lvi, szDir, strFileReadpath, strFileWritepath, &nNbReplacements, argu);
                            kdDebug(23000) << "Out ReplaceFile " << strFileReadpath << endl;

                            if (nRes == REPLACE_SUCCESS || nRes == REPLACE_FILESKIPPED || nRes == REPLACE_SKIPDIR) // If success
                              {
                                nNbRepFiles++;

                                // Update Result ListView
                                fiNew.setFile(strFileWritepath);
                                argu -> qlvResult -> updateItem(lvi, true, fiNew.size(), nNbReplacements);


                                if (argu->bSimulation == false)
                                  {
                                    if (argu -> bBackup) // Create a backup of the file if option is true
                                      {
                                        strBackup = formatFullPath(szDir, dir[i]) + QString(".old");
                                        nRes = ::unlink(strBackup.local8Bit()); // Delete OLD file if exists
                                        nRes = ::rename(strFileReadpath.local8Bit(), strBackup.local8Bit());
                                      }
                                    else // Delete the old file
                                      {
                                        nRes = ::unlink(strFileReadpath.local8Bit());
                                      }
                                    // Rename the new file into OldFileName
                                    nRes = ::rename(strFileWritepath.local8Bit(), strFileReadpath.local8Bit());
                                  }
                                if (nRes == REPLACE_SKIPDIR) // end of ReplaceDirectory() ==> go to next dir
                                  return REPLACE_SUCCESS;
                              }
                            else // If error
                              {
                                // delete file.new
                                if (argu->bSimulation == false)
                                  ::unlink(strFileWritepath.local8Bit());

                                // update Result ListView
                                argu -> qlvResult -> updateItem(lvi, false, 0, 0, g_szErrMsg);

                                if (nRes == REPLACE_ERROR) // error in the file, but continue in current directory
                                  {
                                  }
                                else if (nRes == REPLACE_CANCEL) // end of total operation
                                  return -1;
                              }
                          }
                      }
                  }

                }
          }

        }
    }

  // 2. -*-*-*-*-*-*-*-*-*- Second, list all dir -*-*-*-*-*-*-*-*-*-

  if (argu -> bRecursive) // If we must explore sub directories
    {
      // What type of files we must find

      nFlags = QDir::Dirs | QDir::Readable | QDir::Executable;
      if (argu -> bIgnoreHidden == false)
        nFlags |= QDir::Hidden;
      if (argu -> bFollowSymLinks == false)
        nFlags |= QDir::NoSymLinks;

      dir.setPath(szDir);
      dir.setNameFilter("*");
      dir.setFilter(nFlags);

      for (i=0; i < (int)dir.count(); i++)
        {
          if (strcmp(dir[i], ".") != 0 && strcmp(dir[i], "..") != 0)
            {
              if (argu -> bConfirmDirs == true && bReplace) // If doing a replace and dir confirm activated (do not confirm when searching)
                {
                  strMess = i18n("<qt>Directory: <b>%1</b><br>Full path: <b>%2/%3</b><br><br>Do you want to replace strings in files of this directory?</qt>").arg(dir[i]).arg(szDir).arg(dir[i]);
                  nConfirm = KMessageBox::questionYesNo(argu->mainwnd, strMess, i18n("Replace directory confirmation"));
                }

              if (bReplace == false || argu -> bConfirmDirs == false || nConfirm == KMessageBox::Yes)
                {
                  strDirpath = formatFullPath(szDir, dir[i]);
                  nRes = ReplaceDirectory(strDirpath, argu, bReplace); // Use recursivity
                  if (nRes == -1) // If error
                    return -1; // Stop the operation
                  nNbRepFiles += nRes;
                }
            }
        }
    }

  return nNbRepFiles;
}

// ==================================================================================
bool IsFileGoodSizeProperties(const QString& szFileName, bool bMinSize, bool bMaxSize, uint nMinSize, uint nMaxSize)
{

  // If Minimal Size Option is Checked
  QFileInfo fi;
  fi.setFile(szFileName);

  bool bCond = (bMinSize && fi.size() < nMinSize || bMaxSize && fi.size() > nMaxSize);

  return (!bCond);
}

// ==================================================================================
bool IsFileGoodDateProperties(const QString& szFileName, int nTypeOfAccess, bool bMinDate, bool bMaxDate, QDate qdMinDate, QDate qdMaxDate)
{

  // If Minimal Size Option is Checked
  QFileInfo fi;
  fi.setFile(szFileName);
  QDate dateFiledate; // Date of the file we must to compare with dateLimit

  // Get the File Date
  if (nTypeOfAccess == 0) // Last WRITE date
    dateFiledate = fi.lastModified().date();
  if (nTypeOfAccess == 1) // Last READ date
    dateFiledate = fi.lastRead().date();

  if (bMinDate && dateFiledate < qdMinDate) // Check the Minimal Date (After ...)
    return false;

  if (bMaxDate && dateFiledate > qdMaxDate) // Check the Maximal Date (Before ...)
    return false;

  return true; // File is valid
}

// ==================================================================================
int ReplaceFile(QListViewItem *lvi, const QString &szDir, const QString& szOldFile, const QString& szNewFile, int *nNbReplacements, RepDirArg* argu)
{

  int nFdOldFile=0, nFdNewFile=0; // File descriptors
  void *vBeginOldFile;
  char *cBeginOldFile; // Pointer to the begin of the file
  char *cOldPt; // Pointer to the current data
  int nRes;
  bool bRes;
  uint nOldFileSize;
  QListViewItem *lviCurItem;
  QListViewItem *lviFirst;
  int nItemPos;
  struct stat statFile;
  KExpression kjeSearch(argu -> bCaseSensitive, argu -> bWildcards, argu -> bIgnoreWhitespaces, argu -> cWildcardsWords, argu -> cWildcardsLetters);
  int nMaxLen;
  int nRecursiveLength;
  QString strMess;
  int nConfirm=0;

  KFileReplaceApp *app;
  app = (KFileReplaceApp *) (argu -> mainwnd);

  // Items of the string list
  int nReplaceCount[MAX_STRINGSTOSEARCHREP];
  QString strOld[MAX_STRINGSTOSEARCHREP];
  QString strNew[MAX_STRINGSTOSEARCHREP];

  // 0. Init
  *nNbReplacements = 0;
  QFileInfo fiOld(szOldFile);
  nOldFileSize = fiOld.size();

  // 1. Open files
  //TODO: Replace all direct file manipulation code with KIO/QT one
  QFile oldFile(szOldFile);
  if (!oldFile.open(IO_ReadOnly))
  {
    g_szErrMsg = i18n("<qt>Can't open file <b>%1</b> for reading.</qt>").arg(szOldFile);
    return REPLACE_ERROR;
  }
  nFdOldFile = oldFile.handle();

  QFile newFile(szNewFile);
  if (argu->bSimulation == false) // if a real replace operation
    {

      if (!newFile.open(IO_ReadWrite | IO_Truncate))
        {
          g_szErrMsg = i18n("<qt>Can't open file <b>%1</b> for writing.</qt>").arg(szNewFile);
          return REPLACE_ERROR;
        }
      nFdNewFile = newFile.handle();

      // 2. Put new file the access rights of the old file
      nRes = ::fstat(nFdOldFile, &statFile);
      if (nRes == -1)
        {
          g_szErrMsg = i18n("<qt>Can't read the access rights for file<b>%1</b></qt>").arg(szOldFile);
          return REPLACE_ERROR;
        }

      nRes = ::fchmod(nFdNewFile, statFile.st_mode);
      if (nRes == -1)
        {
          g_szErrMsg = i18n("<qt>Can't set the access rights for file<b>%1</b></qt>").arg(szNewFile);
          //return REPLACE_ERROR; // make bug with files on FAT
        }
    }

  // 3. Map files
  vBeginOldFile = ::mmap((caddr_t)0, nOldFileSize, PROT_READ, MAP_SHARED, nFdOldFile, 0);
  if ((caddr_t) vBeginOldFile == MAP_FAILED)
    {
      g_szErrMsg = i18n("<qt>Can't map file <b>%1</b> for reading.").arg(szOldFile);
      oldFile.close();
      return REPLACE_ERROR;
    }

  cBeginOldFile = (char *) vBeginOldFile;
  cOldPt = cBeginOldFile;

  // 4. Copy strings to search/remplace into strings in memory
  nItemPos = 0;
  lviCurItem = lviFirst = argu -> qlvStrings -> firstChild();
  if (lviCurItem == NULL)
    {
      g_szErrMsg = i18n("Can't list tree items.");
      return REPLACE_ERROR;
    }

  do
    {
      strOld[nItemPos] = lviCurItem -> text(0);
      strNew[nItemPos] = lviCurItem -> text(1);
      nReplaceCount[nItemPos] = 0;
      nItemPos++;
      lviCurItem = lviCurItem -> nextSibling();
    } while(lviCurItem && lviCurItem != lviFirst);

  // 5. Replace strings  --------------------------------------------
  while ( ((cOldPt - (char *) cBeginOldFile) < (int)nOldFileSize)) // While not end of file
    {
      nMaxLen = nOldFileSize - (cOldPt - (char *) cBeginOldFile); // Do not search after end of file

      for (int i=0; i < argu -> qlvStrings -> childCount(); i++) // For all strings to search
        {
          nRecursiveLength = 0;

          bRes = kjeSearch.doesStringMatch(cOldPt, min(argu -> nMaxExpressionLength,nMaxLen), strOld[i].utf8(), strOld[i].length(), true, &nRecursiveLength);

          if (bRes == true) // String matches
            {
                                // Replace
              (*nNbReplacements)++;
              (nReplaceCount[i])++; // Number of replacements for this string
              g_nStringsRep++;

              QString strReplace;

              if (argu -> bWildcardsInReplaceStrings)
                {
                  QStringList strList;

                  kjeSearch.extractWildcardsContentsFromFullString(cOldPt, nRecursiveLength, strOld[i].utf8(), strOld[i].length(), &strList);
                  /*printf ("\n-->List=");
                    for (int yy=0; yy<strList.count(); yy++)
                    printf ("(%s)", strList[yy].ascii());
                    printf ("\n");*/

                  // Crete the replace string which contains the text the wildcards were coding for in the search expression
                  kdDebug(23000) << QString("INITIAL: ****(%1)****").arg(strNew[i]) << endl;
                  strReplace = kjeSearch.addWildcardsContentToString(strNew[i].utf8(), strNew[i].length(), &strList);
                  if ((strReplace == QString::null) && (strNew[i].length()))
                    return REPLACE_ERROR;
                  kdDebug(23000) << QString("FINAL: ****(%1)****").arg(strReplace) << endl;
                }
              else
                {
                  strReplace = strNew[i];
                }

                                // If there are variables --> Copy the contents
              if (argu -> bVariables)
                {
                  strReplace = kjeSearch.substVariablesWithValues(strReplace, szOldFile);
                }

                                // Confirmation ==> Ask to user
              if (argu -> bConfirmStrings == true)
                {
                  QString strOldTxt;
                  int j;

                  // Format found text and replace text
                  for (j=0; cOldPt[j] && j < nRecursiveLength; j++)
                    strOldTxt.append(cOldPt[j]);

                  KConfirmDlg dlg;
                  dlg.setData(szOldFile, szDir, strOldTxt, strReplace);
                  nConfirm = dlg.exec();
                  if (nConfirm == KConfirmDlg::Yes)
                    strReplace = dlg.getReplaceString();
                  else if (nConfirm == KConfirmDlg::Cancel)
                    {
                      g_szErrMsg = i18n("Operation canceled.");
                      return REPLACE_CANCEL;
                    }
                  else if (nConfirm == KConfirmDlg::SkipFile)
                    {
                      g_szErrMsg = i18n("File skipped.");
                      return REPLACE_FILESKIPPED;
                    }
                  else if (nConfirm == KConfirmDlg::SkipDir)
                    {
                      g_szErrMsg = i18n("Directory skipped.");
                      return REPLACE_SKIPDIR;
                    }
                }

                                // If need not confirmation, or if used agree ==> DO THE REPLACE
              if (argu -> bConfirmStrings == false || nConfirm == KConfirmDlg::Yes)
                {
                  // Add detailed result in result list
                  argu -> qlvResult -> increaseStringCount(lvi, strOld[i], strNew[i], strReplace, cOldPt, nRecursiveLength, true);

                  // Write the new replace string
                  if (argu->bSimulation == false)
                    {
                      nRes = ::write(nFdNewFile, strReplace.local8Bit(), strReplace.length());
                      if (nRes != (int)strReplace.length())
                        {
                          g_szErrMsg = i18n("Can't write data.");
                          return REPLACE_ERROR;
                        }
                    }
                  cOldPt += nRecursiveLength; // The length of the string with the wildcards cotents
                  goto end_replace_pos; // Do not make other replace on this byte
                }
            }

        }

      // Searched Text not present: copy char
      if (argu->bSimulation == false)
        {
          nRes = ::write(nFdNewFile, cOldPt, 1);
          if (nRes != 1)
            {
              g_szErrMsg = i18n("<qt>Can't write data in <b>%1<b>.</qt>").arg(szNewFile);
              return REPLACE_ERROR;
            }
        }
      cOldPt++;

    end_replace_pos:;
    }

  // --------------------------------------------

  // Unamp files
  ::munmap(vBeginOldFile, nOldFileSize);

  // Close files
  oldFile.close();

  if (argu->bSimulation == false)
    newFile.close();

  return REPLACE_SUCCESS; // Success
}

// ==================================================================================
int SearchFile(QListViewItem *lvi, const QString &szOldFile, int *nNbReplacements, bool *bAllStringsFound, RepDirArg* argu, bool bHaltOnFirstOccur)
{

  int nFdOldFile; // File descriptors
  void *vBeginOldFile;
  char *cBeginOldFile; // Pointer to the begin of the file
  char *cOldPt; // Pointer to the current data
  uint nOldFileSize;
  QListViewItem *lviCurItem;
  QListViewItem *lviFirst;
  int nItemPos;
  bool bRes;
  KExpression kjeSearch(argu -> bCaseSensitive, argu -> bWildcards, argu -> bIgnoreWhitespaces, argu -> cWildcardsWords, argu -> cWildcardsLetters);
  int nMaxLen;
  int nNbStrings;
  int i, j; // for(;;)

  KFileReplaceApp *app;
  app = (KFileReplaceApp *) (argu -> mainwnd);

  // Items of the string list
  int nReplaceCount[MAX_STRINGSTOSEARCHREP];
  QString strOld[MAX_STRINGSTOSEARCHREP];

  // 0. Init
  *nNbReplacements = 0;
  *bAllStringsFound = false;
  QFileInfo fiOld(szOldFile);
  nOldFileSize = fiOld.size();
  nNbStrings = argu -> qlvStrings -> childCount();

  // 1. Open files
  QFile oldFile(szOldFile);
  if (!oldFile.open(IO_ReadOnly))
  {
    g_szErrMsg = i18n("<qt>Can't open file <b>%1</b> for reading.</qt>").arg(szOldFile);
    return -1;
  }
  nFdOldFile = oldFile.handle();

  // Map files
  vBeginOldFile = ::mmap((caddr_t)0, nOldFileSize, PROT_READ, MAP_SHARED, nFdOldFile, 0);
  if ((caddr_t) vBeginOldFile == MAP_FAILED)
    {
      g_szErrMsg = i18n("<qt>Can't map file <b>%1</b> for reading.</qt>").arg(szOldFile);
      oldFile.close();
      return -1;
    }

  cBeginOldFile = (char *) vBeginOldFile;
  cOldPt = cBeginOldFile;

  // Copy strings to search/remplace into strings in memory
  nItemPos = 0;
  lviCurItem = lviFirst = argu -> qlvStrings -> firstChild();
  if (lviCurItem == NULL)
    {
      g_szErrMsg = i18n("Can't list tree items.");
      return -1;
    }

  // Copy strings to search/remplace into strings in memory
  nItemPos = 0;
  lviCurItem = lviFirst = argu -> qlvStrings -> firstChild();
  if (lviCurItem == NULL)
    {
      g_szErrMsg = i18n("Can't list tree items.");
      return -1;
    }

  do
    {
      strOld[nItemPos] = lviCurItem -> text(0);
      nReplaceCount[nItemPos] = 0;
      nItemPos++;
      lviCurItem = lviCurItem -> nextSibling();
    } while(lviCurItem && lviCurItem != lviFirst);


  // --------------------------------------------
  while ( ((cOldPt - (char *) cBeginOldFile) < (int)nOldFileSize)) // While not end of file
    {
      int nRecursiveLength = 0;
      nMaxLen = nOldFileSize - (cOldPt - ((char *) cBeginOldFile)); // Do not search after end of file

      for (i=0; i < nNbStrings; i++) // For all strings to search
        {
          bRes = kjeSearch.doesStringMatch(cOldPt, min(argu -> nMaxExpressionLength, nMaxLen), strOld[i].utf8(), strOld[i].length(), true, &nRecursiveLength);

          if (bRes == true) // String matches
            {
              if (*bAllStringsFound == false) // test if true now (with the new string found)
                {
                  bool bAllPresent = true;
                  for (j=0; j < nNbStrings; j++)
                    if (!nReplaceCount[j])
                      bAllPresent = false;
                  *bAllStringsFound = bAllPresent;
                }

              // If stop at first success (do not need to know how many found)
              if (bHaltOnFirstOccur && (*bAllStringsFound || !argu->bAllStringsMustBeFound) )
                {
                  *nNbReplacements = 1;
                  ::munmap(vBeginOldFile, nOldFileSize);
                  oldFile.close();
                  return 0; // Success
                }

              (*nNbReplacements)++;
              (nReplaceCount[i])++; // Number of replacements for this string
              if (!bHaltOnFirstOccur)
                g_nStringsRep++;

              // Add detailed result in result list
              if (lvi)
                argu -> qlvResult -> increaseStringCount(lvi, strOld[i], "", "", cOldPt, nRecursiveLength, !bHaltOnFirstOccur);

              cOldPt += nRecursiveLength;
              goto end_search; // Do not make other search on this byte
            }
        }

      cOldPt++;
    end_search:;
    }

  // Unamp files
  ::munmap(vBeginOldFile, nOldFileSize);

  // Close files
  oldFile.close();

  return 0; // Success
}


// ===========================================================================================================================
int GetDiskFreeSpaceForFile(uint *nAvailDiskSpace, const QString &szFilename)
{
  int nRes;
  struct STATFS fsInfo;

  *nAvailDiskSpace = 0;

  nRes = STATFS(szFilename.local8Bit(), &fsInfo); //FIXME: replace with a QT/KDE function
  if (nRes == -1)
    return -1;

  *nAvailDiskSpace = fsInfo.f_bavail * fsInfo.f_bsize;

  return 0;
}

// ===========================================================================================================================
bool HasFileGoodOwners(const QString &szFile, RepDirArg *argu)
{
  QFileInfo fi;
  fi.setFile(szFile);

  // +++++++++++ if must test the user owner +++++++++++++
  if (argu->bOwnerUserBool)
    {
      if (argu->strOwnerUserType == "name")
        {
          if (argu->bOwnerUserMustBe == true) // owner user name must be xxx
            {
              kdDebug(23000) << QString("(%1): owner user name must be %2").arg(szFile).arg(argu->strOwnerUserValue) << endl;
              if (fi.owner() != argu->strOwnerUserValue)
                return false;
            }
          else        // owner user name must NOT be xxx
            {
              kdDebug(23000) << QString("(%1): owner user name must not be %2").arg(szFile).arg(argu->strOwnerUserValue) << endl;
              if (fi.owner() == argu->strOwnerUserValue)
                return false;
            }

        }
      else if (argu->strOwnerUserType == "ID (number)")
        {
          if (argu->bOwnerUserMustBe == true) // owner user ID must be xxx
            {
              kdDebug(23000) << QString("(%1): owner user ID must be %2").arg(szFile).arg(argu->strOwnerUserValue) << endl;
              if (fi.ownerId() != argu->strOwnerUserValue.toULong())
                return false;
            }
          else        // owner user ID must NOT be xxx
            {
              kdDebug(23000) << QString("(%1): owner user ID must not be %2").arg(szFile).arg(argu->strOwnerUserValue) << endl;
              if (fi.ownerId() == argu->strOwnerUserValue.toULong())
                return false;
            }
        }
    }

  // +++++++++++ if must test the group owner +++++++++++++
  if (argu->bOwnerGroupBool)
    {
      if (argu->strOwnerGroupType == "name")
        {
          if (argu->bOwnerGroupMustBe == true) // owner group name must be xxx
            {
              kdDebug(23000) << QString("(%1): owner group name must be %2").arg(szFile).arg(argu->strOwnerGroupValue) << endl;
              if (fi.group() != argu->strOwnerGroupValue)
                return false;
            }
          else        // owner group name must NOT be xxx
            {
              kdDebug(23000) << QString("(%1): owner group name must not be %2").arg(szFile).arg(argu->strOwnerGroupValue) << endl;
              if (fi.group() == argu->strOwnerGroupValue)
                return false;
            }

        }
      else if (argu->strOwnerGroupType == "ID (number)")
        {
          if (argu->bOwnerGroupMustBe == true) // owner group ID must be xxx
            {
              kdDebug(23000) << QString("(%1): owner group ID must be %2").arg(szFile).arg(argu->strOwnerGroupValue) << endl;
              if (fi.groupId() != argu->strOwnerGroupValue.toULong())
                return false;
            }
          else        // owner user ID must NOT be xxx
            {
              kdDebug(23000) << QString("(%1): owner group ID must not be %2").arg(szFile).arg(argu->strOwnerGroupValue) << endl;
              if (fi.groupId() == argu->strOwnerGroupValue.toULong())
                return false;
            }
        }
    }

  return true;
}














