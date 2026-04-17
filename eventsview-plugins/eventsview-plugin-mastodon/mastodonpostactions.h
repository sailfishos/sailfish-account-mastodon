/*
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MASTODONPOSTACTIONS_H
#define MASTODONPOSTACTIONS_H

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtNetwork/QNetworkAccessManager>

namespace Accounts {
class Account;
class Manager;
}

namespace SignOn {
class Error;
class SessionData;
}

class MastodonPostActions : public QObject
{
    Q_OBJECT

public:
    explicit MastodonPostActions(QObject *parent = 0);

    Q_INVOKABLE void favourite(int accountId, const QString &statusId);
    Q_INVOKABLE void unfavourite(int accountId, const QString &statusId);
    Q_INVOKABLE void boost(int accountId, const QString &statusId);
    Q_INVOKABLE void unboost(int accountId, const QString &statusId);

Q_SIGNALS:
    void actionSucceeded(int accountId,
                         const QString &statusId,
                         const QString &action,
                         int favouritesCount,
                         int reblogsCount,
                         bool favourited,
                         bool reblogged);
    void actionFailed(int accountId,
                      const QString &statusId,
                      const QString &action,
                      const QString &errorMessage);

private Q_SLOTS:
    void signOnResponse(const SignOn::SessionData &responseData);
    void signOnError(const SignOn::Error &error);
    void actionFinishedHandler();

private:
    void performAction(int accountId, const QString &statusId, const QString &action);
    void executeActionRequest(int accountId,
                              const QString &statusId,
                              const QString &action,
                              const QString &apiHost,
                              const QString &accessToken);
    void releaseSignOnObjects(QObject *sessionObject);
    QString actionKey(int accountId, const QString &statusId, const QString &action) const;

    Accounts::Manager *m_accountManager;
    QNetworkAccessManager m_networkAccessManager;
    QSet<QString> m_pendingActions;
};

#endif // MASTODONPOSTACTIONS_H
