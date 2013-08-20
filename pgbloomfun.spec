Name:           pgbloomfun
Version:        %{major_version}
Release:        %{minor_version}%{?dist}
Summary:        PostgreSQL bloom filter functions

Group:          Applications/Databases
License:        MIT
Source0:        pgbloomfun-rpm-src.tar.gz
Requires:       postgresql-server

%description
pgbloomfun is an implementation of scalable bloom filter functions for
PostgreSQL.  The implementation automatically scales the filter as more data
is added to it.

%prep
%setup -q -n %{name}

%build
make

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc README LICENSE
%{_libdir}/pgsql/pgbloomfun.so
%{_datadir}/pgsql/*/pgbloomfun*

%changelog
* Wed Aug 21 2013 Oskari Saarenmaa <os@ohmu.fi> - 0.1-1.g1c757cf
- Initial.
