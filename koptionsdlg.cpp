/***************************************************************************
                          kpropertiesdlg.cpp  -  description
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

#include "koptionsdlg.h"
#include "resource.h"

#include <qvbox.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qwhatsthis.h>

#include <klocale.h>


// ===========================================================================================================================
KOptionsDlg::KOptionsDlg(QWidget *parent, const char *name, KSettings settings) : QTabDialog(parent,name,true)
{
  setCaption(i18n("KFileReplace Options"));
  setMinimumSize(450,350);
  resize(500,500);
  setOkButton(QString::null);
  setDefaultButton(i18n("&OK"));
  setHelpButton(i18n("&Default Values"));
  setCancelButton(i18n("&Cancel"));

  // Create pages
  initPage1();
  initPage2();
  initPage3();

  // Get current settings
  m_settings = settings;
  setValues(); // Fill widgets with current values

  connect(this, SIGNAL(defaultButtonPressed()), this, SLOT(slotOk()));
  connect(this, SIGNAL(helpButtonPressed()),this,SLOT(slotDefaults()));
}

// ===========================================================================================================================
KOptionsDlg::~KOptionsDlg()
{
}

// ===========================================================================================================================
void KOptionsDlg::slotOk()
{
  QString strWildcardsLetters, strWildcardsWords;
  int nMaxExpressionLength;

  // ************************ PAGE 1 ****************************
  m_settings.bCaseSensitive = m_checkCaseSensitive -> isChecked();
  m_settings.bRecursive = m_checkRecursive -> isChecked();
  m_settings.bHaltOnFirstOccur = m_checkHaltOnFirstOccur -> isChecked();
  m_settings.bIgnoreWhitespaces = m_checkIgnoreWhitespaces -> isChecked();
  m_settings.bFollowSymLinks = m_checkFollowSymLinks -> isChecked();
  m_settings.bAllStringsMustBeFound = m_checkAllStringsMustBeFound -> isChecked();
  m_settings.bIgnoreHidden = m_checkIgnoreHidden -> isChecked();

  // ************************ PAGE 2 ****************************
  m_settings.bWildcards = m_checkWildcards -> isChecked();

  // Get informations
  strWildcardsLetters = m_editLetter -> text();
  strWildcardsWords = m_editWord -> text();
  nMaxExpressionLength = m_editExpLength -> text().toInt();

  // Check the Search text is not empty
  if (strWildcardsLetters.isEmpty() || strWildcardsWords.isEmpty())
    {
      KMessageBox::error(this, i18n("You must type the symbols for expressions and for characters before clicking on 'Ok'."));
      return;
    }

  // check expression wildcard and character wildcards are not the same
  if (strWildcardsLetters == strWildcardsWords)
    {
      KMessageBox::error(this, i18n("You can't use the same character for <b>expression wildcard</b> and for <b>character wildcard</b>."));
      return ;
    }

  // Check the maximum expression length is a valid integer
  if (nMaxExpressionLength < 2 || nMaxExpressionLength > 10000)
    {
      KMessageBox::error(this, i18n("The maximum wildcard expression length must be a valid number (beetween 2 and 10000)."));
      return;
    }

  // Copy informations into variables
  m_settings.cWildcardsLetters = *(strWildcardsLetters.ascii());
  m_settings.cWildcardsWords = *(strWildcardsWords.ascii());
  m_settings.bWildcardsInReplaceStrings = m_checkWildcardsInReplaceStr -> isChecked();
  m_settings.nMaxExpressionLength = nMaxExpressionLength;

  // ************************ PAGE 3 ****************************
  m_settings.bBackup = m_checkBackup -> isChecked();
  m_settings.bVariables = m_checkVariables -> isChecked();
  m_settings.bConfirmDirs = m_checkConfirmDirs -> isChecked();
  m_settings.bConfirmFiles = m_checkConfirmFiles -> isChecked();
  m_settings.bConfirmStrings = m_checkConfirmStrings -> isChecked();

  accept();
}

// ===========================================================================================================================
void KOptionsDlg::slotDefaults() // Set defaults values for all options of the dialog
{
  // -------------------- PAGE 1 ---------------------
  m_checkCaseSensitive -> setChecked(OPTIONS_DEFAULT_CASESENSITIVE);
  m_checkRecursive -> setChecked(OPTIONS_DEFAULT_RECURSIVE);
  m_checkHaltOnFirstOccur -> setChecked(OPTIONS_DEFAULT_HALTONFIRSTOCCUR);
  m_checkIgnoreWhitespaces -> setChecked(OPTIONS_DEFAULT_IGNOREWHITESPACES);
  m_checkFollowSymLinks -> setChecked(OPTIONS_DEFAULT_FOLLOWSYMLINKS);
  m_checkAllStringsMustBeFound -> setChecked(OPTIONS_DEFAULT_ALLSTRINGSMUSTBEFOUND);
  m_checkIgnoreHidden -> setChecked(OPTIONS_DEFAULT_IGNOREHIDDEN);

  // -------------------- PAGE 2 ---------------------
  m_checkWildcards -> setChecked(OPTIONS_DEFAULT_WILDCARDS);
  m_editLetter -> setText("?");
  m_editWord -> setText("*");
  m_checkWildcardsInReplaceStr -> setChecked(false);
  m_editExpLength -> setText("100");

  // -------------------- PAGE 3 ---------------------
  m_checkBackup -> setChecked(OPTIONS_DEFAULT_BACKUP);
  m_checkVariables -> setChecked(OPTIONS_DEFAULT_VARIABLES);
  m_checkConfirmDirs -> setChecked(OPTIONS_DEFAULT_CONFIRMDIRS);
  m_checkConfirmFiles -> setChecked(OPTIONS_DEFAULT_CONFIRMFILES);
  m_checkConfirmStrings -> setChecked(OPTIONS_DEFAULT_CONFIRMSTRINGS);
}

// ===========================================================================================================================
void KOptionsDlg::setValues() // Copy current values to widgets
{
  QString strTemp;

  // -------------------- PAGE 1 ---------------------
  m_checkCaseSensitive -> setChecked( m_settings.bCaseSensitive );
  m_checkRecursive -> setChecked( m_settings.bRecursive );
  m_checkHaltOnFirstOccur -> setChecked( m_settings.bHaltOnFirstOccur );
  m_checkIgnoreWhitespaces -> setChecked( m_settings.bIgnoreWhitespaces );
  m_checkFollowSymLinks -> setChecked( m_settings.bFollowSymLinks );
  m_checkAllStringsMustBeFound -> setChecked( m_settings.bAllStringsMustBeFound );
  m_checkIgnoreHidden -> setChecked( m_settings.bIgnoreHidden );

  // -------------------- PAGE 2 ---------------------
  m_checkWildcards -> setChecked( m_settings.bWildcards );
  strTemp.sprintf("%c", m_settings.cWildcardsLetters);
  m_editLetter -> setText(strTemp);
  strTemp.sprintf("%c", m_settings.cWildcardsWords);
  m_editWord -> setText(strTemp);
  m_checkWildcardsInReplaceStr -> setChecked(m_settings.bWildcardsInReplaceStrings);
  strTemp.setNum(m_settings.nMaxExpressionLength);
  m_editExpLength -> setText(strTemp);

  // -------------------- PAGE 3 ---------------------
  m_checkBackup -> setChecked( m_settings.bBackup );
  m_checkVariables -> setChecked( m_settings.bVariables );
  m_checkConfirmFiles -> setChecked( m_settings.bConfirmFiles );
  m_checkConfirmStrings -> setChecked( m_settings.bConfirmStrings );
  m_checkConfirmDirs -> setChecked( m_settings.bConfirmDirs );

}

// ===========================================================================================================================
void KOptionsDlg::initPage1()
{
  QVBox *tab1 = new QVBox(this);
  tab1 -> setMargin(5);

  // Create check boxes
  m_checkCaseSensitive = new QCheckBox(i18n("Case sensitive"), tab1,"m_checkCaseSensitive");
  m_checkRecursive = new QCheckBox(i18n("Recursivity: Search/replace in all sub folders"),tab1,"m_checkRecursive");
  m_checkHaltOnFirstOccur = new QCheckBox(i18n("When searching, stop on first string found (faster but no details)"),tab1, "m_checkHaltOnFirstOccur");
  m_checkIgnoreWhitespaces = new QCheckBox(i18n("Ignore whitespaces (\\n,\\r,\\t,multi-spaces) (useful in HTML code)"),tab1, "m_checkIgnoreWhitespaces");
  m_checkFollowSymLinks = new QCheckBox(i18n("Follow symbolic links"),tab1, "m_checkFollowSymLinks");
  m_checkAllStringsMustBeFound = new QCheckBox(i18n("All strings must be found"),tab1, "m_checkAllStringsMustBeFound");
  m_checkIgnoreHidden = new QCheckBox(i18n("Ignore hidden files and directories"),tab1, "m_bCheckIgnoreHidden");

  // Create help QWhatsThis
  QWhatsThis::add(m_checkCaseSensitive, i18n("If enabled, lowers and uppers are differents. If you are "
                                             "searching for \"Linux\" and this option is on, \"LINUX\" won't be found."));
  QWhatsThis::add(m_checkRecursive, i18n("Work in all sub-directories of the main directory of the project"));
  QWhatsThis::add(m_checkHaltOnFirstOccur, i18n("When searching, KFileReplace reads all the data of each "
                                                "file to know how many times each strings appears in the files. If you needn't "
                                                "to have this details, you can enable this options to make the search faster. "
                                                "Then the search will be finished in a file when the first occurrence of a string will be found."));
  QWhatsThis::add(m_checkIgnoreWhitespaces, i18n("Ignore some characters when searching a string: \\r (carriage return), \\n (line feed) \\t (tab), "
                                                 "and multi-spaces. It's very "
                                                 "useful in HTML code, and with the end of the lines. For example, if you are searching for "
                                                 "\"Linux is fast\" in a file, and there is a line feed beetween "
                                                 "\"Linux\" and \"is\", then the string won't be found. This options can solve the problem."));
  QWhatsThis::add(m_checkFollowSymLinks, i18n("If a directory symbolic link is found, it will be opened."));
  QWhatsThis::add(m_checkAllStringsMustBeFound, i18n("The file will be found/replaced only if all strings from the list are found in the file."));
  QWhatsThis::add(m_checkIgnoreHidden, i18n("If this option is enabled, hidden files and directories (files whose names begin with a "
                                            "point as .kde) won't be found / replaced."));

  addTab(tab1, i18n("General"));
}

// ===========================================================================================================================
void KOptionsDlg::initPage2()
{
  QVBox *tab2 = new QVBox(this);
  tab2 -> setMargin(5);

  // Create check boxes
  m_checkWildcards = new QCheckBox(i18n("Enable wildcards"),tab2,"m_checkWildcards");

  QButtonGroup *bgSymbols = new QButtonGroup(2, QGroupBox::Horizontal, i18n("Symbols"), tab2);
  QButtonGroup *bgOptions = new QButtonGroup(2, QGroupBox::Horizontal, i18n("Options"), tab2);

  QLabel *label1 = new QLabel(bgSymbols,"m_labelTitle1");
  label1 -> setText(i18n("For a single char (default='?'):"));
  m_editLetter= new QLineEdit(bgSymbols,"m_editLetter");
  m_editLetter -> setMaxLength(1);

  QLabel *label2 = new QLabel(bgSymbols,"m_labelTitle2");
  label2->setText(i18n("For an expression (default:'*'):"));
  m_editWord= new QLineEdit(bgSymbols,"m_editWord");
  m_editWord -> setMaxLength(1);

  QLabel *labelLength = new QLabel(bgOptions,"m_labelLength");
  labelLength->setText(i18n("Maximum length of a wildcard expression:"));
  m_editExpLength = new QLineEdit(bgOptions,"m_editExpLength");

  m_checkWildcardsInReplaceStr = new QCheckBox(bgOptions,"m_checkWildcardsInReplaceStr");
  m_checkWildcardsInReplaceStr->setText(i18n("Use wildcards in the replace string"));

  // Add QWhatsThis messages
  QWhatsThis::add(m_editExpLength, i18n("This is the maximal length of the search made to find wildcards in a text. For example, if the length is 200, then "
                                        "the '*' symbol can't code for an expression which length is more than 200 letters (bytes)"));
  QWhatsThis::add(m_checkWildcardsInReplaceStr, i18n("If enabled, the contents of the wildcards found in the search string will be copied into the "
                                                     "replace string. For example, if you search for \"The * is under my bed\" and the text is \"The "
                                                     "cat is under my bed\", and the replace string "
                                                     "is \"I have a *\", then the text will be replaced with \"I have a cat\". If this options is "
                                                     "disabled, you will have \"I have a *\"."));

  addTab(tab2, i18n("Wildcards"));
}

// ===========================================================================================================================
void KOptionsDlg::initPage3()
{
  QVBox *tab3 = new QVBox(this);
  tab3 -> setMargin(5);

  // Create check boxes
  QButtonGroup *bgGeneral = new QButtonGroup(1, QGroupBox::Horizontal, i18n("General"), tab3);
  QButtonGroup *bgConfirm = new QButtonGroup(1, QGroupBox::Horizontal, i18n("Confirmation Before Replacing"), tab3);

  m_checkVariables = new QCheckBox(i18n("Enable variables in replace strings"),bgGeneral,"m_checkVariables");
  m_checkBackup = new QCheckBox(i18n("Backup: Create a copy into a .OLD file before replacing strings"),bgGeneral,"m_checkBackup");

  m_checkConfirmDirs = new QCheckBox(i18n("Confirm before replacing in a directory"),bgConfirm,"m_checkConfirmDir");
  m_checkConfirmFiles = new QCheckBox(i18n("Confirm before replacing in a file"),bgConfirm,"m_checkConfirmFiles");
  m_checkConfirmStrings = new QCheckBox(i18n("Confirm when replace each string"),bgConfirm,"m_checkConfirmStrings");

  // Add QWhatsThis messages
  QWhatsThis::add(m_checkVariables, i18n("If enabled, KFileReplace will replace variables with their values in the replace string. For example "
                                         "if the replace string is \"The current time is [$datetime:mm/dd/yyyy$]\", then the date will be written."));
  QWhatsThis::add(m_checkBackup, i18n("If enabled, create backup of replaced files before any modifications. Then you can restore the old data "
                                      "if there is an error durring the replace operation. A copy of the original files will be created, with the .OLD extension."));


  addTab(tab3, i18n("Replace"));
}

#include "koptionsdlg.moc"

