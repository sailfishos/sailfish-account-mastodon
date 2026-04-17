/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MASTODONNOTIFICATIONSDATATYPESYNCADAPTOR_H
#define MASTODONNOTIFICATIONSDATATYPESYNCADAPTOR_H

#include "socialnetworksyncadaptor.h"

#include <QtCore/QMap>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>

namespace Accounts {
    class Account;
}
namespace SignOn {
    class Error;
    class SessionData;
}

class MastodonNotificationsDataTypeSyncAdaptor : public SocialNetworkSyncAdaptor
{
    Q_OBJECT

public:
    MastodonNotificationsDataTypeSyncAdaptor(SocialNetworkSyncAdaptor::DataType dataType, QObject *parent);
    virtual ~MastodonNotificationsDataTypeSyncAdaptor();

    void sync(const QString &dataTypeString, int accountId) override;

protected:
    QString apiHost(int accountId) const;
    virtual void updateDataForAccount(int accountId);
    virtual QString authServiceName() const;
    virtual void beginSync(int accountId, const QString &accessToken) = 0;

protected Q_SLOTS:
    virtual void errorHandler(QNetworkReply::NetworkError err);
    virtual void sslErrorsHandler(const QList<QSslError> &errs);

private Q_SLOTS:
    void signOnError(const SignOn::Error &error);
    void signOnResponse(const SignOn::SessionData &responseData);

private:
    void setCredentialsNeedUpdate(Accounts::Account *account);
    void signIn(Accounts::Account *account);

private:
    QMap<int, QString> m_apiHosts;
};

#endif // MASTODONNOTIFICATIONSDATATYPESYNCADAPTOR_H
