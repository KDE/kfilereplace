/***************************************************************************
                          kexpression.cpp  -  description
                             -------------------
    begin                : Mon Dec 20 1999
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

#include "kexpression.h"
#include "resource.h"
#include "filelib.h"

#include <kdebug.h>
#include <klocale.h>

#include <qfileinfo.h>
#include <qstringlist.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

// =================================================================================================================
// Says if a wildcard string can be found inside a text
// Result: false = String not found
//         true = String found
// Result: nLenMatchingStr = Number of chars there are in the text the string is matching for
bool KExpression::doesStringMatch(const char *szText, int nTxtLen, const char *szString, int nStrWildcardLen, bool bBeginString, int *nLenMatchingStr/*=0*/)
{

  // --------------- 0. Check there are no errors in data passed -------------------

  if (nTxtLen < nStrWildcardLen)
    return false; // Not found

  if (nStrWildcardLen < 1)
    return false;

  // --------------- 1. Prepare comparaison ---------------------------------------

  int i;
  bool bCharMatches;
  int nLen;
  bool bRes;

  // Do the current char matches ?
  if (m_bCaseSensitive == true)
    {
      bCharMatches = (szString[0] == szText[0]);
    }
  else // case insensitive
    {        bCharMatches = (tolower(szString[0]) == tolower(szText[0]));
    }

  // CASE 3: IGNORE WHITESPACES (\t, \n, \r, double-spaces) OPTIONS IS ACTIVATED, AND THERE IS ONE HERE
  // Very important to be before the most general cases (CASES 1 and 2), else the spaces don't work (ex: search "kde under linux" inside "kde under      linux")
  // In the example, the space before "linux" wouldn't be present before the "destruction" of the some spaces chars.
  if ((m_bIgnoreWhitespaces == true) && (bBeginString == false)) // If ignore whitespaces options enabled and can be used here
    {
      // Test if this option is need
      m_bIgnoreWhitespaces = false;
      bRes = doesStringMatch(szText, nTxtLen, szString, nStrWildcardLen, false, nLenMatchingStr);
      m_bIgnoreWhitespaces = true;

      if (bRes == false) // if matches without this option, do not make anything
        {
          bool bIsWhitespace = false;

          if (szText[0] == '\r' || szText[0] == '\t' || szText[0] == '\n' || szText[0] == ' ') // If current char is whitespace
            bIsWhitespace = true;

          /*if (szText[0] == ' ' && nTxtLen > 1 && szText[1] == ' ')
            bIsWhitespace = true;*/

          // if whitespace
          if (bIsWhitespace == true)
            {
              bRes = doesStringMatch(szText+1, nTxtLen-1, szString, nStrWildcardLen, false, &nLen);
              if (bRes) // If string is matching at the next char
                {
                  if (nLenMatchingStr)
                    *nLenMatchingStr = nLen+1;
                  return true;
                }
            }
        }
    }

  // ------------- 2. Make the comparaison ---------------------------------------

  // CASE 1: THE CURRENT CHAR MATCH BECAUSE GOOD CHAR OR CHAR WILDCARD FOUND
  // If current string char is '?' or is good --> char matches --> chech for net char
  if (bCharMatches || ((szString[0] == m_cLetter) && m_bWildcards))
    {
      // The first compared char of the string is good

      // If this is the end of the string
      if (nStrWildcardLen == 1)
        {
          if (nLenMatchingStr) // If result required
            *nLenMatchingStr = 1;

          return true; // Success --> the string matches
        }

      // If this is not the end of the searched string --> continue
      nLen=0;
      bRes = doesStringMatch(szText+1, nTxtLen-1, szString+1, nStrWildcardLen-1, false, &nLen);
      if (nLenMatchingStr) // If result required
        *nLenMatchingStr = nLen + 1; // Recursive + changed here
      return bRes;
    }

  // CASE 2: THERE IS AN EXPRESSION WILDCARD IN THE STRING
  // If current string char is '*' --> check if the word wildcard matches (if the string after '*' match)
  if (m_bWildcards && (szString[0] == m_cWord))
    {
      for (i=0; i < nTxtLen; i++) // With all next chars of the text to explore
        {
          if (nTxtLen >= nStrWildcardLen) // If it is possible the string matches with the text
            //if (nStrWildcardLen-1 >= nTxtLen-i) // If it is possible the string matches with the text
            {
              nLen = 0;
              bRes = doesStringMatch(szText+i, nTxtLen-i, szString+1, nStrWildcardLen-1, false, &nLen);

              if (bRes) // If string is matching at this char position of the text
                {
                  if (nLenMatchingStr) // If result is required
                    *nLenMatchingStr = i+nLen; // How many chars of the text the word wildcard '*' is coding for ?
                  return true;
                }
                                // if bRes == false, the string is not matching at this position, then continue the search, with for
            }
        }
    }

  // CASE 4: (ELSE) THERE IS A BAD CHAR
  // Current string char does not match
  return false;
}

