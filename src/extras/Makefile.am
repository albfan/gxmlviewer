## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
        $(XML_CFLAGS)

bin_PROGRAMS = xmlformatter

xmlformatter_SOURCES = \
	xmlformatter.c

xmlformatter_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(XML_LIBS)


