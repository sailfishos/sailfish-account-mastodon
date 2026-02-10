#include "mastodonshareservicestatus.h"

#include <Accounts/Account>
#include <Accounts/AccountService>
#include <Accounts/Manager>
#include <Accounts/Service>

#include <SignOn/AuthSession>
#include <SignOn/Error>
#include <SignOn/Identity>
#include <SignOn/SessionData>

#include <QtCore/QUrl>
#include <QtCore/QVariantMap>
#include <QtDebug>

MastodonShareServiceStatus::MastodonShareServiceStatus(QObject *parent)
    : QObject(parent)
    , m_auth(new AccountAuthenticator(this))
    , m_accountManager(new Accounts::Manager(this))
    , m_serviceName(QStringLiteral("mastodon-sharing"))
{
}

QString MastodonShareServiceStatus::normalizeApiHost(const QString &rawHost)
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

    QString normalized = QString::fromLatin1(url.toEncoded(QUrl::RemovePath
                                                            | QUrl::RemoveQuery
                                                            | QUrl::RemoveFragment));
    if (normalized.endsWith(QLatin1Char('/'))) {
        normalized.chop(1);
    }

    return normalized;
}

void MastodonShareServiceStatus::signIn(int accountId)
{
    Accounts::Account *account = Accounts::Account::fromId(m_accountManager, accountId, this);
    if (!account) {
        qWarning() << Q_FUNC_INFO << "Failed to retrieve account for id:" << accountId;
        setAccountDetailsState(accountId, Error);
        return;
    }

    const Accounts::Service service(m_accountManager->service(m_serviceName));
    if (!service.isValid()) {
        qWarning() << Q_FUNC_INFO << "Invalid auth service" << m_serviceName;
        account->deleteLater();
        setAccountDetailsState(accountId, Error);
        return;
    }

    account->selectService(service);

    SignOn::Identity *identity = account->credentialsId() > 0
            ? SignOn::Identity::existingIdentity(account->credentialsId())
            : 0;
    if (!identity) {
        qWarning() << Q_FUNC_INFO << "account" << accountId << "has no valid credentials";
        account->deleteLater();
        setAccountDetailsState(accountId, Error);
        return;
    }

    Accounts::AccountService accountService(account, service);
    const QString method = accountService.authData().method();
    const QString mechanism = accountService.authData().mechanism();

    SignOn::AuthSession *session = identity->createSession(method);
    if (!session) {
        qWarning() << Q_FUNC_INFO << "could not create signon session for account" << accountId;
        identity->deleteLater();
        account->deleteLater();
        setAccountDetailsState(accountId, Error);
        return;
    }

    QVariantMap signonSessionData = accountService.authData().parameters();

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
    signonSessionData.insert(QStringLiteral("Host"), configuredHost);

    const QString authPath = account->value(QStringLiteral("auth/oauth2/web_server/AuthPath")).toString().trimmed();
    if (!authPath.isEmpty()) {
        signonSessionData.insert(QStringLiteral("AuthPath"), authPath);
    }

    const QString tokenPath = account->value(QStringLiteral("auth/oauth2/web_server/TokenPath")).toString().trimmed();
    if (!tokenPath.isEmpty()) {
        signonSessionData.insert(QStringLiteral("TokenPath"), tokenPath);
    }

    const QString responseType = account->value(QStringLiteral("auth/oauth2/web_server/ResponseType")).toString().trimmed();
    if (!responseType.isEmpty()) {
        signonSessionData.insert(QStringLiteral("ResponseType"), responseType);
    }

    const QString redirectUri = account->value(QStringLiteral("auth/oauth2/web_server/RedirectUri")).toString().trimmed();
    if (!redirectUri.isEmpty()) {
        signonSessionData.insert(QStringLiteral("RedirectUri"), redirectUri);
    }

    const QVariant scopeValue = account->value(QStringLiteral("auth/oauth2/web_server/Scope"));
    if (scopeValue.isValid()) {
        signonSessionData.insert(QStringLiteral("Scope"), scopeValue);
    }

    const QString clientId = account->value(QStringLiteral("auth/oauth2/web_server/ClientId")).toString().trimmed();
    if (!clientId.isEmpty()) {
        signonSessionData.insert(QStringLiteral("ClientId"), clientId);
    }

    const QString clientSecret = account->value(QStringLiteral("auth/oauth2/web_server/ClientSecret")).toString().trimmed();
    if (!clientSecret.isEmpty()) {
        signonSessionData.insert(QStringLiteral("ClientSecret"), clientSecret);
    }

    signonSessionData.insert(QStringLiteral("UiPolicy"), SignOn::NoUserInteractionPolicy);

    connect(session, SIGNAL(response(SignOn::SessionData)),
            this, SLOT(signOnResponse(SignOn::SessionData)),
            Qt::UniqueConnection);
    connect(session, SIGNAL(error(SignOn::Error)),
            this, SLOT(signOnError(SignOn::Error)),
            Qt::UniqueConnection);

    session->setProperty("account", QVariant::fromValue<Accounts::Account *>(account));
    session->setProperty("identity", QVariant::fromValue<SignOn::Identity *>(identity));
    session->process(SignOn::SessionData(signonSessionData), mechanism);
}

