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

#include "knewprojectdlg.h"
#include "resource.h"
//QT
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
//KDE
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

#include "whatthis.h"

using namespace whatthisNameSpace;


KNewProjectDlg::KNewProjectDlg(QWidget *parent, KConfig *config, const char *name/*=0*/) : KNewProjectDlgS(parent, name, true)
{
   // copy data
 m_config = config;
 QIconSet iconSet = SmallIconSet(QString::fromLatin1("fileopen"));
 QPixmap pixMap = iconSet.pixmap( QIconSet::Small, QIconSet::Normal );
 pbLocation->setIconSet(iconSet);
 pbLocation->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
 
 spbSizeMin->setEnabled(chbSizeMin->isChecked());
 
 spbSizeMax->setEnabled(chbSizeMax->isChecked());
 
 setWhatsThis();
 loadLocationsList();
 loadFiltersList();
 connect(pbLocation, SIGNAL(clicked()), this, SLOT(slotDir()));
 connect(pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
 connect(pbCancel, SIGNAL(clicked()), this, SLOT(reject()));
 connect(chbSizeMin, SIGNAL(toggled(bool)), this, SLOT(slotEnableSpinboxSizeMin(bool)));
 connect(chbSizeMax, SIGNAL(toggled(bool)), this, SLOT(slotEnableSpinboxSizeMax(bool)));
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
      KMessageBox::error(this, i18n("<qt>Some edit boxes are empty in the <b>Owner</b> page.</qt>"));
      return ;
   }

   // =========================================================
   
   // Check all the fields are filled
   if (   (minimumDate && edDateMin->text().isEmpty())
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

void KNewProjectDlg::slotEnableSpinboxSizeMin(bool b)
{
 spbSizeMin->setEnabled(b);
}

void KNewProjectDlg::slotEnableSpinboxSizeMax(bool b)
{
 spbSizeMax->setEnabled(b);
}

void KNewProjectDlg::loadLocationsList()
{
  m_config->setGroup("Directories");
  #if KDE_IS_VERSION(3,1,3)
  QString locationsEntry = m_config->readPathEntry("Directories list");
  #else
  QString locationsEntry = m_config->readEntry("Directories list");
  #endif
  QStringList locationsEntryList = QStringList::split(",",locationsEntry);

  cbLocation->insertStringList(locationsEntryList);
  if(cbLocation->count() == 0)
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

  cbFilter->insertStringList(filtersEntryList);
  if(cbFilter->count() == 0)
    cbFilter->insertItem("*.htm;*.html;*.xml;*.xhtml;*.css;*.js;*.php");
}

void KNewProjectDlg::saveLocationsList()
{
   m_config->setGroup("Directories");
   #if KDE_IS_VERSION(3,1,3)
   QStringList locationsEntryList = m_config->readPathListEntry("Directories list");
   #else
   QStringList locationsEntryList = m_config->readListEntry("Directories list");
   #endif

   QString lineText = cbLocation->lineEdit()->text();
   if(locationsEntryList.contains(lineText) == 0)
     locationsEntryList.append(lineText);

   m_config->writeEntry("Directories list",locationsEntryList.join(","));
   m_config->sync();
}

void KNewProjectDlg::saveFiltersList()
{
   m_config->setGroup("Filters");
   #if KDE_IS_VERSION(3,1,3)
   QStringList filtersEntryList = m_config->readPathListEntry("Filters list");
   #else
   QStringList filtersEntryList = m_config->readListEntry("Filters list");
   #endif

   QString lineText = cbFilter->lineEdit()->text();
   if(filtersEntryList.contains(lineText) == 0)
     filtersEntryList.append(lineText);

   m_config->writeEntry("Filters list",filtersEntryList.join(","));
   m_config->sync();
}

void KNewProjectDlg::setWhatsThis()
{
   QWhatsThis::add(cbLocation, i18n(cbLocationWhatthis));
   QWhatsThis::add(cbFilter, i18n(cbFilterWhatthis));

   QWhatsThis::add(spbSizeMin, i18n(edSizeMinWhatthis));
   QWhatsThis::add(spbSizeMax, i18n(edSizeMaxWhatthis));
   QWhatsThis::add(edDateMin, i18n(edDateMinWhatthis));
   QWhatsThis::add(edDateMax, i18n(edDateMaxWhatthis));
   QWhatsThis::add(cbDateValid, i18n(cbDateValidWhatthis));
}

void KNewProjectDlg::maxFilesSize(bool & bChecked, long unsigned int & nMaxSize)
{
  bChecked = chbSizeMax->isChecked();

  if (bChecked) // If option activated
    nMaxSize = spbSizeMax->value() * 1024; // KB --> Bytes
  else
    nMaxSize = 0;
}

void KNewProjectDlg::minFilesSize(bool & bChecked, long unsigned int & nMinSize)
{
  bChecked = chbSizeMin->isChecked();

  if (bChecked) // If option activated
    nMinSize = spbSizeMin->value() * 1024; // KB --> Bytes
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

