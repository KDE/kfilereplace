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

// QT
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>

// KDE
#include <kconfig.h>
#include <kstandarddirs.h>
//#include <klocale.h>
#include <kapplication.h>
//#include <kdebug.h>

// local
#include "whatthis.h"
#include "koptionsdlg.h"


using namespace whatthisNameSpace;


KOptionsDlg::KOptionsDlg(QWidget *parent, const char *name) : KOptionsDlgS(parent,name,true)
{
  QString configName = locateLocal("config", "kfilereplacerc");
  m_config = new KConfig(configName);
   
  loadOptions();
    
  connect(m_pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(m_pbDefault, SIGNAL(clicked()),this,SLOT(slotDefaults()));
  connect(m_chbBackup, SIGNAL(toggled(bool)), this, SLOT(slotChbBackup(bool)));
  connect(m_pbHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));  
  
  whatsThis();
}

KOptionsDlg::~KOptionsDlg()
{
  delete m_config;
  m_config = 0;
}

void KOptionsDlg::slotOK()
{
  saveOptions();
  accept();
}

/** Set defaults values for all options of the dialog */
void KOptionsDlg::slotDefaults() 
{
  m_chbCaseSensitive->setChecked(CaseSensitiveOption);
  m_chbRecursive->setChecked(RecursiveOption);
  m_chbHaltOnFirstOccurrence->setChecked(StopWhenFirstOccurenceOption);
  
  m_chbFollowSymLinks->setChecked(FollowSymbolicLinksOption);
  m_chbIgnoreHidden->setChecked(IgnoreHiddenOption);
  m_chbRegularExpressions->setChecked(RegularExpressionsOption);
  m_chbIgnoreFiles->setChecked(IgnoreFilesOption);
  
  QStringList bkList = QStringList::split(",",BackupExtensionOption,true);
  
  bool enableBackup = (bkList[0] == "true" ? true : false);
  
  m_chbBackup->setChecked(enableBackup);
  m_leBackup->setEnabled(enableBackup);
  m_tlBackup->setEnabled(enableBackup);
  
  m_leBackup->setText(bkList[1]);
  
  m_chbVariables->setChecked(VariablesOption);
  
  m_chbConfirmStrings->setChecked(ConfirmStringsOption);
}

void KOptionsDlg::slotChbBackup(bool b)
{
  m_leBackup->setEnabled(b);
  m_tlBackup->setEnabled(b);
}

void KOptionsDlg::loadOptions()
{
  m_config->setGroup("Options");
     
  m_chbCaseSensitive->setChecked(m_config->readBoolEntry(rcCaseSensitive, CaseSensitiveOption));
  m_chbRecursive->setChecked(m_config->readBoolEntry(rcRecursive, RecursiveOption));
  QStringList bkList = QStringList::split(",",
                                          m_config->readEntry(rcBackupExtension, BackupExtensionOption),
                                          true);
  bool enableBackup = ((bkList[0] == "true") ? true : false);
  
  m_chbBackup->setChecked(enableBackup);
  m_leBackup->setEnabled(enableBackup);
  m_tlBackup->setEnabled(enableBackup);
  
  m_leBackup->setText(bkList[1]); 
  
  m_chbVariables->setChecked(m_config->readBoolEntry(rcVariables, VariablesOption));
  m_chbRegularExpressions->setChecked(m_config->readBoolEntry(rcRegularExpressions, RegularExpressionsOption));
  m_chbHaltOnFirstOccurrence->setChecked(m_config->readBoolEntry(rcHaltOnFirstOccur, StopWhenFirstOccurenceOption));
  m_chbFollowSymLinks->setChecked(m_config->readBoolEntry(rcFollowSymLinks, FollowSymbolicLinksOption));
  m_chbIgnoreHidden->setChecked(m_config->readBoolEntry(rcIgnoreHidden, IgnoreHiddenOption));
  m_chbIgnoreFiles->setChecked(m_config->readBoolEntry(rcIgnoreFiles, IgnoreFilesOption));
  m_chbConfirmStrings->setChecked(m_config->readBoolEntry(rcConfirmStrings, ConfirmStringsOption));
}

void KOptionsDlg::saveOptions()
{
  m_config->setGroup("Options"); 
  
  m_config->writeEntry(rcCaseSensitive, m_chbCaseSensitive->isChecked());
  m_config->writeEntry(rcRecursive, m_chbRecursive->isChecked());
  m_config->writeEntry(rcHaltOnFirstOccur, m_chbHaltOnFirstOccurrence->isChecked());
  m_config->writeEntry(rcFollowSymLinks, m_chbFollowSymLinks->isChecked());
  m_config->writeEntry(rcIgnoreHidden, m_chbIgnoreHidden->isChecked());
  m_config->writeEntry(rcIgnoreFiles, m_chbIgnoreFiles->isChecked());
    
  QString bkOptions;
  if(m_chbBackup->isChecked())
    bkOptions = "true,"+m_leBackup->text();
  else
    bkOptions = "false,"+m_leBackup->text();
  
  m_config->writeEntry(rcBackupExtension, bkOptions);
  m_config->writeEntry(rcVariables, m_chbVariables->isChecked());
  m_config->writeEntry(rcRegularExpressions, m_chbRegularExpressions->isChecked());
  m_config->writeEntry(rcConfirmStrings, m_chbConfirmStrings->isChecked());
  
  m_config->sync();
}

void KOptionsDlg::slotHelp()
{
  kapp->invokeHelp(QString::null, "kfilereplace");
}

void KOptionsDlg::whatsThis()
{
  // Create help QWhatsThis
  QWhatsThis::add(m_chbCaseSensitive, chbCaseSensitiveWhatthis);
  QWhatsThis::add(m_chbRecursive, chbRecursiveWhatthis);
  QWhatsThis::add(m_chbHaltOnFirstOccurrence, chbHaltOnFirstOccurrenceWhatthis);
  QWhatsThis::add(m_chbFollowSymLinks, chbFollowSymLinksWhatthis);
  QWhatsThis::add(m_chbIgnoreHidden, chbIgnoreHiddenWhatthis); 
  QWhatsThis::add(m_chbIgnoreFiles, chbIgnoreFilesWhatthis); 
  QWhatsThis::add(m_chbRegularExpressions, chbRegularExpressionsWhatthis);
  QWhatsThis::add(m_chbVariables, chbVariablesWhatthis);
  QWhatsThis::add(m_chbBackup, chbBackupWhatthis);
  QWhatsThis::add(m_chbConfirmStrings, chbConfirmStringsWhatthis);
}

#include "koptionsdlg.moc"