void MastodonShareServiceStatus::signOnResponse(const SignOn::SessionData &responseData)
{
    QVariantMap data;
    Q_FOREACH (const QString &key, responseData.propertyNames()) {
        data.insert(key, responseData.getProperty(key));
    }

    SignOn::AuthSession *session = qobject_cast<SignOn::AuthSession *>(sender());
    Accounts::Account *account = session->property("account").value<Accounts::Account *>();
    SignOn::Identity *identity = session->property("identity").value<SignOn::Identity *>();
    const int accountId = account ? account->id() : 0;

    QString accessToken = data.value(QLatin1String("AccessToken")).toString().trimmed();
    if (accessToken.isEmpty()) {
        accessToken = data.value(QLatin1String("access_token")).toString().trimmed();
    }

    if (accountId > 0 && m_accountIdToDetailsIdx.contains(accountId)) {
        AccountDetails &accountDetails(m_accountDetails[m_accountIdToDetailsIdx[accountId]]);
        accountDetails.accessToken = accessToken;
        setAccountDetailsState(accountId, accessToken.isEmpty() ? Error : Populated);
    }

    session->disconnect(this);
    if (identity) {
        identity->destroySession(session);
        identity->deleteLater();
    }
    if (account) {
        account->deleteLater();
    }
}

void MastodonShareServiceStatus::signOnError(const SignOn::Error &error)
{
    SignOn::AuthSession *session = qobject_cast<SignOn::AuthSession *>(sender());
    Accounts::Account *account = session->property("account").value<Accounts::Account *>();
    SignOn::Identity *identity = session->property("identity").value<SignOn::Identity *>();
    const int accountId = account ? account->id() : 0;

    qWarning() << Q_FUNC_INFO << "failed to retrieve credentials for account" << accountId
               << error.type() << error.message();

    if (accountId > 0 && error.type() == SignOn::Error::UserInteraction) {
        setCredentialsNeedUpdate(accountId, m_serviceName);
    }

    session->disconnect(this);
    if (identity) {
        identity->destroySession(session);
        identity->deleteLater();
    }
    if (account) {
        account->deleteLater();
    }

    if (accountId > 0) {
        setAccountDetailsState(accountId, Error);
    }
}

void MastodonShareServiceStatus::setAccountDetailsState(int accountId, AccountDetailsState state)
{
    if (!m_accountIdToDetailsIdx.contains(accountId)) {
        return;
    }

    m_accountDetailsState[accountId] = state;

    bool anyWaiting = false;
    bool anyPopulated = false;
    Q_FOREACH (int id, m_accountDetailsState.keys()) {
        AccountDetailsState accountState = m_accountDetailsState.value(id, Waiting);
        if (accountState == Waiting) {
            anyWaiting = true;
        } else if (accountState == Populated) {
            anyPopulated = true;
        }
    }

    if (!anyWaiting) {
        if (anyPopulated) {
            emit serviceReady();
        } else {
            emit serviceError(QStringLiteral("Unable to retrieve Mastodon account credentials"));
        }
    }
}

