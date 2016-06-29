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

#include "upstoreplugin.h"
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

const QRegExp UpstorePlugin::ERROR_REGEXP("<span class=\"error\">([^<]+)");
const QRegExp UpstorePlugin::FILE_REGEXP("http(s|)://\\w+\\.upsto\\.re/\\w+/[\\w\\.-]+/[^'\"]+");
const QRegExp UpstorePlugin::WAIT_REGEXP("wait (\\d+) minutes before downloading next file");
const QString UpstorePlugin::LOGIN_URL("http://upstore.net/account/login/");
const QString UpstorePlugin::RECAPTCHA_PLUGIN_ID("qdl2-googlerecaptcha");
#if QT_VERSION >= 0x050000
const QString UpstorePlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-upstore");
#else
const QString UpstorePlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-upstore");
#endif
const int UpstorePlugin::MAX_REDIRECTS = 8;

UpstorePlugin::UpstorePlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_waitTimer(0),
    m_waitTime(0),
    m_redirects(0),
    m_ownManager(false)
{
}

QString UpstorePlugin::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));
    
    if (redirect.startsWith("/")) {
        redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
    }
    
    return redirect;
}

ServicePlugin* UpstorePlugin::createPlugin(QObject *parent) {
    return new UpstorePlugin(parent);
}

QNetworkAccessManager* UpstorePlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void UpstorePlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool UpstorePlugin::cancelCurrentOperation() {
    stopWaitTimer();
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void UpstorePlugin::checkUrl(const QString &url) {
    m_redirects = 0;
    QNetworkRequest request(QUrl::fromUserInput(url));
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UpstorePlugin::checkUrlIsValid() {
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
    const QString fileName = response.section("<h2 style=\"margin:0\">", 1, 1).section('<', 0, 0);

    if (fileName.isEmpty()) {
        emit error(tr("File not found"));
    }
    else {
        emit urlChecked(UrlResult(reply->request().url().toString(), fileName));
    }

    reply->deleteLater();
}

void UpstorePlugin::getDownloadRequest(const QString &url) {
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

void UpstorePlugin::fetchDownloadRequest(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UpstorePlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UpstorePlugin::checkDownloadRequest() {
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
        m_hash = response.section("hash\" value=\"", 1, 1).section('"', 0, 0);
        
        if (m_hash.isEmpty()) {
            if (ERROR_REGEXP.indexIn(response) != -1) {
                emit error(ERROR_REGEXP.cap(1));
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

void UpstorePlugin::getWaitTime() {
    m_redirects = 0;
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, "free=Slow+download&hash=" + m_hash.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UpstorePlugin::checkWaitTime() {
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
        emit downloadRequest(QNetworkRequest(FILE_REGEXP.cap()));
    }
    else {
        m_waitTime = response.section("var sec =", 1, 1).section(',', 0, 0).trimmed().toInt() * 1000;
        m_recaptchaKey = response.section("Recaptcha.create('", 1, 1).section('\'', 0, 0);
        
        if ((m_waitTime <= 0) || (m_recaptchaKey.isEmpty())) {
            if (ERROR_REGEXP.indexIn(response) != -1) {
                emit error(ERROR_REGEXP.cap(1));
            }
            else {
                emit error(tr("Unknown error"));
            }
        }
        else {
            startWaitTimer(m_waitTime, SLOT(sendCaptchaRequest()));
        }
    }

    reply->deleteLater();
}

void UpstorePlugin::submitCaptchaResponse(const QString &challenge, const QString &response) {
    m_redirects = 0;
    const QString data =
    QString("kpw=spam&antispam=spam&free=Get+download+link&hash=%1&recaptcha_challenge_field=%2&recaptcha_response_field=%3").arg(m_hash).arg(challenge).arg(response);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptcha()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UpstorePlugin::checkCaptcha() {
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
        emit downloadRequest(QNetworkRequest(FILE_REGEXP.cap()));
    }
    else if (response.contains("Wrong captcha")) {
        startWaitTimer(m_waitTime, SLOT(sendCaptchaRequest()));
    }
    else if (WAIT_REGEXP.indexIn(response) != -1) {
        const int mins = qMax(1, WAIT_REGEXP.cap(1).toInt());
        emit waitRequest(mins * 60000, true);
    }
    else if (ERROR_REGEXP.indexIn(response) != -1) {
        emit error(ERROR_REGEXP.cap(1));
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void UpstorePlugin::sendCaptchaRequest() {
    if (m_recaptchaKey.isEmpty()) {
        emit error(tr("No captcha key found"));
    }
    else {
        emit captchaRequest(RECAPTCHA_PLUGIN_ID, m_recaptchaKey, "submitCaptchaResponse");
    }
}

void UpstorePlugin::submitLogin(const QVariantMap &credentials) {
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

void UpstorePlugin::login(const QString &username, const QString &password) {
    m_redirects = 0;
    const QString data = QString("send=Login&email=%1&password=%2").arg(username).arg(password);
    QNetworkRequest request(LOGIN_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UpstorePlugin::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        fetchDownloadRequest(m_url);
        return;
    }

    fetchDownloadRequest(m_url);
    reply->deleteLater();
}

void UpstorePlugin::startWaitTimer(int msecs, const char* slot) {
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

void UpstorePlugin::stopWaitTimer() {
    if (m_waitTimer) {
        m_waitTimer->stop();
        disconnect(m_waitTimer, SIGNAL(timeout()), this, 0);
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-upstore, UpstorePlugin)
#endif
