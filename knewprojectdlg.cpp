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


//QT
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qdatetimeedit.h>
#include <qlabel.h>
#include <qradiobutton.h> 
#include <qtextedit.h>
#include <qlistview.h>
//KDE
#include <kseparator.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <kdebug.h>
// local
#include "knewprojectdlg.h"
#include "resource.h"
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
  
 loadOptions();
 loadFileSizeFilter();
 loadDateAccessFilter();
 loadOwnerFilter();
 loadLocationsList();
 loadFiltersList();
 loadBackupExtensionFilter();
 setWhatsThis();
 
 connect(pbLocation, SIGNAL(clicked()), this, SLOT(slotDir()));
 connect(pbOK, SIGNAL(clicked()), this, SLOT(slotOK()));
 connect(pbCancel, SIGNAL(clicked()), this, SLOT(reject()));
 
 connect(chbSizeMin, SIGNAL(toggled(bool)), this, SLOT(slotEnableSpinboxSizeMin(bool)));
 connect(chbSizeMax, SIGNAL(toggled(bool)), this, SLOT(slotEnableSpinboxSizeMax(bool)));
 connect(chbDateMin, SIGNAL(toggled(bool)), dedDateMin, SLOT(setEnabled(bool)));
 connect(chbDateMax, SIGNAL(toggled(bool)), dedDateMax, SLOT(setEnabled(bool)));

 connect(chbDateMin,SIGNAL(toggled(bool)),this, SLOT(slotEnableCbValidDate(bool)));
 connect(chbDateMax,SIGNAL(toggled(bool)),this, SLOT(slotEnableCbValidDate(bool)));
 
 connect(chbOwnerUser, SIGNAL(toggled(bool)), this, SLOT(slotEnableChbUser(bool)));
 connect(chbOwnerGroup, SIGNAL(toggled(bool)), this, SLOT(slotEnableChbGroup(bool)));
 connect(chbBackup, SIGNAL(toggled(bool)), this, SLOT(slotEnableChbBackup(bool)));
 
 connect(rbSearchOnly, SIGNAL(toggled(bool)), this, SLOT(slotSearchOnly(bool)));
 connect(rbSearchReplace, SIGNAL(toggled(bool)), this, SLOT(slotSearchReplace(bool)));
 connect(pbAdd, SIGNAL(clicked()), this, SLOT(slotAdd())); 
 connect(pbDel, SIGNAL(clicked()), this, SLOT(slotDel())); 
 pbAdd->setIconSet(QIconSet(SmallIconSet(QString::fromLatin1("next")))); 
 pbDel->setIconSet(QIconSet(SmallIconSet(QString::fromLatin1("back")))); 

}

KNewProjectDlg::~KNewProjectDlg()
{
 m_config = 0L;
}

void KNewProjectDlg::slotDir()
{
   QString directoryString = KFileDialog::getExistingDirectory(QString::null, this, i18n("Project Directory"));
   if (!directoryString.isEmpty())
     cbLocation->setEditText(directoryString);
}

void KNewProjectDlg::slotOK()
{
   // Check that Search text and Filter are not empty
   m_info.setDirectory(cbLocation->currentText());
   m_info.setFilter(cbFilter->currentText());
   
   if (m_info.directory().isEmpty() or m_info.filter().isEmpty())
     {
       KMessageBox::error(this, i18n("You must fill the combo boxes (directory and filter) before continuing."));
       return;
     }
   
   //  OWNER OPTIONS 
   if ((chbOwnerUser->isChecked() and edOwnerUser->text().isEmpty()) or 
       (chbOwnerGroup->isChecked() and edOwnerGroup->text().isEmpty()))
   {
      KMessageBox::error(this, i18n("Some edit boxes are empty in the <b>Owner</b> page."));
      return ;
   }
   
   // Check option "Size Min/Max": check MinSize is not greater than MaxSize
   int minSize = spbSizeMin->value(),
       maxSize = spbSizeMax->value();
   if ((minSize != FileSizeOption) and (maxSize != FileSizeOption))
    if (minSize > maxSize)
   {
      KMessageBox::error(this, i18n("The minimum size is greater than the maximum size."));
      return ;
   }

   saveOptions();
   saveFileSizeFilter();
   saveDateAccessFilter();
   saveOwnerFilter();
   saveLocationsList();
   saveFiltersList();
   saveBackupExtensionFilter();
   setMap();
   accept();
}