int MastodonShareServiceStatus::count() const
{
    return m_accountDetails.count();
}

bool MastodonShareServiceStatus::setCredentialsNeedUpdate(int accountId, const QString &serviceName)
{
    return m_auth->setCredentialsNeedUpdate(accountId, serviceName);
}

void MastodonShareServiceStatus::queryStatus(QueryStatusMode mode)
{
    m_accountDetails.clear();
    m_accountIdToDetailsIdx.clear();
    m_accountDetailsState.clear();

    bool signInActive = false;
    Q_FOREACH (Accounts::AccountId id, m_accountManager->accountList()) {
        Accounts::Account *acc = m_accountManager->account(id);

        if (!acc) {
            qWarning() << Q_FUNC_INFO << "Failed to get account for id:" << id;
            continue;
        }

        acc->selectService(Accounts::Service());

        const Accounts::Service service(m_accountManager->service(m_serviceName));
        const Accounts::ServiceList services = acc->services();
        bool serviceFound = false;
        Q_FOREACH (const Accounts::Service &s, services) {
            if (s.name() == m_serviceName) {
                serviceFound = true;
                break;
            }
        }

        if (acc->enabled() && service.isValid() && serviceFound) {
            if (acc->value(QStringLiteral("CredentialsNeedUpdate")).toBool()) {
                qWarning() << Q_FUNC_INFO << "Credentials need update for account id:" << id;
                continue;
            }

            acc->selectService(service);
            if (acc->value(QStringLiteral("CredentialsNeedUpdate")).toBool()) {
                qWarning() << Q_FUNC_INFO << "Credentials need update for account id:" << id;
                acc->selectService(Accounts::Service());
                continue;
            }

            if (!m_accountIdToDetailsIdx.contains(id)) {
                AccountDetails details;
                details.accountId = id;
                details.apiHost = normalizeApiHost(acc->value(QStringLiteral("api/Host")).toString());

                QUrl apiUrl(details.apiHost);
                details.providerName = apiUrl.host();
                if (details.providerName.isEmpty()) {
                    details.providerName = details.apiHost;
                    if (details.providerName.startsWith(QLatin1String("https://"))) {
                        details.providerName.remove(0, 8);
                    } else if (details.providerName.startsWith(QLatin1String("http://"))) {
                        details.providerName.remove(0, 7);
                    }
                    const int separator = details.providerName.indexOf(QLatin1Char('/'));
                    if (separator > 0) {
                        details.providerName.truncate(separator);
                    }
                }

                details.displayName = acc->displayName();

                m_accountIdToDetailsIdx.insert(id, m_accountDetails.size());
                m_accountDetails.append(details);
            }

            if (mode == SignInMode) {
                signInActive = true;
                m_accountDetailsState.insert(id, Waiting);
                signIn(id);
            }

            acc->selectService(Accounts::Service());
        }
    }

    if (!signInActive) {
        emit serviceReady();
    }
}

MastodonShareServiceStatus::AccountDetails MastodonShareServiceStatus::details(int index) const
{
    if (index < 0 || index >= m_accountDetails.size()) {
        qWarning() << Q_FUNC_INFO << "Index out of range";
        return AccountDetails();
    }

    return m_accountDetails.at(index);
}

MastodonShareServiceStatus::AccountDetails MastodonShareServiceStatus::detailsByIdentifier(int accountIdentifier) const
{
    if (!m_accountIdToDetailsIdx.contains(accountIdentifier)) {
        qWarning() << Q_FUNC_INFO << "No details known for account with identifier" << accountIdentifier;
        return AccountDetails();
    }

    return m_accountDetails[m_accountIdToDetailsIdx[accountIdentifier]];
}
