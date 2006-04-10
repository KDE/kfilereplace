/***************************************************************************
                           report.cpp  -  Report document class
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
// QT
#include <qstring.h>
#include <qfile.h>

// KDE
#include <klistview.h>
#include <kmessagebox.h>
#include <kuser.h>

// local
#include "report.h"
#include "configurationclasses.h"

void Report::createReportFile()
{
  QString xmlFileName = m_docPath + ".xml",
          cssFileName = m_docPath + ".css";

  // Generates a report file
  // a) Open the file
  QFile report(xmlFileName);
  if (!report.open( IO_WriteOnly ))
    {
      KMessageBox::error(0, i18n("<qt>Cannot open the file <b>%1</b>.</qt>", xmlFileName));
      return ;
    }

  // b) Write the header of the XML file

  QDateTime datetime = QDateTime::currentDateTime(Qt::LocalTime);
  QString dateString = datetime.toString(Qt::LocalDate);
  KUser user;
  QString columnTextFour,
          columnReplaceWith;
  if(!m_isSearchFlag)
    {
      columnTextFour = i18n("Replaced Strings");
      columnReplaceWith = i18n("Replace with");
    }
  else
    {
      columnTextFour = i18n("Total number occurrences");
      columnReplaceWith = i18n("-");
    }

  QString css =  cssFileName.mid(cssFileName.findRev("/")+1,cssFileName.length()-(cssFileName.findRev("/")+1));
  QTextStream oTStream( &report );
  oTStream << "<?xml version=\"1.0\"?>\n"
              "<?xml-stylesheet href=\""+css+"\" type=\"text/css\"?>"
              "<report>\n"
              " <title> "+i18n("KFileReplace Report")+" </title>\n"
              " <createdby>"+user.fullName()+"("+user.loginName()+")"+"</createdby>\n"
              " <date>"+dateString+"</date>\n"
              "<hr/>\n"
              " <table>\n"
              " <tablecaption> "+i18n("Searching/Replacing Strings Table")+" </tablecaption>\n"
              " <header>\n"
              "  <row>\n"
              "   <searchfor class=\"header\">"+i18n("Search for")+"</searchfor>\n";

              if(!m_isSearchFlag)
                oTStream<< "   <replacewith class=\"header\" >"+columnReplaceWith+"</replacewith>\n";

              oTStream<< "  </row>\n"
              " </header>\n";
  // c) Write the strings list
  QListViewItem *lviCurItem,
                *lviFirst;

  lviCurItem = lviFirst = m_stringsView->firstChild();

  if(lviCurItem == 0)
    return ;

  QString rowType="a1";

  do
    { QString rowTag = "<row >\n"
                       " <searchfor class=\""+rowType+"\"><![CDATA["+lviCurItem->text(0)+"]]></searchfor>\n"
                       " <replacewith class=\""+rowType+"\"><![CDATA["+lviCurItem->text(1)+"]]></replacewith>\n"
                       "</row>\n";

      oTStream << rowTag;

      rowType = ((rowType == "a1") ? "a2" : "a1");

      lviCurItem = lviCurItem->nextSibling();
    } while(lviCurItem && lviCurItem != lviFirst);

  oTStream<< "</table>\n";

  oTStream<< "<whiteline/>\n"
             " <table>\n"
             "  <tablecaption> "+i18n("Results Table")+ " </tablecaption>"
             "  <header>\n"
             "   <row>\n"
             "    <name class=\"header\">"+i18n("Name")+"</name>\n"
             "    <folder class=\"header\">"+i18n("Folder")+"</folder>\n";
             if(m_isSearchFlag)
               {
                 oTStream<< "    <oldsize class=\"header\">"+i18n("Size")+"</oldsize>\n";
               }
             else
               {
                 oTStream<< "    <oldsize class=\"header\">"+i18n("Old Size")+"</oldsize>\n"
                            "    <newsize class=\"header\">"+i18n("New Size")+"</newsize>\n";
               }
  oTStream<< "    <replacedstrings class=\"header\" >"+columnTextFour+"</replacedstrings>\n"
             "    <owneruser class=\"header\">"+i18n("Owner User")+"</owneruser>\n"
             "    <ownergroup class=\"header\">"+i18n("Owner Group")+"</ownergroup>\n"
             "   </row>\n"
             "  </header>\n";

  // d) Write the result list

  lviCurItem = lviFirst = m_resultsView->firstChild();

  if(lviCurItem == 0)
    return ;

  unsigned int totalOccurrences = 0;

  rowType="a1";

  do
    { QString rowTag = "   <row >\n"
                       "    <name class=\""+rowType+"\"><![CDATA["+lviCurItem->text(0)+"]]></name>\n"
                           "    <folder class=\""+rowType+"\"><![CDATA["+lviCurItem->text(1)+"]]></folder>\n";
      if(m_isSearchFlag)
        {
          rowTag += "    <oldsize class=\""+rowType+"\"><![CDATA["+lviCurItem->text(2)+"]]></oldsize>\n"
                    "    <replacedstrings class=\""+rowType+"\"><![CDATA["+lviCurItem->text(3)+"]]></replacedstrings>\n"
                    "    <owneruser class=\""+rowType+"\"><![CDATA["+lviCurItem->text(4)+"]]></owneruser>\n"
                    "    <ownergroup class=\""+rowType+"\"><![CDATA["+lviCurItem->text(5)+"]]></ownergroup>\n"
                    "   </row>\n";
        }
      else
        {
          rowTag += "    <oldsize class=\""+rowType+"\"><![CDATA["+lviCurItem->text(2)+"]]></oldsize>\n"
                    "    <newsize class=\""+rowType+"\"><![CDATA["+lviCurItem->text(3)+"]]></newsize>\n"
                    "    <replacedstrings class=\""+rowType+"\"><![CDATA["+lviCurItem->text(4)+"]]></replacedstrings>\n"
                    "    <owneruser class=\""+rowType+"\"><![CDATA["+lviCurItem->text(5)+"]]></owneruser>\n"
                    "    <ownergroup class=\""+rowType+"\"><![CDATA["+lviCurItem->text(6)+"]]></ownergroup>\n"
                    "   </row>\n";
        }

      oTStream << rowTag;

      rowType = ((rowType == "a1") ? "a2" : "a1");

      if(m_isSearchFlag)
        totalOccurrences += lviCurItem->text(3).toInt();
      else
        totalOccurrences += lviCurItem->text(4).toInt();

      lviCurItem = lviCurItem->nextSibling();
    } while(lviCurItem && lviCurItem != lviFirst);


  // e) Write the end of the file

   oTStream<< " </table>\n"
              "<totaloccurrences>"
           << totalOccurrences
           << "</totaloccurrences>\n"
              "</report>\n";

   report.close();
}

void Report::createStyleSheet()
{
  QString cssFileName = m_docPath +".css";
  QFile styleSheet(cssFileName);
  if (!styleSheet.open( IO_WriteOnly ))
    {
      KMessageBox::error(0, i18n("<qt>Cannot open the file <b>%1</b>.</qt>", cssFileName));
      return ;
    }

  QTextStream oTStream( &styleSheet );

  QString css = "title { display:block;font:40px bold sans-serif; }\n\n"
                "createdby:before { content :\""+i18n("Created by")+": \"; }\n"
                "createdby { display:inline; }\n\n"
                "date:before { content :\"-"+i18n("date")+": \"; }\n"
                "date { display:inline; }\n\n"
                "totaloccurrences:before { content :\""+i18n("Total occurrences")+": \"; }\n"
                "totaloccurrences { display:block;text-align:right; font-weight:bold;margin-top:5px;margin-right:5px;}\n"
                "tablecaption {display:table-caption;font:20px bold sans-serif;}\n\n"
                "hr {display:block;background:black;height:1px;margin:5px 0px 5px;}\n"
                "whiteline {display:block;height:16px;}\n\n"
                "searchfor {\n"
                "           display:table-cell;\n"
                "           border:1px solid black;\n"
                "           padding:0 7px 0; }\n\n";

                if(!m_isSearchFlag)
                  {
                    css += "replacewith {\n"
                           "             display:table-cell;\n"
                           "             border:1px solid black;\n"
                        "             padding:0 7px 0; }\n\n";
                  }

                css += "folder {\n"
                       "        display:table-cell;\n"
                       "        border:1px solid black;\n"
                       "        padding:0 7px 0; }\n\n"
                       "header { display: table-header-group; }\n\n"
                       "name {\n"
                       "      display:table-cell;\n"
                       "      border:1px solid black;\n"
                       "      padding:0 7px 0; }\n\n"
                       "newsize {\n"
                       "         display:table-cell;\n"
                       "         border:1px solid black;\n"
                       "         padding:0 7px 0;\n"
                       "         text-align:right; }\n\n"
                       "oldsize {\n"
                       "         display:table-cell;\n"
                       "         border:1px solid black;\n"
                       "         padding:0 7px 0;\n"
                       "         text-align:right; }\n\n"
                       "ownergroup {\n"
                       "            display:table-cell;\n"
                       "            border:1px solid black;\n"
                       "            padding:0 7px 0; }\n\n"
                       "owneruser {\n"
                       "           display:table-cell;\n"
                       "           border:1px solid black;\n"
                       "           padding:0 7px 0; }\n\n"
                "replacedstrings {\n"
                "                 text-align:right;\n"
                "                 display:table-cell;\n"
                "                 border:1px solid black;\n"
                "                 padding:0 7px 0; }\n\n"
                "*[class~=header] {\n"
                "                  background : lightgray;\n"
                "                  text-align : center; }\n\n"
                "row { display : table-row; }\n\n"
                "table {\n"
                "       display:table;\n"
                "       border-collapse: collapse; }\n\n"
                "*[class~=a1] {\n"
                "              background-color:aliceblue;\n"
                "              font-weight : bold;font-size:15px; }\n\n"
                "*[class~=a2] {\n"
                "              background-color:khaki;\n"
                "              font-weight : bold;\n"
                "              font-size:15px; }\n\n";

  oTStream << css;

  styleSheet.close();
}

void Report::createDocument(const QString& docPath)
{
  m_docPath = docPath;

  createStyleSheet();
  createReportFile();
}

