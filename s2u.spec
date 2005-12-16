Summary: System to user tools
Name: s2u
Version: 0.6
Release: %mkrel 1
URL: http://www.mandrivalinux.com/
Source0: %{name}-%{version}.tar.bz2
License: GPL
Group: Graphical desktop/Other
BuildRoot: %{_tmppath}/%{name}-buildroot
BuildRequires: dbus-devel
BuildRequires: gtk+2-devel
Requires: dbus
Requires: initscripts >= 7.06-52mdk

%description
Use dbus to communicate between from the system to the users.

%prep
%setup -q

%build
%make CFLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall_std

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc ChangeLog README AUTHORS LICENSE
%_bindir/s2u
%_sysconfdir/X11/xinit.d/s2u.sh
%_sysconfdir/sysconfig/network-scripts/hostname.d/s2u
%config(noreplace) %_sysconfdir/dbus-1/system.d/*.conf

# MAKE THE CHANGES IN CVS: NO PATCH OR SOURCE ALLOWED

%changelog
* Fri Dec 16 2005 Frederic Lepied <flepied@mandriva.com> 0.6-1mdk
- switch to Mandriva
- log actions by default
- mkrel

* Wed Mar 09 2005 Frederic Crozat <fcrozat@mandrakesoft.com> 0.5-3mdk 
- add new signal to start update-menus if requested by system
- remove dbus-x11 requires

* Tue Mar 08 2005 Frederic Crozat <fcrozat@mandrakesoft.com> 0.5-2mdk 
- connect to X server, so s2u exits when X exits

* Mon Mar 07 2005 Frederic Crozat <fcrozat@mandrakesoft.com> 0.5-1mdk 
- Release 0.5 :
 no longer use session bus, use system bus instead (fix Mdk bug #13166)

* Mon Oct 18 2004 Gwenole Beauchesne <gbeauchesne@mandrakesoft.com> 0.4-1mdk
- lib64 fixes

* Wed Aug 25 2004 Frederic Lepied <flepied@mandrakesoft.com> 0.3-1mdk
- don't put noreplace on these scripts
- changes in cvs
- use $DISPLAY in temp filename

* Wed Aug 25 2004 Götz Waschk <waschk@linux-mandrake.com> 0.2-2mdk
- mark config files
- fix file list
- drop prefix
- fix buildrequires

* Wed Aug 18 2004 Frederic Lepied <flepied@mandrakesoft.com> 0.2-1mdk
- add a require on dbus
- put temporary file in /tmp instead of /var/tmp and use a naming
  which includes the user name

* Tue Aug 03 2004 Christiaan Welvaart <cjw@daneel.dyndns.org> 0.1-2mdk
- fix buildrequires

* Sat Jul 31 2004 Frederic Lepied <flepied@mandrakesoft.com> 0.1-1mdk
- monitor hostname change

# end of file
