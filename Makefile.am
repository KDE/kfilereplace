## Makefile.am for kfilereplace

# set the include path for X, qt and KDE
INCLUDES  = $(all_includes)

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = . pics toolbar

# these are the headers for your project
noinst_HEADERS = kaddstringdlg.h kfilereplaceiface.h \
                 kfilereplacelib.h kconfirmdlg.h knewprojectdlg.h resource.h \
                 kaboutkfilereplace.h kfilereplaceview.h koptionsdlg.h


# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkfilereplacepart.la
libkfilereplacepart_la_SOURCES = kfilereplacepart.cpp kfilereplaceview.cpp kfilereplaceviewwdg.ui kaboutkfilereplace.cpp kaddstringdlg.cpp kaddstringdlgs.ui kconfirmdlg.cpp kconfirmdlgs.ui knewprojectdlg.cpp knewprojectdlgs.ui koptionsdlg.cpp koptionsdlgs.ui kfilereplacelib.cpp
libkfilereplacepart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkfilereplacepart_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KHTML)

kde_services_DATA = kfilereplacepart.desktop


messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kfilereplace.pot

KDE_ICON = AUTO

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/kfilereplacepart
rc_DATA = kfilereplacepartui.rc
