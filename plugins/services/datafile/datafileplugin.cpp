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

#include "datafileplugin.h"
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

const QRegExp DataFilePlugin::ERROR_REGEXP("(class=\"error-msg\">|class=\"error-text\" >)([^<]+)");
const QRegExp DataFilePlugin::FILE_REGEXP("http(s|)://[a-zA-Z]+\\d+\\.datafile\\.com/[^'\"]+");
const QString DataFilePlugin::AJAX_URL("http://www.datafile.com/files/ajax.html");
const QString DataFilePlugin::CHECK_URL("http://www.datafile.com/linkchecker.html");
const QString DataFilePlugin::LOGIN_URL("https://www.datafile.com/login.html");
const QString DataFilePlugin::RECAPTCHA_PLUGIN_ID("qdl2-googlerecaptcha");
#if QT_VERSION >= 0x050000
const QString DataFilePlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                          + "/.config/qdl2/plugins/qdl2-datafile");
#else
const QString DataFilePlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                          + "/.config/qdl2/plugins/qdl2-datafile");
#endif
const int DataFilePlugin::MAX_REDIRECTS = 8;

using namespace QtJson;

DataFilePlugin::DataFilePlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_waitTimer(0),
    m_waitTime(0),
    m_redirects(0),
    m_ownManager(false)
{
}

QString DataFilePlugin::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));
    
    if (redirect.startsWith("/")) {
        redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
    }
    
    return redirect;
}

ServicePlugin* DataFilePlugin::createPlugin(QObject *parent) {
    return new DataFilePlugin(parent);
}

QNetworkAccessManager* DataFilePlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void DataFilePlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool DataFilePlugin::cancelCurrentOperation() {
    stopWaitTimer();
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void DataFilePlugin::checkUrl(const QString &url) {
    m_redirects = 0;
    m_url = url;
    QNetworkRequest request(CHECK_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, QByteArray("btn=&links=" + url.toUtf8()));
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void DataFilePlugin::checkUrlIsValid() {
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
    
    if (response.contains("Ok")) {
        emit urlChecked(UrlResult(m_url, m_url.mid(m_url.lastIndexOf("/") + 1)));
    }
    else {
        emit error(tr("File not found"));
    }

    reply->deleteLater();
}

void DataFilePlugin::getDownloadRequest(const QString &url) {
    m_redirects = 0;
    m_url = url;
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);

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

void DataFilePlugin::fetchDownloadRequest(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void DataFilePlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void DataFilePlugin::checkDownloadRequest() {
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
        m_recaptchaKey = response.section("recaptcha/api/challenge?k=", 1, 1).section("\"", 0, 0);
        m_fileId = response.section("getFileDownloadLink('", 1, 1).section("'", 0, 0);
        m_waitTime = response.section("contdownTimer('", 1, 1).section("'", 0, 0).toInt() * 1000;
        
        if ((m_recaptchaKey.isEmpty()) || (m_fileId.isEmpty()) || (m_waitTime <= 0)) {
            if (ERROR_REGEXP.indexIn(response) != -1) {
                const QString errorString = ERROR_REGEXP.cap(2).trimmed();
                
                if (errorString.contains("downloading another file")) {
                    emit waitRequest(600000, true);
                }
                else {
                    emit error(errorString);
                }
            }
            else {
                emit error(tr("Unknown error"));
            }
        }
        else {
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, m_recaptchaKey, "submitCaptchaResponse");
        }
    }

    reply->deleteLater();
}

void DataFilePlugin::submitCaptchaResponse(const QString &challenge, const QString &response) {
    m_redirects = 0;
    m_recaptchaChallenge = challenge;
    m_recaptchaResponse = response;
    const QString data = QString("doaction=validateCaptcha&recaptcha_challenge_field=%1&recaptcha_response_field=%2&fileid=%3").arg(challenge).arg(response).arg(m_fileId);
    QNetworkRequest request(AJAX_URL);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("Referer", m_url.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaResponse()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void DataFilePlugin::checkCaptchaResponse() {
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
        m_token = response.value("token").toString();

        if (m_token.isEmpty()) {
            emit error(tr("Unknown error"));
        }
        else {
            startWaitTimer(m_waitTime, SLOT(getDownloadLink()));
        }
    }
    else {
        QString errorString = response.value("msg").toString();
        
        if (errorString.contains("words is not valid")) {
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, m_recaptchaKey, "submitCaptchaResponse");
        }
        else {
            emit error(errorString.isEmpty() ? tr("Unknown error") : errorString.remove(QRegExp("<[^>]*>")));
        }
    }

    reply->deleteLater();
}

void DataFilePlugin::getDownloadLink() {
    m_redirects = 0;
    const QString data = QString("doaction=getFileDownloadLink&recaptcha_challenge_field=%1&recaptcha_response_field=%2&token=%3&fileid=%4").arg(m_recaptchaChallenge).arg(m_recaptchaResponse).arg(m_token).arg(m_fileId);
    QNetworkRequest request(AJAX_URL);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("Referer", m_url.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void DataFilePlugin::checkDownloadLink() {
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

    const QVariantMap response = Json::parse(QString::fromUtf8(reply->readAll())).toMap();
    const bool success = response.value("success", false).toBool();
    
    if (success) {
        const QString link = response.value("link").toString();
        
        if (link.isEmpty()) {
            emit error(tr("Unknown error"));
        }
        else {
            emit downloadRequest(QNetworkRequest(link));
        }
    }
    else {
        QString errorString = response.value("msg").toString();
        emit error(errorString.isEmpty() ? tr("Unknown error") : errorString.remove(QRegExp("<[^>]*>")));
    }
    
    reply->deleteLater();
}

void DataFilePlugin::submitLogin(const QVariantMap &credentials) {
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

void DataFilePlugin::login(const QString &username, const QString &password) {
    m_redirects = 0;
    const QString data = QString("login=%1&password=%2").arg(username).arg(password);
    QNetworkRequest request(LOGIN_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void DataFilePlugin::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        fetchDownloadRequest(m_url);
        return;
    }
    
    fetchDownloadRequest(m_url);
    reply->deleteLater();
}

void DataFilePlugin::startWaitTimer(int msecs, const char* slot) {
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

void DataFilePlugin::stopWaitTimer() {
    if (m_waitTimer) {
        m_waitTimer->stop();
        disconnect(m_waitTimer, SIGNAL(timeout()), this, 0);
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-datafile, DataFilePlugin)
#endif
