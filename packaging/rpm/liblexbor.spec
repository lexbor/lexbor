Name:     liblexbor
Version:  %{lib_version}
Release:  1%{?dist}
Epoch:    1
Summary:  Lexbor is development of an open source HTML Renderer library.
License:  Apache 2.0
Group:    Development/Tools
URL:      https://github.com/lexbor/lexbor
Source0:  %{name}-%{version}.tar.gz

%if 0%{?rhel}%{?fedora}
BuildRequires: cmake >= 2.8
BuildRequires: make
BuildRequires: gcc
BuildRequires: gcc-c++
%endif

%description
Modules:
core, css, dom, encoding, html, ns, tag, utils

%package devel
Requires: %{name} = %{epoch}:%{version}-%{release}
Summary:  Static library and headers for the liblexbor.
Provides: %{name}-devel
Group:    Development/Tools

%description devel
Modules:
core, css, dom, encoding, html, ns, tag, utils

%prep
%setup -qn %{name}-%{version}

%build
export CFLAGS='%{optflags}'
export CXXFLAGS='%{optflags}'
cmake -DCMAKE_BUILD_TYPE=RELEASE -DLEXBOR_BUILD_SEPARATELY=ON \
      -DLEXBOR_BUILD_STATIC=ON -DLEXBOR_BUILD_SHARED=ON \
      -DCMAKE_INSTALL_PREFIX:PATH=%{_prefix} -DCMAKE_INSTALL_LIBDIR=%{_libdir} .
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
%{_libdir}/%{name}.*
%exclude %{_libdir}/*.a

%files devel
%defattr(-,root,root,-)
%doc README.md LICENSE NOTICE
%{_libdir}/%{name}_static.a
%{_includedir}/*

# Modules
%package core
Version:  0.3.1
Release:  1%{?dist}
Epoch:    1
Summary:  Shared "core" library from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools

%description core
The main module of the Lexbor project.
The module includes various algorithms and methods for working with memory:
AVL Tree, Array, String, Memory Pool and so on.

%files core
%defattr(-,root,root,-)
%{_libdir}/liblexbor-core*
%exclude %{_libdir}/*.a

%package core-devel
Version:  0.3.1
Release:  1%{?dist}
Epoch:    1
Summary:  Static "core" library and headers from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools

%description core-devel
The main module of the Lexbor project.
The module includes various algorithms and methods for working with memory:
AVL Tree, Array, String, Memory Pool and so on.

%files core-devel
%defattr(-,root,root,-)
%{_libdir}/*lexbor-core_static.a
%{_includedir}/lexbor/core/*

%package css
Version:  0.1.0
Release:  1%{?dist}
Epoch:    1
Summary:  Shared "css" library from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core = %{epoch}:0.3.1-%{release}

%description css
The module implemented by CSS specification.

%files css
%defattr(-,root,root,-)
%{_libdir}/liblexbor-css*
%exclude %{_libdir}/*.a

%package css-devel
Version:  0.1.0
Release:  1%{?dist}
Epoch:    1
Summary:  Static "css" library and headers from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core-devel = %{epoch}:0.3.1-%{release}

%description css-devel
The module implemented by CSS specification.

%files css-devel
%defattr(-,root,root,-)
%{_libdir}/*lexbor-css_static.a
%{_includedir}/lexbor/css/*

%package dom
Version:  0.2.1
Release:  1%{?dist}
Epoch:    1
Summary:  Shared "dom" library from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core = %{epoch}:0.3.1-%{release}
Requires: liblexbor-tag = %{epoch}:0.2.0-%{release}
Requires: liblexbor-ns = %{epoch}:0.2.0-%{release}

%description dom
The module implemented by DOM specification.
Includes functions for manipulating DOM tree: nodes, attributes, events.

%files dom
%defattr(-,root,root,-)
%{_libdir}/liblexbor-dom*
%exclude %{_libdir}/*.a

%package dom-devel
Version:  0.2.1
Release:  1%{?dist}
Epoch:    1
Summary:  Static "dom" library and headers from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core-devel = %{epoch}:0.3.1-%{release}
Requires: liblexbor-tag-devel = %{epoch}:0.2.0-%{release}
Requires: liblexbor-ns-devel = %{epoch}:0.2.0-%{release}

%description dom-devel
The module implemented by DOM specification.
Includes functions for manipulating DOM tree: nodes, attributes, events.

%files dom-devel
%defattr(-,root,root,-)
%{_libdir}/*lexbor-dom_static.a
%{_includedir}/lexbor/dom/*

%package encoding
Version:  2.0.0
Release:  1%{?dist}
Epoch:    1
Summary:  Shared "encoding" library from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core = %{epoch}:0.3.1-%{release}

%description encoding
The module implemented by Encode specification.
Includes functions for encoding and decoding text.

Supports encodings:
big5, euc-jp, euc-kr, gbk, ibm866, iso-2022-jp, iso-8859-10, iso-8859-13,
iso-8859-14, iso-8859-15, iso-8859-16, iso-8859-2, iso-8859-3, iso-8859-4,
iso-8859-5, iso-8859-6, iso-8859-7, iso-8859-8, iso-8859-8-i, koi8-r, koi8-u,
shift_jis, utf-16be, utf-16le, utf-8, gb18030, macintosh, replacement,
windows-1250, windows-1251, windows-1252, windows-1253, windows-1254,
windows-1255, windows-1256, windows-1257, windows-1258, windows-874,
x-mac-cyrillic, x-user-defined.

%files encoding
%defattr(-,root,root,-)
%{_libdir}/liblexbor-encoding*
%exclude %{_libdir}/*.a

%package encoding-devel
Version:  2.0.0
Release:  1%{?dist}
Epoch:    1
Summary:  Static "encoding" library and headers from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core-devel = %{epoch}:0.3.1-%{release}

%description encoding-devel
The module implemented by Encode specification.
Includes functions for encoding and decoding text.

Supports encodings:
big5, euc-jp, euc-kr, gbk, ibm866, iso-2022-jp, iso-8859-10, iso-8859-13,
iso-8859-14, iso-8859-15, iso-8859-16, iso-8859-2, iso-8859-3, iso-8859-4,
iso-8859-5, iso-8859-6, iso-8859-7, iso-8859-8, iso-8859-8-i, koi8-r, koi8-u,
shift_jis, utf-16be, utf-16le, utf-8, gb18030, macintosh, replacement,
windows-1250, windows-1251, windows-1252, windows-1253, windows-1254,
windows-1255, windows-1256, windows-1257, windows-1258, windows-874,
x-mac-cyrillic, x-user-defined.

%files encoding-devel
%defattr(-,root,root,-)
%{_libdir}/*lexbor-encoding_static.a
%{_includedir}/lexbor/encoding/*

%package html
Version:  0.3.0
Release:  1%{?dist}
Epoch:    1
Summary:  Shared "html" library from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core = %{epoch}:0.3.1-%{release}
Requires: liblexbor-dom = %{epoch}:0.2.1-%{release}
Requires: liblexbor-ns = %{epoch}:0.2.0-%{release}
Requires: liblexbor-tag = %{epoch}:0.2.0-%{release}

%description html
The module implemented by HTML specification.
Includes functions for parsing HTML, build DOM tree and DOM/HTML serialization.

%files html
%defattr(-,root,root,-)
%{_libdir}/liblexbor-html*
%exclude %{_libdir}/*.a

%package html-devel
Version:  0.3.0
Release:  1%{?dist}
Epoch:    1
Summary:  Static "html" library and headers from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core-devel = %{epoch}:0.3.1-%{release}
Requires: liblexbor-dom-devel = %{epoch}:0.2.1-%{release}
Requires: liblexbor-ns-devel = %{epoch}:0.2.0-%{release}
Requires: liblexbor-tag-devel = %{epoch}:0.2.0-%{release}

%description html-devel
The module implemented by HTML specification.
Includes functions for parsing HTML, build DOM tree and DOM/HTML serialization.

%files html-devel
%defattr(-,root,root,-)
%{_libdir}/*lexbor-html_static.a
%{_includedir}/lexbor/html/*

%package ns
Version:  0.2.0
Release:  1%{?dist}
Epoch:    1
Summary:  Shared "ns" library from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core = %{epoch}:0.3.1-%{release}

%description ns
DOM/HTML namespace module. It is helper module for parsing HTML.

%files ns
%defattr(-,root,root,-)
%{_libdir}/liblexbor-ns*
%exclude %{_libdir}/*.a

%package ns-devel
Version:  0.2.0
Release:  1%{?dist}
Epoch:    1
Summary:  Static "ns" library and headers from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core-devel = %{epoch}:0.3.1-%{release}

%description ns-devel
DOM/HTML namespace module. It is helper module for parsing HTML.

%files ns-devel
%defattr(-,root,root,-)
%{_libdir}/*lexbor-ns_static.a
%{_includedir}/lexbor/ns/*

%package tag
Version:  0.2.0
Release:  1%{?dist}
Epoch:    1
Summary:  Shared "tag" library from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core = %{epoch}:0.3.1-%{release}

%description tag
DOM/HTML tags module. It is helper module for parsing HTML.

%files tag
%defattr(-,root,root,-)
%{_libdir}/liblexbor-tag*
%exclude %{_libdir}/*.a

%package tag-devel
Version:  0.2.0
Release:  1%{?dist}
Epoch:    1
Summary:  Static "tag" library and headers from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core-devel = %{epoch}:0.3.1-%{release}

%description tag-devel
DOM/HTML tags module. It is helper module for parsing HTML.

%files tag-devel
%defattr(-,root,root,-)
%{_libdir}/*lexbor-tag_static.a
%{_includedir}/lexbor/tag/*

%package utils
Version:  0.2.1
Release:  1%{?dist}
Epoch:    1
Summary:  Shared "utils" library from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core = %{epoch}:0.3.1-%{release}

%description utils
The module contains helpers functions, such as: WARC and HTTP parsing.

%files utils
%defattr(-,root,root,-)
%{_libdir}/liblexbor-utils*
%exclude %{_libdir}/*.a

%package utils-devel
Version:  0.2.1
Release:  1%{?dist}
Epoch:    1
Summary:  Static "utils" library and headers from the Lexbor project.
License:  Apache 2.0
URL:      https://github.com/lexbor/lexbor
Group:    Development/Tools
Requires: liblexbor-core-devel = %{epoch}:0.3.1-%{release}

%description utils-devel
The module contains helpers functions, such as: WARC and HTTP parsing.

%files utils-devel
%defattr(-,root,root,-)
%{_libdir}/*lexbor-utils_static.a
%{_includedir}/lexbor/utils/*
# End of modules

%changelog
* Mon Apr 01 2019 Please, see changelog at https://github.com/lexbor/lexbor/blob/master/CHANGELOG.md - 0.2.0-1
- release