/***************************************************************************
                          kpropertiesdlg.h  -  description
                             -------------------
    begin                : Tue Dec 28 1999
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

#ifndef KNEWPROJECTDLG_H
#define KNEWPROJECTDLG_H

#include "knewprojectdlgs.h"
#include "apistruct.h"

#include <qtabdialog.h>
#include <qdatetime.h>
#include <qwhatsthis.h>
#include <qregexp.h>

class KComboBox;
class QPushButton;
class QLabel;
class KConfig;
class QCheckBox;

// ===========================================================================================================================
class KNewProjectDlg : public KNewProjectDlgS
{
  Q_OBJECT

 public:
  KNewProjectDlg(QWidget *parent, KConfig *config, const char *name=0);
  virtual ~KNewProjectDlg();
  void loadComboValues();
  void saveComboValues();
  void addCurrentStringToCombo();
  void setDatas(const QString& strDir, const QString& strFilter);
  void setWhatsThis();
  void getMaxFilesSize(bool *bChecked, long unsigned int *nMaxSize);
  void getMinFilesSize(bool *bChecked, long unsigned int *nMinSize);

 private:
  KConfig *m_config;

 protected:

 protected slots:
  void slotDir();
  void slotOk();

 public: // retrun results
  QString getDirectory();
  QString getFilter();

  int getTypeOfAccess();
  bool getBoolMinDate();
  bool getBoolMaxDate();
  QDate getMinDate();
  QDate getMaxDate();

  bool getBoolMinSize();
  bool getBoolMaxSize();
  unsigned long int getMinSize();
  unsigned long int getMaxSize();

  bool getBoolOwnerUser();
  bool getBoolOwnerGroup();

  bool getOwnerUserMustBe();
  bool getOwnerGroupMustBe();

  QString getOwnerUserType();
  QString getOwnerGroupType();

  QString getOwnerUserValue();
  QString getOwnerGroupValue();

 private:
  QDate m_qdMinDate;
  QDate m_qdMaxDate;
  unsigned long int m_nMinSize;
  unsigned long int m_nMaxSize;

};

#endif
