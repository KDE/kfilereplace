/***************************************************************************
                          knewprojectdlg.cpp  -  description
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


//QT
#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qdatetimeedit.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qlistview.h>

//KDE
#include <kseparator.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kapplication.h>

// local
#include "knewprojectdlg.h"
#include "whatthis.h"

using namespace whatthisNameSpace;


KNewProjectDlg::KNewProjectDlg(QWidget *parent, const char *name) : KNewProjectDlgS(parent, name)
{
  m_searchNowFlag = "";

  QIconSet iconSet = SmallIconSet("fileopen");
  QPixmap pixMap = iconSet.pixmap( QIconSet::Small, QIconSet::Normal );

  m_pbLocation->setIconSet(iconSet);
  m_pbLocation->setFixedSize(pixMap.width() + 8, pixMap.height() + 8);

  m_pbSearchNow->setEnabled(false);

  connect(m_pbLocation, SIGNAL(clicked()), this, SLOT(slotDir()));
  connect(m_pbCancel, SIGNAL(clicked()), this, SLOT(slotReject()));
  connect(m_pbSearchNow, SIGNAL(clicked()), this, SLOT(slotSearchNow()));
  connect(m_pbSearchLater, SIGNAL(clicked()), this, SLOT(slotSearchLater()));
  connect(m_leSearch, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearchLineEdit(const QString&)));
  connect(m_chbSizeMin, SIGNAL(toggled(bool)), this, SLOT(slotEnableSpinboxSizeMin(bool)));
  connect(m_chbSizeMax, SIGNAL(toggled(bool)), this, SLOT(slotEnableSpinboxSizeMax(bool)));
  connect(m_chbDateMin, SIGNAL(toggled(bool)), m_dedDateMin, SLOT(setEnabled(bool)));
  connect(m_chbDateMax, SIGNAL(toggled(bool)), m_dedDateMax, SLOT(setEnabled(bool)));
  connect(m_chbDateMin,SIGNAL(toggled(bool)),this, SLOT(slotEnableCbValidDate(bool)));
  connect(m_chbDateMax,SIGNAL(toggled(bool)),this, SLOT(slotEnableCbValidDate(bool)));
  connect(m_chbOwnerUser, SIGNAL(toggled(bool)), this, SLOT(slotEnableChbUser(bool)));
  connect(m_chbOwnerGroup, SIGNAL(toggled(bool)), this, SLOT(slotEnableChbGroup(bool)));
  connect(m_chbBackup, SIGNAL(toggled(bool)), this, SLOT(slotEnableChbBackup(bool)));
  connect(m_pbHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));

  whatsThis();
}

KNewProjectDlg::~KNewProjectDlg()
{
}

void KNewProjectDlg::readOptions(const RCOptions& info)
{
  m_option = info;

  loadOptions();
  loadFileSizeOptions();
  loadDateAccessOptions();
  loadOwnerOptions();
  loadBackupExtensionOptions();
  loadLocationsList();
  loadFiltersList();
}

RCOptions KNewProjectDlg::writeOptions()
{
  saveOptions();
  saveFileSizeOptions();
  saveDateAccessOptions();
  saveOwnerOptions();
  saveLocationsList();
  saveFiltersList();
  saveBackupExtensionOptions();

  return m_option;
}

void KNewProjectDlg::slotDir()
{
  QString directoryString = KFileDialog::getExistingDirectory(QString::null, this, i18n("Project Directory"));
  if(!directoryString.isEmpty())
    m_cbLocation->setEditText(directoryString);
}

void KNewProjectDlg::slotOK()
{
   // Check that Search text and Filter are not empty
   m_option.setDirectories(m_cbLocation->currentText());
   m_option.setFilters(m_cbFilter->currentText());

   m_option.setQuickSearchString(m_searchNowFlag+m_leSearch->text());
   m_option.setQuickReplaceString(m_searchNowFlag+m_leReplace->text());

   if (m_option.directories().isEmpty() || m_option.filters().isEmpty())
     {
       KMessageBox::error(this, i18n("You must fill the combo boxes (location and filter) before continuing."));
       return;
     }

   //  OWNER OPTIONS
   if ((m_chbOwnerUser->isChecked() && m_edOwnerUser->text().isEmpty()) ||
       (m_chbOwnerGroup->isChecked() && m_edOwnerGroup->text().isEmpty()))
   {
      KMessageBox::error(this, i18n("Some edit boxes are empty in the <b>Owner</b> page."));
      return ;
   }

   // Check option "Size Min/Max": check MinSize is not greater than MaxSize
   int minSize = m_spbSizeMin->value(),
       maxSize = m_spbSizeMax->value();
   if ((minSize != FileSizeOption) && (maxSize != FileSizeOption))
    if (minSize > maxSize)
   {
      KMessageBox::error(this, i18n("The minimum size is greater than the maximum size."));
      return ;
   }

  accept();
}

void KNewProjectDlg::slotReject()
{
  m_option.setQuickSearchString(m_searchNowFlag);
  m_option.setQuickReplaceString(m_searchNowFlag);

  reject();
}

void KNewProjectDlg::slotSearchNow()
{ //Add a 'N' to represent the status search-now
  m_searchNowFlag = "N";
  slotOK();
}

void KNewProjectDlg::slotSearchLater()
{ //Add a 'L' to represent the status search-later
  m_searchNowFlag = "L";
  slotOK();
}

void KNewProjectDlg::slotSearchLineEdit(const QString& t)
{
  m_pbSearchNow->setEnabled(!t.isEmpty());
}

void KNewProjectDlg::slotEnableSpinboxSizeMin(bool b)
{
  m_spbSizeMin->setEnabled(b);
}

void KNewProjectDlg::slotEnableSpinboxSizeMax(bool b)
{
  m_spbSizeMax->setEnabled(b);
}

void KNewProjectDlg::slotEnableCbValidDate(bool b)
{
  Q_UNUSED(b);
  m_cbDateValid->setEnabled(m_chbDateMax->isChecked() || m_chbDateMin->isChecked());
}

void KNewProjectDlg::slotEnableChbUser(bool b)
{
  m_cbOwnerUserType->setEnabled(b);
  m_cbOwnerUserBool->setEnabled(b);
  m_edOwnerUser->setEnabled(b);
}

void KNewProjectDlg::slotEnableChbGroup(bool b)
{
  m_cbOwnerGroupType->setEnabled(b);
  m_cbOwnerGroupBool->setEnabled(b);
  m_edOwnerGroup->setEnabled(b);
}

void KNewProjectDlg::slotEnableChbBackup(bool b)
{
  m_leBackup->setEnabled(b);
  m_tlBackup->setEnabled(b);
}

void KNewProjectDlg::loadOptions()
{
  m_chbIncludeSubfolders->setChecked(m_option.recursive());
  m_chbCaseSensitive->setChecked(m_option.caseSensitive());
  m_chbEnableVariables->setChecked(m_option.variables());
  m_chbRegularExpressions->setChecked(m_option.regularExpressions());
}

void KNewProjectDlg::loadFileSizeOptions()
{
  int size = m_option.minSize();
  if(size == FileSizeOption)
    {
      m_chbSizeMin->setChecked(false);
      m_spbSizeMin->setEnabled(false);
      m_spbSizeMin->setValue(0);
    }
  else
    {
      m_chbSizeMin->setChecked(true);
      m_spbSizeMin->setEnabled(true);
      m_spbSizeMin->setValue(size);
    }

  size = m_option.maxSize();
  if(size == FileSizeOption)
    {
      m_chbSizeMax->setChecked(false);
      m_spbSizeMax->setEnabled(false);
      m_spbSizeMax->setValue(0);
    }
  else
    {
      m_chbSizeMax->setChecked(true);
      m_spbSizeMax->setEnabled(true);
      m_spbSizeMax->setValue(size);
    }
}

void KNewProjectDlg::loadDateAccessOptions()
{
  // ================== DATE OPTIONS ========================

  QString date = m_option.minDate();
  if(date == AccessDateOption)
    {
      m_chbDateMin->setChecked(false);
      m_dedDateMin->setDate(m_dedDateMin->minValue());
      m_dedDateMin->setEnabled(false);
    }
  else
    {
      m_chbDateMin->setChecked(true);
      m_dedDateMin->setDate(QDate::fromString(date,Qt::ISODate));
      m_dedDateMin->setEnabled(true);
    }

  date = m_option.maxDate();
  if(date == AccessDateOption)
    {
      m_chbDateMax->setChecked(false);
      m_dedDateMax->setDate(m_dedDateMax->maxValue());
      m_dedDateMax->setEnabled(false);
    }
  else
    {
      m_chbDateMax->setChecked(true);
      m_dedDateMax->setDate(QDate::fromString(date,Qt::ISODate));
      m_dedDateMax->setEnabled(true);
    }

  m_cbDateValid->setEnabled(m_chbDateMax->isChecked() || m_chbDateMin->isChecked());

}

void KNewProjectDlg::loadOwnerOptions()
{
  bool enableOwner = m_option.ownerUserIsChecked();

  m_chbOwnerUser->setChecked(enableOwner);
  m_cbOwnerUserType->setEnabled(enableOwner);
  m_cbOwnerUserBool->setEnabled(enableOwner);
  m_edOwnerUser->setEnabled(enableOwner);

  m_cbOwnerUserType->setCurrentText(m_option.ownerUserType());
  m_cbOwnerUserBool->setCurrentText(m_option.ownerUserBool());

  m_edOwnerUser->setText(m_option.ownerUserValue());

  enableOwner = m_option.ownerGroupIsChecked();

  m_chbOwnerGroup->setChecked(enableOwner);
  m_cbOwnerGroupType->setEnabled(enableOwner);
  m_cbOwnerGroupBool->setEnabled(enableOwner);
  m_edOwnerGroup->setEnabled(enableOwner);

  m_cbOwnerGroupType->setCurrentText(m_option.ownerGroupType());
  m_cbOwnerGroupBool->setCurrentText(m_option.ownerGroupBool());
  m_edOwnerGroup->setText(m_option.ownerGroupValue());
}

void KNewProjectDlg::loadLocationsList()
{
  m_cbLocation->insertStringList(QStringList::split(",",m_option.directories()));
}

void KNewProjectDlg::loadFiltersList()
{
  m_cbFilter->insertStringList(QStringList::split(",",m_option.filters()));
}

void KNewProjectDlg::loadBackupExtensionOptions()
{
  bool enableBackup = m_option.backup();

  m_chbBackup->setChecked(enableBackup);
  m_leBackup->setEnabled(enableBackup);
  m_tlBackup->setEnabled(enableBackup);
  m_leBackup->setText(m_option.backupExtension());
}

void KNewProjectDlg::saveOptions()
{
  m_option.setRecursive(m_chbIncludeSubfolders->isChecked());
  m_option.setCaseSensitive(m_chbCaseSensitive->isChecked());
  m_option.setVariables(m_chbEnableVariables->isChecked());
  m_option.setRegularExpressions(m_chbRegularExpressions->isChecked());
}

void KNewProjectDlg::saveFileSizeOptions()
{
  if(m_chbSizeMax->isChecked())
    m_option.setMaxSize(m_spbSizeMax->value());
  else
    m_option.setMaxSize(FileSizeOption);

  if(m_chbSizeMin->isChecked())
    m_option.setMinSize(m_spbSizeMin->value());
  else
    m_option.setMinSize(FileSizeOption);
}

void KNewProjectDlg::saveDateAccessOptions()
{
  if(m_chbDateMin->isChecked() || m_chbDateMax->isChecked())
    m_option.setDateAccess(m_cbDateValid->currentText());
  else
    m_option.setDateAccess(ValidAccessDateOption);

  if(m_chbDateMin->isChecked())
    {
      QString date = m_dedDateMin->date().toString(Qt::ISODate);
      m_option.setMinDate(date);
    }
  else
    m_option.setMinDate(AccessDateOption);

  if(m_chbDateMax->isChecked())
    {
      QString date = m_dedDateMax->date().toString(Qt::ISODate);
      m_option.setMaxDate(date);
    }
  else
    m_option.setMaxDate(AccessDateOption);
}

void KNewProjectDlg::saveOwnerOptions()
{
  bool isChecked = m_chbOwnerUser->isChecked();
  if(isChecked)
    {
      m_option.setOwnerUserIsChecked(true);
      m_option.setOwnerUserType(m_cbOwnerUserType->currentText());
      m_option.setOwnerUserBool(m_cbOwnerUserBool->currentText());
      m_option.setOwnerUserValue(m_edOwnerUser->text());
    }
  else
    {
      m_option.setOwnerUserIsChecked(false);
      m_option.setOwnerUserType("Name");
      m_option.setOwnerUserBool("Equals To");
      m_option.setOwnerUserValue("");
    }

  isChecked = m_chbOwnerGroup->isChecked();
  if(isChecked)
    {
      m_option.setOwnerGroupIsChecked(true);
      m_option.setOwnerGroupType(m_cbOwnerGroupType->currentText());
      m_option.setOwnerGroupBool(m_cbOwnerGroupBool->currentText());
      m_option.setOwnerGroupValue(m_edOwnerGroup->text());
    }
  else
    {
      m_option.setOwnerGroupIsChecked(false);
      m_option.setOwnerGroupType("Name");
      m_option.setOwnerGroupBool("Equals To");
      m_option.setOwnerGroupValue("");
    }  
}

void KNewProjectDlg::saveLocationsList()
{
  QString current = m_cbLocation->currentText(), list = current;

  int count = m_cbLocation->listBox()->count(),
      i;
  for(i = 0; i < count; i++)
    {
      QString text =  m_cbLocation->listBox()->item(i)->text();
      if(text != current)
        list += ","+text;
    }
  m_option.setDirectories(list);
}

void KNewProjectDlg::saveFiltersList()
{
  QString current = m_cbFilter->currentText(), list = current;

  int count = m_cbFilter->listBox()->count(),
      i;
  for(i = 0; i < count; i++)
    {
      QString text =  m_cbFilter->listBox()->item(i)->text();
      if(text != current)
        list += ","+text;
    }
  m_option.setFilters(list);
}

void KNewProjectDlg::saveBackupExtensionOptions()
{
  m_option.setBackup(m_chbBackup->isChecked());
  m_option.setBackupExtension(m_leBackup->text());
}

void KNewProjectDlg::setDatas(const QString& directoryString, const QString& filterString)
{
  if (not directoryString.isEmpty())
    m_cbLocation->setEditText(directoryString);

  if (not filterString.isEmpty())
    m_cbFilter->setEditText(filterString);
}

bool KNewProjectDlg::contains(QListView* lv,const QString& s, int column)
{
  QListViewItem* i = lv->firstChild();
  while (i != 0)
    {
      if(i->text(column) == s)
        return true;
      i = i->nextSibling();
    }
  return false;
}

void KNewProjectDlg::whatsThis()
{
  QWhatsThis::add(m_cbLocation, cbLocationWhatthis);
  QWhatsThis::add(m_cbFilter, cbFilterWhatthis);

  QWhatsThis::add(m_spbSizeMin, edSizeMinWhatthis);
  QWhatsThis::add(m_spbSizeMax, edSizeMaxWhatthis);

  QWhatsThis::add(m_cbDateValid, cbDateValidWhatthis);
  QWhatsThis::add(m_chbDateMin, chbDateMinWhatthis);
  QWhatsThis::add(m_chbDateMax, chbDateMaxWhatthis);

  QWhatsThis::add(m_chbIncludeSubfolders, chbRecursiveWhatthis);
  QWhatsThis::add(m_chbRegularExpressions, chbRegularExpressionsWhatthis);
  QWhatsThis::add(m_chbEnableVariables, chbVariablesWhatthis);
  QWhatsThis::add(m_chbCaseSensitive, chbCaseSensitiveWhatthis);
  QWhatsThis::add(m_chbBackup, chbBackupWhatthis);
  QWhatsThis::add(m_leBackup, chbBackupWhatthis);
  QWhatsThis::add(m_leSearch, leSearchWhatthis);
  QWhatsThis::add(m_leReplace, leReplaceWhatthis);
}

#include "knewprojectdlg.moc"

