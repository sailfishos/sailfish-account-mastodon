/****************************************************************************
 **
 ** Copyright (C) 2026 Open Mobile Platform LLC.
 **
 ** This program/library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public License
 ** version 2.1 as published by the Free Software Foundation.
 **
 ** This program/library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program/library; if not, write to the Free
 ** Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 ** 02110-1301 USA
 **
 ****************************************************************************/

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
    static QString normalizeApiHost(const QString &rawHost);
    void setCredentialsNeedUpdate(Accounts::Account *account);
    void signIn(Accounts::Account *account);

private:
    QMap<int, QString> m_apiHosts;
};

#endif // MASTODONDATATYPESYNCADAPTOR_H
