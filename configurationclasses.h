/***************************************************************************
                          configurationclasses.h  -  description
                             -------------------
    begin                : Sat Apr 17 2004
    copyright            : (C) 2004 Emiliano Gulmini 
    email                : <emi_barbarossa@yahoo.it>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef CONFIGURATIONCLASSES_H
#define CONFIGURATIONCLASSES_H 

#include <qstring.h>
#include <qstringlist.h> 
#include <qdatetime.h>

// entry strings in the kfilereplacerc file
const QString rcDirectoriesList="Directories list";
const QString rcFiltersList="Filters list";
const QString rcRecentFiles="Recent files";
const QString rcAllStringsMustBeFound="All strings must be found";
const QString rcCaseSensitive="Case sensitive";
const QString rcConfirmDirs="Confirm directories";
const QString rcConfirmFiles="Confirm files";
const QString rcConfirmStrings="Confirm strings";
const QString rcFollowSymLinks="Follow symbolic links";
const QString rcHaltOnFirstOccur="Halt on first occurrence";
const QString rcIgnoreHidden="Ignore hidden files";
const QString rcIgnoreWhitespaces="Ignore special characters";
const QString rcRecursive="Search/replace in sub folders";
const QString rcVariables="Enable variables";
const QString rcWildcards="Enable wildcards";
const QString rcMinFileSize="Minimum file size";
const QString rcMaxFileSize="Maximum file size";
const QString rcValidAccessDate="Access mode";
const QString rcMinDate="Minimum access date";
const QString rcMaxDate="Maximum access date";
const QString rcOwnerUser="Owner user filters";
const QString rcOwnerGroup="Owner group filters";
const QString rcSearchMode="Search only mode";
const QString rcBackupExtension="Backup file extension";

// Default configuration options
const bool RecursiveOption = true;
const bool CaseSensitiveOption = false;
const bool FollowSymbolicLinksOption = false;
const bool WildcardsOption = false;
const bool VariablesOption = false;
const bool ConfirmFilesOption = false;
const bool ConfirmStringsOption = false;
const bool ConfirmDirectoriesOption = false;
const bool StopWhenFirstOccurenceOption = false;
const bool IgnoreWhiteSpacesOption = false;
const bool IgnoreHiddenOption = false;
const int FileSizeOption = -1;
const QString AccessDateOption="unknown";
const QString ValidAccessDateOption="unknown";
const QString OwnerOption="false,Name,Equals To,???";
const bool SearchMode=true;
const QString BackupExtension="false,~";

// This class store configuration informations
class ConfigurationInformation
{
  private:
    QString m_directory,
            m_filter;
           
    int m_minSize,
        m_maxSize;
       
   QString m_dateAccess,
           m_minDate,
           m_maxDate;
        
    bool m_caseSensitive,
         m_recursive,
         m_followSymLinks,
         m_allStringsMustBeFound,
         m_backup,
         m_wildcards;
       
    bool m_variables,
         m_confirmFiles,
         m_confirmStrings,
         m_confirmDirs,
         m_haltOnFirstOccur,
         m_ignoreWhitespaces,
         m_ignoreHidden,
         m_simulation,
	 m_searchMode;
	 
    bool m_ownerUserIsChecked,
         m_ownerGroupIsChecked;   
	 
    QString m_ownerUserType,
            m_ownerGroupType,
            m_ownerUserValue,
            m_ownerGroupValue,
	    m_ownerUserBool,
            m_ownerGroupBool;
   
    QString m_backupExtension;

  public:
    ConfigurationInformation& operator=(const ConfigurationInformation& ci) 
    {
      m_directory = ci.m_directory;
      m_filter = ci.m_filter;
           
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
      m_wildcards = ci.m_wildcards;
       
      m_variables = ci.m_variables;
      m_confirmFiles = ci.m_confirmFiles;
      m_confirmStrings = ci.m_confirmStrings;
      m_confirmDirs = ci.m_confirmDirs;
      m_haltOnFirstOccur = ci.m_haltOnFirstOccur;
      m_ignoreWhitespaces = ci.m_ignoreWhitespaces;
      m_ignoreHidden = ci.m_ignoreHidden;
      m_simulation = ci.m_simulation;
      m_searchMode = ci.m_searchMode;
      
      m_ownerUserIsChecked = ci.m_ownerUserIsChecked;
      m_ownerGroupIsChecked = ci.m_ownerGroupIsChecked;
      
      m_ownerUserBool = ci.m_ownerUserBool;
      m_ownerGroupBool = ci.m_ownerGroupBool;
             
      m_ownerUserType = ci.m_ownerUserType;
      m_ownerGroupType = ci.m_ownerGroupType;
      
      m_ownerUserValue = ci.m_ownerUserValue;
      m_ownerGroupValue = ci.m_ownerGroupValue;
     
      return (*this);
    }
    
  public:
    void setDirectory(const QString ndir) { m_directory = ndir; }
    QString directory() const { return m_directory; }
    void setFilter(const QString nfilter) { m_filter = nfilter; }
    QString filter() const { return m_filter; }
    
    void setMinSize(int min) { m_minSize = min; }
    int minSize() const { return m_minSize; }
    void setMaxSize(int max) { m_maxSize = max; }
    int maxSize() const { return m_maxSize; }
    
    void setDateAccess(const QString& da) { m_dateAccess = da; }
    QString dateAccess() const { return m_dateAccess; }
    void setMinDate(const QString& min) { m_minDate = min; }
    QString minDate() const { return m_minDate; }
    void setMaxDate(const QString& max) { m_maxDate = max; }
    QString maxDate() const { return m_maxDate; }
    
    void setCaseSensitive(bool cs) { m_caseSensitive = cs; }
    bool caseSensitive() const { return m_caseSensitive; }
    void setRecursive(bool r) { m_recursive = r; }
    bool recursive() const { return m_recursive; }
    void setFollowSymLinks(bool sl) { m_followSymLinks = sl; }
    bool followSymLinks() const { return m_followSymLinks; }
    void setWildcards(bool wc) { m_wildcards = wc; }
    bool wildcards() const { return m_wildcards; }
    void setBackup(bool backup) { m_backup = backup; }
    bool backup() const { return m_backup; }
    void setVariables(bool v) { m_variables = v; }
    bool variables() const { return m_variables; }
    void setConfirmFiles(bool c) { m_confirmFiles = c; } 
    bool confirmFiles() const { return m_confirmFiles; }
    void setConfirmStrings(bool c) { m_confirmStrings = c; } 
    bool confirmStrings() const { return m_confirmStrings; }
    void setConfirmDirs(bool c) { m_confirmDirs = c; } 
    bool confirmDirs() const { return m_confirmDirs; }
    void setHaltOnFirstOccur(bool fo) { m_haltOnFirstOccur = fo; }  
    bool haltOnFirstOccur() const { return m_haltOnFirstOccur; }
    void setIgnoreWhitespaces(bool iw) { m_ignoreWhitespaces = iw; }
    bool ignoreWhitespaces() const { return m_ignoreWhitespaces; }
    void setIgnoreHidden(bool ih) { m_ignoreHidden = ih; }
    bool ignoreHidden() const { return m_ignoreHidden; }
    void setSimulation(bool s) { m_simulation = s; }
    bool simulation() const { return m_simulation; }
    void setSearchMode(bool s) { m_searchMode = s; }
    bool searchMode() const { return m_searchMode; }
    
    void setOwnerUserIsChecked(bool chk) { m_ownerUserIsChecked = chk; }
    bool ownerUserIsChecked() const { return m_ownerUserIsChecked; }
    void setOwnerGroupIsChecked(bool chk) { m_ownerGroupIsChecked = chk; }
    bool ownerGroupIsChecked() const { return m_ownerGroupIsChecked; }   
    void setOwnerUserBool(const QString& user) { m_ownerUserBool = user; }
    QString ownerUserBool() const { return m_ownerUserBool; }
    void setOwnerGroupBool(const QString& group) { m_ownerGroupBool = group; }
    QString ownerGroupBool() const { return m_ownerGroupBool; }
       
    void setOwnerUserType(const QString& user) { m_ownerUserType = user; }
    QString ownerUserType() const { return m_ownerUserType; }
    void setOwnerGroupType(const QString& group) { m_ownerGroupType = group; }
    QString ownerGroupType() const { return m_ownerGroupType; }
    
    void setOwnerUserValue(const QString& user) { m_ownerUserValue = user; }
    QString ownerUserValue() const { return m_ownerUserValue; } 
    void setOwnerGroupValue(const QString& group) { m_ownerGroupValue = group; }
    QString ownerGroupValue() const { return m_ownerGroupValue; }
    
    void setBackupExtension(const QString& bkext) { m_backupExtension = bkext; }
    QString backupExtension() const { return m_backupExtension; }
        
};

#endif
