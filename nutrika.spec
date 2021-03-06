Name: nutrika
Version: 1.0.1
Release: 1%{dist}
Summary: Nutrient calculator and analyzer
Group: Applications/Engineering and Scientific
License: GPLv3+
URL: https://sites.google.com/site/projectnutrika/home
Source: %{name}-%{version}.tar.lz
BuildRequires: gtk3-devel,sqlite-devel,gettext,desktop-file-utils

%description
Shows detailed nutrition content (per 100g and DRI percentages) of various 
food items. Values of food items are also summed up for custom made products, 
and for a plan of a certain period.

%define _source_payload w9.lzdio
%define _binary_payload w9.lzdio

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
%find_lang %{name}
desktop-file-validate %{buildroot}/%{_datadir}/applications/nutrika.desktop
gzip %{buildroot}/%{_mandir}/man1/nutrika.1

%files -f %{name}.lang
%defattr(-,root,root,-)
%doc README ChangeLog AUTHORS
%{_bindir}/*
%{_datadir}/applications/nutrika.desktop
%{_datadir}/nutrika
%{_datadir}/pixmaps/nutrika.png
%{_mandir}/man1/nutrika.1.gz

%changelog
* Thu Jul 25 2013 Peter Sulyok <sulyokpeti@gmail.com> 1.0.1
- fixed price storing bug
* Wed Jun 19 2013 Peter Sulyok <sulyokpeti@gmail.com> 1.0
- schedule turned into plan, introduced P/V ratio
* Tue Mar 15 2011 Peter Sulyok <sulyokpeti@gmail.com> 0.2.1-2
- fixed spec file, added man page
* Fri Mar 04 2011 Peter Sulyok <sulyokpeti@gmail.com> 0.2.1-1
- bugfix: weight did not change before setting weight unit
* Mon Feb 14 2011 Peter Sulyok <sulyokpeti@gmail.com> 0.2-1
- I18N, added lb and oz units, icon
* Mon Feb 14 2011 Peter Sulyok <sulyokpeti@gmail.com>
- New release 0.1

