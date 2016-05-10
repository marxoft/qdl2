/*
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

#include "rapidgatorplugin.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QTimer>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#include <QUrlQuery>
#else
#include <QDesktopServices>
#endif

QRegExp RapidGatorPlugin::FILE_REGEXP("http://pr\\d+\\.rapidgator\\.net//\\?r=download/index&session_id=[^'\"]+");
QString RapidGatorPlugin::LOGIN_URL("https://rapidgator.net/auth/login");
QString RapidGatorPlugin::SESSION_ID_URL("http://rapidgator.net/download/AjaxStartTimer");
QString RapidGatorPlugin::DOWNLOAD_LINK_URL("http://rapidgator.net/download/AjaxGetDownloadLink");
QString RapidGatorPlugin::CAPTCHA_URL("http://rapidgator.net/download/captcha");
QString RapidGatorPlugin::RECAPTCHA_PLUGIN_ID("solvemediarecaptcha");
QString RapidGatorPlugin::RECAPTCHA_KEY("oy3wKTaFP368dkJiGUqOVjBR2rOOR7GR");
#if QT_VERSION >= 0x050000
QString RapidGatorPlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                      + "/.config/qdl2/plugins/rapidgator");
#else
QString RapidGatorPlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                      + "/.config/qdl2/plugins/rapidgator");
#endif
int RapidGatorPlugin::MAX_REDIRECTS = 8;

using namespace QtJson;

RapidGatorPlugin::RapidGatorPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_waitTimer(0),
    m_secs(0),
    m_redirects(0),
    m_ownManager(false)
{
}

ServicePlugin* RapidGatorPlugin::createPlugin(QObject *parent) {
    return new RapidGatorPlugin(parent);
}

QNetworkAccessManager* RapidGatorPlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void RapidGatorPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool RapidGatorPlugin::cancelCurrentOperation() {
    stopWaitTimer();
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void RapidGatorPlugin::checkUrl(const QString &url) {
    m_redirects = 0;
    QNetworkRequest request(QUrl::fromUserInput(url));
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void RapidGatorPlugin::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
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
    
    if (response.contains("File not found")) {
        emit error(tr("File not found"));
    }
    else {
        const QString fileName = response.section("<title>Download file ", 1, 1).section('<', 0, 0).trimmed();
        
        if (fileName.isEmpty()) {
            emit error(tr("Unknown error"));
        }
        else {
            emit urlChecked(UrlResult(reply->request().url().toString(), fileName));
        }
    }

    reply->deleteLater();
}

void RapidGatorPlugin::getDownloadRequest(const QString &url) {
    m_redirects = 0;
    m_url = QUrl::fromUserInput(url);
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);

    if (settings.value("Account/useLogin", false).toBool()) {
        const QString username = settings.value("Account/username").toString();
        const QString password = settings.value("Account/password").toString();

        if ((username.isEmpty()) || (password.isEmpty())) {
            QVariantList list;
            QVariantMap usernameMap;
            usernameMap["type"] = "text";
            usernameMap["label"] = tr("&Username");
            usernameMap["key"] = "username";
            list << usernameMap;
            QVariantMap passwordMap;
            passwordMap["type"] = "password";
            passwordMap["label"] = tr("&Password");
            passwordMap["key"] = "password";
            list << passwordMap;
            QVariantMap storeMap;
            storeMap["type"] = "boolean";
            storeMap["label"] = tr("&Store credentials");
            storeMap["key"] = "store";
            list << storeMap;
            emit settingsRequest(tr("Login"), list, "submitLogin");
        }   
        else {
            login(username, password);
        }

        return;
    }
    
    fetchDownloadRequest(m_url);
}

void RapidGatorPlugin::fetchDownloadRequest(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void RapidGatorPlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void RapidGatorPlugin::checkDownloadRequest() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

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
    else if (response.contains("File Not Found")) {
        emit error(tr("File Not Found"));
    }
    else if (response.contains("Please try again later")) {
        emit waitRequest(600000, true);
    }
    else {
        m_fileId = response.section("var fid = ", 1, 1).section(';', 0, 0);
        m_secs = response.section("var secs = ", 1, 1).section(';', 0, 0).toInt();
        const int mins = response.section("not less than ", 1, 1).section(" min", 0, 0).toInt();

        if ((m_fileId.isEmpty()) || (m_secs <= 0)) {
            emit error(tr("Unknown error"));
        }
        else if (mins > 0) {
            emit waitRequest(mins * 60000, true);
        }
        else {
            getSessionId();
        }
    }

    reply->deleteLater();
}

void RapidGatorPlugin::getSessionId() {
    m_redirects = 0;
    QUrl url(SESSION_ID_URL);
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("fid", m_fileId);
    url.setQuery(query);
#else
    url.addQueryItem("fid", m_fileId);
#endif
    QNetworkRequest request(url);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkSessionId()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void RapidGatorPlugin::checkSessionId() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

    if (!redirect.isEmpty()) {
        if (FILE_REGEXP.indexIn(redirect) == 0) {
            emit downloadRequest(QNetworkRequest(redirect));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkSessionId()));
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
        const QVariantMap map = Json::parse(response).toMap();
        
        if (map.value("state").toString() == "started") {
            m_sessionId = map.value("sid").toString();
            startWaitTimer(m_secs * 1000, SLOT(getDownloadLink()));
        }
        else {
            emit error(tr("Unknown error"));
        }
    }

    reply->deleteLater();
}

void RapidGatorPlugin::getDownloadLink() {
    m_redirects = 0;
    QUrl url(DOWNLOAD_LINK_URL);
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("sid", m_sessionId);
    url.setQuery(query);
#else
    url.addQueryItem("sid", m_sessionId);
#endif
    QNetworkRequest request(url);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void RapidGatorPlugin::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

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
        emit downloadRequest(QNetworkRequest(FILE_REGEXP.cap()));
    }
    else {
        const QVariantMap map = Json::parse(response).toMap();
        
        if (map.value("state").toString() == "done") {
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, RECAPTCHA_KEY, "submitCaptchaResponse");
        }
        else {
            emit error(tr("Unknown error"));
        }
    }

    reply->deleteLater();
}

void RapidGatorPlugin::submitCaptchaResponse(const QString &challenge, const QString &response) {
    m_redirects = 0;
    const QString data = QString("DownloadCaptchaForm[captcha]=&adcopy_challenge=%1&adcopy_response=%2")
                                .arg(challenge).arg(response);
    QNetworkRequest request(CAPTCHA_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptcha()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void RapidGatorPlugin::checkCaptcha() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

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
        emit downloadRequest(QNetworkRequest(FILE_REGEXP.cap()));
    }
    else if (response.contains("verification code is incorrect")) {
        emit captchaRequest(RECAPTCHA_PLUGIN_ID, RECAPTCHA_KEY, "submitCaptchaResponse");
    }
    else if (response.contains("reached your daily downloads limit")) {
        emit waitRequest(600000, true);
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void RapidGatorPlugin::submitLogin(const QVariantMap &credentials) {
    if ((credentials.contains("username")) && (credentials.contains("password"))) {
        const QString username = credentials.value("username").toString();
        const QString password = credentials.value("password").toString();

        if ((!username.isEmpty()) && (!password.isEmpty())) {
            if (credentials.value("store", false).toBool()) {
                QSettings settings(CONFIG_FILE, QSettings::IniFormat);
                settings.setValue("Account/username", username);
                settings.setValue("Account/password", password);
            }
            
            login(username, password);
            return;
        }
    }

    emit error(tr("Invalid login credentials provided"));
}

void RapidGatorPlugin::login(const QString &username, const QString &password) {
    m_redirects = 0;
    const QString data = QString("LoginForm[email]=%1&LoginForm[password]=%2&LoginForm[rememberMe]=1")
                                .arg(username).arg(password);
    QNetworkRequest request(LOGIN_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void RapidGatorPlugin::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        fetchDownloadRequest(m_url);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

    reply->deleteLater();

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkLogin()));
            return;
        }
    }

    fetchDownloadRequest(m_url);
}

void RapidGatorPlugin::startWaitTimer(int msecs, const char* slot) {
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

void RapidGatorPlugin::stopWaitTimer() {
    if (m_waitTimer) {
        m_waitTimer->stop();
        disconnect(m_waitTimer, SIGNAL(timeout()), this, 0);
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(rapidgator, RapidGatorPlugin)
#endif
