# Copyright (C) 2013-2026 Jolla Ltd.

Name: sailfish-account-mastodon
License: LGPLv3
Version: 1.0.0
Release: 1
Source0: %{name}-%{version}.tar.bz2
Summary: SailfishOS account plugin for Mastodon
BuildRequires: qt5-qmake
BuildRequires: qt5-qttools-linguist
BuildRequires: sailfish-svg2png
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Sql)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(mlite5)
BuildRequires: pkgconfig(buteosyncfw5) >= 0.10.0
BuildRequires: pkgconfig(accounts-qt5)
BuildRequires: pkgconfig(libsignon-qt5)
BuildRequires: pkgconfig(socialcache)
BuildRequires: pkgconfig(libsailfishkeyprovider)
BuildRequires: pkgconfig(sailfishaccounts)
BuildRequires: pkgconfig(nemotransferengine-qt5) >= 2.0.0
BuildRequires:  pkgconfig(nemonotifications-qt5)
Requires: jolla-settings-accounts-extensions-onlinesync
Requires: qmf-oauth2-plugin >= 0.0.7
Requires: buteo-syncfw-qt5-msyncd
Requires: systemd
Requires: lipstick-jolla-home-qt5-components >= 1.2.50
Requires: eventsview-extensions
Requires: sailfishsilica-qt5 >= 1.1.108
Requires: declarative-transferengine-qt5 >= 0.3.13
Requires: nemo-transferengine-qt5 >= 2.0.0
Requires(post): %{_libexecdir}/manage-groups
Requires(postun): %{_libexecdir}/manage-groups

%description
%{summary}. Supports displaying current feed in the Events View, 
sharing images, and notifications.

%package -n sailfish-account-mastodon-ts-devel
Summary: Translation source files for sailfish-account-mastodon
Requires: %{name} = %{version}-%{release}

%description -n sailfish-account-mastodon-ts-devel
Translation source files for sailfish-account-mastodon components.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 "VERSION=%{version}"
%make_build

%install
%qmake5_install

%post
/sbin/ldconfig
%{_libexecdir}/manage-groups add account-mastodon || :

%postun
/sbin/ldconfig
if [ "$1" -eq 0 ]; then
    %{_libexecdir}/manage-groups remove account-mastodon || :
fi

%files
%{_libdir}/libmastodoncommon.so.*
%exclude %{_libdir}/libmastodoncommon.so
%{_libdir}/libmastodonbuteocommon.so.*
%exclude %{_libdir}/libmastodonbuteocommon.so
%{_datadir}/accounts/providers/mastodon.provider
%{_datadir}/accounts/services/mastodon-microblog.service
%{_datadir}/accounts/services/mastodon-sharing.service
%{_datadir}/accounts/ui/MastodonSettingsDisplay.qml
%{_datadir}/accounts/ui/mastodon.qml
%{_datadir}/accounts/ui/mastodon-settings.qml
%{_datadir}/accounts/ui/mastodon-update.qml
%{_libdir}/qt5/qml/com/jolla/settings/accounts/mastodon/*
%{_datadir}/translations/settings-accounts-mastodon_eng_en.qm
%{_datadir}/themes/sailfish-default/silica/*/icons/icon-l-mastodon.png
%{_libdir}/buteo-plugins-qt5/oopp/libmastodon-posts-client.so
%config %{_sysconfdir}/buteo/profiles/client/mastodon-posts.xml
%config %{_sysconfdir}/buteo/profiles/sync/mastodon.Posts.xml
%{_libdir}/buteo-plugins-qt5/oopp/libmastodon-notifications-client.so
%config %{_sysconfdir}/buteo/profiles/client/mastodon-notifications.xml
%config %{_sysconfdir}/buteo/profiles/sync/mastodon.Notifications.xml
%{_libdir}/qt5/qml/com/jolla/eventsview/mastodon/*
%{_datadir}/lipstick/eventfeed/mastodon-delegate.qml
%{_datadir}/lipstick/eventfeed/MastodonFeedItem.qml
%{_datadir}/translations/lipstick-jolla-home-mastodon_eng_en.qm

%{_libdir}/nemo-transferengine/plugins/sharing/libmastodonshareplugin.so
%{_libdir}/nemo-transferengine/plugins/transfer/libmastodontransferplugin.so
%{_datadir}/nemo-transferengine/plugins/sharing/MastodonSharePost.qml

%files -n sailfish-account-mastodon-ts-devel
%{_datadir}/translations/source/settings-accounts-mastodon.ts
%{_datadir}/translations/source/lipstick-jolla-home-mastodon.ts
