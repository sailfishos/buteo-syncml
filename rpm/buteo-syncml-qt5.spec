Name: buteo-syncml-qt5
Version: 0.5.16
Release: 1
Summary: SyncML library for MeeGo sync
License: LGPLv2+
URL: https://github.com/sailfishos/buteo-syncml/
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

%package devel
Summary: Development files for %{name}
Requires: %{name} = %{version}-%{release}

%description devel
%{summary}.

%package tests
Summary: Development files for %{name}
Requires: %{name} = %{version}-%{release}

%description tests
%{summary}.

%prep
%setup -q

%build
%qmake5 "VERSION=%{version}" -recursive libbuteosyncml.pro
%make_build

%install
%qmake_install

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%config %{_sysconfdir}/buteo/*.xsd
%config %{_sysconfdir}/buteo/*.xml
%{_libdir}/*.so.*

%files devel
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/*.prl
%{_libdir}/pkgconfig/*.pc

%files tests
/opt/tests/%{name}/*
