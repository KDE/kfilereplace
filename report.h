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
#include "configurationclasses.h"
class KListView;

/** Report class is used to create a pair of files (a xml and a css file) which are
  * a short statistical report of the operations.
  */

class Report
{
  private:
    KListView* m_stringsView,
             * m_resultsView;
    QString m_docPath;
    bool m_isSearchFlag;
    RCOptions* m_option;

  public:
    Report(RCOptions* info, KListView* rv, KListView* sv) { m_option = info;
                                                            m_resultsView = rv;
                                                            m_stringsView = sv;
                                                            m_isSearchFlag = m_option->m_searchingOnlyMode;
                                                          }
    ~Report() { m_option = 0; }
  private:
    void createReportFile();
    void createStyleSheet();

  public:
    void createDocument(const QString& docPath);
};

#endif // REPORT_H