void KNewProjectDlg::slotAdd()
{
  if(rbSearchOnly->isChecked())
    {
      QString text = m_editSearch->text();
      if(!text.isEmpty() and !contains(stringView,text,0))
        {
	  QListViewItem* lvi = new QListViewItem(stringView);
	  lvi->setText(0,text);
          m_editSearch->clear();
	}     
    }
  else
    { 
      QString searchText = m_editSearch->text(),
              replaceText = m_editReplace->text();
      
      if(!searchText.isEmpty() and 
         !replaceText.isEmpty() and
         !contains(stringView,searchText,0) and 
	 !contains(stringView,replaceText,1))
        {
	  QListViewItem* lvi = new QListViewItem(stringView);
	  lvi->setText(0,searchText);
	  m_editSearch->clear();
	  lvi->setText(1,replaceText);
	  m_editReplace->clear();
	}    
    }
}

void KNewProjectDlg::slotDel()
{
  // Choose current item or selected item
  QListViewItem* currItem = stringView->currentItem();
  
  // Do nothing if list is empty
  if(currItem == 0)
    return;
      
  if(rbSearchOnly->isChecked())
    {
      m_editSearch->setText(currItem->text(0));
      delete currItem;
    }
  else
    {
      m_editSearch->setText(currItem->text(0));
      m_editReplace->setText(currItem->text(1));
      delete currItem;
    }
}

void KNewProjectDlg::slotSearchOnly(bool b)
{
  m_editSearch->setEnabled(b);
  m_editReplace->setEnabled(false);
  m_labelSearch->setEnabled(b);
  m_labelReplace->setEnabled(false);
  
  stringView->clear();
}

void KNewProjectDlg::slotSearchReplace(bool b)
{
  m_editSearch->setEnabled(b);
  m_editReplace->setEnabled(b);
  m_labelSearch->setEnabled(b);
  m_labelReplace->setEnabled(b);
  
  stringView->clear();
}

void KNewProjectDlg::slotEnableSpinboxSizeMin(bool b)
{
  spbSizeMin->setEnabled(b);
}

void KNewProjectDlg::slotEnableSpinboxSizeMax(bool b)
{
  spbSizeMax->setEnabled(b);
}

void KNewProjectDlg::slotEnableCbValidDate(bool b)
{
  Q_UNUSED(b);
  cbDateValid->setEnabled(chbDateMax->isChecked() or chbDateMin->isChecked());
}

void KNewProjectDlg::slotEnableChbUser(bool b)
{
  cbOwnerUserType->setEnabled(b);
  cbOwnerUserBool->setEnabled(b);
  edOwnerUser->setEnabled(b);
}

void KNewProjectDlg::slotEnableChbGroup(bool b)
{
  cbOwnerGroupType->setEnabled(b);
  cbOwnerGroupBool->setEnabled(b);
  edOwnerGroup->setEnabled(b);
}

void KNewProjectDlg::slotEnableChbBackup(bool b)
{
  leBackup->setEnabled(b);
  tlBackup->setEnabled(b);
}

void KNewProjectDlg::loadOptions()
{
  m_config->setGroup("Options");
  chbIncludeSubfolders->setChecked(m_config->readBoolEntry(rcRecursive,RecursiveOption));
  chbCaseSensitive->setChecked(m_config->readBoolEntry(rcCaseSensitive, CaseSensitiveOption));  
  chbEnableVariables->setChecked(m_config->readBoolEntry(rcVariables, VariablesOption));
  chbWildcards->setChecked(m_config->readBoolEntry(rcWildcards, WildcardsOption));
}

