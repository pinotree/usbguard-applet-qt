Name:           usbguard-applet-qt
Version:        0.3
Release:        1%{?dist}
Summary:        USBGuard Qt applet
Group:          Applications/System
License:        GPLv2+
URL:            https://dkopecek.github.io/usbguard
Source0:        https://dkpoecek.github.io/usbguard/dist/%{name}-%{version}.tar.gz

Requires: usbguard
BuildRequires: usbguard-devel
BuildRequires: qt-devel
BuildRequires: cmake

%description
USBGuard Qt applet for interaction with the USBGuard daemon.

%prep
%setup -q

%build
%cmake .
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

%files
%defattr(-,root,root,-)
%license LICENSE
%{_bindir}/usbguard-applet-qt
%{_mandir}/man1/usbguard-applet-qt.1.gz
%{_datadir}/applications/usbguard-applet-qt.desktop

%changelog
* Thu Apr 09 2015 Daniel Kopecek <dkopecek@redhat.com> 0.3-1
- Update to version 0.3
- removed the explicit usage of BuildRoot tag and RPM_BUILD_ROOT variable
- ship LICENSE in the binary rpm
- install a desktop file
- install a manual page

* Fri Apr 03 2015 Daniel Kopecek <dkopecek@redhat.com> 0.2-1
- Update to version 0.2
- cmake based build System
- Updated description
- Corrected package group

* Tue Mar 17 2015 Daniel Kopecek <dkopecek@redhat.com> 0.1-1
- Initial package
