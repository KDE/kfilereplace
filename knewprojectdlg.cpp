/***************************************************************************
                          knewprojectdlg.cpp  -  description
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

#include "knewprojectdlg.h"
#include "resource.h"

#include <qvbox.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qmultilinedit.h>
#include <qdir.h>
#include <qframe.h>

#include <kseparator.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>


// ===========================================================================================================================
KNewProjectDlg::KNewProjectDlg(QWidget *parent, KConfig *config, const char *name/*=0*/) : KNewProjectDlgS(parent, name, true)
{
   // copy data
   m_config = config;

   setWhatsThis();
   loadComboValues();
   connect(m_btnDir, SIGNAL(clicked()), this, SLOT(slotDir()));
}

// ===========================================================================================================================
KNewProjectDlg::~KNewProjectDlg()
{
}

// ===========================================================================================================================
QString KNewProjectDlg::getDirectory()
{
  return m_comboDirectory -> currentText();
}

// ===========================================================================================================================
QString KNewProjectDlg::getFilter()
{
  return m_comboFilter -> currentText();
}

void KNewProjectDlg::slotDir()
{
   QString strDir;

   strDir = KFileDialog::getExistingDirectory(QString::null, this, i18n("Project directory"));
   if (strDir.isEmpty())
      return ;

   m_comboDirectory -> setEditText(strDir);
}

// ===========================================================================================================================
void KNewProjectDlg::slotOk()
{
   bool bMinSize, bMaxSize, bMinDate, bMaxDate;

   // Check the Search text and the Filter are not empty
   if (getDirectory().isEmpty() || getFilter().isEmpty())
   {
      KMessageBox::error(this, i18n("You must fill the combo boxes (directory and filter) before continue"));
      return;
   }

   // Copy text into variables
   addCurrentStringToCombo();
   saveComboValues();

   // ================== SIZE OPTIONS ========================
   getMaxFilesSize(&bMaxSize, &m_nMaxSize);
   getMinFilesSize(&bMinSize, &m_nMinSize);

   // ================== DATE OPTIONS ========================
   QString strMinDate, strMaxDate;

   bMinDate = m_checkDateMin -> isChecked();
   bMaxDate = m_checkDateMax -> isChecked();
   strMinDate = m_editDateMin -> text();
   strMaxDate = m_editDateMax -> text();

   // ================== OWNER OPTIONS ========================
   bool bOwnerUser, bOwnerGroup;
   bOwnerUser = getBoolOwnerUser();
   bOwnerGroup = getBoolOwnerGroup();

   if ((bOwnerUser && m_editOwnerUser->text().isEmpty()) || (bOwnerGroup && m_editOwnerGroup->text().isEmpty()))
   {
      KMessageBox::error(this, i18n("Some edit boxes are empty in the <b>Owner</b> page."));
      return ;
   }

   // =========================================================

   // Check all the fields are filled
   if (   (bMaxSize && m_editSizeMax -> text().isEmpty())
          || (bMinSize && m_editSizeMin -> text().isEmpty())
          || (bMinDate && m_editDateMin -> text().isEmpty())
          || (bMaxDate && m_editDateMax -> text().isEmpty()) )
   {
      KMessageBox::error(this, i18n("Some edit boxes are empty"));
      return ;
   }

   // Check option "Size Min/Max": check MinSize is not greater than MaxSize
   if (bMaxSize && bMinSize && (m_nMaxSize < m_nMinSize))
   {
      KMessageBox::error(this, i18n("The minimum size is greater than the maximum size."));
      return ;
   }

   // Check option "Date Min/Max": check date is valid
   // Check 2 things: * QRegExp ==> valid text (2/2/4)
   //                 * QDate::isValid()

   // 1. Check the first Date: DateMin (After...)
   if (bMinDate) // If "DateMin" option in checked
   {
      if ( !QDate::fromString(strMinDate, Qt::ISODate).isValid())
      {
         KMessageBox::error(this, i18n("The dates must be in the YYYY-MM-DD format."));
         return ;
      }
   }

   // 2. Check the second Date: DateMax (Before...)
   if (bMaxDate) // If "DateMax" option in checked
   {
      if ( !QDate::fromString(strMaxDate, Qt::ISODate).isValid())
      {
         KMessageBox::error(this, i18n("The dates must be in the YYYY-MM-DD format."));
         return ;
      }
   }

   // 3. Convert Strings to QDate
   if (bMinDate) // If "DateMin" option in checked
   {
      m_qdMinDate = QDate::fromString(strMinDate, Qt::ISODate);
   }

   if (bMaxDate) // If "DateMax" option in checked
   {
      m_qdMaxDate = QDate::fromString(strMaxDate, Qt::ISODate);
   }

   // 4. Check DateMax is not before DateMin
   if (bMinDate && bMaxDate && m_qdMinDate > m_qdMaxDate)
   {
      KMessageBox::error(this, i18n("<qt>The <i>accessed after</i> date is after the <i>accessed before</i> date."));
      return ;
   }

   accept();
}


