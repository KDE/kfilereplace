/***************************************************************************
                          knewprojectdlg.cpp  -  description
                             -------------------
    begin                : Tue Dec 28 1999
    copyright            : (C) 1999 by Fran�ois Dupoux
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

#include "knewprojectdlg.h"
#include "resource.h"

//#include <qvbox.h>
//#include <qbuttongroup.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
//#include <qlabel.h>
#include <qcheckbox.h>
//#include <qdir.h>
//#include <qdatetime.h>
//#include <qiconset.h>
//#include <qpixmap.h>

#include <kseparator.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <kiconloader.h>


KNewProjectDlg::KNewProjectDlg(QWidget *parent, KConfig *config, const char *name/*=0*/) : KNewProjectDlgS(parent, name, true)
{
   // copy data
 m_config = config;  
 QIconSet iconSet = SmallIconSet(QString::fromLatin1("fileopen"));
 QPixmap pixMap = iconSet.pixmap( QIconSet::Small, QIconSet::Normal );
 pbLocation->setIconSet(iconSet);
 pbLocation->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
 setWhatsThis();
 loadLocationsList();
 loadFiltersList();
 connect(pbLocation, SIGNAL(clicked()), this, SLOT(slotDir()));
 connect(pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
 connect(pbCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

KNewProjectDlg::~KNewProjectDlg()
{
 m_config = 0L;
}

QString KNewProjectDlg::location()
{
  return cbLocation->currentText();
}

QString KNewProjectDlg::filter()
{
  return cbFilter->currentText();
}

void KNewProjectDlg::slotDir()
{
   QString directoryString;

   directoryString = KFileDialog::getExistingDirectory(QString::null, this, i18n("Project Directory"));
   if (!directoryString.isEmpty())
    cbLocation->setEditText(directoryString);
}

 
void KNewProjectDlg::slotOK()
{
   bool minimumSize, 
        maximumSize, 
        minimumDate, 
        maximumDate;
   
   // Check the Search text and the Filter are not empty
   if (location().isEmpty() || filter().isEmpty())
   {
      KMessageBox::error(this, i18n("You must fill the combo boxes (directory and filter) before continuing."));
      return;
   }

   // Copy text into variables
   //addCurrentStringToCombo();
   saveLocationsList();
   saveFiltersList();
  
   // ================== SIZE OPTIONS ========================
   maxFilesSize(maximumSize, m_MaximumSizeNumber);
   minFilesSize(minimumSize, m_MinimumSizeNumber);

   // ================== DATE OPTIONS ========================
   QString minimumDateString, maximumDateString;

   minimumDate = chbDateMin->isChecked();
   maximumDate = chbDateMax->isChecked();
   minimumDateString = edDateMin->text();
   maximumDateString = edDateMax->text();

   // ================== OWNER OPTIONS ========================
   bool OwnerUser, OwnerGroup;
   OwnerUser = isOwnerUser();
   OwnerGroup = isOwnerGroup();

   if ((OwnerUser && edOwnerUser->text().isEmpty()) || (OwnerGroup && edOwnerGroup->text().isEmpty()))
   {
      KMessageBox::error(this, i18n("Some edit boxes are empty in the <b>Owner</b> page."));
      return ;
   }

   // =========================================================

   // Check all the fields are filled
   if (   (maximumSize && edSizeMax->text().isEmpty())
          || (minimumSize && edSizeMin->text().isEmpty())
          || (minimumDate && edDateMin->text().isEmpty())
          || (maximumDate && edDateMax->text().isEmpty()) )
   {
      KMessageBox::error(this, i18n("Some edit boxes are empty"));
      return ;
   }

   // Check option "Size Min/Max": check MinSize is not greater than MaxSize
   if (maximumSize && minimumSize && (m_MaximumSizeNumber < m_MinimumSizeNumber))
   {
      KMessageBox::error(this, i18n("The minimum size is greater than the maximum size."));
      return ;
   }

   // Check option "Date Min/Max": check date is valid
   // Check 2 things: * QRegExp ==> valid text (2/2/4)
   //                 * QDate::isValid()

   // 1. Check the first Date: DateMin (After...)
   if (minimumDate) // If "DateMin" option in checked
   {
      if ( !QDate::fromString(minimumDateString, Qt::ISODate).isValid())
      {
         KMessageBox::error(this, i18n("The dates must be in the YYYY-MM-DD format."));
         return ;
      }
   }

   // 2. Check the second Date: DateMax (Before...)
   if (maximumDate) // If "DateMax" option in checked
   {
      if ( !QDate::fromString(maximumDateString, Qt::ISODate).isValid())
      {
         KMessageBox::error(this, i18n("The dates must be in the YYYY-MM-DD format."));
         return ;
      }
   }

   // 3. Convert Strings to QDate
   if (minimumDate) // If "DateMin" option in checked
     m_MinDate = QDate::fromString(minimumDateString, Qt::ISODate);

   if (maximumDate) // If "DateMax" option in checked
    m_MaxDate = QDate::fromString(maximumDateString, Qt::ISODate);
  
   // 4. Check DateMax is not before DateMin
   if (minimumDate && maximumDate && m_MinDate > m_MaxDate)
   {
      KMessageBox::error(this, i18n("<qt>The <i>accessed after</i> date is after the <i>accessed before</i> date."));
      return ;
   }

   accept();
}

void KNewProjectDlg::loadLocationsList()
{
  m_config->setGroup("Directories");
  #if KDE_IS_VERSION(3,1,3)
  QStringList locationsEntryList = m_config->readPathListEntry("Directories list");
  #else
  QStringList locationsEntryList = m_config->readListEntry("Directories list");
  #endif

  bool flag = false;
  QStringList::Iterator locationsIt;
  for (locationsIt = locationsEntryList.begin(); locationsIt != locationsEntryList.end(); ++locationsIt)
   {
    cbLocation->insertItem((*locationsIt));
    flag = true;
   }

  if(!flag)
    cbLocation->insertItem(QDir::current().path());
}

void KNewProjectDlg::loadFiltersList()
{
  m_config->setGroup("Filters");
  #if KDE_IS_VERSION(3,1,3)
  QStringList filtersEntryList = m_config->readPathListEntry("Filters list");
  #else
  QStringList filtersEntryList = m_config->readListEntry("Filters list");
  #endif
  
  bool flag = false;
  QStringList::Iterator filtersIt;
  for (filtersIt = filtersEntryList.begin(); filtersIt != filtersEntryList.end(); ++filtersIt)
  {
   cbFilter->insertItem((*filtersIt));
   flag = true;
  }
  
  if(!flag)
    cbFilter->insertItem("*.htm;*.html;*.xml;*.xhtml;*.css;*.js;*.php");
}

void KNewProjectDlg::saveLocationsList()
{
   m_config->setGroup("Directories");
   QStringList locationsEntryList;
   int i;
   for (i = 0; i <= cbLocation->maxCount() ; i++)
    locationsEntryList.append(cbLocation-> text(i));
   
   m_config->writeEntry("Directories list",locationsEntryList.join(","));
   m_config->sync();
} 

void KNewProjectDlg::saveFiltersList()
{
   m_config->setGroup("Filters");
   QStringList filtersEntryList;
   int i;
   for (i = 0; i <= cbFilter->maxCount(); i++)
    filtersEntryList.append(cbFilter-> text(i));
    
   m_config->writeEntry("Filters list",filtersEntryList.join(","));
   m_config->sync();
}

void KNewProjectDlg::setWhatsThis()
{
   QWhatsThis::add(cbLocation, i18n("This is the directory where the search or the replace operation will be done."));
   QWhatsThis::add(cbFilter, i18n("Fix the filter of the files you want to search. For example, write \"*.htm\" to search or replace "
                                       "all web pages. You can put more than an only filter, when using ';'. For example \"*.html;*.txt;*.xml\""));

   QWhatsThis::add(edSizeMin, i18n("Minimal size of files. For example, if you put 1024 KB, all files which size is less than 1 MB will not be taken"));
   QWhatsThis::add(edSizeMax, i18n("Maximal size of files. For example, if you put 2048 KB, all files which size is more than 2 MB will not be taken"));
   QWhatsThis::add(edDateMin, i18n("Minimal date of files in YYYY/MM/DD format. For example, if you put 2000/01/31, all files which were "
                                       "(modified/read) before the 31 January 2000 will not be taken"));
   QWhatsThis::add(edDateMax, i18n("Maximal date of files in YYYY/MM/DD format. For example, if you put 1999/12/31, all files which were "
                                       "(modified/read) after the 31 December 1999 will not be taken"));
   QWhatsThis::add(cbDateValid, i18n("Select \"Writing\" if you want to use the date of the last modification, or \"reading\" to use the last "
                                       "read access to the file"));
}
 
void KNewProjectDlg::maxFilesSize(bool & bChecked, long unsigned int & nMaxSize)
{
  bChecked = chbSizeMax->isChecked();

  if (bChecked) // If option activated
    nMaxSize = edSizeMax->text().toULong() * 1024; // KB --> Bytes
  else
    nMaxSize = 0;
}
 
void KNewProjectDlg::minFilesSize(bool & bChecked, long unsigned int & nMinSize)
{
  bChecked = chbSizeMin->isChecked();

  if (bChecked) // If option activated
    nMinSize = edSizeMin->text().toULong() * 1024; // KB --> Bytes
  else
    nMinSize = 0;
}
 
int KNewProjectDlg::accessType()
{
  return cbDateValid->currentItem();
}
 
bool KNewProjectDlg::isMinDate()
{
  return chbDateMin->isChecked();
}
 
bool KNewProjectDlg::isMaxDate()
{
  return chbDateMax->isChecked();
}
 
bool KNewProjectDlg::isMinSize()
{
  return chbSizeMin->isChecked();
}
 
bool KNewProjectDlg::isMaxSize()
{
  return chbSizeMax->isChecked();
}
 
bool KNewProjectDlg::isOwnerUser()
{
  return chbOwnerUser->isChecked();
}
 
bool KNewProjectDlg::isOwnerGroup()
{
  return chbOwnerGroup->isChecked();
}
 
bool KNewProjectDlg::ownerUserMustBe()
{
  return (cbOwnerUserBool->currentText() == i18n("must be"));
}
 
bool KNewProjectDlg::ownerGroupMustBe()
{
  return (cbOwnerGroupBool->currentText() == i18n("must be"));
}
 
QString KNewProjectDlg::ownerUserValue()
{
  return edOwnerUser->text();
}
 
QString KNewProjectDlg::ownerGroupValue()
{
  return edOwnerGroup->text();
}
 
QString KNewProjectDlg::ownerUserType()
{
  return cbOwnerUserType->currentText();
}
 
QString KNewProjectDlg::ownerGroupType()
{
  return cbOwnerGroupType->currentText();
}
 
QDate KNewProjectDlg::minDate()
{
  return m_MinDate;
}
 
QDate KNewProjectDlg::maxDate()
{
  return m_MaxDate;
}
 
unsigned long int KNewProjectDlg::minSize()
{
  return m_MinimumSizeNumber;
}
 
unsigned long int KNewProjectDlg::maxSize()
{
  return m_MaximumSizeNumber;
}
 
void KNewProjectDlg::setDatas(const QString& directoryString, const QString& FilterString)
{
  if (!directoryString.isEmpty())
    cbLocation->setEditText(directoryString);

  if (!FilterString.isEmpty())
    cbFilter->setEditText(FilterString);
}

#include "knewprojectdlg.moc"

