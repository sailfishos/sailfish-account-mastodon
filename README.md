# sailfish-account-mastodon

`sailfish-account-mastodon` adds Mastodon integration to Sailfish OS. It lets
you sign in with a Mastodon account, share to Mastodon from the system share
sheet, and surface Mastodon activity in core Sailfish OS views.

The goal is to make Mastodon feel like a built-in online account instead of a
separate app silo. Once configured, the integration handles authentication,
sharing, post sync, and notification sync through the standard Sailfish OS
account and background sync framework.

## What It Does

- Adds a Mastodon account provider to Sailfish OS Accounts.
- Supports sign-in against Mastodon servers with OAuth.
- Registers an application on the selected server during setup.
- Adds a Mastodon target to the Sailfish OS sharing flow.
- Supports sharing photos, videos, links, and plain text posts.
- Shows Mastodon posts in Events view.
- Delivers Mastodon notifications through Sailfish OS system notifications.
- Provides quick favourite and boost actions for posts shown in Events view.

## User Experience

After adding an account, Mastodon becomes available through the same system
plumbing used by other Sailfish OS online services:

- account setup and credential refresh happen through Sailfish Accounts
- sharing uses Transfer Engine
- background updates use Buteo sync plugins
- posts appear in Events view
- unread Mastodon notifications appear as Sailfish OS notifications

This project is intended for people who want Mastodon support that feels
integrated with the rest of the operating system rather than bolted on top.

## Notes

- Events view currently shows Mastodon posts, not Mastodon notification entries.
- Notification sync tracks unread state using Mastodon markers and local sync
  state.
- The account UI asks for the server you want to use, so it is not limited to a
  single Mastodon instance.

## Licensing

This repository contains a mix of `BSD-3-Clause` and
`LGPL-2.1-or-later` source files. REUSE metadata in the tree records the
license for each file.

The `LGPL-2.1-or-later` parts are the shared sync and cache layer:

- `buteo-plugins/buteo-common/*`
- `buteo-plugins/buteo-sync-plugin-mastodon-posts/*`
- `buteo-plugins/buteo-sync-plugin-mastodon-notifications/*`
- `common/mastodonpostsdatabase.*`
- `eventsview-plugins/eventsview-plugin-mastodon/mastodonpostsmodel.*`

These files are kept under LGPL because they are adapted from existing
Sailfish OS social sync and social cache code, especially the public
`buteo-sync-plugins-social` and `libsocialcache` codebases. The more
Mastodon-specific helper, UI, and integration files are BSD-licensed unless
noted otherwise.

## Third-Party Marks

Mastodon name and logos are trademarks of Mastodon gGmbH. The included
Mastodon icon is used only to identify compatibility or integration with
Mastodon. This project is not affiliated with or endorsed by Mastodon gGmbH.

See:

- `LICENSES/LicenseRef-Mastodon-Trademark-Policy.txt`
- <https://joinmastodon.org/branding>
- <https://joinmastodon.org/trademark>

<!--
SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd

SPDX-License-Identifier: BSD-3-Clause
-->
