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

#include "filefactoryplugin.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QTimer>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

using namespace QtJson;

QRegExp FileFactoryPlugin::FILE_REGEXP("http(s|)://\\w\\d+\\.filefactory\\.com/get/\\w/[^'\"]+");
QRegExp FileFactoryPlugin::NOT_FOUND_REGEXP("file is no longer available|file has been deleted");
QString FileFactoryPlugin::LOGIN_URL("http://www.filefactory.com/member/login.php");
QString FileFactoryPlugin::CAPTCHA_URL("http://www.filefactory.com/file/checkCaptcha.php");
QString FileFactoryPlugin::RECAPTCHA_PLUGIN_ID("googlerecaptcha");
#if QT_VERSION >= 0x050000
QString FileFactoryPlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                       + "/.config/qdl2/plugins/filefactory");
#else
QString FileFactoryPlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                       + "/.config/qdl2/plugins/filefactory");
#endif
int FileFactoryPlugin::MAX_REDIRECTS = 8;

FileFactoryPlugin::FileFactoryPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_waitTimer(0),
    m_redirects(0),
    m_ownManager(false)
{
}

ServicePlugin* FileFactoryPlugin::createPlugin(QObject *parent) {
    return new FileFactoryPlugin(parent);
}

QNetworkAccessManager* FileFactoryPlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void FileFactoryPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool FileFactoryPlugin::cancelCurrentOperation() {
    stopWaitTimer();
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void FileFactoryPlugin::checkUrl(const QString &url) {
    m_redirects = 0;
    QNetworkRequest request(QUrl::fromUserInput(url));
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkUrlIsValid() {
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
        if (m_redirects < MAX_REDIRECTS) {
            if (redirect.startsWith("/")) {
                redirect.prepend("http://www.filefactory.com");
            }
            
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

    if (response.contains(NOT_FOUND_REGEXP)) {
        emit error(tr("File not found"));
    }
    else {
        const QString fileName = response.section("file_name", 1, 1).section("<h2>", 1, 1).section('<', 0, 0);
        
        if (fileName.isEmpty()) {
            emit error(tr("File not found"));
        }
        else {
            emit urlChecked(UrlResult(reply->request().url().toString(), fileName));
        }
    }

    reply->deleteLater();
}

void FileFactoryPlugin::getDownloadRequest(const QString &url) {
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
            usernameMap["label"] = tr("Username");
            usernameMap["key"] = "username";
            list << usernameMap;
            QVariantMap passwordMap;
            passwordMap["type"] = "password";
            passwordMap["label"] = tr("Password");
            passwordMap["key"] = "password";
            list << passwordMap;
            QVariantMap storeMap;
            storeMap["type"] = "boolean";
            storeMap["label"] = tr("Store credentials");
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

void FileFactoryPlugin::fetchDownloadRequest(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkDownloadRequest() {
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
            if (redirect.startsWith("/")) {
                redirect.prepend("http://www.filefactory.com");
            }
            
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
        const int secs = response.section("data-delay=\"", 1, 1).section('"', 0, 0).toInt();

        if (secs > 0) {
            m_url = QUrl(FILE_REGEXP.cap());
            startWaitTimer(secs * 1000, SLOT(sendDownloadRequest()));
        }
        else {
            emit error(tr("Unknown error"));
        }
    }
    else if (response.contains(NOT_FOUND_REGEXP)) {
        emit error(tr("File not found"));
    }
    else {
        m_check = response.section("check: '", 1, 1).section('\'', 0, 0);
        m_recaptchaKey = response.section("Recaptcha.create( \"", 1, 1).section('"', 0, 0);
        
        if ((m_check.isEmpty()) || (m_recaptchaKey.isEmpty())) {
            emit error(tr("No captcha key found"));
        }
        else {
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, m_recaptchaKey, "submitCaptchaResponse");
        }
    }

    reply->deleteLater();
}

void FileFactoryPlugin::submitCaptchaResponse(const QString &challenge, const QString &response) {
    m_redirects = 0;
    const QString data = QString("recaptcha_challenge_field=%1&recaptcha_response_field=%2&check=%3")
                                .arg(challenge).arg(response).arg(m_check);
    QNetworkRequest request(CAPTCHA_URL);
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkCaptcha() {
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
            if (redirect.startsWith("/")) {
                redirect.prepend("http://www.filefactory.com");
            }
            
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
    const QVariantMap map = Json::parse(response).toMap();

    if (map.value("status") == "ok") {
        QString path = map.value("path").toString();

        if (!path.isEmpty()) {
            if (path.startsWith("/")) {
                path.prepend("http://www.filefactory.com");
            }
            
            getDownloadLink(path);
        }
        else {
            emit error(tr("Unknown error"));
        }
    }
    else if (map.value("message").toString().startsWith("Entered code")) {
        emit captchaRequest(RECAPTCHA_PLUGIN_ID, m_recaptchaKey, "submitCaptchaResponse");
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void FileFactoryPlugin::getDownloadLink(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkDownloadLink() {
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
            if (redirect.startsWith("/")) {
                redirect.prepend("http://www.filefactory.com");
            }
            
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
        const int secs = response.section("data-delay=\"", 1, 1).section('"', 0, 0).toInt();

        if (secs > 0) {
            m_url = QUrl(FILE_REGEXP.cap());
            startWaitTimer(secs * 1000, SLOT(sendDownloadRequest()));
        }
        else {
            emit error(tr("Unknown error"));
        }
    }
    else if (response.contains(QRegExp("file is no longer available|file has been deleted"))) {
        emit error(tr("File not found"));
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void FileFactoryPlugin::sendDownloadRequest() {
    if (m_url.isEmpty()) {
        emit error(tr("Unknown error"));
    }
    else {
        emit downloadRequest(QNetworkRequest(m_url));
    }
}

void FileFactoryPlugin::submitLogin(const QVariantMap &credentials) {
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

void FileFactoryPlugin::login(const QString &username, const QString &password) {
    m_redirects = 0;
    QString data = QString("email=%1&password=%2").arg(username).arg(password);
    QNetworkRequest request(LOGIN_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        fetchDownloadRequest(m_url);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

    reply->deleteLater();

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            if (redirect.startsWith("/")) {
                redirect.prepend("http://www.filefactory.com");
            }
            
            followRedirect(redirect, SLOT(checkLogin()));
            return;
        }
    }

    fetchDownloadRequest(m_url);
}

void FileFactoryPlugin::startWaitTimer(int msecs, const char* slot) {
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

void FileFactoryPlugin::stopWaitTimer() {
    if (m_waitTimer) {
        m_waitTimer->stop();
        disconnect(m_waitTimer, SIGNAL(timeout()), this, 0);
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(filefactory, FileFactoryPlugin)
#endif