// ===========================================================================================================================
void KNewProjectDlg::loadComboValues()
{
   ///////////////////////////////////////////////////////////////////
   // read the m_config file entries
   QString str, strTemp;
   int i;
   int nInserted;

   // Combo box informations
   m_config->setGroup("Directories");
   nInserted = 0;
   for(i = 0; i < MAX_STRINGSINCOMBO; i++)
   {
      strTemp.setNum(i+1);
      str = m_config->readEntry(strTemp, "");
      if (!str.isEmpty())
      {
         m_comboDirectory->insertItem(str);
         nInserted++;
      }
   }
   if (nInserted == 0) // If no string in m_config file
      m_comboDirectory->insertItem(QDir::current().path());

   m_config->setGroup("Filters");
   nInserted = 0;
   for(i = 0; i < MAX_STRINGSINCOMBO; i++)
   {
      strTemp.setNum(i+1);
      str = m_config->readEntry(strTemp, "");
      if (!str.isEmpty())
      {
         m_comboFilter -> insertItem(str);
         nInserted++;
      }

   }
   if (nInserted == 0) // If no string in m_config file
      m_comboFilter -> insertItem("*.htm;*.html;*.xml");
}

// ===========================================================================================================================
void KNewProjectDlg::saveComboValues()
{
   ///////////////////////////////////////////////////////////////////
   // save the m_config file entries
   QString str, strTemp;
   int i;

   // Combo box informations
   m_config->setGroup("Directories");
   for(i = 0; i < (int) m_comboDirectory-> count(); i++)
   {
      strTemp.setNum(i+1);
      m_config->writeEntry(strTemp.ascii(), m_comboDirectory-> text(i));
   }

   m_config->setGroup("Filters");
   for(i = 0; i < (int) m_comboFilter -> count(); i++)
   {
      strTemp.setNum(i+1);
      m_config->writeEntry(strTemp.ascii(), m_comboFilter -> text(i));
   }
}

// ===========================================================================================================================
void KNewProjectDlg::addCurrentStringToCombo()
{
   // Get strings from combo Boxes and Insert items to Combo boxes
   bool bAlready;
   KComboBox* combo[2] = {m_comboDirectory, m_comboFilter};
   const char *szString[2] = {getDirectory().ascii(), getFilter().ascii()};

   for (int i=0; i < 2; i++)
   {
      // Check string is not already in the combo
      bAlready = false;
      for (int j=0; j < combo[i] -> count(); j++)
      {
         if ( strcmp(szString[i], combo[i] -> text(j)) == 0 )
            bAlready = true;
      }

      if (!bAlready) // Add it
      {
         if (combo[i] -> count() >= MAX_STRINGSINCOMBO) // If full
            combo[i] -> removeItem(4);

         combo[i] -> insertItem(szString[i], 0);
      }
   }
}

// ===========================================================================================================================
void KNewProjectDlg::setWhatsThis()
{
   QWhatsThis::add(m_comboDirectory, i18n("This is the directory where the search or the replace operation will be done."));
   QWhatsThis::add(m_comboFilter, i18n("Fix the filter of the files you want to search. For example, write \"*.htm\" to search or replace "
                                       "all web pages. You can put more than an only filter, when using ';'. For example \"*.html;*.txt;*.xml\""));

   QWhatsThis::add(m_editSizeMin, i18n("Minimal size of files. For example, if you put 1024 KB, all files which size is less than 1 MB won't be taken"));
   QWhatsThis::add(m_editSizeMax, i18n("Maximal size of files. For example, if you put 2048 KB, all files which size is more than 2 MB won't be taken"));
   QWhatsThis::add(m_editDateMin, i18n("Minimal date of files in YYYY/MM/DD format. For example, if you put 2000/01/31, all files which were "
                                       "(modified/read) before the 31 January 2000 won't be taken"));
   QWhatsThis::add(m_editDateMax, i18n("Maximal date of files in YYYY/MM/DD format. For example, if you put 1999/12/31, all files which were "
                                       "(modified/read) after the 31 December 1999 won't be taken"));
   QWhatsThis::add(m_comboDateRW, i18n("Select \"Writing\" if you want to use the date of the last modification, or \"reading\" to use the last "
                                       "read access to the file"));
}


