Summary: xml viewer
Name: gxmlviewer
Version: 1.0
Release: 2
Copyright: GPL
Group: X11/Utilities
Source0: gxmlviewer-1.0-2.tar.gz

%description
XML viewer for gnome with Netscape plugin.









%prep
#%setup is the default macro that unpacks your source
# from the SOURCES directory into the BUILD directory
%setup -n $RPM_PACKAGE_NAME









%build
#cd gxmlviewer
./configure
make








%install
strip src/gxmlviewer
cp src/gxmlviewer /usr/local/bin
cp src/nsplugin/npxmlviewer /usr/lib/netscape/plugins/npgxmlviewer.so








%clean
# Commands to clean up after you install your package
rm -rf $RPM_BUILD_ROOT








%files
/usr/local/bin/gxmlviewer
/usr/lib/netscape/plugins/npgxmlviewer.so
