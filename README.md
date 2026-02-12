# sailfish-account-mastodon

Sailfish OS account integration for Mastodon.

## Repository Components

### `common/`
- Shared C++ library code used by multiple plugins.
- Includes socialcache-backed storage for Mastodon posts and shared Mastodon auth helpers.

### `settings/`
- Sailfish Accounts provider, service definitions, and account UI.
- OAuth2 (`web_server`) account flow with per-instance Mastodon app registration.
- Translations:
  - QML translation-loader module at `/usr/lib*/qt5/qml/com/jolla/settings/accounts/mastodon/` loads `settings-accounts-mastodon` catalogs for `qsTrId` strings.
  - Engineering English catalog: `/usr/share/translations/settings-accounts-mastodon_eng_en.qm`
  - Translation source catalog: `/usr/share/translations/source/settings-accounts-mastodon.ts`
  - Provider/service metadata uses `<translations>/usr/share/translations/settings-accounts-mastodon</translations>` for metadata string translation paths.
- Services:
  - `mastodon-microblog`: sync service for posts and notifications.
  - `mastodon-sharing`: Transfer Engine sharing service.

### `buteo-plugins/`
- Buteo sync plugins and shared social sync framework code.
- Includes:
  - `buteo-sync-plugin-mastodon-posts`
  - `buteo-sync-plugin-mastodon-notifications`
- Installs Buteo client profile and sync profile XML files.

### `eventsview-plugins/`
- Events view extension for Mastodon posts.
- Includes delegate/feed item QML and `MastodonPostsModel`.

### `transferengine-plugins/`
- Transfer Engine integration for Mastodon sharing.
- `mastodonshareplugin/`: sharing method discovery + metadata.
- `mastodontransferplugin/`: media upload + status creation.
- Single share UI entry: `MastodonSharePost.qml` handles both media and text/link posting.
- Supports:
  - image sharing (`image/jpeg`, `image/png`)
  - link/text sharing (`text/x-url`, `text/plain`) with title/link extraction from share resources.

### `icons/`
- Mastodon SVG assets and `sailfish-svg2png` conversion setup.
- Uses canonical icon names only:
  - `icons/icon-l-mastodon`
### `rpm/`
- Packaging for all modules in `rpm/sailfish-account-mastodon.spec`.
- Packages:
  - `sailfish-account-mastodon` (all runtime components)
  - `sailfish-account-mastodon-ts-devel` (translation source files only)
- `%qmake5_install` already installs icon outputs from the `icons/` subproject; avoid a second explicit `icons` `make install` in `%install`.
- Translation source `.ts` files are packaged in `sailfish-account-mastodon-ts-devel` (runtime package ships `.qm` only).
- Runtime package ships the Mastodon settings translation-loader QML plugin under `%{_libdir}/qt5/qml/com/jolla/settings/accounts/mastodon/`.

### Root project
- `sailfish-account-mastodon.pro` ties subprojects together.

## Current Notification Behavior

- Events view shows Mastodon posts (not notification entries).
- Events view post metadata line includes replies, favourites, and boosts alongside elapsed timestamp.
- Long-pressing a Mastodon post reveals quick actions for favourite and boost, calling Mastodon API endpoints directly with account OAuth credentials.
- System notifications are produced by `buteo-sync-plugin-mastodon-notifications`.
- Notifications sync starts from Mastodon server marker (`notifications.last_read_id`) and uses local cursor dedupe via per-account `LastFetchedNotificationId`.
- Each unread Mastodon notification is published as a separate Sailfish system notification.
- Mastodon marker (`last_read_id`) is updated only when no local Mastodon notifications remain for that account.
- Notification template profile dispatches per-account sync profiles on schedule (default every 30 minutes), not only at boot.

## Build Requirements

This project targets Sailfish OS build tooling.

Full build/package validation is not possible without Sailfish SDK access (target sysroot + Sailfish packages).

Required SDK-provided dependencies include (not exhaustive):
- `buteosyncfw5`
- `socialcache`
- `sailfishaccounts`
- `nemotransferengine-qt5`
- related Qt/account stack packages listed in `rpm/sailfish-account-mastodon.spec`

## Typical Build Flow (Inside Sailfish SDK)

1. Enter Sailfish SDK shell/target.
2. Build from repository root (`qmake` / `make`).
3. Build RPM package(s) from `rpm/sailfish-account-mastodon.spec`.

Outside Sailfish SDK, only static validation (wiring, paths, spec consistency) should be considered reliable.
