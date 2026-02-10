TEMPLATE = subdirs
SUBDIRS += \
    common \
    settings \
    transferengine-plugins \
    buteo-plugins \
    eventsview-plugins \
    icons

buteo-plugins.depends = common
transferengine-plugins.depends = common
eventsview-plugins.depends = common

OTHER_FILES += rpm/sailfish-account-mastodon.spec
