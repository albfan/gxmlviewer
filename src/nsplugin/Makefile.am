## Process this file with automake to produce Makefile.in

SUBDIRS = common

PLUGIN_DEFINES= -DXP_UNIX -DSNPRINTF

INCLUDES = \
	$(PLUGIN_DEFINES) \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR)


bin_PROGRAMS = npxmlviewer

npxmlviewer_SOURCES = \
	UnixShell.c \
	stubs.c jri.h jri_md.h jritypes.h \
	npapi.h npupp.h

npxmlviewer_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS)

npxmlviewer_LDFLAGS = -shared