void KNewProjectDlg::loadFileSizeFilter()
{
  m_config->setGroup("Options");
  //  FILE SIZE OPTIONS 
  int size = m_config->readNumEntry(rcMinFileSize,FileSizeOption);
  
  if(size == FileSizeOption)
    {
      chbSizeMin->setChecked(false);
      spbSizeMin->setEnabled(false);
      spbSizeMin->setValue(0);
    }
  else
    {
      chbSizeMin->setChecked(true);
      spbSizeMin->setEnabled(true);
      spbSizeMin->setValue(size);
    }
  
  size = m_config->readNumEntry(rcMaxFileSize,FileSizeOption);
  
  if(size == FileSizeOption)
    {
      chbSizeMax->setChecked(false);
      spbSizeMax->setEnabled(false);
      spbSizeMax->setValue(0);
    }
  else
    {
      chbSizeMax->setChecked(true);
      spbSizeMax->setEnabled(true);
      spbSizeMax->setValue(size);
    }  
}

void KNewProjectDlg::loadDateAccessFilter()
{
  m_config->setGroup("Options");
  // ================== DATE OPTIONS ========================
  
  QString validDate = m_config->readEntry(rcValidAccessDate,ValidAccessDateOption);
      
  QString dateMin = m_config->readEntry(rcMinDate,AccessDateOption);
  if(dateMin == AccessDateOption)
    {
      chbDateMin->setChecked(false);
      dedDateMin->setDate(dedDateMin->minValue());
      dedDateMin->setEnabled(false);
    }
  else
    {
      chbDateMin->setChecked(true);
      dedDateMin->setDate(QDate::fromString(dateMin,Qt::ISODate));
      dedDateMin->setEnabled(true);
    }
  
  QString dateMax = m_config->readEntry(rcMaxDate,AccessDateOption);  
  if(dateMax == AccessDateOption)
    {
      chbDateMax->setChecked(false);
      dedDateMax->setDate(dedDateMax->maxValue());
      dedDateMax->setEnabled(false);
    }
  else
    {
      chbDateMax->setChecked(true);
      dedDateMax->setDate(QDate::fromString(dateMax,Qt::ISODate));
      dedDateMax->setEnabled(true);
    }
    
  cbDateValid->setEnabled(chbDateMax->isChecked() or chbDateMin->isChecked());
  
  
}

void KNewProjectDlg::loadOwnerFilter()
{
  m_config->setGroup("Options");
  
  QStringList ownerList = QStringList::split(",",m_config->readEntry(rcOwnerUser, OwnerOption),true);
  
  if(ownerList[0] == "true")
    {
      chbOwnerUser->setChecked(true);
      cbOwnerUserType->setEnabled(true);
      cbOwnerUserBool->setEnabled(true);
      edOwnerUser->setEnabled(true);
    }   
  else
    {
      chbOwnerUser->setChecked(false);
      cbOwnerUserType->setEnabled(false);
      cbOwnerUserBool->setEnabled(false);
      edOwnerUser->setEnabled(false);
    }
    
    
  cbOwnerUserType->setCurrentText(ownerList[1]);
  
  cbOwnerUserBool->setCurrentText(ownerList[2]);
  
  if(ownerList[3] == "???")
    edOwnerUser->clear();
  else
    edOwnerUser->setText(ownerList[3]);
    
  ownerList = QStringList::split(",",m_config->readEntry(rcOwnerGroup, OwnerOption),true);
  
  if(ownerList[0] == "true")
    {
      chbOwnerGroup->setChecked(true);
      cbOwnerGroupType->setEnabled(true);
      cbOwnerGroupBool->setEnabled(true);
      edOwnerGroup->setEnabled(true);
    }
  else
    {
      chbOwnerGroup->setChecked(false);
      cbOwnerGroupType->setEnabled(false);
      cbOwnerGroupBool->setEnabled(false);
      edOwnerGroup->setEnabled(false);
    }
    
  cbOwnerGroupType->setCurrentText(ownerList[1]);
  
  cbOwnerGroupBool->setCurrentText(ownerList[2]);
  
  if(ownerList[3] == "???")
    edOwnerGroup->clear();
  else
    edOwnerGroup->setText(ownerList[3]);
}

