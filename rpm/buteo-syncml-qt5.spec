Name: buteo-syncml-qt5
Version: 0.5.0
Release: 1
Summary: SyncML library for MeeGo sync
Group: System/Libraries
License: LGPLv2.1
URL: https://github.com/nemomobile/buteo-syncml
Source0: %{name}-%{version}.tar.gz
BuildRequires: doxygen
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5XmlPatterns)
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5Sql)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(libwbxml2)
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
Group: Development/Libraries
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
Group: System/Libraries
Requires: %{name} = %{version}-%{release}

%description tests
%{summary}.

%files tests
%defattr(-,root,root,-)
/opt/tests/buteo-syncml/test-definition/tests.xml
/opt/tests/buteo-syncml/libbuteosyncml-tests
/opt/tests/buteo-syncml/runstarget.sh
/opt/tests/buteo-syncml/data/


%prep
%setup -q


%build
qmake -qt=5 -recursive libbuteosyncml.pro
make %{?_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
