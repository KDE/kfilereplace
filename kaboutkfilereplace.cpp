/***************************************************************************
                          kaboutkfilereplace.cpp  -  description
                             -------------------
    begin                : Sat Jul 22 2000
    copyright            : (C) 2000 by François Dupoux
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

#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <qlabel.h>

#include "kaboutkfilereplace.h"
#include "config.h"

KAboutKFileReplace::KAboutKFileReplace(QWidget *parent, const char *name, bool modal)
  :KAboutDialog(AbtKDEStandard | AbtProduct, "KFileReplace", KDialogBase::Close, KDialogBase::Close, parent, name, modal)
{
  setTitle("KFileReplace for KDE 3");
  setImage(locate("data", "kfilereplace/pics/about.png"));
  setImageFrame(false);
  setProduct(QString(PACKAGE), QString(VERSION), QString("François Dupoux"), QString("2002"));

  // 2. Add text to the dialog
  const QString strUseful = i18n(""
                                 "<p><b>KFileReplace</b> is an application which allows you to replace multiple "
                                 "strings in all files of a directory, with some powerful options. "
                                 "For example, you can search for the string \"www.old-site.com/info.html\" "
                                 "and replace it with \"www.new-site.org/\" in all \"*.html\" files of a directory. "
                                 "Then it is very useful when updating a link which has changed in all web pages "
                                 "of a website. But it can make win a lot of time in other situations. "
                                 "For example, when porting an application from KDE 1.x to KDE 2.x, you have a "
                                 "lot of functions names or files names to change, as \"kmsgbox.h\" with "
                                 "\"kmessagebox.h\".</p>"
                                 "<p>There are very powerful options in the new version: the wildcards and the variables "
                                 "support.</p>"
                                 "<p>The wildcards (* for an expression, and ? for a single character) can be used in the "
                                 "search and in the replace strings!</p>"
                                 "<p>The variables, can be the date, or the currently filename, in the format you want.</p>");

  const QString strContribute = i18n("<p><b>Everyone is welcome to contribute</b> to this project. "
                                     "You do not have to be a developer to do it. You can do many things:<br>"
                                     "- Write the SGML documentation in English;<br>"
                                     "- Translate the SGML documentation, or the program strings (in PO files);<br>"
                                     "- Write some C++ code to add new features;<br>"
                                     "- Send ideas about new features which would be useful;<br>"
                                     "- Send bug reports to the author;<br>"
                                     "- Draw new icons.<br></p>"
                                     " "
                                     "<p><b>Please, contact the author if you want to work to help this project</b>, then we will "
                                     "check the work is not already done, or being done. You also need to download "
                                     "the last version before writing anything: do not translate an old documentation version!  "
                                     "You can look at the TODO file to have more details about things to do in the project.</p>");

  // About page
  KAboutContainer *cAbout;
  cAbout = addScrolledContainerPage(i18n("&About"), KAboutDialog::AlignCenter, KAboutDialog::AlignLeft);
  if (cAbout)
    {
      static const QString strText1 = i18n("<p><b>KFileReplace</b> is distributed under the <b>GNU GPL (General Public License) license</b>.<br>"
                                           "Please, report all bugs you find to the author. <p>");

      cAbout->addWidget(new QLabel(strText1, topLevelWidget()));
      cAbout->addPerson(QString::null, QString("dupoux@dupoux.com"), QString("http://kfilereplace.sourceforge.net/"), QString::null, true);
      cAbout->addPerson(QString::null, QString(" "), QString(""), QString::null, true);
    }

  // Text pages
  addTextPage(i18n("&What KFileReplace does"), strUseful, true);
  addTextPage(i18n("&Contribute"), strContribute, true);

  // Authors page
  KAboutContainer *cAuthors = addScrolledContainerPage(i18n("Aut&hors"), KAboutDialog::AlignCenter, KAboutDialog::AlignLeft);
  if (cAuthors)
    {
      cAuthors->addPerson(QString("François Dupoux"), QString("dupoux@dupoux.com"), QString::null, i18n("Main author and French translator"), true);
      cAuthors->addPerson(QString("Andras Mantia"), QString("amantia@kde.org"), QString::null, i18n("KPartification, integration with Quanta Plus"), true);
      cAuthors->addPerson(QString("Heiko Goller"), QString("heiko.goller@tuebingen.mpg.de"), QString::null, QString("German translator"), true);
    }

  // GPL license text
  const KAboutData *aboutData = KGlobal::instance()->aboutData();
  if (!aboutData->license().isEmpty())
    addLicensePage(i18n("&License"), aboutData->license(), true);

}


KAboutKFileReplace::~KAboutKFileReplace()
{
}
