Name:       harbour-seaprint

Summary:    SeaPrint
Version:    0
Release:    1
Group:      Qt/Qt
License:    GPLv3
URL:        https://github.com/attah/harbour-seaprint
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  qt5-qttools-linguist
BuildRequires:  desktop-file-utils
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(poppler)
BuildRequires:  pkgconfig(poppler-glib)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(libjpeg)

%define _source_payload w6T.xzdio
%define _binary_payload w6T.xzdio

%description
Network printing for Sailfish OS

%prep
%setup -q -n %{name}-%{version}

%build

%qmake5  \
    VERSION=%{version}

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
