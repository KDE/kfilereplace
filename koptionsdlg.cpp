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

#include "koptionsdlg.h"
#include "resource.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "whatthis.h"

using namespace whatthisNameSpace;


KOptionsDlg::KOptionsDlg(QWidget *parent, const char *name, Settings settings) : KOptionsDlgS(parent,name,true)
{
  whatsThisPage1();
  whatsThisPage2();
  whatsThisPage3();

  // Get current settings
  m_settings = settings;

  loadOptions();

  connect(pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(pbDefault, SIGNAL(clicked()),this,SLOT(slotDefaults()));
}

KOptionsDlg::~KOptionsDlg()
{
}

void KOptionsDlg::slotOK()
{
  saveOptions();

  accept();
}

void KOptionsDlg::slotDefaults() // Set defaults values for all options of the dialog
{
  // -------------------- PAGE 1 ---------------------
  chbCaseSensitive->setChecked(CaseSensitiveOption);
  chbRecursive->setChecked(RecursiveOption);
  chbHaltOnFirstOccurrence->setChecked(StopWhenFirstOccurenceOption);
  chbIgnoreWhitespaces->setChecked(IgnoreWhiteSpacesOption);
  chbFollowSymLinks->setChecked(FollowSymbolicLinksOption);
  chbAllStringsMustBeFound->setChecked(AllStringsMustBeFoundOption);
  chbIgnoreHidden->setChecked(IgnoreHiddenOption);

  // -------------------- PAGE 2 ---------------------
  chbWildcards->setChecked(WildcardsOption);
  edLetter->setText("?");
  edWord->setText("*");
  chbWildcardsInReplaceStr->setChecked(false);
  edLength->setText("100");

  // -------------------- PAGE 3 ---------------------
  chbBackup->setChecked(BackupOption);
  chbVariables->setChecked(VariablesOption);
  chbConfirmDirs->setChecked(ConfirmDirectoriesOption);
  chbConfirmFiles->setChecked(ConfirmFilesOption);
  chbConfirmStrings->setChecked(ConfirmStringsOption);
}

Settings KOptionsDlg::settings()
{
 return m_settings;
}

void KOptionsDlg::loadOptions()
{
  // ************************ PAGE 1 ****************************
  chbCaseSensitive->setChecked(m_settings.bCaseSensitive);
  chbRecursive->setChecked(m_settings.bRecursive);
  chbHaltOnFirstOccurrence->setChecked(m_settings.bHaltOnFirstOccur);
  chbIgnoreWhitespaces->setChecked(m_settings.bIgnoreWhitespaces);
  chbFollowSymLinks->setChecked(m_settings.bFollowSymLinks);
  chbAllStringsMustBeFound->setChecked(m_settings.bAllStringsMustBeFound);
  chbIgnoreHidden->setChecked(m_settings.bIgnoreHidden);

  // ************************ PAGE 2 ****************************
  chbWildcards->setChecked(m_settings.bWildcards);

  edLetter->setText(QChar(m_settings.cWildcardsLetters));
  edWord->setText(QChar(m_settings.cWildcardsWords));
  edLength->setText(QString::number(m_settings.nMaxExpressionLength,10));

  chbWildcardsInReplaceStr->setChecked(m_settings.bWildcardsInReplaceStrings);

  // ************************ PAGE 3 ****************************
  chbBackup->setChecked(m_settings.bBackup);
  chbVariables->setChecked(m_settings.bVariables);
  chbConfirmDirs->setChecked(m_settings.bConfirmDirs);
  chbConfirmFiles->setChecked(m_settings.bConfirmFiles);
  chbConfirmStrings->setChecked(m_settings.bConfirmStrings);

}

void KOptionsDlg::saveOptions()
{
  QString strWildcardsLetters,
          strWildcardsWords;
  int nMaxExpressionLength;

  // ************************ PAGE 1 ****************************
  m_settings.bCaseSensitive = chbCaseSensitive->isChecked();
  m_settings.bRecursive = chbRecursive->isChecked();
  m_settings.bHaltOnFirstOccur = chbHaltOnFirstOccurrence->isChecked();
  m_settings.bIgnoreWhitespaces = chbIgnoreWhitespaces->isChecked();
  m_settings.bFollowSymLinks = chbFollowSymLinks->isChecked();
  m_settings.bAllStringsMustBeFound = chbAllStringsMustBeFound->isChecked();
  m_settings.bIgnoreHidden = chbIgnoreHidden->isChecked();

  // ************************ PAGE 2 ****************************
  m_settings.bWildcards = chbWildcards->isChecked();

  // Get informations
  strWildcardsLetters = edLetter->text();
  strWildcardsWords = edWord->text();
  nMaxExpressionLength = edLength->text().toInt();

  // Check the Search text is not empty
  if (strWildcardsLetters.isEmpty() || strWildcardsWords.isEmpty())
    {
      KMessageBox::error(this, i18n("You must type the symbols for expressions and for characters before clicking on 'OK'."));
      return;
    }

  // check expression wildcard and character wildcards are not the same
  if (strWildcardsLetters == strWildcardsWords)
    {
      KMessageBox::error(this, i18n("You cannot use the same character for <b>expression wildcard</b> and for <b>character wildcard</b>."));
      return ;
    }

  // Check the maximum expression length is a valid integer
  if (nMaxExpressionLength < 2 || nMaxExpressionLength > 10000)
    {
      KMessageBox::error(this, i18n("The maximum wildcard expression length must be a valid number (between 2 and 10000)."));
      return;
    }

  // Copy informations into variables
  m_settings.cWildcardsLetters = *(strWildcardsLetters.ascii());
  m_settings.cWildcardsWords = *(strWildcardsWords.ascii());
  m_settings.bWildcardsInReplaceStrings = chbWildcardsInReplaceStr->isChecked();
  m_settings.nMaxExpressionLength = nMaxExpressionLength;

  // ************************ PAGE 3 ****************************
  m_settings.bBackup = chbBackup->isChecked();
  m_settings.bVariables = chbVariables->isChecked();
  m_settings.bConfirmDirs = chbConfirmDirs->isChecked();
  m_settings.bConfirmFiles = chbConfirmFiles->isChecked();
  m_settings.bConfirmStrings = chbConfirmStrings->isChecked();
}

void KOptionsDlg::whatsThisPage1()
{
  // Create help QWhatsThis
  QWhatsThis::add(chbCaseSensitive, i18n(chbCaseSensitiveWhatthis));
  QWhatsThis::add(chbRecursive, i18n(chbRecursiveWhatthis));
  QWhatsThis::add(chbHaltOnFirstOccurrence, i18n(chbHaltOnFirstOccurrenceWhatthis));
  QWhatsThis::add(chbIgnoreWhitespaces, i18n(chbIgnoreWhitespacesWhatthis));
  QWhatsThis::add(chbFollowSymLinks, i18n(chbFollowSymLinksWhatthis));
  QWhatsThis::add(chbAllStringsMustBeFound, i18n(chbAllStringsMustBeFoundWhatthis));
  QWhatsThis::add(chbIgnoreHidden, i18n(chbIgnoreHiddenWhatthis));
}

void KOptionsDlg::whatsThisPage2()
{
  // Add QWhatsThis messages
  QWhatsThis::add(edLength, i18n(edLengthWhatthis));
  QWhatsThis::add(chbWildcardsInReplaceStr, i18n(chbWildcardsInReplaceStrWhatthis));
}

void KOptionsDlg::whatsThisPage3()
{
  // Add QWhatsThis messages
  QWhatsThis::add(chbVariables, i18n(chbVariablesWhatthis));
  QWhatsThis::add(chbBackup, i18n(chbBackupWhatthis));
}
#include "koptionsdlg.moc"
