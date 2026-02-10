# sailfish-account-mastodon

Sailfish OS account integration for Mastodon.

## What This Repository Contains

This repository is the initial public form of the Mastodon plugin and contains all of its account integration components.

### `common/`
- Shared C++ library code used by multiple plugins.
- Includes Mastodon posts database support built on `socialcache`.

### `settings/`
- Sailfish Accounts provider and services definitions.
- Account UI QML files for account creation/settings/credentials update.
- Uses OAuth2 (`web_server`) account flow with per-instance Mastodon app registration.
- Current services:
  - `mastodon-microblog` (sync posts)
  - `mastodon-sharing` (Transfer Engine sharing)

### `buteo-plugins/`
- Social sync plugins for Buteo.
- Includes shared Buteo social plugin framework code and Mastodon posts sync plugin/profile files.
- Installs Buteo client and sync profile XML files.

### `eventsview-plugins/`
- Events view extension QML/C++ plugin for Mastodon posts.
- Includes delegate/feed item QML and `MastodonPostsModel`.

### `transferengine-plugins/`
- Transfer Engine integration for Mastodon image sharing.
- `mastodonshareplugin/`: sharing method discovery and share UI metadata.
- `mastodontransferplugin/`: upload implementation (upload media + create status).
- Shared account credential/status helper: `mastodonshareservicestatus.*`.

### `icons/`
- Mastodon SVG icon assets and Sailfish icon conversion setup (`sailfish-svg2png`).
- Provides themed service icons used by provider/settings/transfer UI.

### `rpm/`
- RPM spec for packaging all components.
- Defines subpackages:
  - `sailfish-account-mastodon`
  - `buteo-sync-plugin-mastodon-posts`
  - `eventsview-extensions-mastodon`
  - `transferengine-plugin-mastodon`
  - `sailfish-account-mastodon-features-all`

### Root project file
- `sailfish-account-mastodon.pro` ties all subprojects together.

## Build Requirements

This project is designed for the Sailfish OS build environment.

A full build is not possible without Sailfish SDK access (including target sysroot/tooling and Sailfish-specific development packages).

In particular, dependencies like these must come from the Sailfish SDK target environment:
- `buteosyncfw5`
- `socialcache`
- `sailfishaccounts`
- `nemotransferengine-qt5`
- other Sailfish/Qt account stack packages listed in `rpm/sailfish-account-mastodon.spec`

## Typical Build Flow (Inside Sailfish SDK)

1. Enter Sailfish SDK shell/target.
2. Run qmake build from repository root.
3. Build RPM package(s) from `rpm/sailfish-account-mastodon.spec`.

If you are outside Sailfish SDK, use static checks and code review only.
