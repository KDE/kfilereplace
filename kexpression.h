/***************************************************************************
                          kexpression.h  -  description
                             -------------------
    begin                : Mon Dec 20 1999
    copyright            : (C) 1999 by François Dupoux
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

#ifndef KEXPRESSION_H
#define KEXPRESSION_H

class QString;
extern QString g_szErrMsg;

#include <qdatetime.h>
class QStringList;

class KExpression
{
        public:
                KExpression(bool bCaseSensitive, bool bWildcards, bool bIgnoreWhitespaces, char cWord='*', char cLetter='?');
                void setWordWildcard(char cWord); 
                void setLetterWildcard(char cLetter); 
                void setCaseSensitive(bool bCaseSensitive);
                void setIgnoreWhitespaces(bool bIgnoreWhitespaces); 
                char getWordWildcard();
                char getLetterWildcard();
                bool isCaseSensitive();
                bool doesIgnoreWhitespaces();
                bool areWildcards();

                bool doesStringMatch(const char *szText, int nTxtLen, const char *szString, int nStrWildcardLen, bool bBeginString, int *nLenMatchingStr=0);

                int extractWildcardsContentsFromFullString(const char *szText, int nTxtLen, const char *szString, int nStrWildcardLen, QStringList *strlResult);
                QString addWildcardsContentToString(const char *szNewString, int nNewStrLen, QStringList *strList);

                // In these functions, "const char *szFilepath" is passed because it can be need by the functions (as the date, we can calculate)
                QString substVariablesWithValues(const QString &strOriginal, const char *szFilepath);

private: // Functions
                QString getVariableValue(const QString &strVarName, const QString &strVarFormat, const char *szFilepath);
                QString formatDateTime(const QDateTime& dt, const QString &strVarFormat);

private: // Datas
                char m_cLetter;
                char m_cWord;
                bool m_bCaseSensitive;
                bool m_bWildcards;
                bool m_bIgnoreWhitespaces;
};

#endif
