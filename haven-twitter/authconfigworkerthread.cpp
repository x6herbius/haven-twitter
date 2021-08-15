#include <QtDebug>
#include "authconfigworkerthread.h"

AuthConfigWorkerThread::AuthConfigWorkerThread(QObject* parent) :
    QThread(parent)
{
}

AuthConfigWorkerThread::~AuthConfigWorkerThread()
{
}

void AuthConfigWorkerThread::setAPIKey(const QString& key)
{
    m_ApiKey = key;
}

void AuthConfigWorkerThread::setAPISecret(const QString& secret)
{
    m_ApiSecret = secret;
}

void AuthConfigWorkerThread::setAccessToken(const QString& token)
{
    m_AccessToken = token;
}

void AuthConfigWorkerThread::setAccessTokenSecret(const QString& secret)
{
    m_AccessTokenSecret = secret;
}

void AuthConfigWorkerThread::setUsername(const QString& username)
{
    m_Username = username;
}

void AuthConfigWorkerThread::setPassword(const QString& password)
{
    m_Password = password;
}

void AuthConfigWorkerThread::run()
{
    m_ReceivedNewAccessToken = false;

    qDebug() << "AuthConfigWorkerThread: Setting credentials.";
    tcObj.setTwitterUsername(m_Username.toStdString());
    tcObj.setTwitterPassword(m_Password.toStdString());
    tcObj.getOAuth().setConsumerKey(m_ApiKey.toStdString());
    tcObj.getOAuth().setConsumerSecret(m_ApiSecret.toStdString());

    if ( m_AccessToken.isEmpty() || m_AccessTokenSecret.isEmpty() )
    {
        qDebug() << "AuthConfigWorkerThread: Requesting new access token credentials.";

        m_ReceivedNewAccessToken = true;

        std::string authURL;
        tcObj.oAuthRequestToken(authURL);
        tcObj.oAuthHandlePIN(authURL);
    }
    else
    {
        qDebug() << "AuthConfigWorkerThread: Sccess token credentials were provided, not re-requesting.";

        // Set from details that we have.
        tcObj.getOAuth().setOAuthTokenKey(m_AccessToken.toStdString());
        tcObj.getOAuth().setOAuthTokenSecret(m_AccessTokenSecret.toStdString());
    }

    qDebug() << "AuthConfigWorkerThread: Performing OAuth authentication.";
    tcObj.oAuthAccessToken();

    if( !tcObj.accountVerifyCredGet() )
    {
        std::string replyMsg;
        tcObj.getLastCurlError(replyMsg);

        qWarning() << "AuthConfigWorkerThread: Failed to authenticate with Twitter. Response:" << replyMsg.c_str();
        return;
    }

    qDebug() << "AuthConfigWorkerThread: OAuth authentication succeeded.";

    if ( m_ReceivedNewAccessToken )
    {
        qDebug() << "AuthConfigWorkerThread: Received new access token credentials.";

        std::string accessToken;
        tcObj.getOAuth().getOAuthTokenKey(accessToken);

        std::string accessTokenSecret;
        tcObj.getOAuth().getOAuthTokenSecret(accessTokenSecret);

        m_AccessToken = QString(accessToken.c_str());
        m_AccessTokenSecret = QString(accessTokenSecret.c_str());
        emit newAccessTokenAcquired(m_AccessToken, m_AccessTokenSecret);
    }
}