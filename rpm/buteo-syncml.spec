Name: buteo-syncml
Version: 0.4.9
Release: 1
Summary: SyncML library for MeeGo sync
Group: System/Libraries
License: LGPLv2.1
URL: http://meego.gitorious.com/meego-middleware/buteo-syncml
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: qt-devel, doxygen
BuildRequires: pkgconfig(libwbxml2)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: openobex-devel
BuildRequires: buteo-syncfw-devel

%description
%{summary}.

%files
%defattr(-,root,root,-)
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


%package tests
Summary: Development files for %{name}
Group: System/Libraries
Requires: %{name} = %{version}-%{release}

%description tests
%{summary}.

%files tests
%defattr(-,root,root,-)
%{_bindir}/*-tests
%{_datadir}/libmeegosyncml-tests

%prep
%setup -q


%build
qmake libmeegosyncml.pro
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make INSTALL_ROOT=%{buildroot} install


%clean
rm -rf %{buildroot}


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
