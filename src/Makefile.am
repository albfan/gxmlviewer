## Process this file with automake to produce Makefile.in

SUBDIRS = nsplugin extras 

INCLUDES = \
	-I$(top_srcdir)/intl \
        $(XML_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GNOME_INCLUDEDIR) \
	$(BONOBO_CFLAGS)

bin_PROGRAMS = gxmlviewer xmlview-control

gxmlviewer_SOURCES = \
	gxmlviewer.h \
	main.c constants.h \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
        xmlparser.c xmlparser.h \
	xmlwindow.c xmlwindow.h

gxmlviewer_LDADD = $(GTK_LIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) $(XML_LIBS)

xmlview_control_SOURCES = \
	xmlview-control.c \
	xmlview-control.h \
	xmlparser.c \
	xmlparser.h \
	support.c \
	support.h \
	xmlwindow.c \
	xmlwindow.h

xmlview_control_LDADD = \
	$(GTK_LIBS) \
	$(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) \
	$(XML_LIBS) $(BONOBO_LIBS)

oafdir = $(datadir)/oaf
oaf_in_files = GNOME_XMLView.oaf.in
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

EXTRA_DIST = $(oaf_in_files) $(oaf_DATA)

