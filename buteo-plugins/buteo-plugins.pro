TEMPLATE = subdirs
SUBDIRS += \
    buteo-common \
    buteo-sync-plugin-mastodon-posts

buteo-sync-plugin-mastodon-posts.depends = buteo-common
