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


KNewProjectDlg::KNewProjectDlg(QWidget *parent, KConfig *config, const char *name/*=0*/) : KNewProjectDlgS(parent, name, true)
{
  m_config = config;
  m_searchNowFlag = "";

  QIconSet iconSet = SmallIconSet("fileopen");
  QPixmap pixMap = iconSet.pixmap( QIconSet::Small, QIconSet::Normal );

  m_pbLocation->setIconSet(iconSet);
  m_pbLocation->setFixedSize(pixMap.width() + 8, pixMap.height() + 8);

  m_pbSearchNow->setEnabled(false);
  m_pbSearchLater->setEnabled(false);

  loadOptions();
  loadFileSizeFilter();
  loadDateAccessFilter();
  loadOwnerFilter();
  loadBackupExtensionFilter();
  loadLocationsList();
  loadFiltersList();

  connect(m_pbLocation, SIGNAL(clicked()), this, SLOT(slotDir()));
  connect(m_pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
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
  m_config = 0L;
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
   m_info.setDirectory(m_cbLocation->currentText());
   m_info.setFilter(m_cbFilter->currentText());

   m_info.setQuickSearchString(m_searchNowFlag+m_leSearch->text());
   m_info.setQuickReplaceString(m_searchNowFlag+m_leReplace->text());

   if (m_info.directory().isEmpty() || m_info.filter().isEmpty())
     {
       KMessageBox::error(this, i18n("You must fill the combo boxes (directory and filter) before continuing."));
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

   saveOptions();
   saveFileSizeFilter();
   saveDateAccessFilter();
   saveOwnerFilter();
   saveLocationsList();
   saveFiltersList();
   saveBackupExtensionFilter();

   accept();
}

void KNewProjectDlg::slotReject()
{
  // to reject simply we accept nothing... logical, isn't it?
  m_info.setQuickSearchString(m_searchNowFlag);
  m_info.setQuickReplaceString(m_searchNowFlag);

  accept();
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
  if(t.isEmpty())
    {
      m_pbSearchNow->setEnabled(false);
      m_pbSearchLater->setEnabled(false);
    }
  else
    {
      m_pbSearchNow->setEnabled(true);
      m_pbSearchLater->setEnabled(true);
    }
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
  m_config->setGroup("Options");
  m_chbIncludeSubfolders->setChecked(m_config->readBoolEntry(rcRecursive,RecursiveOption));
  m_chbCaseSensitive->setChecked(m_config->readBoolEntry(rcCaseSensitive, CaseSensitiveOption));
  m_chbEnableVariables->setChecked(m_config->readBoolEntry(rcVariables, VariablesOption));
  m_chbRegularExpressions->setChecked(m_config->readBoolEntry(rcRegularExpressions, RegularExpressionsOption));
}

void KNewProjectDlg::loadFileSizeFilter()
{
  m_config->setGroup("Options");
  //  FILE SIZE OPTIONS
  int size = m_config->readNumEntry(rcMinFileSize,FileSizeOption);

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

  size = m_config->readNumEntry(rcMaxFileSize,FileSizeOption);

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

void KNewProjectDlg::loadDateAccessFilter()
{
  m_config->setGroup("Options");
  // ================== DATE OPTIONS ========================

  QString validDate = m_config->readEntry(rcValidAccessDate,ValidAccessDateOption);

  QString dateMin = m_config->readEntry(rcMinDate,AccessDateOption);
  if(dateMin == AccessDateOption)
    {
      m_chbDateMin->setChecked(false);
      m_dedDateMin->setDate(m_dedDateMin->minValue());
      m_dedDateMin->setEnabled(false);
    }
  else
    {
      m_chbDateMin->setChecked(true);
      m_dedDateMin->setDate(QDate::fromString(dateMin,Qt::ISODate));
      m_dedDateMin->setEnabled(true);
    }

  QString dateMax = m_config->readEntry(rcMaxDate,AccessDateOption);
  if(dateMax == AccessDateOption)
    {
      m_chbDateMax->setChecked(false);
      m_dedDateMax->setDate(m_dedDateMax->maxValue());
      m_dedDateMax->setEnabled(false);
    }
  else
    {
      m_chbDateMax->setChecked(true);
      m_dedDateMax->setDate(QDate::fromString(dateMax,Qt::ISODate));
      m_dedDateMax->setEnabled(true);
    }

  m_cbDateValid->setEnabled(m_chbDateMax->isChecked() || m_chbDateMin->isChecked());
}

void KNewProjectDlg::loadOwnerFilter()
{
  m_config->setGroup("Options");

  QStringList ownerList = QStringList::split(",",m_config->readEntry(rcOwnerUser, OwnerOption),true);

  bool enableOwner = ((ownerList[0] == "true") ? true : false);

  m_chbOwnerUser->setChecked(enableOwner);
  m_cbOwnerUserType->setEnabled(enableOwner);
  m_cbOwnerUserBool->setEnabled(enableOwner);
  m_edOwnerUser->setEnabled(enableOwner);

  m_cbOwnerUserType->setCurrentText(ownerList[1]);
  m_cbOwnerUserBool->setCurrentText(ownerList[2]);

  if(ownerList[3] == "???")
    m_edOwnerUser->clear();
  else
    m_edOwnerUser->setText(ownerList[3]);

  ownerList = QStringList::split(",",m_config->readEntry(rcOwnerGroup, OwnerOption),true);

  enableOwner = ((ownerList[0] == "true") ? true : false);

  m_chbOwnerGroup->setChecked(enableOwner);
  m_cbOwnerGroupType->setEnabled(enableOwner);
  m_cbOwnerGroupBool->setEnabled(enableOwner);
  m_edOwnerGroup->setEnabled(enableOwner);

  m_cbOwnerGroupType->setCurrentText(ownerList[1]);
  m_cbOwnerGroupBool->setCurrentText(ownerList[2]);

  if(ownerList[3] == "???")
    m_edOwnerGroup->clear();
  else
    m_edOwnerGroup->setText(ownerList[3]);
}

void KNewProjectDlg::loadLocationsList()
{
  m_config->setGroup("Directories");
  #if KDE_IS_VERSION(3,1,3)
  QString locationsEntry = m_config->readPathEntry(rcDirectoriesList);
  #else
  QString locationsEntry = m_config->readEntry(rcDirectoriesList);
  #endif
  QStringList locationsEntryList = QStringList::split(",",locationsEntry);

  m_cbLocation->insertStringList(locationsEntryList);
  if(m_cbLocation->count() == 0)
    m_cbLocation->insertItem(QDir::current().path());
}

void KNewProjectDlg::loadFiltersList()
{
  m_config->setGroup("Filters");
  #if KDE_IS_VERSION(3,1,3)
  QStringList filtersEntryList = m_config->readPathListEntry(rcFiltersList);
  #else
  QStringList filtersEntryList = m_config->readListEntry(rcFiltersList);
  #endif

  m_cbFilter->insertStringList(filtersEntryList);
  if(m_cbFilter->count() == 0)
    m_cbFilter->insertItem("*.htm;*.html;*.xml;*.xhtml;*.css;*.js;*.php");
}

void KNewProjectDlg::loadBackupExtensionFilter()
{
  m_config->setGroup("Options");

  QString backupExtension = m_config->readEntry(rcBackupExtension,BackupExtensionOption);
  QStringList bkList = QStringList::split(",",backupExtension,true);

  bool enableBackup = (bkList[0] == "true" ? true : false);

  m_chbBackup->setChecked(enableBackup);
  m_leBackup->setEnabled(enableBackup);
  m_tlBackup->setEnabled(enableBackup);
  m_leBackup->setText(bkList[1]);
}

void KNewProjectDlg::saveOptions()
{
  m_config->setGroup("Options");

  m_config->writeEntry(rcRecursive, m_chbIncludeSubfolders->isChecked());
  m_config->writeEntry(rcCaseSensitive, m_chbCaseSensitive->isChecked());
  m_config->writeEntry(rcVariables, m_chbEnableVariables->isChecked());
  m_config->writeEntry(rcRegularExpressions, m_chbRegularExpressions->isChecked());

  m_config->sync();
}

void KNewProjectDlg::saveFileSizeFilter()
{
  m_config->setGroup("Options");
  //  FILE SIZE OPTIONS
  if(m_chbSizeMax->isChecked())
    m_config->writeEntry(rcMaxFileSize, m_spbSizeMax->value());
  else
    m_config->writeEntry(rcMaxFileSize,FileSizeOption );

  if(m_chbSizeMin->isChecked())
    m_config->writeEntry(rcMinFileSize, m_spbSizeMin->value());
  else
    m_config->writeEntry(rcMinFileSize,FileSizeOption );

  m_config->sync();
}

void KNewProjectDlg::saveDateAccessFilter()
{
  m_config->setGroup("Options");
  //  DATE OPTIONS
  if(m_chbDateMin->isChecked() or m_chbDateMax->isChecked())
    m_config->writeEntry(rcValidAccessDate, m_cbDateValid->currentText());
  else
    m_config->writeEntry(rcValidAccessDate,ValidAccessDateOption);

  if(m_chbDateMin->isChecked())
    {
      QString date = m_dedDateMin->date().toString(Qt::ISODate);
      m_config->writeEntry(rcMinDate, date);
    }
  else
    m_config->writeEntry(rcMinDate, AccessDateOption);

  if(m_chbDateMax->isChecked())
    {
      QString date = m_dedDateMax->date().toString(Qt::ISODate);
      m_config->writeEntry(rcMaxDate, date);
    }
  else
    m_config->writeEntry(rcMaxDate, AccessDateOption);

  m_config->sync();
}

void KNewProjectDlg::saveOwnerFilter()
{
  m_config->setGroup("Options");

  QString list;
  if(m_chbOwnerUser->isChecked())
    list = "true,"+ m_cbOwnerUserType->currentText()+","+m_cbOwnerUserBool->currentText()+","+m_edOwnerUser->text();
  else
    list = OwnerOption;

  m_config->writeEntry(rcOwnerUser,list);
  if(m_chbOwnerGroup->isChecked())
    list = "true,"+ m_cbOwnerGroupType->currentText()+","+m_cbOwnerGroupBool->currentText()+","+m_edOwnerGroup->text();
  else
    list = OwnerOption;

  m_config->writeEntry(rcOwnerGroup,list);
  m_config->sync();
}

void KNewProjectDlg::saveLocationsList()
{
  m_config->setGroup("Directories");

  QString current = m_cbLocation->currentText();
  QStringList locationsEntryList;
  int count = m_cbLocation->listBox()->count(),
      i;
  for(i = 0; i < count; i++)
    {
      QString text =  m_cbLocation->listBox()->item(i)->text();
      if(text != current)
        locationsEntryList.append(text);
    }
  locationsEntryList.prepend(current);

  m_config->writeEntry(rcDirectoriesList,locationsEntryList.join(","));
  m_config->sync();
}

void KNewProjectDlg::saveFiltersList()
{
  m_config->setGroup("Filters");

  QString current = m_cbFilter->currentText();
  QStringList filtersEntryList;
  int count = m_cbFilter->listBox()->count(),
      i;
  for(i = 0; i < count; i++)
    {
      QString text =  m_cbFilter->listBox()->item(i)->text();
      if(text != current)
        filtersEntryList.append(text);
    }
  filtersEntryList.prepend(current);

  m_config->writeEntry(rcFiltersList,filtersEntryList.join(","));
  m_config->sync();
}

void KNewProjectDlg::saveBackupExtensionFilter()
{
  m_config->setGroup("Options");
  QString bkOptions;
  if(m_chbBackup->isChecked())
    bkOptions = "true,"+m_leBackup->text();
  else
    bkOptions = "false,"+m_leBackup->text();

  m_config->writeEntry(rcBackupExtension,bkOptions);
  m_config->sync();
}

void KNewProjectDlg::setDatas(const QString& directoryString, const QString& filterString)
{
  if (not directoryString.isEmpty())
    m_cbLocation->setEditText(directoryString);

  if (not filterString.isEmpty())
    m_cbFilter->setEditText(filterString);
}

QString KNewProjectDlg::currentDir() const
{
  return m_cbLocation->currentText();
}

QString KNewProjectDlg::currentFilter() const
{
  return m_cbFilter->currentText();
}

QString KNewProjectDlg::quickSearchString() const
{
  return m_info.quickSearchString();
}

QString KNewProjectDlg::quickReplaceString() const
{
  return m_info.quickReplaceString();
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

void KNewProjectDlg::slotHelp()
{
  kapp->invokeHelp(QString::null, "kfilereplace");
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