// =================================================================================================================
int KExpression::extractWildcardsContentsFromFullString(const char *szText, int nTxtLen, const char *szString, int nStrWildcardLen, QStringList *strlResult)
{
  int nLen;
  bool bRes;
  int nRes;
  int i, j;
  char szTemp[MAX_TEXTLEN];

  // --------------- 0. Check there are no errors in data passed -------------------

  if (nTxtLen < nStrWildcardLen)
    return -1;

  if (nStrWildcardLen < 1)
    return -1;

  // ------------- 1. Make extraction ---------------------------------------

  // CASE 1: THIS IS NOT A WILDCARD
  if ((szString[0] != m_cLetter) && (szString[0] != m_cWord))
    {
      // if this is the end of the string
      if (nStrWildcardLen == 1)
        return 0;

      // This is good, recursivity will continue to read the string
      nRes = extractWildcardsContentsFromFullString(szText+1, nTxtLen-1, szString+1, nStrWildcardLen-1, strlResult);
      return nRes;
    }

  // CASE 2: THERE IS AN SIMPLE CHAR WILDCARD IN THE STRING '?'
  if (szString[0] == m_cLetter)
    {
      sprintf(szTemp, "%c", szText[0]);
      strlResult -> append(szTemp);
      //printf ("APPEND CHAR (%c)\n", szText[0]);

      // if this is the end of the string
      if (nStrWildcardLen == 1)
        return 0;

      // Continue the operation
      nRes = extractWildcardsContentsFromFullString(szText+1, nTxtLen-1, szString+1, nStrWildcardLen-1, strlResult);
      return nRes;
    }

  // CASE 3: THERE IS AN EXPRESSION WILDCARD IN THE STRING '*'
  if (szString[0] == m_cWord)
    {        // We need to know how many chars the expression wildcards is coding for

      for (i=0; i < nTxtLen; i++) // With all next chars of the text to explore
        {
          if (nTxtLen-i >= nStrWildcardLen-1) // If it is possible the string matches with the text
            {
              nLen = 0;
              bRes = doesStringMatch(szText+i, nTxtLen-i, szString+1, nStrWildcardLen-1, false, &nLen);
              if (bRes) // If string is matching at this char position of the text
                {
                  for (j=0; j < i; j++)
                    szTemp[j] = szText[j];
                  szTemp[j] = 0;

                  // Copy the passed text, what '*' was coding for
                  strlResult->append(szTemp);
                  //printf ("APPEND WORD ***(%s)***\n\n", szTemp);

                  // if this is the end of the string
                  if (nStrWildcardLen+i == 0)
                    return 0;

                  // Continue the operation
                  nRes = extractWildcardsContentsFromFullString(szText+i, nTxtLen-i, szString+1, nStrWildcardLen-1, strlResult);
                  return nRes;

                }
                                // (bRes == false) must be impossible for all positions
            }
        }
    }

  return 0;
}

