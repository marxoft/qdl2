/**
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "tezfilesplugin.h"
#include "captchatype.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QTime>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

const QRegExp TezfilesPlugin::ERROR_REGEXP("(<h4 class='error'.+>)([^<]+)");
const QRegExp TezfilesPlugin::FILE_REGEXP("(http(s|)://tezfiles\\.com|)/file/url\\.html\\?file=[^'\"]+");
const QRegExp TezfilesPlugin::FILENAME_REGEXP("(<h1 style=\"margin-top: \\d+px;\">)([^<]+)");
const QRegExp TezfilesPlugin::WAITTIME_REGEXP("(<div id=\"download-wait-timer\".+>\\s+)(\\d+)(\\s+</div>)");
const QString TezfilesPlugin::LOGIN_URL("http://tezfiles.com/login.html");
const QString TezfilesPlugin::RECAPTCHA_PLUGIN_ID("qdl2-genericrecaptcha");

const int TezfilesPlugin::MAX_REDIRECTS = 8;

TezfilesPlugin::TezfilesPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_waitTimer(0),
    m_redirects(0),
    m_ownManager(false)
{
}

QString TezfilesPlugin::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));
    
    if (redirect.startsWith("/")) {
        redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
    }
    
    return redirect;
}

QNetworkAccessManager* TezfilesPlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void TezfilesPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
    if (!manager) {
        return;
    }
    
    if ((m_ownManager) && (m_nam)) {
        delete m_nam;
        m_nam = 0;
    }

    m_nam = manager;
    m_ownManager = false;
}

bool TezfilesPlugin::cancelCurrentOperation() {
    stopWaitTimer();
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void TezfilesPlugin::checkUrl(const QString &url, const QVariantMap &) {
    m_redirects = 0;
    QNetworkRequest request(QUrl::fromUserInput(url));
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void TezfilesPlugin::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    const QString redirect = getRedirect(reply);
    
    if (!redirect.isEmpty()) {
        if (FILE_REGEXP.indexIn(redirect) == 0) {
            emit urlChecked(UrlResult(reply->request().url().toString(),
                            redirect.mid(redirect.lastIndexOf("/") + 1)));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkUrlIsValid()));
        }
        else {
            emit error(tr("Maximum redirects reached"));
        }

        reply->deleteLater();
        return;
    }

    switch (reply->error()) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        reply->deleteLater();
        return;
    default:
        emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
        reply->deleteLater();
        return;
    }

    const QString response = QString::fromUtf8(reply->readAll());
    
    if (FILENAME_REGEXP.indexIn(response) != -1) {
        const QString fileName = FILENAME_REGEXP.cap(2).trimmed();
        
        if (!fileName.isEmpty()) {
            emit urlChecked(UrlResult(reply->request().url().toString(), fileName));
        }
        else {
            emit error(tr("File not found"));
        }
    }
    else {
        emit error(tr("File not found"));
    }

    reply->deleteLater();
}

void TezfilesPlugin::getDownloadRequest(const QString &url, const QVariantMap &settings) {
    m_redirects = 0;
    m_url = QUrl::fromUserInput(url);

    if (settings.value("Account/useLogin", false).toBool()) {
        const QString username = settings.value("Account/username").toString();
        const QString password = settings.value("Account/password").toString();

        if ((username.isEmpty()) || (password.isEmpty())) {
            QVariantList list;
            QVariantMap usernameMap;
            usernameMap["type"] = "text";
            usernameMap["label"] = tr("Username");
            usernameMap["key"] = "username";
            list << usernameMap;
            QVariantMap passwordMap;
            passwordMap["type"] = "password";
            passwordMap["label"] = tr("Password");
            passwordMap["key"] = "password";
            list << passwordMap;
            emit settingsRequest(tr("Login"), list, "submitLogin");
        }   
        else {
            login(username, password);
        }

        return;
    }
    
    fetchDownloadRequest(m_url);
}

void TezfilesPlugin::fetchDownloadRequest(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void TezfilesPlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void TezfilesPlugin::checkDownloadRequest() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    const QString redirect = getRedirect(reply);
    
    if (!redirect.isEmpty()) {
        if (FILE_REGEXP.indexIn(redirect) == 0) {
            emit downloadRequest(QNetworkRequest(redirect));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkDownloadRequest()));
        }
        else {
            emit error(tr("Maximum redirects reached"));
        }

        reply->deleteLater();
        return;
    }

    switch (reply->error()) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        reply->deleteLater();
        return;
    default:
        emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
        reply->deleteLater();
        return;
    }

    const QString response = QString::fromUtf8(reply->readAll());

    if (FILE_REGEXP.indexIn(response) != -1) {
        QString url = FILE_REGEXP.cap();
        
        if (url.startsWith("/")) {
            url.prepend(reply->url().scheme() + "://" + reply->url().authority());
        }
        
        emit downloadRequest(QNetworkRequest(url));
    }
    else {
        m_fileId = response.section("slow_id\" value=\"", 1, 1).section('"', 0, 0);
        
        if (m_fileId.isEmpty()) {
            if (ERROR_REGEXP.indexIn(response) != -1) {
                emit error(ERROR_REGEXP.cap(2));
            }
            else {
                emit error(tr("Unknown error"));
            }
        }
        else {
            getWaitTime();
        }
    }
        
    reply->deleteLater();
}

void TezfilesPlugin::getWaitTime() {
    m_redirects = 0;
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = networkAccessManager()->post(request, "yt0=&slow_id=" + m_fileId.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void TezfilesPlugin::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    const QString redirect = getRedirect(reply);
    
    if (!redirect.isEmpty()) {
        if (FILE_REGEXP.indexIn(redirect) == 0) {
            emit downloadRequest(QNetworkRequest(redirect));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkWaitTime()));
        }
        else {
            emit error(tr("Maximum redirects reached"));
        }

        reply->deleteLater();
        return;
    }

    switch (reply->error()) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        reply->deleteLater();
        return;
    default:
        emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
        reply->deleteLater();
        return;
    }

    const QString response = QString::fromUtf8(reply->readAll());

    if (FILE_REGEXP.indexIn(response) != -1) {
        QString url = FILE_REGEXP.cap();
        
        if (url.startsWith("/")) {
            url.prepend(reply->url().scheme() + "://" + reply->url().authority());
        }
        
        emit downloadRequest(QNetworkRequest(url));
    }
    else if (response.contains("Downloading is not possible")) {
        const QTime time = QTime::fromString(response.section("Please wait", 1, 1).section("to download", 0, 0)
                                            .trimmed(), "hh:mm:ss");

        if (time.isValid()) {
            emit waitRequest(QTime(0, 0).msecsTo(time), true);
        }
        else {
            emit error(tr("Unknown error"));
        }
    }
    else {
        QString recaptchaKey = response.section("/file/captcha.html?v=", 1, 1).section('"', 0, 0);

        if (recaptchaKey.isEmpty()) {
            emit error(tr("No captcha key found"));
        }
        else {
            recaptchaKey.prepend(QString("http://%1/file/captcha.html?v=").arg(reply->url().host()));
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, CaptchaType::Image, recaptchaKey, "submitCaptchaResponse");
        }
    }

    reply->deleteLater();
}

void TezfilesPlugin::submitCaptchaResponse(const QString &, const QString &response) {
    m_redirects = 0;
    const QString data = QString("CaptchaForm[code]=%1&free=1&freeDownloadRequest=1&uniqueId=%2&yt0=")
                                .arg(response).arg(m_fileId);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptcha()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void TezfilesPlugin::checkCaptcha() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    const QString redirect = getRedirect(reply);
    
    if (!redirect.isEmpty()) {
        if (FILE_REGEXP.indexIn(redirect) == 0) {
            emit downloadRequest(QNetworkRequest(redirect));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkCaptcha()));
        }
        else {
            emit error(tr("Maximum redirects reached"));
        }

        reply->deleteLater();
        return;
    }

    switch (reply->error()) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        reply->deleteLater();
        return;
    default:
        emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
        reply->deleteLater();
        return;
    }

    const QString response = QString::fromUtf8(reply->readAll());

    if (FILE_REGEXP.indexIn(response) != -1) {
        QString url = FILE_REGEXP.cap();
        
        if (url.startsWith("/")) {
            url.prepend(reply->url().scheme() + "://" + reply->url().authority());
        }
        
        emit downloadRequest(QNetworkRequest(url));
    }
    else if (response.contains("verification code is incorrect")) {
        QString recaptchaKey = response.section("/file/captcha.html?v=", 1, 1).section('"', 0, 0);

        if (recaptchaKey.isEmpty()) {
            emit error(tr("No captcha key found"));
        }
        else {
            recaptchaKey.prepend(QString("http://%1/file/captcha.html?v=").arg(reply->url().host()));
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, CaptchaType::Image, recaptchaKey, "submitCaptchaResponse");
        }
    }
    else if (WAITTIME_REGEXP.indexIn(response) != -1) {
        const int secs = WAITTIME_REGEXP.cap(2).toInt();

        if (secs > 0) {
            startWaitTimer(secs * 1000, SLOT(getDownloadLink()));
        }
        else {
            emit error(tr("Unknown error"));
        }
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void TezfilesPlugin::getDownloadLink() {
    m_redirects = 0;
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = networkAccessManager()->post(request, "free=1&uniqueId=" + m_fileId.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void TezfilesPlugin::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    const QString redirect = getRedirect(reply);
    
    if (!redirect.isEmpty()) {
        if (FILE_REGEXP.indexIn(redirect) == 0) {
            emit downloadRequest(QNetworkRequest(redirect));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkDownloadLink()));
        }
        else {
            emit error(tr("Maximum redirects reached"));
        }

        reply->deleteLater();
        return;
    }

    switch (reply->error()) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        reply->deleteLater();
        return;
    default:
        emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
        reply->deleteLater();
        return;
    }

    const QString response = QString::fromUtf8(reply->readAll());

    if (FILE_REGEXP.indexIn(response) != -1) {
        QString url = FILE_REGEXP.cap();

        if (url.startsWith("/")) {
            url.prepend(reply->url().scheme() + "://" + reply->url().authority());
        }

        emit downloadRequest(QNetworkRequest(url));
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void TezfilesPlugin::submitLogin(const QVariantMap &credentials) {
    if ((credentials.contains("username")) && (credentials.contains("password"))) {
        const QString username = credentials.value("username").toString();
        const QString password = credentials.value("password").toString();

        if ((!username.isEmpty()) && (!password.isEmpty())) {
            login(username, password);
            return;
        }
    }

    emit error(tr("Invalid login credentials provided"));
}

void TezfilesPlugin::login(const QString &username, const QString &password) {
    m_redirects = 0;
    const QString data = QString("LoginForm[username]=%1&LoginForm[password]=%2").arg(username).arg(password);
    QNetworkRequest request(LOGIN_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void TezfilesPlugin::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        fetchDownloadRequest(m_url);
        return;
    }

    fetchDownloadRequest(m_url);
    reply->deleteLater();
}

void TezfilesPlugin::startWaitTimer(int msecs, const char* slot) {
    if (!m_waitTimer) {
        m_waitTimer = new QTimer(this);
        m_waitTimer->setSingleShot(true);
    }

    m_waitTimer->setInterval(msecs);
    m_waitTimer->start();
    emit waitRequest(msecs, false);
    disconnect(m_waitTimer, SIGNAL(timeout()), this, 0);
    connect(m_waitTimer, SIGNAL(timeout()), this, slot);
}

void TezfilesPlugin::stopWaitTimer() {
    if (m_waitTimer) {
        m_waitTimer->stop();
        disconnect(m_waitTimer, SIGNAL(timeout()), this, 0);
    }
}

ServicePlugin* TezfilesPluginFactory::createPlugin(QObject *parent) {
    return new TezfilesPlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-tezfiles, TezfilesPluginFactory)
#endif
