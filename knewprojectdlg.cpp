/***************************************************************************
                          knewprojectdlg.cpp  -  description
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


//QT

#include <qcheckbox.h>
#include <qspinbox.h>
#include <q3datetimeedit.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <q3textedit.h>
#include <q3listview.h>
//Added by qt3to4:
#include <QPixmap>

//KDE
#include <kseparator.h>
#include <kmessagebox.h>
#include <kcharsets.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <kglobal.h>
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


KNewProjectDlg::KNewProjectDlg(RCOptions* info, QWidget *parent, const char *name) : KNewProjectDlgS(parent, name)
{
  m_searchNowFlag = "";
  m_option = info;

  initGUI();

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

void KNewProjectDlg::saveRCOptions()
{
  saveOptions();
  saveFileSizeOptions();
  saveDateAccessOptions();
  saveOwnerOptions();
  saveLocationsList();
  saveFiltersList();
  saveBackupExtensionOptions();
}

void KNewProjectDlg::slotDir()
{
  QString directoryString = KFileDialog::getExistingDirectory(KUrl(), this, i18n("Project Directory"));
  if(!directoryString.isEmpty())
    m_cbLocation->setEditText(directoryString);
}

void KNewProjectDlg::slotOK()
{
   // Check that Search text and Filter are not empty
   m_option->m_directories = m_cbLocation->currentText();
   m_option->m_filters = m_cbFilter->currentText();
   if(!m_leSearch->text().isEmpty())
     {
       if(m_leReplace->text().isEmpty())
         m_option->m_searchingOnlyMode = true;
       else
         m_option->m_searchingOnlyMode = false;
     }
   m_option->m_quickSearchString = m_searchNowFlag + m_leSearch->text();
   m_option->m_quickReplaceString = m_searchNowFlag + m_leReplace->text();

   if (m_option->m_directories.isEmpty() || m_option->m_filters.isEmpty())
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
  m_option->m_quickSearchString = m_searchNowFlag;
  m_option->m_quickReplaceString = m_searchNowFlag;

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

//PRIVATE
void KNewProjectDlg::initGUI()
{
  QIcon iconSet = SmallIconSet("fileopen");
  QPixmap pixMap = iconSet.pixmap( QIcon::Small, QIcon::Normal );

  m_pbLocation->setIconSet(iconSet);
  m_pbLocation->setFixedSize(pixMap.width() + 8, pixMap.height() + 8);

  m_pbSearchNow->setEnabled(false);

  loadOptions();
  loadFileSizeOptions();
  loadDateAccessOptions();
  loadOwnerOptions();
  loadBackupExtensionOptions();
  loadLocationsList();
  loadFiltersList();
}

void KNewProjectDlg::loadOptions()
{
  QStringList availableEncodingNames(KGlobal::charsets()->availableEncodingNames());
  m_cbEncoding->addItems(availableEncodingNames);
  int idx = -1;
  int utf8Idx = -1;
  for (uint i = 0; i < availableEncodingNames.count(); i++)
  {
    if (availableEncodingNames[i] == m_option->m_encoding)
    {
      idx = i;
      break;
    }
    if (availableEncodingNames[i] == "utf8")
    {
      utf8Idx = i;
    }
  }
  if (idx != -1)
    m_cbEncoding->setCurrentIndex(idx);
  else 
    m_cbEncoding->setCurrentIndex(utf8Idx);
    
  m_chbIncludeSubfolders->setChecked(m_option->m_recursive);
  m_chbCaseSensitive->setChecked(m_option->m_caseSensitive);
  m_chbEnableVariables->setChecked(m_option->m_variables);
  m_chbRegularExpressions->setChecked(m_option->m_regularExpressions);
}

void KNewProjectDlg::loadFileSizeOptions()
{
  int size = m_option->m_minSize;
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

  size = m_option->m_maxSize;
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

  QString date = m_option->m_minDate;
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

  date = m_option->m_maxDate;
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
  bool enableOwner = m_option->m_ownerUserIsChecked;

  m_chbOwnerUser->setChecked(enableOwner);
  m_cbOwnerUserType->setEnabled(enableOwner);
  m_cbOwnerUserBool->setEnabled(enableOwner);
  m_edOwnerUser->setEnabled(enableOwner);

  m_cbOwnerUserType->setCurrentText(m_option->m_ownerUserType);
  m_cbOwnerUserBool->setCurrentText(m_option->m_ownerUserBool);

  m_edOwnerUser->setText(m_option->m_ownerUserValue);

  enableOwner = m_option->m_ownerGroupIsChecked;

  m_chbOwnerGroup->setChecked(enableOwner);
  m_cbOwnerGroupType->setEnabled(enableOwner);
  m_cbOwnerGroupBool->setEnabled(enableOwner);
  m_edOwnerGroup->setEnabled(enableOwner);

  m_cbOwnerGroupType->setCurrentText(m_option->m_ownerGroupType);
  m_cbOwnerGroupBool->setCurrentText(m_option->m_ownerGroupBool);
  m_edOwnerGroup->setText(m_option->m_ownerGroupValue);
}

void KNewProjectDlg::loadLocationsList()
{
  m_cbLocation->addItems(QStringList::split(",",m_option->m_directories));
}

void KNewProjectDlg::loadFiltersList()
{
  m_cbFilter->addItems(QStringList::split(",",m_option->m_filters));
}

void KNewProjectDlg::loadBackupExtensionOptions()
{
  bool enableBackup = m_option->m_backup;

  m_chbBackup->setChecked(enableBackup);
  m_leBackup->setEnabled(enableBackup);
  m_tlBackup->setEnabled(enableBackup);
  m_leBackup->setText(m_option->m_backupExtension);
}

void KNewProjectDlg::saveOptions()
{
  m_option->m_encoding = m_cbEncoding->currentText();
  m_option->m_recursive = m_chbIncludeSubfolders->isChecked();
  m_option->m_caseSensitive = m_chbCaseSensitive->isChecked();
  m_option->m_variables = m_chbEnableVariables->isChecked();
  m_option->m_regularExpressions = m_chbRegularExpressions->isChecked();
}

void KNewProjectDlg::saveFileSizeOptions()
{
  if(m_chbSizeMax->isChecked())
    m_option->m_maxSize = m_spbSizeMax->value();
  else
    m_option->m_maxSize = FileSizeOption;

  if(m_chbSizeMin->isChecked())
    m_option->m_minSize = m_spbSizeMin->value();
  else
    m_option->m_minSize = FileSizeOption;
}

void KNewProjectDlg::saveDateAccessOptions()
{
  if(m_chbDateMin->isChecked() || m_chbDateMax->isChecked())
    m_option->m_dateAccess = m_cbDateValid->currentText();
  else
    m_option->m_dateAccess = ValidAccessDateOption;

  if(m_chbDateMin->isChecked())
    {
      QString date = m_dedDateMin->date().toString(Qt::ISODate);
      m_option->m_minDate = date;
    }
  else
    m_option->m_minDate = AccessDateOption;

  if(m_chbDateMax->isChecked())
    {
      QString date = m_dedDateMax->date().toString(Qt::ISODate);
      m_option->m_maxDate = date;
    }
  else
    m_option->m_maxDate = AccessDateOption;
}

void KNewProjectDlg::saveOwnerOptions()
{
  bool isChecked = m_chbOwnerUser->isChecked();
  if(isChecked)
    {
      m_option->m_ownerUserIsChecked = true;
      m_option->m_ownerUserType = m_cbOwnerUserType->currentText();
      m_option->m_ownerUserBool = m_cbOwnerUserBool->currentText();
      m_option->m_ownerUserValue = m_edOwnerUser->text();
    }
  else
    {
      m_option->m_ownerUserIsChecked = false;
      m_option->m_ownerUserType = "Name";
      m_option->m_ownerUserBool = "Equals To";
      m_option->m_ownerUserValue = "";
    }

  isChecked = m_chbOwnerGroup->isChecked();
  if(isChecked)
    {
      m_option->m_ownerGroupIsChecked = true;
      m_option->m_ownerGroupType = m_cbOwnerGroupType->currentText();
      m_option->m_ownerGroupBool = m_cbOwnerGroupBool->currentText();
      m_option->m_ownerGroupValue = m_edOwnerGroup->text();
    }
  else
    {
      m_option->m_ownerGroupIsChecked = false;
      m_option->m_ownerGroupType = "Name";
      m_option->m_ownerGroupBool = "Equals To";
      m_option->m_ownerGroupValue = "";
    }
}

void KNewProjectDlg::saveLocationsList()
{
  QString current = m_cbLocation->currentText(), list = current;

  int count = m_cbLocation->count(),
      i;
  for(i = 0; i < count; i++)
    {
      QString text =  m_cbLocation->itemText(i);
      if(text != current)
        list += ','+text;
    }
  m_option->m_directories = list;
}

void KNewProjectDlg::saveFiltersList()
{
  QString current = m_cbFilter->currentText(), list = current;

  int count = m_cbFilter->count(),
      i;
  for(i = 0; i < count; i++)
    {
      QString text =  m_cbFilter->itemText(i);
      if(text != current)
        list += ','+text;
    }
  m_option->m_filters = list;
}

void KNewProjectDlg::saveBackupExtensionOptions()
{
  QString backupExt = m_leBackup->text();
  m_option->m_backup = (m_chbBackup->isChecked() && !backupExt.isEmpty());
  m_option->m_backupExtension = backupExt;
}

void KNewProjectDlg::setDatas(const QString& directoryString, const QString& filterString)
{
  if (!directoryString.isEmpty())
    m_cbLocation->setEditText(directoryString);

  if (!filterString.isEmpty())
    m_cbFilter->setEditText(filterString);
}

bool KNewProjectDlg::contains(Q3ListView* lv,const QString& s, int column)
{
  Q3ListViewItem* i = lv->firstChild();
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
  m_cbLocation->setWhatsThis( cbLocationWhatthis);
  m_cbFilter->setWhatsThis( cbFilterWhatthis);

  m_spbSizeMin->setWhatsThis( edSizeMinWhatthis);
  m_spbSizeMax->setWhatsThis( edSizeMaxWhatthis);

  m_cbDateValid->setWhatsThis( cbDateValidWhatthis);
  m_chbDateMin->setWhatsThis( chbDateMinWhatthis);
  m_chbDateMax->setWhatsThis( chbDateMaxWhatthis);

  m_chbIncludeSubfolders->setWhatsThis( chbRecursiveWhatthis);
  m_chbRegularExpressions->setWhatsThis( chbRegularExpressionsWhatthis);
  m_chbEnableVariables->setWhatsThis( chbVariablesWhatthis);
  m_chbCaseSensitive->setWhatsThis( chbCaseSensitiveWhatthis);
  m_chbBackup->setWhatsThis( chbBackupWhatthis);
  m_leBackup->setWhatsThis( chbBackupWhatthis);
  m_leSearch->setWhatsThis( leSearchWhatthis);
  m_leReplace->setWhatsThis( leReplaceWhatthis);
}

#include "knewprojectdlg.moc"

