## Process this file with automake to produce Makefile.in

SUBDIRS = nsplugin

INCLUDES = \
	-I$(top_srcdir)/intl \
        $(XML_CFLAGS) \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = gxmlviewer

gxmlviewer_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
        xmlparser.c xmlparser.h

gxmlviewer_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) $(XML_LIBS)

