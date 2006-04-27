kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kfilereplacepart/icons FILES  )

kde4_footer()



#original Makefile.am contents follow:

#appdir = $(kde_datadir)/kfilereplacepart/icons
#app_ICON = AUTO
#
