/***************************************************************************
                          koptionsdlg.cpp  -  description
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

// qt
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
// kde
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
//#include <kdebug.h>

// local
#include "resource.h"
#include "whatthis.h"

#include "koptionsdlg.h"


using namespace whatthisNameSpace;


KOptionsDlg::KOptionsDlg(QWidget *parent, const char *name) : KOptionsDlgS(parent,name,true)
{
  QString configName = locateLocal("config", "kfilereplacerc");
  m_config = new KConfig(configName);
  whatsThisPage();
  
  loadOptions();

  connect(pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(pbDefault, SIGNAL(clicked()),this,SLOT(slotDefaults()));
  connect(chbBackup, SIGNAL(toggled(bool)), this, SLOT(slotChbBackup(bool)));
}

KOptionsDlg::~KOptionsDlg()
{
}

void KOptionsDlg::slotOK()
{
  saveOptions();
  accept();
}

/** Set defaults values for all options of the dialog */
void KOptionsDlg::slotDefaults() 
{
  chbCaseSensitive->setChecked(CaseSensitiveOption);
  chbRecursive->setChecked(RecursiveOption);
  chbHaltOnFirstOccurrence->setChecked(StopWhenFirstOccurenceOption);
  chbIgnoreWhitespaces->setChecked(IgnoreWhiteSpacesOption);
  chbFollowSymLinks->setChecked(FollowSymbolicLinksOption);
  chbIgnoreHidden->setChecked(IgnoreHiddenOption);
  chbWildcards->setChecked(WildcardsOption);
  QStringList bkList = QStringList::split(",",BackupExtension,true);
  if(bkList[0] == "true")
    {
      chbBackup->setChecked(true);
      leBackup->setEnabled(true);
      tlBackup->setEnabled(true);
    }
  else
    {
      chbBackup->setChecked(false);
      leBackup->setEnabled(false);
      tlBackup->setEnabled(false);
    }
  leBackup->setText(bkList[1]);
  
  chbVariables->setChecked(VariablesOption);
  chbConfirmDirs->setChecked(ConfirmDirectoriesOption);
  chbConfirmFiles->setChecked(ConfirmFilesOption);
  chbConfirmStrings->setChecked(ConfirmStringsOption);
}

void KOptionsDlg::slotChbBackup(bool b)
{
  leBackup->setEnabled(b);
  tlBackup->setEnabled(b);
}

void KOptionsDlg::loadOptions()
{
  m_config->setGroup("Options");
     
  chbCaseSensitive->setChecked(m_config->readBoolEntry(rcCaseSensitive, CaseSensitiveOption));
  chbRecursive->setChecked(m_config->readBoolEntry(rcRecursive, RecursiveOption));
  QStringList bkList = QStringList::split(",",
                                          m_config->readEntry(rcBackupExtension, BackupExtension),
					  true);
  if(bkList[0] == "true")
    {
      chbBackup->setChecked(true);
      leBackup->setEnabled(true);
      tlBackup->setEnabled(true);
    }
  else
    {
      chbBackup->setChecked(false);
      leBackup->setEnabled(false);
      tlBackup->setEnabled(false);
    }
  
  leBackup->setText(bkList[1]);  
    
  chbVariables->setChecked(m_config->readBoolEntry(rcVariables, VariablesOption));
  chbWildcards->setChecked(m_config->readBoolEntry(rcWildcards, WildcardsOption));
  chbHaltOnFirstOccurrence->setChecked(m_config->readBoolEntry(rcHaltOnFirstOccur, StopWhenFirstOccurenceOption));
  chbIgnoreWhitespaces->setChecked(m_config->readBoolEntry(rcIgnoreWhitespaces, IgnoreWhiteSpacesOption));
  chbFollowSymLinks->setChecked(m_config->readBoolEntry(rcFollowSymLinks, FollowSymbolicLinksOption));
  chbIgnoreHidden->setChecked(m_config->readBoolEntry(rcIgnoreHidden, IgnoreHiddenOption));

  chbConfirmDirs->setChecked(m_config->readBoolEntry(rcConfirmDirs, ConfirmDirectoriesOption));
  chbConfirmFiles->setChecked(m_config->readBoolEntry(rcConfirmFiles, ConfirmFilesOption));
  chbConfirmStrings->setChecked(m_config->readBoolEntry(rcConfirmStrings, ConfirmStringsOption));
   
}

void KOptionsDlg::saveOptions()
{
  m_config->setGroup("Options"); 
  
  m_config->writeEntry(rcCaseSensitive, chbCaseSensitive->isChecked());
  m_config->writeEntry(rcRecursive, chbRecursive->isChecked());
  m_config->writeEntry(rcHaltOnFirstOccur, chbHaltOnFirstOccurrence->isChecked());
  m_config->writeEntry(rcIgnoreWhitespaces, chbIgnoreWhitespaces->isChecked());
  m_config->writeEntry(rcFollowSymLinks, chbFollowSymLinks->isChecked());
  m_config->writeEntry(rcIgnoreHidden, chbIgnoreHidden->isChecked());
  QString bkOptions;
  if(chbBackup->isChecked())
    bkOptions = "true";
  else
    bkOptions = "false";
  
  bkOptions += ","+leBackup->text();
  
  m_config->writeEntry(rcBackupExtension, bkOptions);
  m_config->writeEntry(rcVariables, chbVariables->isChecked());
  m_config->writeEntry(rcWildcards, chbWildcards->isChecked());
  m_config->writeEntry(rcConfirmDirs, chbConfirmDirs->isChecked());
  m_config->writeEntry(rcConfirmFiles, chbConfirmFiles->isChecked());
  m_config->writeEntry(rcConfirmStrings, chbConfirmStrings->isChecked());
  
  m_config->sync();
}

void KOptionsDlg::whatsThisPage()
{
  // Create help QWhatsThis
  QWhatsThis::add(chbCaseSensitive, i18n(chbCaseSensitiveWhatthis));
  QWhatsThis::add(chbRecursive, i18n(chbRecursiveWhatthis));
  QWhatsThis::add(chbHaltOnFirstOccurrence, i18n(chbHaltOnFirstOccurrenceWhatthis));
  QWhatsThis::add(chbIgnoreWhitespaces, i18n(chbIgnoreWhitespacesWhatthis));
  QWhatsThis::add(chbFollowSymLinks, i18n(chbFollowSymLinksWhatthis));
  QWhatsThis::add(chbIgnoreHidden, i18n(chbIgnoreHiddenWhatthis));  
  QWhatsThis::add(chbWildcards, i18n(chbWildcardsWhatthis));
  QWhatsThis::add(chbVariables, i18n(chbVariablesWhatthis));
  QWhatsThis::add(chbBackup, i18n(chbBackupWhatthis));
}

#include "koptionsdlg.moc"
