Name: sailfish-account-mastodon
License: LGPLv2+
Version: 0.1.0
Release: 1
Source0: %{name}-%{version}.tar.bz2
Summary: Account plugin for Mastodon
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
Requires(post): %{_libexecdir}/manage-groups
Requires(postun): %{_libexecdir}/manage-groups

%description
%{summary}.

%package -n buteo-sync-plugin-mastodon-posts
Summary: Provides synchronisation of Mastodon posts
Requires: %{name} = %{version}-%{release}
Requires: buteo-syncfw-qt5-msyncd
Requires: systemd
Requires(post): systemd

%description -n buteo-sync-plugin-mastodon-posts
Provides synchronisation of Mastodon posts.

%package -n eventsview-extensions-mastodon
Summary: Provides integration of Mastodon posts into Events view
Requires: lipstick-jolla-home-qt5-components >= 1.2.50
Requires: eventsview-extensions

%description -n eventsview-extensions-mastodon
Provides integration of Mastodon posts into Events view.

%package -n transferengine-plugin-mastodon
Summary: Mastodon image sharing plugin for Transfer Engine
Requires: sailfishsilica-qt5 >= 1.1.108
Requires: declarative-transferengine-qt5 >= 0.3.13
Requires: nemo-transferengine-qt5 >= 2.0.0
Requires: %{name} = %{version}-%{release}

%description -n transferengine-plugin-mastodon
Mastodon image sharing plugin for Transfer Engine.

%package features-all
Summary: Meta package to include all Mastodon account features
Requires: %{name} = %{version}-%{release}
Requires: buteo-sync-plugin-mastodon-posts
Requires: eventsview-extensions-mastodon
Requires: transferengine-plugin-mastodon

%description features-all
This package includes all Mastodon account features.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 "VERSION=%{version}"
%make_build

%install
%qmake5_install
cd icons
make INSTALL_ROOT=%{buildroot} install
for icon in $(find %{buildroot}%{_datadir}/themes/sailfish-default/silica -type f -name icon-l-mastodon.png); do
    dir=$(dirname "$icon")
    cp -a "$icon" "$dir/graphic-service-mastodon.png"
    cp -a "$icon" "$dir/graphic-m-service-mastodon.png"
    cp -a "$icon" "$dir/graphic-s-service-mastodon.png"
done

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
%{_datadir}/themes/sailfish-default/silica/*/icons/graphic-service-mastodon.png
%{_datadir}/themes/sailfish-default/silica/*/icons/graphic-m-service-mastodon.png
%{_datadir}/themes/sailfish-default/silica/*/icons/graphic-s-service-mastodon.png
%{_datadir}/themes/sailfish-default/silica/*/icons/icon-l-mastodon.png

%files -n buteo-sync-plugin-mastodon-posts
%{_libdir}/buteo-plugins-qt5/oopp/libmastodon-posts-client.so
%config %{_sysconfdir}/buteo/profiles/client/mastodon-posts.xml
%config %{_sysconfdir}/buteo/profiles/sync/mastodon.Posts.xml

%files -n eventsview-extensions-mastodon
%{_libdir}/qt5/qml/com/jolla/eventsview/mastodon/*
%{_datadir}/lipstick/eventfeed/mastodon-delegate.qml
%{_datadir}/lipstick/eventfeed/MastodonFeedItem.qml
%{_datadir}/translations/lipstick-jolla-home-mastodon_eng_en.qm
%{_datadir}/translations/source/lipstick-jolla-home-mastodon.ts

%files -n transferengine-plugin-mastodon
%{_libdir}/nemo-transferengine/plugins/sharing/libmastodonshareplugin.so
%{_libdir}/nemo-transferengine/plugins/transfer/libmastodontransferplugin.so
%{_datadir}/nemo-transferengine/plugins/sharing/MastodonShareImage.qml

%files features-all
# Empty by design.
