/***************************************************************************
                          configurationclasses.cpp  -  description
                             -------------------
    begin                : Sat Sep 11 2004
    copyright            : (C) 2004 Emiliano Gulmini
    email                : emi_barbarossa@yahoo.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// QT

// KDE

// local
#include "configurationclasses.h"
#include "whatthis.h"

using namespace whatthisNameSpace;

//RCOptions Class
RCOptions::RCOptions()
{
  m_searchingOnlyMode = false;
}

RCOptions& RCOptions::operator=(const RCOptions& ci)
{
  //m_callResetActions = ci.m_callResetActions;

  m_directories = ci.m_directories;
  m_filters = ci.m_filters;
  m_currentDirectory = ci.m_currentDirectory;
  m_minSize = ci.m_minSize;
  m_maxSize = ci.m_maxSize;

  m_dateAccess = ci.m_dateAccess;
  m_minDate = ci.m_minDate;
  m_maxDate = ci.m_maxDate;

  m_caseSensitive = ci.m_caseSensitive;
  m_recursive = ci.m_recursive;
  m_followSymLinks = ci.m_followSymLinks;
  m_allStringsMustBeFound = ci.m_allStringsMustBeFound;
  m_backup = ci.m_backup;
  m_backupExtension = ci.m_backupExtension;
  m_ignoreFiles = ci.m_ignoreFiles;
  m_regularExpressions = ci.m_regularExpressions;

  m_variables = ci.m_variables;
  m_haltOnFirstOccur = ci.m_haltOnFirstOccur;
  m_ignoreHidden = ci.m_ignoreHidden;
  m_simulation = ci.m_simulation;
  m_searchingOnlyMode = ci.m_searchingOnlyMode;

  m_ownerUserIsChecked = ci.m_ownerUserIsChecked;
  m_ownerGroupIsChecked = ci.m_ownerGroupIsChecked;

  m_ownerUserBool = ci.m_ownerUserBool;
  m_ownerGroupBool = ci.m_ownerGroupBool;

  m_ownerUserType = ci.m_ownerUserType;
  m_ownerGroupType = ci.m_ownerGroupType;

  m_ownerUserValue = ci.m_ownerUserValue;
  m_ownerGroupValue = ci.m_ownerGroupValue;

  m_mapStringsView = ci.m_mapStringsView;

  m_quickSearchString =  ci.m_quickSearchString;
  m_quickReplaceString = ci.m_quickReplaceString;

  m_recentStringFileList = ci.m_recentStringFileList;

  m_notifyOnErrors = ci.m_notifyOnErrors;

  return (*this);
}

//ResultViewEntry Class
ResultViewEntry::ResultViewEntry(const QString &nkey, const QString &ndata, bool regexp, bool caseSensitive)
{
  m_caseSensitive = caseSensitive;
  m_regexp = regexp;

  if(regexp)
    {
      m_rxKey = QRegExp('('+nkey+')', caseSensitive, false);
    }
  else
    {
      m_key = nkey;
    }
  m_data = ndata;
  m_matchedStringsOccurrence = 0;
  m_pos = 0;
}

int ResultViewEntry::lineNumber(const QString& line) const
{
  return line.mid(0,m_pos).count('\n')+1;
}

int ResultViewEntry::columnNumber(const QString& line) const
{
  return(m_pos - line.lastIndexOf('\n',m_pos));
}

void ResultViewEntry::incOccurrences()
{
  m_matchedStringsOccurrence++;
}

int ResultViewEntry::occurrences() const
{
  return m_matchedStringsOccurrence;
}

bool ResultViewEntry::regexp()const
{
  return m_regexp;
}

int ResultViewEntry::pos(const QString& line)
{
  if(m_regexp)
    m_pos = m_rxKey.search(line,m_pos);
  else
    m_pos = line.find(m_key, m_pos, m_caseSensitive);

  return m_pos;
}

void ResultViewEntry::incPos()
{
  int kl = keyLength(),
      dl = dataLength();

  if(kl < dl)
    m_pos += kl;
  else
    m_pos += dl;

}

QString ResultViewEntry::capturedText(const QString& line)
{
  QString cap;

  if(m_regexp)
    cap = m_rxKey.cap(1);
  else
    cap =line.mid(m_pos,m_key.length());

  return cap;
}

QString ResultViewEntry::message(const QString& capturedText, int x, int y) const
{
  QString data = m_data;
  //return i18n(" captured text \"%1\" replaced with \"%2\" at line: %3, column: %4 ", capturedText, data, x, y);
  return i18n(" Line:%3,Col:%4 - \"%1\" -> \"%2\"", capturedText, data, x, y);
}

int ResultViewEntry::keyLength() const
{
  if(m_regexp)
    return m_rxKey.matchedLength();
  else
    return m_key.length();
}

int ResultViewEntry::dataLength() const
{
  return m_data.length();
}

void ResultViewEntry::updateLine(QString& line)
{
  line.insert(m_pos, m_data);
  line.remove(m_pos + dataLength(), keyLength());
}