// ===========================================================================================================================
void KNewProjectDlg::getMaxFilesSize(bool *bChecked, long unsigned int *nMaxSize)
{
  *bChecked = m_checkSizeMax -> isChecked();

  if (*bChecked) // If option activated
    *nMaxSize = m_editSizeMax -> text().toULong() * 1024; // KB --> Bytes
  else
    *nMaxSize = 0;
}

// ===========================================================================================================================
void KNewProjectDlg::getMinFilesSize(bool *bChecked, long unsigned int *nMinSize)
{
  *bChecked = m_checkSizeMin -> isChecked();

  if (*bChecked) // If option activated
    *nMinSize = m_editSizeMin -> text().toULong() * 1024; // KB --> Bytes
  else
    *nMinSize = 0;
}

// ===========================================================================================================================
int KNewProjectDlg::getTypeOfAccess()
{
  return m_comboDateRW -> currentItem();
}

// ===========================================================================================================================
bool KNewProjectDlg::getBoolMinDate()
{
  return m_checkDateMin -> isChecked();
}

// ===========================================================================================================================
bool KNewProjectDlg::getBoolMaxDate()
{
  return m_checkDateMax -> isChecked();
}

// ===========================================================================================================================
bool KNewProjectDlg::getBoolMinSize()
{
  return m_checkSizeMin -> isChecked();
}

// ===========================================================================================================================
bool KNewProjectDlg::getBoolMaxSize()
{
  return m_checkSizeMax -> isChecked();
}

// ===========================================================================================================================
bool KNewProjectDlg::getBoolOwnerUser()
{
  return m_checkOwnerUser -> isChecked();
}

// ===========================================================================================================================
bool KNewProjectDlg::getBoolOwnerGroup()
{
  return m_checkOwnerGroup -> isChecked();
}

// ===========================================================================================================================
bool KNewProjectDlg::getOwnerUserMustBe()
{
  return (m_comboOwnerUserBool -> currentText() == i18n("must be"));
}

// ===========================================================================================================================
bool KNewProjectDlg::getOwnerGroupMustBe()
{
  return (m_comboOwnerGroupBool -> currentText() == i18n("must be"));
}

// ===========================================================================================================================
QString KNewProjectDlg::getOwnerUserValue()
{
  return m_editOwnerUser->text();
}

// ===========================================================================================================================
QString KNewProjectDlg::getOwnerGroupValue()
{
  return m_editOwnerGroup->text();
}

// ===========================================================================================================================
QString KNewProjectDlg::getOwnerUserType()
{
  return m_comboOwnerUserType -> currentText();
}

// ===========================================================================================================================
QString KNewProjectDlg::getOwnerGroupType()
{
  return m_comboOwnerGroupType -> currentText();
}

// ===========================================================================================================================
QDate KNewProjectDlg::getMinDate()
{
  return m_qdMinDate;
}

// ===========================================================================================================================
QDate KNewProjectDlg::getMaxDate()
{
  return m_qdMaxDate;
}

// ===========================================================================================================================
unsigned long int KNewProjectDlg::getMinSize()
{
  return m_nMinSize;
}

// ===========================================================================================================================
unsigned long int KNewProjectDlg::getMaxSize()
{
  return m_nMaxSize;
}

// ===========================================================================================================================
void KNewProjectDlg::setDatas(const QString& strDir, const QString& strFilter)
{
  if (!strDir.isEmpty())
    m_comboDirectory->setEditText(strDir);

  if (!strFilter.isEmpty())
    m_comboFilter->setEditText(strFilter);
}


#include "knewprojectdlg.moc"

