/*
 * Copyright (C) 2013-2026 Jolla Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
