## Makefile.am for kfilereplace

# set the include path for X, qt and KDE
INCLUDES  = $(all_includes)

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = . pics toolbar

# these are the headers for your project
noinst_HEADERS = kaddstringdlg.h kfilereplaceiface.h kfilereplacelib.h \
	knewprojectdlg.h kfilereplaceview.h koptionsdlg.h report.h commandengine.h \
	configurationclasses.h kfilereplace.h
		 
# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkfilereplacepart.la
libkfilereplacepart_la_SOURCES = kfilereplacepart.cpp kfilereplaceview.cpp \
	kfilereplaceviewwdg.ui kaddstringdlg.cpp kaddstringdlgs.ui knewprojectdlg.cpp \
	knewprojectdlgs.ui koptionsdlg.cpp koptionsdlgs.ui kfilereplacelib.cpp report.cpp \
	commandengine.cpp configurationclasses.cpp
				  
libkfilereplacepart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkfilereplacepart_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KHTML)

kde_services_DATA = kfilereplacepart.desktop


messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kfilereplace.pot

KDE_ICON = AUTO
apps_DATA = kfilereplacepart.desktop
appsdir = $(kde_appsdir)/Utilities

xdg_apps_DATA = kfilereplace.desktop

# this is where the XML-GUI resource file goes
partrcdir = $(kde_datadir)/kfilereplacepart
partrc_DATA = kfilereplacepartui.rc

shellrcdir = $(kde_datadir)/kfilereplace
shellrc_DATA = kfilereplaceui.rc
bin_PROGRAMS = kfilereplace
kfilereplace_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kfilereplace_LDADD = $(LIB_KPARTS)
kfilereplace_SOURCES = kfilereplace.cpp main.cpp
