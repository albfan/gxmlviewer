## Process this file with automake to produce Makefile.in

PLUGIN_DEFINES= -DXP_UNIX -DSNPRINTF

INCLUDES = \
	-Iinclude \
	$(PLUGIN_DEFINES) \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR)


bin_PROGRAMS = npxmlviewer

npxmlviewer_SOURCES = \
	UnixShell.c \
	stubs.c

npxmlviewer_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS)

npxmlviewer_LDFLAGS = -shared

