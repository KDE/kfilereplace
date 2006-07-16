

add_subdirectory( toolbar )

########### next target ###############

set(kfilereplacepart_PART_SRCS 
   kfilereplacepart.cpp 
   kfilereplaceview.cpp 
   kaddstringdlg.cpp 
   knewprojectdlg.cpp 
   koptionsdlg.cpp 
   kfilereplacelib.cpp 
   report.cpp 
   commandengine.cpp 
   configurationclasses.cpp )

qt4_add_dbus_adaptor( kfilereplacepart_PART_SRCS org.kde.kfilereplace.xml kfilereplacepart.h KFileReplacePart )

kde4_automoc(${kfilereplacepart_PART_SRCS})

kde4_add_ui3_files(kfilereplacepart_PART_SRCS 
   kfilereplaceviewwdg.ui 
   kaddstringdlgs.ui 
   knewprojectdlgs.ui 
   koptionsdlgs.ui )

kde4_add_plugin(kfilereplacepart WITH_PREFIX ${kfilereplacepart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfilereplacepart )

target_link_libraries(kfilereplacepart  ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KHTML_LIBS} )

install(TARGETS kfilereplacepart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kfilereplace_SRCS kfilereplace.cpp main.cpp )

kde4_automoc(${kfilereplace_SRCS})

kde4_add_executable(kfilereplace ${kfilereplace_SRCS})

target_link_libraries(kfilereplace  ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} )

install(TARGETS kfilereplace  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kfilereplace.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES  kfilereplacepartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kfilereplacepart)
install( FILES  kfilereplaceui.rc  DESTINATION ${DATA_INSTALL_DIR}/kfilereplace)
install( FILES  kfilereplacepart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

### Makefile.am for kfilereplace
#
## set the include path for X, qt and KDE
#INCLUDES  = $(all_includes)
#
## this has all of the subdirectories that make will recurse into.  if
## there are none, comment this out
#SUBDIRS = toolbar
#
## these are the headers for your project
#noinst_HEADERS = kaddstringdlg.h kfilereplaceiface.h kfilereplacelib.h \
#	knewprojectdlg.h kfilereplaceview.h koptionsdlg.h report.h commandengine.h \
#	configurationclasses.h kfilereplace.h
#		 
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkfilereplacepart.la
#libkfilereplacepart_la_SOURCES = kfilereplacepart.cpp kfilereplaceview.cpp \
#	kfilereplaceviewwdg.ui kaddstringdlg.cpp kaddstringdlgs.ui knewprojectdlg.cpp \
#	knewprojectdlgs.ui koptionsdlg.cpp koptionsdlgs.ui kfilereplacelib.cpp report.cpp \
#	commandengine.cpp configurationclasses.cpp
#				  
#libkfilereplacepart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#libkfilereplacepart_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KHTML)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kfilereplace.pot
#
#KDE_ICON = AUTO
#kde_services_DATA = kfilereplacepart.desktop
#xdg_apps_DATA = kfilereplace.desktop
#
## this is where the XML-GUI resource file goes
#partrcdir = $(kde_datadir)/kfilereplacepart
#partrc_DATA = kfilereplacepartui.rc
#
#shellrcdir = $(kde_datadir)/kfilereplace
#shellrc_DATA = kfilereplaceui.rc
#bin_PROGRAMS = kfilereplace
#kfilereplace_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#kfilereplace_LDADD = $(LIB_KPARTS)
#kfilereplace_SOURCES = kfilereplace.cpp main.cpp
