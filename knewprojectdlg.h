/***************************************************************************
                          knewprojectdlg.h  -  description
                             -------------------
    begin                : Tue Dec 28 1999
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

#ifndef KNEWPROJECTDLG_H
#define KNEWPROJECTDLG_H

#include "knewprojectdlgs.h"
#include "configurationclasses.h"

class KConfig;

class KNewProjectDlg : public KNewProjectDlgS
{
  Q_OBJECT
  
  public:
    ConfigurationInformation m_info;
    
  private:
    KConfig *m_config;
    QMap<QString,QString> m_map;
  
  public:
    KNewProjectDlg(QWidget *parent, KConfig *config, const char *name=0);
    virtual ~KNewProjectDlg();
    
  public:
    void loadOptions();
    void loadFileSizeFilter();
    void loadDateAccessFilter();
    void loadOwnerFilter();
    void loadLocationsList();
    void loadFiltersList();
    void loadBackupExtensionFilter();
    QString currentDir() const; 
    QString currentFilter() const;
    QMap<QString,QString> stringsMap() const; 
    void saveOptions();
    void saveFileSizeFilter();
    void saveDateAccessFilter();
    void saveOwnerFilter();
    void saveLocationsList();
    void saveFiltersList();
    void saveBackupExtensionFilter();
    void setDatas(const QString& directoryString, const QString& filterString);
    void setWhatsThis();
     
  protected slots:
    void slotDir();
    void slotOK();
    void slotAdd();
    void slotDel();
    void slotSearchOnly(bool b);
    void slotSearchReplace(bool b);
    void slotEnableSpinboxSizeMin(bool b);
    void slotEnableSpinboxSizeMax(bool b);
    void slotEnableCbValidDate(bool b);
    void slotEnableChbUser(bool b);
    void slotEnableChbGroup(bool b); 
    void slotEnableChbBackup(bool b);
  private:
    bool contains(QListView* lv,const QString& s, int column);
    void setMap();  
};

#endif  // KNewProjectDlg
