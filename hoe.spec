Summary: Subaru HDS OPE file Editor
Name: hoe
Version: 3.5.10
Release: 1%{?_dist_release}
License: GPL3
Group: Applications/Engineering
Packager: Akito Tajitsu <tajitsu@naoj.org>
URL: http://www.naoj.org/Observing/Instruments/HDS/hoe/
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
Vendor: Subaru Telescope, National Astoronomical Observatory of Japan
Requires:	gtk2 
Requires:	cairo
Requires:       libxml2
Requires:       libssh2
Requires:       openssl
Requires:       json-c
BuildRequires:	gtk2-devel
BuildRequires:	cairo-devel
BuildRequires:  libxml2-devel
BuildRequires:  libssh2-devel
BuildRequires:  openssl-devel
BuildRequires:  json-c-devel

%description
HDS OPE file Editor (HOE) is a tool-program to make operation (OPE) 
files for observations with HDS (High Dispersion Spectrograph) of
Subaru Telescope, NAOJ.
It also has a function as an Echelle format simulator and an
Exposure Time Calculator for HDS.

%prep
%setup -q -n hoe-%{version}

%build
./configure

make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/local
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf %{buildroot}

%files
/usr/local/bin/hoe

%changelog
* Mon Feb 14 2018 Akito Tajitsu <tajitsu@naoj.org>
- first release for version 3.3.0
