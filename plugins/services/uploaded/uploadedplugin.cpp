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

#include "uploadedplugin.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>
#include <QStringList>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

const QRegExp UploadedPlugin::FILE_REGEXP("http(s|)://([-\\w]+|)stor\\d+\\.uploaded\\.net/dl/[-\\w]+");
const QString UploadedPlugin::LOGIN_URL("http://uploaded.net/io/login");
const QString UploadedPlugin::BASE_FILE_URL("http://uploaded.net/file/");
const QString UploadedPlugin::NOT_FOUND_URL("http://uploaded.net/404");
const QString UploadedPlugin::CAPTCHA_URL("http://uploaded.net/io/ticket/captcha/");
const QString UploadedPlugin::RECAPTCHA_PLUGIN_ID("qdl2-googlerecaptcha");
const QString UploadedPlugin::RECAPTCHA_KEY("6Lcqz78SAAAAAPgsTYF3UlGf2QFQCNuPMenuyHF3");
#if QT_VERSION >= 0x050000
const QString UploadedPlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                          + "/.config/qdl2/plugins/qdl2-uploaded");
#else
const QString UploadedPlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                          + "/.config/qdl2/plugins/qdl2-uploaded");
#endif
const int UploadedPlugin::MAX_REDIRECTS = 8;

using namespace QtJson;

UploadedPlugin::UploadedPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_waitTimer(0),
    m_redirects(0),
    m_ownManager(false)
{
}

QString UploadedPlugin::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));
    
    if (redirect.startsWith("/")) {
        redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
    }
    
    return redirect;
}

ServicePlugin* UploadedPlugin::createPlugin(QObject *parent) {
    return new UploadedPlugin(parent);
}

