## Process this file with automake to produce Makefile.in
# $Id: Makefile.am,v 1.4 2001/08/13 01:50:42 sean_stuckless Exp $

SUBDIRS = common

PLUGIN_DEFINES= -DXP_UNIX -DSNPRINTF

INCLUDES = \
	$(PLUGIN_DEFINES) \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR)


bin_PROGRAMS = npgxmlviewer.so

npgxmlviewer_so_SOURCES = \
	UnixShell.c \
	jri.h jri_md.h jritypes.h \
	npapi.h npupp.h

npgxmlviewer_so_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS)

npgxmlviewer_so_LDFLAGS = -shared

PLUGIN_DIR=/usr/lib/netscape/plugins

install-exec-local: $(bin_PROGRAMS)
	@if test -d $(DESTDIR)$(PLUGIN_DIR) ; then \
	  echo "Installing Netscape Pluggin in $(DESTDIR)$(PLUGIN_DIR)" ; \
	  echo rm -f $(DESTDIR)$(bindir)/$(bin_PROGRAMS) ; \
	  rm -f $(DESTDIR)$(bindir)/$(bin_PROGRAMS) ; \
	  if test -f $(bin_PROGRAMS) ; then \
	    echo "  $(INSTALL_PROGRAM) $(bin_PROGRAMS) $(DESTDIR)$(PLUGIN_DIR)/`echo $(bin_PROGRAMS)|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	     $(INSTALL_PROGRAM) $(bin_PROGRAMS) $(DESTDIR)$(PLUGIN_DIR)/`echo $(bin_PROGRAMS)|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	else \
	  echo "Netscape plugin will not be installed."; \
	fi; 

uninstall-local:
	@if test -f $(DESTDIR)$(PLUGIN_DIR)/$(bin_PROGRAMS) ; then \
	   echo "Uninstalling netscape plugin" ; \
	   echo "rm -f $(DESTDIR)$(PLUGIN_DIR)/$(bin_PROGRAMS)" ; \
	   rm -f $(DESTDIR)$(PLUGIN_DIR)/$(bin_PROGRAMS) ; \
	fi ; 


