/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MASTODONDATATYPESYNCADAPTOR_H
#define MASTODONDATATYPESYNCADAPTOR_H

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

class MastodonDataTypeSyncAdaptor : public SocialNetworkSyncAdaptor
{
    Q_OBJECT

public:
    MastodonDataTypeSyncAdaptor(SocialNetworkSyncAdaptor::DataType dataType, QObject *parent);
    virtual ~MastodonDataTypeSyncAdaptor();

    void sync(const QString &dataTypeString, int accountId) override;

protected:
    QString apiHost(int accountId) const;
    virtual void updateDataForAccount(int accountId);
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

#endif // MASTODONDATATYPESYNCADAPTOR_H
