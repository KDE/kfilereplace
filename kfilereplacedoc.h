/***************************************************************************
                          kfilereplacedoc.h  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by Fran�ois Dupoux
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

#ifndef KFILEREPLACEDOC_H
#define KFILEREPLACEDOC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h> /* mode_t and off_t */
#include <sys/mman.h>
#include <sys/param.h>


// include files for QT
#include <qobject.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qstring.h>


// forward declaration of the KFileReplace classes
class KFileReplaceView;
#ifdef APP
class KFileReplaceApp;
#else
class KFileReplacePart;
#endif

/**        KFileReplaceDoc provides a document object for a document-view model.
  *
        * The KFileReplaceDoc class provides a document object that can be used in conjunction with the classes KFileReplaceApp and KFileReplaceView
        * to create a document-view model for standard KDE applications based on KApplication and KTMainWindow. Thereby, the document object
        * is created by the KFileReplaceApp instance and contains the document structure with the according methods for manipulation of the document
        * data by KFileReplaceView objects. Also, KFileReplaceDoc contains the methods for serialization of the document data from and to files.
        *
        * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
        * @version KDevelop version 0.4 code generation
        */
class KFileReplaceDoc : public QObject
{
  Q_OBJECT

 public:
  /** Constructor for the fileclass of the application */
  KFileReplaceDoc(QWidget* parentWidget, QObject* parent, const char *name=0);
  /** Destructor for the fileclass of the application */
  ~KFileReplaceDoc();
        /** adds a view to the document which represents the document contents. Usually this is your main view. */
  void addView(KFileReplaceView* m_pView);
        /** removes a view from the list of currently connected views */
        void removeView(KFileReplaceView* m_pView);
  /** sets the modified flag for the document after a modifying action on the view connected to the document.*/
  void setModified(bool modified=true){ b_modified=modified; }
        /** returns if the document is modified or not. Use this to determine if your document needs saving by the user on closing.*/
  bool isModified(){ return b_modified;}
        /** "save modified" - asks the user for saving if the document is modified */
        bool saveModified();
        /** deletes the document's contents */
        void deleteContents();
        /** initializes the document generally */
        bool newDocument(const QString& strArguDir=QString::null, const QString& strArguFilter=QString::null, bool showNewProjectDlg = true);
        /** closes the acutal document */
        void closeDocument();
        /** sets the path to the file connected with the document */
        void pathName(const char* path_name);
        /** returns the pathname of the current document file*/
        const QString& getPathName() const;
        /** sets the filename of the document */
        void title(const char* title);
        /** returns the title of the document */
        const QString& getTitle() const;

 public slots:
         /** calls repaint() on all views connected to the document object and is called by the view by which the document has been changed.
           * As this view normally repaints itself, it is excluded from the paintEvent. */
         void slotUpdateAllViews(KFileReplaceView* pSender);

 public:
         /** the list of the views currently connected to the document */
        static QPtrList<KFileReplaceView>* viewList;
 private:
         /** the modified flag of the current document */
         bool b_modified;
        QString m_title;
        QString m_path;
#ifdef APP
        KFileReplaceApp *m_parent;
#else
        KFileReplacePart *m_parent;
#endif

public: // Project data

        // General data
        QString m_strProjectDirectory;
        QString m_strProjectFilter;

        // Size Criteria
        bool m_bMinSize;
        bool m_bMaxSize;
        uint m_nMinSize;
        uint m_nMaxSize;

        // Date Critera
        int m_nTypeOfAccess; // Read or Write access
        bool m_bMinDate;
        bool m_bMaxDate;
        QDate m_qdMinDate;
        QDate m_qdMaxDate;

        // Owner criteria
        bool m_bOwnerUserBool;
        bool m_bOwnerGroupBool;
        bool m_bOwnerUserMustBe;
        bool m_bOwnerGroupMustBe;
        QString m_strOwnerUserType;
        QString m_strOwnerGroupType;
        QString m_strOwnerUserValue;
        QString m_strOwnerGroupValue;
};

#endif // KFILEREPLACEDOC_H





















