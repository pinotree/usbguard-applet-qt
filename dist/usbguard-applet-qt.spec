Name:           usbguard-applet-qt
Version:        0.2
Release:        1%{?dist}
Summary:        USBGuard Qt applet
Group:          System Environment/Libraries
License:        GPLv2+
URL:            http://nonesofar/
Source0:        %{name}%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}%{version}-%{release}-root-%(%{__id_u} -n)

# UDev
Requires: usbguard
BuildRequires: usbguard-devel
BuildRequires: qt-devel

%description
USBGuard Qt applet

%prep
%setup -q -n "%{name}%{version}"
chmod +w ./usr/lib64 && rm -rf ./usr/

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

qmake-qt4 -makefile QtApplet.pro
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_bindir}
install -p usbguard-applet-qt $RPM_BUILD_ROOT%{_bindir}/usbguard-applet-qt

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/usbguard-applet-qt

%changelog
* Tue Mar 24 2015 Daniel Kopecek <dkopecek@redhat.com> 0.2-1
- Update to version 0.2

* Tue Mar 17 2015 Daniel Kopecek <dkopecek@redhat.com> 0.1-1
- Initial package