void KNewProjectDlg::loadLocationsList()
{
  m_config->setGroup("Directories");
  #if KDE_IS_VERSION(3,1,3)
  QString locationsEntry = m_config->readPathEntry(rcDirectoriesList);
  #else
  QString locationsEntry = m_config->readEntry(rcDirectoriesList);
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
  QStringList filtersEntryList = m_config->readPathListEntry(rcFiltersList);
  #else
  QStringList filtersEntryList = m_config->readListEntry(rcFiltersList);
  #endif

  cbFilter->insertStringList(filtersEntryList);
  if(cbFilter->count() == 0)
    cbFilter->insertItem("*.htm;*.html;*.xml;*.xhtml;*.css;*.js;*.php");
}

void KNewProjectDlg::loadBackupExtensionFilter()
{
  m_config->setGroup("Options");
  
  QString backupExtension = m_config->readEntry(rcBackupExtension,BackupExtension);
  QStringList bkList = QStringList::split(",",backupExtension,true);
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
}

void KNewProjectDlg::saveOptions()
{
  m_config->setGroup("Options");
  m_config->writeEntry(rcRecursive, chbIncludeSubfolders->isChecked());
  m_config->writeEntry(rcCaseSensitive, chbCaseSensitive->isChecked());  
  m_config->writeEntry(rcVariables, chbEnableVariables->isChecked());
  m_config->writeEntry(rcWildcards, chbWildcards->isChecked());
        
  m_config->sync();
}

void KNewProjectDlg::saveFileSizeFilter()
{
  m_config->setGroup("Options");
  //  FILE SIZE OPTIONS 
  if(chbSizeMax->isChecked())
    m_config->writeEntry(rcMaxFileSize, spbSizeMax->value());
  else
    m_config->writeEntry(rcMaxFileSize,FileSizeOption );
           
  if(chbSizeMin->isChecked())
    m_config->writeEntry(rcMinFileSize, spbSizeMin->value());
  else
    m_config->writeEntry(rcMinFileSize,FileSizeOption );
    
  m_config->sync();
}

void KNewProjectDlg::saveDateAccessFilter()
{
  m_config->setGroup("Options");
  //  DATE OPTIONS 
  if(chbDateMin->isChecked() or chbDateMax->isChecked())
    m_config->writeEntry(rcValidAccessDate, cbDateValid->currentText());
  else
    m_config->writeEntry(rcValidAccessDate,ValidAccessDateOption);
  
  if(chbDateMin->isChecked())
    {
      QString date = dedDateMin->date().toString(Qt::ISODate);
      m_config->writeEntry(rcMinDate, date);
    }
  else
    m_config->writeEntry(rcMinDate, AccessDateOption);
       
  if(chbDateMax->isChecked())
    {
      QString date = dedDateMax->date().toString(Qt::ISODate);
      m_config->writeEntry(rcMaxDate, date);
    }
  else
    m_config->writeEntry(rcMaxDate, AccessDateOption);
        
  m_config->sync();
}

