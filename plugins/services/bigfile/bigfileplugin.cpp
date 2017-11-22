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

#include "bigfileplugin.h"
#include "captchatype.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

const QRegExp BigFilePlugin::ERROR_REGEXP("(class=\"icon_err\">\\s+<h1>)([^<]+)");
const QRegExp BigFilePlugin::FILE_REGEXP("http(s|)://[^w]+\\.bigfile\\.to/file/[^'\"]+");
const QString BigFilePlugin::LOGIN_URL("https://bigfile.to/login.php");
const QString BigFilePlugin::RECAPTCHA_URL("https://www.bigfile.to/checkReCaptcha.php");
const QString BigFilePlugin::RECAPTCHA_PLUGIN_ID("qdl2-googlerecaptcha");

const int BigFilePlugin::MAX_REDIRECTS = 8;

using namespace QtJson;

BigFilePlugin::BigFilePlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_waitTimer(0),
    m_redirects(0),
    m_ownManager(false)
{
}

QString BigFilePlugin::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));
    
    if (redirect.startsWith("/")) {
        redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
    }
    
    return redirect;
}

QNetworkAccessManager* BigFilePlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void BigFilePlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool BigFilePlugin::cancelCurrentOperation() {
    stopWaitTimer();
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void BigFilePlugin::checkUrl(const QString &url, const QVariantMap &) {
    m_redirects = 0;
    QNetworkRequest request(QUrl::fromUserInput(url));
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void BigFilePlugin::checkUrlIsValid() {
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
    const QString fileName = response.section("id=\"file_name\" title=\"", 1, 1).section("\"", 0, 0);

    if (fileName.isEmpty()) {
        if (ERROR_REGEXP.indexIn(response) != -1) {
            emit error(ERROR_REGEXP.cap(2));
        }
        else {
            emit error(tr("File not found"));
        }
    }
    else {
        emit urlChecked(UrlResult(reply->request().url().toString(), fileName));
    }

    reply->deleteLater();
}

void BigFilePlugin::getDownloadRequest(const QString &url, const QVariantMap &settings) {
    m_redirects = 0;
    m_url = QUrl::fromUserInput(url);

    if (settings.value("Account/useLogin", false).toBool()) {
        const QString username = settings.value("Account/username").toString();
        const QString password = settings.value("Account/password").toString();

        if ((username.isEmpty()) || (password.isEmpty())) {
            QVariantList list;
            QVariantMap usernameMap;
            usernameMap["type"] = "text";
            usernameMap["label"] = tr("Email");
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

void BigFilePlugin::fetchDownloadRequest(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void BigFilePlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void BigFilePlugin::checkDownloadRequest() {
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
        emit downloadRequest(QNetworkRequest(FILE_REGEXP.cap()));
    }
    else {
        m_recaptchaKey = response.section("reCAPTCHA_publickey='", 1, 1).section("'", 0, 0);
        m_recaptchaShort = response.section("name=\"recaptcha_shortencode_field\" value=\"", 1, 1).section("\"", 0, 0);
        
        if ((m_recaptchaKey.isEmpty()) || (m_recaptchaShort.isEmpty())) {
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

void BigFilePlugin::getWaitTime() {
    m_redirects = 0;
    QNetworkRequest request(m_url);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, "downloadLink=wait");
    connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void BigFilePlugin::checkWaitTime() {
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

    const QVariantMap response = Json::parse(QString::fromUtf8(reply->readAll())).toMap();
    const int secs = response.value("waitTime").toInt();
    
    if (secs > 0) {
        startWaitTimer(secs * 1000, SLOT(getCaptcha()));
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void BigFilePlugin::getCaptcha() {
    m_redirects = 0;
    QNetworkRequest request(m_url);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, "checkDownload=check");
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptcha()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void BigFilePlugin::checkCaptcha() {
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
    
    const QVariantMap map = Json::parse(QString::fromUtf8(reply->readAll())).toMap();
    
    if (map.value("success") == "showCaptcha") {
        emit captchaRequest(RECAPTCHA_PLUGIN_ID, CaptchaType::Image, m_recaptchaKey, "submitCaptchaResponse");
    }
    else if (map.value("fail") == "timeLimit") {
        emit waitRequest(600000, true);
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void BigFilePlugin::submitCaptchaResponse(const QString &challenge, const QString &response) {
    m_redirects = 0;
    const QString data = QString("recaptcha_challenge_field=%1&recaptcha_response_field=%2&recaptcha_shortencode_field=%3").arg(challenge).arg(response).arg(m_recaptchaShort);
    QNetworkRequest request(RECAPTCHA_URL);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaResponse()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void BigFilePlugin::checkCaptchaResponse() {
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
            followRedirect(redirect, SLOT(checkCaptchaResponse()));
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

    const QVariantMap response = Json::parse(QString::fromUtf8(reply->readAll())).toMap();
    const bool success = response.value("success", false).toBool();
    
    if (success) {
        requestDownloadLink();
    }
    else {
        const QString errorString = response.value("error").toString();
        
        if (errorString == "incorrect-captcha-sol") {
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, CaptchaType::Image, m_recaptchaKey, "submitCaptchaResponse");
        }
        else {
            emit error(tr("Unknown error"));
        }
    }

    reply->deleteLater();
}

void BigFilePlugin::requestDownloadLink() {
    m_redirects = 0;
    QNetworkRequest request(m_url);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, "downloadLink=show");
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLinkRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void BigFilePlugin::checkDownloadLinkRequest() {
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
            followRedirect(redirect, SLOT(checkDownloadLinkRequest()));
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

    getDownloadLink();
}

void BigFilePlugin::getDownloadLink() {
    m_redirects = 0;
    QNetworkRequest request(m_url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, "download=normal");
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void BigFilePlugin::checkDownloadLink() {
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
        emit downloadRequest(QNetworkRequest(FILE_REGEXP.cap(0)));
    }
    else if (ERROR_REGEXP.indexIn(response) != -1) {
        emit error(ERROR_REGEXP.cap(2));
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void BigFilePlugin::submitLogin(const QVariantMap &credentials) {
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

void BigFilePlugin::login(const QString &username, const QString &password) {
    m_redirects = 0;
    const QString data = QString("userName=%1&userPassword=%2").arg(username).arg(password);
    QNetworkRequest request(LOGIN_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void BigFilePlugin::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        fetchDownloadRequest(m_url);
        return;
    }

    fetchDownloadRequest(m_url);
    reply->deleteLater();
}

void BigFilePlugin::startWaitTimer(int msecs, const char* slot) {
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

void BigFilePlugin::stopWaitTimer() {
    if (m_waitTimer) {
        m_waitTimer->stop();
        disconnect(m_waitTimer, SIGNAL(timeout()), this, 0);
    }
}

ServicePlugin* BigFilePluginFactory::createPlugin(QObject *parent) {
    return new BigFilePlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-bigfile, BigFilePluginFactory)
#endif
