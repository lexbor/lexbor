Name:     liblexbor
Version:  %{lib_version}
Release:  1%{?dist}
Epoch:    1
Summary:  Lexbor is a HTML renderer library without dependencies.
License:  Apache 2.0
Group:    Development/Tools
URL:      https://github.com/lexbor/lexbor
Source0:  %{name}-%{version}.tar.gz

%if 0%{?rhel}%{?fedora}
BuildRequires: cmake >= 3.0
BuildRequires: make
BuildRequires: gcc
BuildRequires: gcc-c++
%endif

%description
The lexbor is being developed using the C language, without dependencies, and will benefit from the following unique features:

Easy to port to any platform
Embeddable, binding for third party programming languages
Speed
Full specifications support

%package devel
Requires: %{name} = %{epoch}:%{version}-%{release}
Summary:  Development files for %{name}
Provides: %{name}-devel
Group:    Development/Tools

%description devel
This package contains necessary header files for %{name} development.

%prep
%setup -qn %{name}-%{version}

%build
export CFLAGS='%{optflags}'
export CXXFLAGS='%{optflags}'
cmake -DCMAKE_BUILD_TYPE=RELEASE -DLEXBOR_BUILD_STATIC=ON -DLEXBOR_BUILD_SHARED=ON -DCMAKE_INSTALL_PREFIX:PATH=%{_prefix} -DCMAKE_INSTALL_LIBDIR=%{_libdir} .
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc README.md LICENSE NOTICE
%{_libdir}/%{lib_ext}

%files devel
%defattr(-,root,root,-)
%doc README.md LICENSE NOTICE
%{_libdir}/*.a
%{_includedir}/*

%changelog
* Mon Apr 01 2019 Please, see changelog at https://github.com/lexbor/lexbor/blob/master/CHANGELOG.md - 0.2.0-1
- release