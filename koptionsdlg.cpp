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


KOptionsDlg::KOptionsDlg(QWidget *parent, const char *name, Settings settings) : KOptionsDlgS(parent,name,true)
{
  whatsThisPage1();
  whatsThisPage2();
  whatsThisPage3();

  // Get current settings
  m_settings = settings;

  connect(pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(pbDefault, SIGNAL(clicked()),this,SLOT(slotDefaults()));
}

KOptionsDlg::~KOptionsDlg()
{
}

void KOptionsDlg::slotOK()
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
      KMessageBox::error(this, i18n("You must type the symbols for expressions and for characters before clicking on 'Ok'."));
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
void KOptionsDlg::whatsThisPage1()
{
  // Create help QWhatsThis
  QWhatsThis::add(chbCaseSensitive,
                  i18n("If enabled, lowers and uppers are different. If you are "
                       "searching for \"Linux\" and this option is on, \"LINUX\" will not be found."));
  QWhatsThis::add(chbRecursive,
                  i18n("Work in all subfolders of the main folder of the project"));
  QWhatsThis::add(chbHaltOnFirstOccurrence,
                  i18n("When searching, KFileReplace reads all the data of each "
                       "file to know how many times each strings appears in the files. If you need not"
                       "to have this details, you can enable this options to make the search faster. "
                       "Then the search will be finished in a file when the first occurrence of a string will be found."));
  QWhatsThis::add(chbIgnoreWhitespaces,
                  i18n("Ignore some characters when searching a string: \\r (carriage return), \\n (line feed) \\t (tab), "
                       "and multi-spaces. It's very "
                       "useful in HTML code, and with the end of the lines. For example, if you are searching for "
                       "\"Linux is fast\" in a file, and there is a line feed between "
                       "\"Linux\" and \"is\", then the string will not be found. This options can solve the problem."));
  QWhatsThis::add(chbFollowSymLinks,
                  i18n("If a folder symbolic link is found, it will be opened."));
  QWhatsThis::add(chbAllStringsMustBeFound,
                  i18n("The file will be found/replaced only if all strings from the list are found in the file."));
  QWhatsThis::add(chbIgnoreHidden,
                  i18n("If this option is enabled, hidden files and folders (files whose names begin with a "
                       "point as .kde) will not be found / replaced."));
}

void KOptionsDlg::whatsThisPage2()
{
  // Add QWhatsThis messages
  QWhatsThis::add(edLength,
                  i18n("This is the maximal length of the search made to find wildcards in a text. For example, if the length is 200, then "
                       "the '*' symbol cannot code for an expression which length is more than 200 letters (bytes)"));
  QWhatsThis::add(chbWildcardsInReplaceStr,
                  i18n("If enabled, the contents of the wildcards found in the search string will be copied into the "
                       "replace string. For example, if you search for \"The * is under my bed\" and the text is \"The "
                       "cat is under my bed\", and the replace string "
                       "is \"I have a *\", then the text will be replaced with \"I have a cat\". If this options is "
                       "disabled, you will have \"I have a *\"."));
}

void KOptionsDlg::whatsThisPage3()
{
  // Add QWhatsThis messages
  QWhatsThis::add(chbVariables,
                  i18n("If enabled, KFileReplace will replace variables with their values in the replace string. For example "
                  "if the replace string is \"The current time is [$datetime:mm/dd/yyyy$]\", then the date will be written."));
  QWhatsThis::add(chbBackup,
                  i18n("If enabled, create backup of replaced files before any modifications. Then you can restore the old data "
                  "if there is an error during the replace operation. A copy of the original files will be created, with the .OLD extension."));
}
#include "koptionsdlg.moc"
