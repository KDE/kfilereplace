/***************************************************************************
                          kexpression.h  -  description
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

#ifndef KEXPRESSION_H
#define KEXPRESSION_H

extern char g_szErrMsg[];

#include <qdatetime.h>
class QStringList;

class KExpression
{
        public:

                KExpression(bool bCaseSensitive, bool bWildcards, bool bIgnoreWhitespaces, char cWord='*', char cLetter='?')
                {m_cLetter = cLetter; m_cWord = cWord; m_bCaseSensitive = bCaseSensitive; m_bWildcards = bWildcards; m_bIgnoreWhitespaces = bIgnoreWhitespaces;}

                void setWordWildcard(char cWord) {m_cWord = cWord;}
                void setLetterWildcard(char cLetter) {m_cLetter = cLetter;}
                void setCaseSensitive(bool bCaseSensitive) {m_bCaseSensitive = bCaseSensitive;}
                void setIgnoreWhitespaces(bool bIgnoreWhitespaces) {m_bIgnoreWhitespaces = bIgnoreWhitespaces;}
                char getWordWildcard() {return m_cWord;}
                char getLetterWildcard() {return m_cLetter;}
                bool isCaseSensitive() {return m_bCaseSensitive;}
                bool doesIgnoreWhitespaces() {return m_bIgnoreWhitespaces;}
                bool areWildcards() {return m_bWildcards;}

                bool doesStringMatch(const char *szText, int nTxtLen, const char *szString, int nStrWildcardLen, bool bBeginString, int *nLenMatchingStr=0);

                int extractWildcardsContentsFromFullString(const char *szText, int nTxtLen, const char *szString, int nStrWildcardLen, QStringList *strlResult);
                QString addWildcardsContentToString(const char *szNewString, int nNewStrLen, QStringList *strList);

                // In these functions, "const char *szFilepath" is passed because it can be need by the functions (as the date, we can calculate)
                QString substVariablesWithValues(QString strOriginal, const char *szFilepath);

private: // Functions
                QString getVariableValue(QString strVarName, QString strVarFormat, const char *szFilepath);
                QString formatDateTime(QDateTime dt, QString strVarFormat);



private: // Datas
                char m_cLetter;
                char m_cWord;
                bool m_bCaseSensitive;
                bool m_bWildcards;
                bool m_bIgnoreWhitespaces;
};

#endif
