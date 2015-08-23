#
# spec file for package kio_adb
#
# Copyright (c) 2015 Christian Ferbar
#
#

Name:		kio_adb
Version:	0.1
Release:	1
License:	GPL 3
Summary:	kio-slave adb
Url:		http://www.github.com/ferbar
Group:		kde
Source:		%{name}-%{version}.tar.gz
# Patch:
BuildRequires:	libkde4-devel
# PreReq:
# Provides:
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
%{summary}

%prep
%setup -q -n %{name}-%{version}
echo %{buildroot}
echo  $RPM_BUILD_ROOT 
pwd


%build
  %cmake_kde4 -d build
  %make_jobs

#echo "build target ----------------------------------------------"
# make %{?_smp_mflags}
#echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
#pwd
#mkdir build
#cd build
#cmake -DCMAKE_BUILD_TYPE=debugfull ..

%install
# make install DESTDIR=%{buildroot} %{?_smp_mflags}
  cd build
  %makeinstall

%post

%postun

%files
%defattr(-,root,root)
#%doc ChangeLog README COPYING
#/usr/lib64/kde4/kio_adb.so
#/usr/share/kde4/services/adb.protocol

%{_kde4_modulesdir}/kio_adb.so
#%dir %{_kde4_appsdir}/konqueror/
#%dir %{_kde4_appsdir}/konqueror/dirtree/
#%dir %{_kde4_appsdir}/konqueror/dirtree/remote/
#%{_kde4_appsdir}/konqueror/dirtree/remote/mtp-network.desktop
#%dir %{_kde4_appsdir}/remoteview/
#%{_kde4_appsdir}/remoteview/mtp-network.desktop
#%dir %{_kde4_appsdir}/solid/
#%dir %{_kde4_appsdir}/solid/actions/
#%{_kde4_appsdir}/solid/actions/solid_mtp.desktop
%{_kde4_servicesdir}/adb.protocol