// =================================================================================================================
QString KExpression::addWildcardsContentToString(const char *szNewString, int nNewStrLen, QStringList *strList)
{
  QString strReplace;
  bool bWildcards;
  int nLenMatchingStr;
  int nWildcardsAsked;
  QStringList strlTemp;
  char szWildcard[64];
  bool bRes;
  int i;

  bWildcards = m_bWildcards; // save old value
  m_bWildcards = true;
  memset(szWildcard, 0, sizeof(szWildcard)-1);
  sprintf(szWildcard, "[#%c#]", m_cWord); // is "[#*#]" in general

  i=0;
  while(i < nNewStrLen)
    {
      // check if the current expression of (szNewString+i) is "[#*#]" (with * wildcard)
      bRes = doesStringMatch(szNewString+i, nNewStrLen-i, szWildcard, strlen(szWildcard), false, &nLenMatchingStr);

      if (bRes != true) // not a wildcard
        {
          strReplace.append(szNewString[i]);
          i++;
        }
      else // If there is a wildcard: [#*#]
        {
          strlTemp.clear();
          extractWildcardsContentsFromFullString(szNewString+i, nNewStrLen-i, szWildcard, strlen(szWildcard), &strlTemp);

          if (!strlTemp.count()) // if empty
            kdError(23000) << QString("strlTemp.count() == 0 --- String0=(%1)").arg(strlTemp[0]) << endl;
          nWildcardsAsked = strlTemp[0].toLong();

          // strList = number of wildcards in the search string
          if (nWildcardsAsked >= (int)strList->count())
            {
              g_szErrMsg = i18n("<qt>You want to insert wildcard number %1 with [#%2#], but there are only %3 wildcards! The fisrt is [#0#] "
                                       "and not [#1#]. Then the last is [#%4#].</qt>").arg(nWildcardsAsked).arg(nWildcardsAsked).arg(strList->count()).arg( strList->count()-1);
              kdDebug(23000) << g_szErrMsg << endl;
              return QString::null;
            }

          strReplace.append((*strList)[nWildcardsAsked]);

          i += strlen(szWildcard);
        }
    }

  // restore value
  m_bWildcards = bWildcards; // save old value

  return strReplace;
}

// =================================================================================================================
QString KExpression::substVariablesWithValues(QString strOriginal, const char *szFilepath)
{
  // Save the "m_bWildcards" and "m_bIgnoreWhitespaces" values because current function need m_bWildcards to be true
  int nWildcards;
  bool bIgnoreWhitespaces;

  nWildcards = m_bWildcards;
  m_bWildcards = true;
  bIgnoreWhitespaces = m_bIgnoreWhitespaces;
  m_bIgnoreWhitespaces = false;

  QString strFormat;
  QString strResult;
  int nLenMatchingStr;
  bool bRes;
  QString strVarName, strVarFormat;
  QString strTemp;
  int i;
  QStringList strList;

  strFormat.sprintf("[$%c:%c$]", m_cWord, m_cWord); // "[$VarName:VarFormat$]"

  // For all chars of the original string...
  i = 0;
  while (i < (int)strOriginal.length())
    {
      nLenMatchingStr = 0;
      bRes = doesStringMatch(strOriginal.ascii()+i, strOriginal.length()-i, strFormat.ascii(), strFormat.length(), true, &nLenMatchingStr);

      if (bRes == true) // If a variable was found
        {
          // Get the variable name, and the variable format: "[$VarName:VarFormat$]"
          extractWildcardsContentsFromFullString(strOriginal.ascii()+i, strOriginal.length()-i, strFormat.ascii(), strFormat.length(), &strList);

          strVarName = strList[0];
          strVarFormat = strList[1];

          strTemp = getVariableValue(strVarName, strVarFormat, szFilepath);
          printf("VAR: (%s, %s) ---> (%s)\n", strVarName.ascii(), strVarFormat.ascii(), strTemp.ascii());
          if (strTemp == QString::null) // If error
            {
              m_bIgnoreWhitespaces = bIgnoreWhitespaces;
              return QString::null;
            }


          // Add variable value to the result string
          strResult += strTemp;

          i += nLenMatchingStr; // Position in the string
        }
      else // If no variable was found
        {
          strResult.append( (strOriginal.ascii())[i] );
          i++;
        }
    }

  // Restore the "m_bWildcards" value
  m_bWildcards = nWildcards;
  m_bIgnoreWhitespaces = bIgnoreWhitespaces;

  return strResult;
}

