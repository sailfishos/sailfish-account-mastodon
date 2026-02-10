# sailfish-account-mastodon

Sailfish OS account integration for Mastodon.

## Repository Components

### `common/`
- Shared C++ library code used by multiple plugins.
- Includes socialcache-backed databases for Mastodon posts and notifications.

### `settings/`
- Sailfish Accounts provider, service definitions, and account UI.
- OAuth2 (`web_server`) account flow with per-instance Mastodon app registration.
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
- Transfer Engine integration for Mastodon image sharing.
- `mastodonshareplugin/`: sharing method discovery + metadata.
- `mastodontransferplugin/`: media upload + status creation.

### `icons/`
- Mastodon SVG assets and `sailfish-svg2png` conversion setup.
- Uses canonical icon names only:
  - `icons/icon-l-mastodon`
### `rpm/`
- Packaging for all modules in `rpm/sailfish-account-mastodon.spec`.
- Subpackages:
  - `sailfish-account-mastodon`
  - `buteo-sync-plugin-mastodon-posts`
  - `buteo-sync-plugin-mastodon-notifications`
  - `eventsview-extensions-mastodon`
  - `transferengine-plugin-mastodon`
- Main package requires all feature subpackages.

### Root project
- `sailfish-account-mastodon.pro` ties subprojects together.

## Current Notification Behavior

- Events view shows Mastodon posts (not notification entries).
- System notifications are produced by `buteo-sync-plugin-mastodon-notifications`.
- Notifications sync fetches unread items using Mastodon markers (`last_read_id`).
- Dismissing the Sailfish notification marks those items as read on Mastodon via markers API.
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
