Summary: System to user tools
Name: s2u
Version: 0.1
Release: 1mdk
URL: http://www.mandrakelinux.com/
Source0: %{name}-%{version}.tar.bz2
License: GPL
Group: Graphical desktop/Other
BuildRoot: %{_tmppath}/%{name}-buildroot
Prefix: %{_prefix}
Requires: dbus-x11
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
/etc/X11/xinit.d/s2u.sh
/etc/sysconfig/network-scripts/hostname.d/s2u

%changelog
* Sat Jul 31 2004 Frederic Lepied <flepied@mandrakesoft.com> 0.1-1mdk
- monitor hostname change

# end of file
