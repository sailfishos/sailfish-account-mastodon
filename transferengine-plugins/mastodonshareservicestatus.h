/*
 * Copyright (C) 2013-2026 Jolla Ltd.
 */

#ifndef MASTODONSHARESERVICESTATUS_H
#define MASTODONSHARESERVICESTATUS_H

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QVector>

#include <accountauthenticator.h>

namespace Accounts {
class Account;
class Manager;
}

namespace SignOn {
class Error;
class SessionData;
}

class MastodonShareServiceStatus : public QObject
{
    Q_OBJECT

public:
    explicit MastodonShareServiceStatus(QObject *parent = 0);

    enum QueryStatusMode {
        PassiveMode = 0,
        SignInMode = 1
    };

    void queryStatus(QueryStatusMode mode = SignInMode);

    struct AccountDetails {
        int accountId = 0;
        QString providerName;
        QString displayName;
        QString accessToken;
        QString apiHost;
    };

    AccountDetails details(int index = 0) const;
    AccountDetails detailsByIdentifier(int accountIdentifier) const;
    int count() const;

    bool setCredentialsNeedUpdate(int accountId, const QString &serviceName);

Q_SIGNALS:
    void serviceReady();
    void serviceError(const QString &message);

private Q_SLOTS:
    void signOnResponse(const SignOn::SessionData &responseData);
    void signOnError(const SignOn::Error &error);

private:
    enum AccountDetailsState {
        Waiting,
        Populated,
        Error
    };

    void setAccountDetailsState(int accountId, AccountDetailsState state);
    void signIn(int accountId);

    AccountAuthenticator *m_auth;
    Accounts::Manager *m_accountManager;
    QString m_serviceName;
    QVector<AccountDetails> m_accountDetails;
    QHash<int, int> m_accountIdToDetailsIdx;
    QHash<int, AccountDetailsState> m_accountDetailsState;
};

#endif // MASTODONSHARESERVICESTATUS_H
