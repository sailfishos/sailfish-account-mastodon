// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: BSD-3-Clause

#include <QCoreApplication>
#include <QLocale>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QTranslator>
#include <QtQml>

class AppTranslator : public QTranslator
{
    Q_OBJECT
public:
    explicit AppTranslator(QObject *parent)
        : QTranslator(parent)
    {
        qApp->installTranslator(this);
    }

    ~AppTranslator() override
    {
        qApp->removeTranslator(this);
    }
};

class MastodonAccountsTranslationsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.jolla.settings.accounts.mastodon")

public:
    void initializeEngine(QQmlEngine *engine, const char *uri) override
    {
        Q_UNUSED(uri)

        AppTranslator *engineeringEnglish = new AppTranslator(engine);
        engineeringEnglish->load("settings-accounts-mastodon_eng_en", "/usr/share/translations");

        AppTranslator *translator = new AppTranslator(engine);
        translator->load(QLocale(), "settings-accounts-mastodon", "-", "/usr/share/translations");
    }

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("com.jolla.settings.accounts.mastodon"));
        qmlRegisterUncreatableType<MastodonAccountsTranslationsPlugin>(uri, 1, 0,
                                                                        "MastodonTranslationPlugin",
                                                                        QString());
    }
};

#include "plugin.moc"
