/***************************************************************************
                           report.h  -  Report document class
                                      -------------------
    begin                : fri aug  13 15:29:46 CEST 2004

    copyright            : (C) 2004 Emiliano Gulmini
    email                : emi_barbarossa@yahoo.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef REPORT_H
#define REPORT_H

// QT
class QString;

// KDE
#include <klocale.h>
class KListView;

class Report
{
  private:
    KListView* m_stringView,
             * m_resultView;
    QString m_docPath;
    bool m_isSearchFlag;
    
  public:
    Report() {}
   
  private:
    void createReportFile();
    void createStyleSheet();
    
  public:
    void createDocument(const QString& docPath, KListView* stringView, KListView* resultView, bool isSearchFlag);
};

#endif // REPORT_H
