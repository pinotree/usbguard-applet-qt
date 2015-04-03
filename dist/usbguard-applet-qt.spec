Name:           usbguard-applet-qt
Version:        0.2
Release:        1%{?dist}
Summary:        USBGuard Qt applet
Group:          Applications/System
License:        GPLv2+
URL:            http://dkopecek.github.io/usbguard
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}%{version}-%{release}-root-%(%{__id_u} -n)

Requires: usbguard
BuildRequires: usbguard-devel
BuildRequires: qt-devel
BuildRequires: cmake

%description
USBGuard Qt applet for interaction with the USBGuard daemon.

%prep
%setup -q

%build
%ifarch sparc64
#sparc64 need big PIE
export CXXFLAGS="$RPM_OPT_FLAGS -fPIE"
export CFLAGS=$CXXFLAGS
export LDFLAGS="-pie -Wl,-z,relro -Wl,-z,now"
%else
export CXXFLAGS="$RPM_OPT_FLAGS -fpie"
export CFLAGS=$CXXFLAGS
export LDFLAGS="-pie -Wl,-z,relro -Wl,-z,now"
%endif

mkdir _build && cd _build
cmake ..
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_bindir}
install -p _build/usbguard-applet-qt $RPM_BUILD_ROOT%{_bindir}/usbguard-applet-qt

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/usbguard-applet-qt

%changelog
* Fri Apr 03 2015 Daniel Kopecek <dkopecek@redhat.com> 0.2-1
- Update to version 0.2
- cmake based build System
- Updated description
- Corrected package group

* Tue Mar 17 2015 Daniel Kopecek <dkopecek@redhat.com> 0.1-1
- Initial package
