Name: buteo-syncml-qt5
Version: 0.5.9
Release: 1
Summary: SyncML library for MeeGo sync
License: LGPLv2+
URL: https://git.sailfishos.org/mer-core/buteo-syncml
Source0: %{name}-%{version}.tar.gz
BuildRequires: doxygen
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5XmlPatterns)
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5Sql)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(libwbxml2) >= 0.11.6
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(openobex)
BuildRequires: pkgconfig(buteosyncfw5) >= 0.6.24

%description
%{summary}.

%files
%defattr(-,root,root,-)
%config %{_sysconfdir}/buteo/*.xsd
%config %{_sysconfdir}/buteo/*.xml
%{_libdir}/*.so.*

%package devel
Summary: Development files for %{name}
Requires: %{name} = %{version}-%{release}

%description devel
%{summary}.

%files devel
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/*.prl
%{_libdir}/pkgconfig/*.pc


%package tests
Summary: Development files for %{name}
Requires: %{name} = %{version}-%{release}

%description tests
%{summary}.

%files tests
%defattr(-,root,root,-)
/opt/tests/%{name}/*


%prep
%setup -q


%build
%qmake5 "VERSION=%{version}" -recursive libbuteosyncml.pro
make %{?_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