QNetworkAccessManager* UploadedPlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void UploadedPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool UploadedPlugin::cancelCurrentOperation() {
    stopWaitTimer();
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void UploadedPlugin::checkUrl(const QString &url) {
    m_redirects = 0;
    QNetworkRequest request(QUrl::fromUserInput(url));
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UploadedPlugin::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Unknown error"));
        return;
    }

    const QString redirect = getRedirect(reply);

    if (!redirect.isEmpty()) {
        if (FILE_REGEXP.indexIn(redirect) == 0) {
            emit urlChecked(UrlResult(reply->request().url().toString(),
                            redirect.mid(redirect.lastIndexOf("/") + 1)));
        }
        else if (redirect == NOT_FOUND_URL) {
            emit error(tr("File not found"));
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
    const QString fileName = response.section("id=\"filename\">", 1, 1).section("<", 0, 0).trimmed()
                                     .replace("&hellip;", ".");

    if (fileName.isEmpty()) {
        // Check if it's a folder
        if (response.contains("id=\"fileList\">")) {
            const QString table = response.section("id=\"fileList\">", 1, 1).section("</table>", 0, 0);
            QStringList files = table.split("id=\"", QString::SkipEmptyParts);
            
            if (files.isEmpty()) {
                emit error(tr("No files found"));
            }
            else {
                UrlResultList list;
                
                while (!files.isEmpty()) {
                    const QString &file = files.takeFirst();
                    const QString id = file.left(file.indexOf('"'));
                    const QString fileName = file.section("this))\">", 1, 1).section('<', 0, 0);
                    
                    if ((!id.isEmpty()) && (!fileName.isEmpty())) {
                        list << UrlResult(QString("http://uploaded.net/file/" + id), fileName);
                    }
                }

                if (list.isEmpty()) {
                    emit error(tr("No files found"));
                }
                else {
                    QString folderName = response.section("<title>", 1, 1).section("<", 0, 0);

                    if (folderName.isEmpty()) {
                        folderName = list.first().fileName.section(".", 0, -2);
                    }
                    
                    emit urlChecked(list, folderName);
                }
            }
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

void UploadedPlugin::getDownloadRequest(const QString &url) {
    m_redirects = 0;
    m_fileId = url.section(QRegExp("/file/|/ul.to/"), -1);
    m_url = QUrl(BASE_FILE_URL + m_fileId);
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

void UploadedPlugin::fetchDownloadRequest(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UploadedPlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UploadedPlugin::checkDownloadRequest() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    const QString redirect = getRedirect(reply);

    if (!redirect.isEmpty()) {
        if (FILE_REGEXP.indexIn(redirect) == 0) {
            emit downloadRequest(buildDownloadRequest(redirect));
        }
        else if (redirect == NOT_FOUND_URL) {
            emit error(tr("File not found"));
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
        emit downloadRequest(buildDownloadRequest(FILE_REGEXP.cap()));
    }
    else {
        const int secs = response.section("Current waiting period: <span>", 1, 1).section('<', 0, 0).toInt();

        if (secs > 0) {
            if (secs > 30) {
                emit waitRequest(secs * 1000, true);
            }
            else {
                startWaitTimer(secs * 1000, SLOT(sendCaptchaRequest()));
            }
        }
        else {
            emit error(tr("Unknown error"));
        }
    }

    reply->deleteLater();
}

void UploadedPlugin::submitCaptchaResponse(const QString &challenge, const QString &response) {
    m_redirects = 0;
    const QString data = QString("recaptcha_challenge_field=%1&recaptcha_response_field=%2")
                                .arg(challenge).arg(response);
    QNetworkRequest request(CAPTCHA_URL + m_fileId);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("X-Prototype-Version", "1.6.1");
    request.setRawHeader("Host", "uploaded.net");
    request.setRawHeader("Origin", "http://uploaded.net");
    request.setRawHeader("Referer", QString(BASE_FILE_URL + m_fileId).toUtf8());
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptcha()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UploadedPlugin::checkCaptcha() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    const QString redirect = getRedirect(reply);

    if (!redirect.isEmpty()) {
        if (FILE_REGEXP.indexIn(redirect) == 0) {
            emit downloadRequest(buildDownloadRequest(redirect));
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
        emit downloadRequest(buildDownloadRequest(FILE_REGEXP.cap()));
    }
    else {
        const QVariantMap map = Json::parse(response).toMap();
        const QString errorString = map.value("err").toString();

        if (errorString == "captcha") {
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, RECAPTCHA_KEY, "submitCaptchaResponse");
        }
        else if ((errorString == "limit-dl") || (errorString.contains(QRegExp("max|Download-Slots|Free-Downloads",
                 Qt::CaseInsensitive)))) {
            emit waitRequest(600000, true);
        }
        else {
            emit error(tr("Unknown error"));
        }
    }

    reply->deleteLater();
}

void UploadedPlugin::submitLogin(const QVariantMap &credentials) {
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

void UploadedPlugin::login(const QString &username, const QString &password) {
    m_redirects = 0;
    const QString data = QString("id=%1&pw=%2").arg(username).arg(password);
    QNetworkRequest request(LOGIN_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void UploadedPlugin::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        fetchDownloadRequest(m_url);
        return;
    }

    fetchDownloadRequest(m_url);
    reply->deleteLater();
}

void UploadedPlugin::sendCaptchaRequest() {
    emit captchaRequest(RECAPTCHA_PLUGIN_ID, RECAPTCHA_KEY, "submitCaptchaResponse");
}

void UploadedPlugin::startWaitTimer(int msecs, const char* slot) {
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

void UploadedPlugin::stopWaitTimer() {
    if (m_waitTimer) {
        m_waitTimer->stop();
        disconnect(m_waitTimer, SIGNAL(timeout()), this, 0);
    }
}

QNetworkRequest UploadedPlugin::buildDownloadRequest(const QUrl &url) const {
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Host", url.host().toUtf8());
    request.setRawHeader("Referer", QString(BASE_FILE_URL + m_fileId).toUtf8());
    request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.4 (KHTML, like Gecko) Ubuntu/12.10 Chromium/22.0.1229.94 Chrome/22.0.1229.94 Safari/537.4");
    return request;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-uploaded, UploadedPlugin)
#endif