void KNewProjectDlg::saveOwnerFilter()
{
  m_config->setGroup("Options");
  
  QString list;
  if(chbOwnerUser->isChecked())
    {
      list = "true,"+ cbOwnerUserType->currentText()+","+cbOwnerUserBool->currentText()+","+edOwnerUser->text();
    }
  else
    list = OwnerOption;
  
  m_config->writeEntry(rcOwnerUser,list);      
  if(chbOwnerGroup->isChecked())
    {
      list = "true,"+ cbOwnerGroupType->currentText()+","+cbOwnerGroupBool->currentText()+","+edOwnerGroup->text();
    }
  else
    list = OwnerOption;
  
  m_config->writeEntry(rcOwnerGroup,list);         
  
  m_config->sync();
}

void KNewProjectDlg::saveLocationsList()
{
   m_config->setGroup("Directories");
   #if KDE_IS_VERSION(3,1,3)
   QStringList locationsEntryList = m_config->readPathListEntry(rcDirectoriesList);
   #else
   QStringList locationsEntryList = m_config->readListEntry(rcDirectoriesList);
   #endif

   QString lineText = cbLocation->lineEdit()->text();
   if(locationsEntryList.contains(lineText) == 0)
     locationsEntryList.prepend(lineText);

   m_config->writeEntry(rcDirectoriesList,locationsEntryList.join(","));
   m_config->sync();
}

void KNewProjectDlg::saveFiltersList()
{
  m_config->setGroup("Filters");
  #if KDE_IS_VERSION(3,1,3)
  QStringList filtersEntryList = m_config->readPathListEntry(rcFiltersList);
  #else
  QStringList filtersEntryList = m_config->readListEntry(rcFiltersList);
  #endif
   
  QString lineText = cbFilter->lineEdit()->text();
  if(filtersEntryList.contains(lineText) == 0)
    filtersEntryList.prepend(lineText);

  m_config->writeEntry(rcFiltersList,filtersEntryList.join(","));
  m_config->sync();
}

void KNewProjectDlg::saveBackupExtensionFilter()
{
  m_config->setGroup("Options");
  QString bkOptions;
  if(chbBackup->isChecked()) 
    bkOptions = "true";
  else
    bkOptions = "false";
  
  bkOptions += ","+leBackup->text();
  
  m_config->writeEntry(rcBackupExtension,bkOptions);
  m_config->sync();
}

void KNewProjectDlg::setWhatsThis()
{
  QWhatsThis::add(cbLocation, i18n(cbLocationWhatthis));
  QWhatsThis::add(cbFilter, i18n(cbFilterWhatthis));

  QWhatsThis::add(spbSizeMin, i18n(edSizeMinWhatthis));
  QWhatsThis::add(spbSizeMax, i18n(edSizeMaxWhatthis));
  
  QWhatsThis::add(cbDateValid, i18n(cbDateValidWhatthis));
  QWhatsThis::add(chbDateMin, i18n(chbDateMinWhatthis));
  QWhatsThis::add(chbDateMax, i18n(chbDateMaxWhatthis));
}

void KNewProjectDlg::setDatas(const QString& directoryString, const QString& filterString)
{
  if (!directoryString.isEmpty())
    cbLocation->setEditText(directoryString);

  if (!filterString.isEmpty())
    cbFilter->setEditText(filterString);
}

QString KNewProjectDlg::currentDir() const 
{ 
  return cbLocation->currentText(); 
}

QString KNewProjectDlg::currentFilter() const 
{ 
  return cbFilter->currentText(); 
}

QMap<QString,QString> KNewProjectDlg::stringsMap() const
{
  return m_map;
}

bool KNewProjectDlg::contains(QListView* lv,const QString& s, int column)
{
  QListViewItem* i = lv->firstChild();
  while (i != 0)
    {
      if(i->text(column) == s) 
        return true;
      i = i->nextSibling();
    }
  return false;
}

void KNewProjectDlg::setMap()
{
  m_map.clear();
  QListViewItem* i = stringView->firstChild();
  while (i != 0)
    {
      m_map[i->text(0)] = i->text(1);
      i = i->nextSibling();
    }
}
#include "knewprojectdlg.moc"

