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

#include "fileboomplugin.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QTimer>
#include <QTime>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

QRegExp FileBoomPlugin::FILE_REGEXP("(http(s|)://fboom\\.me|)/file/url\\.html\\?file=[^'\"]+");
QString FileBoomPlugin::LOGIN_URL("http://fboom.me/login.html");
QString FileBoomPlugin::RECAPTCHA_PLUGIN_ID("qdl2-genericrecaptcha");
#if QT_VERSION >= 0x050000
QString FileBoomPlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                    + "/.config/qdl2/plugins/fileboom");
#else
QString FileBoomPlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                    + "/.config/qdl2/plugins/fileboom");
#endif
int FileBoomPlugin::MAX_REDIRECTS = 8;

FileBoomPlugin::FileBoomPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_waitTimer(0),
    m_redirects(0),
    m_ownManager(false)
{
}

ServicePlugin* FileBoomPlugin::createPlugin(QObject *parent) {
    return new FileBoomPlugin(parent);
}

QNetworkAccessManager* FileBoomPlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void FileBoomPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool FileBoomPlugin::cancelCurrentOperation() {
    stopWaitTimer();
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void FileBoomPlugin::checkUrl(const QString &url) {
    m_redirects = 0;
    QNetworkRequest request(QUrl::fromUserInput(url));
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileBoomPlugin::checkUrlIsValid() {
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
            if (redirect.host().isEmpty()) {
                redirect.setScheme("http");
                redirect.setHost("fboom.me");
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
    const QString fileName = response.section("icon-download\"></i>", 1, 1).section("</div>", 0, 0).trimmed();

    if (fileName.isEmpty()) {
        emit error(tr("File not found"));
    }
    else {
        emit urlChecked(UrlResult(reply->request().url().toString(), fileName));
    }

    reply->deleteLater();
}

void FileBoomPlugin::getDownloadRequest(const QString &url) {
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

void FileBoomPlugin::fetchDownloadRequest(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileBoomPlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileBoomPlugin::checkDownloadRequest() {
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
            QUrl url(redirect);
            
            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }
            
            emit downloadRequest(QNetworkRequest(url));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            QUrl url(redirect);
            
            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }
            
            followRedirect(url, SLOT(checkDownloadRequest()));
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
        QUrl url(FILE_REGEXP.cap());
        
        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("fboom.me");
        }
        
        emit downloadRequest(QNetworkRequest(url));
    }
    else {
        m_fileId = response.section("data-slow-id=\"", 1, 1).section('"', 0, 0);
        
        if (m_fileId.isEmpty()) {
            if (response.contains("system is unable to find")) {
                emit error(tr("File not found"));
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

void FileBoomPlugin::getWaitTime() {
    m_redirects = 0;
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = networkAccessManager()->post(request, "slow_id=" + m_fileId.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileBoomPlugin::checkWaitTime() {
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
            QUrl url(redirect);
            
            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }
            
            emit downloadRequest(QNetworkRequest(url));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            QUrl url(redirect);
            
            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }
            
            followRedirect(url, SLOT(checkWaitTime()));
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
        QUrl url(FILE_REGEXP.cap());
        
        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("fboom.me");
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
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, recaptchaKey, "submitCaptchaResponse");
        }
    }

    reply->deleteLater();
}

void FileBoomPlugin::submitCaptchaResponse(const QString &, const QString &response) {
    m_redirects = 0;
    const QString data = QString("CaptchaForm[code]=%1&free=1&freeDownloadRequest=1&uniqueId=%2").arg(response)
                                                                                                 .arg(m_fileId);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptcha()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileBoomPlugin::checkCaptcha() {
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
            QUrl url(redirect);
            
            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }
            
            emit downloadRequest(QNetworkRequest(url));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            QUrl url(redirect);
            
            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }
            
            followRedirect(url, SLOT(checkCaptcha()));
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
        QUrl url(FILE_REGEXP.cap());
        
        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("fboom.me");
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
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, recaptchaKey, "submitCaptchaResponse");
        }
    }
    else {
        const int secs = response.section("tik-tak\">", 1, 1).section('<', 0, 0).toInt();

        if (secs > 0) {
            startWaitTimer(secs * 1000, SLOT(getDownloadLink()));
        }
        else {
            emit error(tr("Unknown error"));
        }
    }

    reply->deleteLater();
}

void FileBoomPlugin::getDownloadLink() {
    m_redirects = 0;
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = networkAccessManager()->post(request, "free=1&uniqueId=" + m_fileId.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileBoomPlugin::checkDownloadLink() {
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
            QUrl url(redirect);
            
            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }
            
            emit downloadRequest(QNetworkRequest(url));
        }
        else if (m_redirects < MAX_REDIRECTS) {
            QUrl url(redirect);
            
            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }
            
            followRedirect(url, SLOT(checkDownloadLink()));
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
        QUrl url(FILE_REGEXP.cap());

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("fboom.me");
        }

        emit downloadRequest(QNetworkRequest(url));
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void FileBoomPlugin::submitLogin(const QVariantMap &credentials) {
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

void FileBoomPlugin::login(const QString &username, const QString &password) {
    m_redirects = 0;
    const QString data = QString("LoginForm[username]=%1&LoginForm[password]=%2").arg(username).arg(password);
    QNetworkRequest request(LOGIN_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileBoomPlugin::checkLogin() {
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

void FileBoomPlugin::startWaitTimer(int msecs, const char* slot) {
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

void FileBoomPlugin::stopWaitTimer() {
    if (m_waitTimer) {
        m_waitTimer->stop();
        disconnect(m_waitTimer, SIGNAL(timeout()), this, 0);
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-fileboom, FileBoomPlugin)
#endif