// =================================================================================================================
QString KExpression::getVariableValue(QString strVarName, QString strVarFormat, const char *szFilepath)
{
  QFileInfo fi;
  fi.setFile(szFilepath);
  fi.convertToAbs();
  QString strTemp;

  //kDebugInfo("VARIABLES: [%s]: (%s)(%s)\n", szFilepath, strVarName.ascii(), strVarFormat.ascii());

  // ******************************* FILENAME ******************************************************************
  if (strVarName == "filename") // Ex: "/home/fdupoux/kfilereplace.htm"
    {
      if (strVarFormat == "fullpath") // Must copy "/home/fdupoux/kfilereplace.htm"
        {
          return fi.filePath();
        }
      else if (strVarFormat == "path") // Must copy "/home/fdupoux"
        {
          return fi.dirPath(true); // absolute path
        }
      else if (strVarFormat == "fullname") // Must copy "kfilereplace.htm"
        {
          return fi.fileName();
        }
      else if (strVarFormat == "basename") // Must copy "kfilereplace"
        {
          return fi.baseName();
        }
      else // Invalid "filename" format
        {
          return QString::null;
        }
    }
  // ******************************* FILE-LAST-WRITE-TIME *******************************************************
  else if (strVarName == "filelwtime")
    {
      return formatDateTime(fi.lastModified(), strVarFormat);
    }
  // ******************************* FILE-LAST-READ-TIME *******************************************************
  else if (strVarName == "filelrtime")
    {
      return formatDateTime(fi.lastRead(), strVarFormat);
    }
  // ******************************* FILE-SIZE *******************************************************
  else if (strVarName == "filesize")
    {
      if (strVarFormat == "bytes") // ex: 111222333
        {
          strTemp.sprintf("%lu", (DWORD) fi.size());
          return strTemp;
        }
      if (strVarFormat == "best") // ex: 125 MB
        {
          return formatSize(fi.size());
        }
      else // invalid format
        {
          return QString::null;
        }
    }
  // ******************************* FILE-OWNER *******************************************************
  else if (strVarName == "owner")
    {
      if (strVarFormat == "userid")
        {
          strTemp.sprintf("%d", fi.ownerId());
          return strTemp;
        }
      else if (strVarFormat == "groupid")
        {
          strTemp.sprintf("%d", fi.groupId());
          return strTemp;
        }
      else if (strVarFormat == "username")
        {
          return fi.owner();
        }
      else if (strVarFormat == "groupname")
        {
          return fi.group();
        }
      else // invalid format
        {
          return QString::null;
        }
    }
  // ******************************* DATETIME ******************************************************************
  else if (strVarName == "datetime")
    {
      return formatDateTime(QDateTime::currentDateTime(), strVarFormat);
    }
  // ***********************************************************************************************************
  else // ERROR: unknown variable
    {
      return QString::null;
    }

}

// =================================================================================================================
QString KExpression::formatDateTime(QDateTime dt, QString strVarFormat)
{
  QString strTemp;

  if (strVarFormat == "mm/dd/yyyy") // Ex: "12/31/1999"
    {
      strTemp.sprintf("%.2d/%.2d/%.4d", dt.date().month(), dt.date().day(), dt.date().year());
      return strTemp;
    }
  else if (strVarFormat == "dd/mm/yyyy") // Ex: "31/12/1999"
    {
      strTemp.sprintf("%.2d/%.2d/%.4d", dt.date().day(), dt.date().month(), dt.date().year());
      return strTemp;
    }
  else if (strVarFormat == "yyyy/mm/dd") // Ex: "1999/12/31"
    {
      strTemp.sprintf("%.4d/%.2d/%.2d", dt.date().year(), dt.date().month(), dt.date().day());
      return strTemp;
    }
  else if (strVarFormat == "string") // Ex: "Say May 20 1995"
    {
      strTemp.sprintf ("%s", dt.date().toString().ascii());
      return strTemp;
    }
  else if ((strVarFormat == "yyyy/dd/mm hh:mm:ss") || (strVarFormat == "date&time")) // Ex: "1999/31/12 15:26:46"
    {
      strTemp.sprintf("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", dt.date().year(), dt.date().month(), dt.date().day(), dt.time().hour(), dt.time().minute(), dt.time().second());
      return strTemp;
    }
  else if (strVarFormat == "hh/mm/ss") // Ex: "15:26:46"
    {
      strTemp.sprintf("%.2d:%.2d:%.2d", dt.time().hour(), dt.time().minute(), dt.time().second());
      return strTemp;
    }
  else // Invalid "date" format
    {
      return QString::null;
    }
}
