/***************************************************************************
                          koptionsdlg.cpp  -  description
                             -------------------
    begin                : Tue Dec 28 1999
    copyright            : (C) 1999 by Franï¿½is Dupoux
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

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>

// KDE
#include <kcharsets.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
//#include <kdebug.h>

// local
#include "whatthis.h"
#include "koptionsdlg.h"


using namespace whatthisNameSpace;


KOptionsDlg::KOptionsDlg(RCOptions* info, QWidget *parent, const char *name) : KOptionsDlgS(parent,name,true)
{
  m_config = new KConfig("kfilereplacerc");
  m_option = info;

  initGUI();

  connect(m_pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(m_pbDefault, SIGNAL(clicked()),this,SLOT(slotDefaults()));
  connect(m_chbBackup, SIGNAL(toggled(bool)), this, SLOT(slotChbBackup(bool)));
  connect(m_pbHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
  connect(m_chbConfirmStrings, SIGNAL(toggled(bool)), this, SLOT(slotChbConfirmStrings(bool)));
  connect(m_chbShowConfirmDialog, SIGNAL(toggled(bool)), this, SLOT(slotChbShowConfirmDialog(bool)));

  whatsThis();
}

KOptionsDlg::~KOptionsDlg()
{
}

//PRIVATE SLOTS
void KOptionsDlg::slotOK()
{
  saveRCOptions();
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
  m_chbConfirmStrings->setChecked(AskConfirmReplaceOption);

  QStringList bkList = BackupExtensionOption.split(",",QString::KeepEmptyParts);

  bool enableBackup = (bkList[0] == "true" ? true : false);

  m_chbBackup->setChecked(enableBackup);
  m_leBackup->setEnabled(enableBackup);
  m_tlBackup->setEnabled(enableBackup);

  m_leBackup->setText(bkList[1]);

  m_chbVariables->setChecked(VariablesOption);

  m_chbNotifyOnErrors->setChecked(NotifyOnErrorsOption);
}

void KOptionsDlg::slotChbBackup(bool b)
{
  m_leBackup->setEnabled(b);
  m_tlBackup->setEnabled(b);
}

void KOptionsDlg::slotChbConfirmStrings(bool b)
{
  KConfigGroup grp(m_config, "Notification Messages");
  if(b)
  {
    m_chbShowConfirmDialog->setEnabled(true);
    m_chbShowConfirmDialog->setChecked(true);
    grp.writeEntry(rcDontAskAgain,"no");
  }
  else
  {
    m_chbShowConfirmDialog->setEnabled(false);
    m_chbShowConfirmDialog->setChecked(false);
    grp.writeEntry(rcDontAskAgain,"yes");
  }
}

void KOptionsDlg::slotChbShowConfirmDialog(bool b)
{
  KConfigGroup grp(m_config, "Notification Messages");
  if(b)
  {
    grp.writeEntry(rcDontAskAgain,"no");
  }
  else
  {
    grp.writeEntry(rcDontAskAgain,"yes");
  }
}

//PRIVATE
void KOptionsDlg::initGUI()
{
  m_config->sync();
  KConfigGroup grp(m_config, "Notification Messages");
  m_option->m_notifyOnErrors = grp.readEntry(rcNotifyOnErrors, true);

  QString dontAskAgain = grp.readEntry(rcDontAskAgain, QString("no"));

  m_chbConfirmStrings->setChecked(m_option->m_askConfirmReplace);

  if(m_chbConfirmStrings->isChecked())
  {
    if(dontAskAgain == "yes")
      m_chbShowConfirmDialog->setChecked(false);
    else
      m_chbShowConfirmDialog->setChecked(true);
  }

  QStringList availableEncodingNames(KGlobal::charsets()->availableEncodingNames());
  m_cbEncoding->addItems( availableEncodingNames );
  int idx = -1;
  int utf8Idx = -1;
  for (int i = 0; i < availableEncodingNames.count(); i++)
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

  m_chbCaseSensitive->setChecked(m_option->m_caseSensitive);
  m_chbRecursive->setChecked(m_option->m_recursive);

  bool enableBackup = m_option->m_backup;

  m_chbBackup->setChecked(enableBackup);
  m_leBackup->setEnabled(enableBackup);
  m_tlBackup->setEnabled(enableBackup);

  m_leBackup->setText(m_option->m_backupExtension);

  m_chbVariables->setChecked(m_option->m_variables);
  m_chbRegularExpressions->setChecked(m_option->m_regularExpressions);
  m_chbHaltOnFirstOccurrence->setChecked(m_option->m_haltOnFirstOccur);
  m_chbFollowSymLinks->setChecked(m_option->m_followSymLinks);
  m_chbIgnoreHidden->setChecked(m_option->m_ignoreHidden);
  m_chbIgnoreFiles->setChecked(m_option->m_ignoreFiles);

  m_chbNotifyOnErrors->setChecked(m_option->m_notifyOnErrors);
}

void KOptionsDlg::saveRCOptions()
{
  m_option->m_encoding = m_cbEncoding->currentText();
  m_option->m_caseSensitive = m_chbCaseSensitive->isChecked();
  m_option->m_recursive = m_chbRecursive->isChecked();
  QString backupExt = m_leBackup->text();
  m_option->m_backup = (m_chbBackup->isChecked() && !backupExt.isEmpty());
  m_option->m_backupExtension = backupExt;
  m_option->m_variables = m_chbVariables->isChecked();
  m_option->m_regularExpressions = m_chbRegularExpressions->isChecked();
  m_option->m_haltOnFirstOccur = m_chbHaltOnFirstOccurrence->isChecked();
  m_option->m_followSymLinks = m_chbFollowSymLinks->isChecked();
  m_option->m_ignoreHidden = m_chbIgnoreHidden->isChecked();
  m_option->m_ignoreFiles = m_chbIgnoreFiles->isChecked();
  m_option->m_askConfirmReplace = m_chbConfirmStrings->isChecked();
  m_option->m_notifyOnErrors = m_chbNotifyOnErrors->isChecked();

  KConfigGroup grp(m_config, "Notification Messages");
  grp.writeEntry(rcNotifyOnErrors, m_option->m_notifyOnErrors);

  m_config->sync();
}

void KOptionsDlg::whatsThis()
{
  // Create help QWhatsThis
  m_chbCaseSensitive->setWhatsThis( chbCaseSensitiveWhatthis);
  m_chbRecursive->setWhatsThis( chbRecursiveWhatthis);
  m_chbHaltOnFirstOccurrence->setWhatsThis( chbHaltOnFirstOccurrenceWhatthis);
  m_chbFollowSymLinks->setWhatsThis( chbFollowSymLinksWhatthis);
  m_chbIgnoreHidden->setWhatsThis( chbIgnoreHiddenWhatthis);
  m_chbIgnoreFiles->setWhatsThis( chbIgnoreFilesWhatthis);
  m_chbRegularExpressions->setWhatsThis( chbRegularExpressionsWhatthis);
  m_chbVariables->setWhatsThis( chbVariablesWhatthis);
  m_chbBackup->setWhatsThis( chbBackupWhatthis);
  m_chbConfirmStrings->setWhatsThis( chbConfirmStringsWhatthis);
}

#include "koptionsdlg.moc"
