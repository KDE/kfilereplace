/***************************************************************************
                          knewprojectdlg.h  -  description
                             -------------------
    begin                : Tue Dec 28 1999
    copyright            : (C) 1999 by Fran�is Dupoux
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
#include "apistruct.h"

class QDate;
class KConfig;

class KNewProjectDlg : public KNewProjectDlgS
{
  Q_OBJECT

 public:
  KNewProjectDlg(QWidget *parent, KConfig *config, const char *name=0);
  virtual ~KNewProjectDlg();

 private:
  bool m_searchLater;
  KConfig *m_config;
  QDate m_MinDate;
  QDate m_MaxDate;
  unsigned long int m_MinimumSizeNumber;
  unsigned long int m_MaximumSizeNumber;

 protected slots:
  void slotDir();
  void slotLater();
  void slotOK();
  void slotEnableSpinboxSizeMin(bool b);
  void slotEnableSpinboxSizeMax(bool b);
 public:
  QString location();
  QString filter();
  QString searchFor();
  QString replaceWith();

  bool includeSubfolders();
  bool caseSensitive();
  bool enableWildcards();
  bool enableVariables();

  int accessType();
  bool isMinDate();
  bool isMaxDate();
  QDate minDate();
  QDate maxDate();

  bool isMinSize();
  bool isMaxSize();
  unsigned long int minSize();
  unsigned long int maxSize();

  bool isOwnerUser();
  bool isOwnerGroup();

  bool ownerUserMustBe();
  bool ownerGroupMustBe();

  QString ownerUserType();
  QString ownerGroupType();

  QString ownerUserValue();
  QString ownerGroupValue();

  void loadLocationsList();
  void loadFiltersList();

  void saveLocationsList();
  void saveFiltersList();

 // void addCurrentStringToCombo();
  void setDatas(const QString& strDir, const QString& strFilter);
  void setWhatsThis();

  void maxFilesSize(bool & bChecked, long unsigned int & nMaxSize);
  void minFilesSize(bool & bChecked, long unsigned int & nMinSize);

  bool searchLater() {return m_searchLater;}
};

#endif
