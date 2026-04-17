/*
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MASTODONAUTHUTILS_H
#define MASTODONAUTHUTILS_H

#include <QtCore/QVariantMap>
#include <QtCore/QUrl>

#include <Accounts/Account>

#include <SignOn/SessionData>

namespace MastodonAuthUtils {

inline QString normalizeApiHost(const QString &rawHost)
{
    QString host = rawHost.trimmed();
    if (host.isEmpty()) {
        host = QStringLiteral("https://mastodon.social");
    }
    if (!host.startsWith(QLatin1String("https://"))
            && !host.startsWith(QLatin1String("http://"))) {
        host.prepend(QStringLiteral("https://"));
    }

    QUrl url(host);
    if (!url.isValid() || url.host().isEmpty()) {
        return QStringLiteral("https://mastodon.social");
    }

    QString normalized = QString::fromLatin1(url.toEncoded(QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment));
    if (normalized.endsWith(QLatin1Char('/'))) {
        normalized.chop(1);
    }
    return normalized;
}

inline QString signOnHost(Accounts::Account *account)
{
    QString configuredHost = account->value(QStringLiteral("auth/oauth2/web_server/Host")).toString().trimmed();
    if (configuredHost.isEmpty()) {
        configuredHost = normalizeApiHost(account->value(QStringLiteral("api/Host")).toString());
    }

    if (configuredHost.startsWith(QLatin1String("https://"))) {
        configuredHost.remove(0, 8);
    } else if (configuredHost.startsWith(QLatin1String("http://"))) {
        configuredHost.remove(0, 7);
    }

    const int separator = configuredHost.indexOf(QLatin1Char('/'));
    if (separator > -1) {
        configuredHost.truncate(separator);
    }
    while (configuredHost.endsWith(QLatin1Char('/'))) {
        configuredHost.chop(1);
    }
    if (configuredHost.isEmpty()) {
        configuredHost = QStringLiteral("mastodon.social");
    }

    return configuredHost;
}

inline void addSignOnSessionParameters(Accounts::Account *account, QVariantMap *sessionData)
{
    sessionData->insert(QStringLiteral("Host"), signOnHost(account));

    const QString authPath = account->value(QStringLiteral("auth/oauth2/web_server/AuthPath")).toString().trimmed();
    if (!authPath.isEmpty()) {
        sessionData->insert(QStringLiteral("AuthPath"), authPath);
    }

    const QString tokenPath = account->value(QStringLiteral("auth/oauth2/web_server/TokenPath")).toString().trimmed();
    if (!tokenPath.isEmpty()) {
        sessionData->insert(QStringLiteral("TokenPath"), tokenPath);
    }

    const QString responseType = account->value(QStringLiteral("auth/oauth2/web_server/ResponseType")).toString().trimmed();
    if (!responseType.isEmpty()) {
        sessionData->insert(QStringLiteral("ResponseType"), responseType);
    }

    const QString redirectUri = account->value(QStringLiteral("auth/oauth2/web_server/RedirectUri")).toString().trimmed();
    if (!redirectUri.isEmpty()) {
        sessionData->insert(QStringLiteral("RedirectUri"), redirectUri);
    }

    const QVariant scopeValue = account->value(QStringLiteral("auth/oauth2/web_server/Scope"));
    if (scopeValue.isValid()) {
        sessionData->insert(QStringLiteral("Scope"), scopeValue);
    }

    const QString clientId = account->value(QStringLiteral("auth/oauth2/web_server/ClientId")).toString().trimmed();
    if (!clientId.isEmpty()) {
        sessionData->insert(QStringLiteral("ClientId"), clientId);
    }

    const QString clientSecret = account->value(QStringLiteral("auth/oauth2/web_server/ClientSecret")).toString().trimmed();
    if (!clientSecret.isEmpty()) {
        sessionData->insert(QStringLiteral("ClientSecret"), clientSecret);
    }

    sessionData->insert(QStringLiteral("UiPolicy"), SignOn::NoUserInteractionPolicy);
}

inline QString accessToken(const QVariantMap &sessionResponseData)
{
    QString token = sessionResponseData.value(QLatin1String("AccessToken")).toString().trimmed();
    if (token.isEmpty()) {
        token = sessionResponseData.value(QLatin1String("access_token")).toString().trimmed();
    }
    return token;
}

inline QVariantMap responseDataToMap(const SignOn::SessionData &responseData)
{
    QVariantMap data;
    foreach (const QString &key, responseData.propertyNames()) {
        data.insert(key, responseData.getProperty(key));
    }
    return data;
}

} // namespace MastodonAuthUtils

#endif // MASTODONAUTHUTILS_H
