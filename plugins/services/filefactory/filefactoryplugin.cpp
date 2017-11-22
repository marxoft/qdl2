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

#include "filefactoryplugin.h"
#include "captchatype.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

using namespace QtJson;

const QRegExp FileFactoryPlugin::FILE_REGEXP("http(s|)://\\w+\\.filefactory\\.com/get/\\w/[^'\"]+");
const QRegExp FileFactoryPlugin::FOLDER_LINK_REGEXP("<a href=\"(http(s|)://www\\.filefactory\\.com/file/[^\"]+)\">([^<]+)");
const QRegExp FileFactoryPlugin::WAIT_ERROR("Please try again in <span>((\\d+)( hour, | hours, )|)((\\d+)( min, | mins, )|)((\\d+)( secs))");
const QRegExp FileFactoryPlugin::OTHER_ERROR("class=\"alert alert-danger\">\\s+<h2>[\\w\\s]+</h2>\\s+<p>([^\\.<]+)");
const QString FileFactoryPlugin::LIMIT_EXCEEDED_ERROR("exceeded the hourly limit for free users");
const QString FileFactoryPlugin::PASSWORD_PROTECTED_ERROR("Password Protected Folder");
const QString FileFactoryPlugin::LOGIN_URL("http://www.filefactory.com/member/login.php");
const QString FileFactoryPlugin::CAPTCHA_URL("http://www.filefactory.com/file/checkCaptcha.php");
const QString FileFactoryPlugin::RECAPTCHA_PLUGIN_ID("qdl2-googlerecaptcha");

const int FileFactoryPlugin::MAX_REDIRECTS = 8;

FileFactoryPlugin::FileFactoryPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_waitTimer(0),
    m_passwordSlot("checkUrl"),
    m_redirects(0),
    m_ownManager(false)
{
}

QString FileFactoryPlugin::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));
    
    if (redirect.startsWith("/")) {
        redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
    }
    
    return redirect;
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

void FileFactoryPlugin::checkUrl(const QString &url, const QVariantMap &) {
    m_redirects = 0;
    m_passwordSlot.clear();
    QNetworkReply *reply = networkAccessManager()->get(QNetworkRequest(QUrl::fromUserInput(url)));
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkUrlIsValid() {
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
    
    if (response.contains("<table id=\"folder_files\"")) {
        // Get file links from folder
        UrlResultList list;
        const QString table = response.section("<table id=\"folder_files\"", 1, 1).section("</table>", 0, 0);
        int pos = 0;
        
        while ((pos = FOLDER_LINK_REGEXP.indexIn(table, pos)) != -1) {
            const QString link = FOLDER_LINK_REGEXP.cap(1);
            const QString fileName = FOLDER_LINK_REGEXP.cap(3);
            
            if ((!link.isEmpty()) && (!fileName.isEmpty())) {
                list << UrlResult(link, fileName);
            }
            
            pos += FOLDER_LINK_REGEXP.matchedLength();
        }
        
        if (!list.isEmpty()) {
            const QString &packageName = list.first().fileName;
            emit urlChecked(list, packageName.left(packageName.lastIndexOf(".")));
        }
        else {
            emit error(tr("No files found"));
        }
    }
    else {
        const QString fileName = response.section("class=\"file-name\">", 1, 1).section('<', 0, 0);
        
        if (fileName.isEmpty()) {
            if (response.contains(PASSWORD_PROTECTED_ERROR)) {
                // Link is password protected, so request password
                m_url = reply->url();
                m_passwordSlot = SLOT(checkUrlIsValid());
                QVariantList settings;
                QVariantMap password;
                password["type"] = "password";
                password["label"] = "Password";
                password["key"] = "password";
                settings << password;
                emit settingsRequest(tr("Enter folder password"), settings, "submitFolderPassword");
            }
            else if (OTHER_ERROR.indexIn(response) != -1) {
                emit error(OTHER_ERROR.cap(1).trimmed());
            }
            else {
                emit error(tr("File not found"));
            }
        }
        else {
            emit urlChecked(UrlResult(reply->request().url().toString(), fileName));
        }
    }

    reply->deleteLater();
}

void FileFactoryPlugin::getDownloadRequest(const QString &url, const QVariantMap &settings) {
    m_redirects = 0;
    m_passwordSlot.clear();
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

void FileFactoryPlugin::fetchDownloadRequest(const QUrl &url) {
    m_redirects = 0;
    QNetworkReply *reply = networkAccessManager()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkReply *reply = networkAccessManager()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkDownloadRequest() {
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
        if (response.contains("Download with FileFactory TrafficShare")) {
            // No waiting required, so request download
            emit downloadRequest(QNetworkRequest(FILE_REGEXP.cap()));
        }
        else {
            const int secs = response.section("data-delay=\"", 1, 1).section('"', 0, 0).toInt();
            
            if (secs > 0) {
                m_url = QUrl(FILE_REGEXP.cap());
                startWaitTimer(secs * 1000, SLOT(getWaitTime()));
            }
            else {
                emit error(tr("Unknown error"));
            }
        }
    }
    else if (response.contains(PASSWORD_PROTECTED_ERROR)) {
        // Link is password protected, so request password
        m_url = reply->url();
        m_passwordSlot = SLOT(checkDownloadRequest());
        QVariantList settings;
        QVariantMap password;
        password["type"] = "password";
        password["label"] = "Password";
        password["key"] = "password";
        settings << password;
        emit settingsRequest(tr("Enter folder password"), settings, "submitFolderPassword");
    }
    else if (OTHER_ERROR.indexIn(response) != -1) {
        emit error(OTHER_ERROR.cap(1).trimmed());
    }
    else {
        m_check = response.section("check: '", 1, 1).section('\'', 0, 0);
        m_recaptchaKey = response.section("Recaptcha.create( \"", 1, 1).section('"', 0, 0);
        
        if ((m_check.isEmpty()) || (m_recaptchaKey.isEmpty())) {
            emit error(tr("No captcha key found"));
        }
        else {
            emit captchaRequest(RECAPTCHA_PLUGIN_ID, CaptchaType::Image, m_recaptchaKey, "submitCaptchaResponse");
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
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptcha()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkCaptcha() {
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
    const QVariantMap map = Json::parse(response).toMap();

    if (map.value("status") == "ok") {
        QString path = map.value("path").toString();

        if (!path.isEmpty()) {
            if (path.startsWith("/")) {
                path.prepend(reply->url().scheme() + "://" + reply->url().authority());
            }
            
            getDownloadLink(path);
        }
        else {
            emit error(tr("Unknown error"));
        }
    }
    else if (map.value("message").toString().startsWith("Entered code")) {
        emit captchaRequest(RECAPTCHA_PLUGIN_ID, CaptchaType::Image, m_recaptchaKey, "submitCaptchaResponse");
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void FileFactoryPlugin::getDownloadLink(const QUrl &url) {
    m_redirects = 0;
    QNetworkReply *reply = networkAccessManager()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkDownloadLink() {
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
        const int secs = response.section("data-delay=\"", 1, 1).section('"', 0, 0).toInt();

        if (secs > 0) {
            m_url = QUrl(FILE_REGEXP.cap());
            startWaitTimer(secs * 1000, SLOT(getWaitTime()));
        }
        else {
            emit error(tr("Unknown error"));
        }
    }
    else if (OTHER_ERROR.indexIn(response) != -1) {
        emit error(OTHER_ERROR.cap(1).trimmed());
    }
    else {
        emit error(tr("Unknown error"));
    }

    reply->deleteLater();
}

void FileFactoryPlugin::getWaitTime() {
    m_redirects = 0;
    QNetworkReply *reply = networkAccessManager()->head(QNetworkRequest(m_url));
    connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void FileFactoryPlugin::checkWaitTime() {
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
    
    const QUrl url = reply->url();
    
    if (FILE_REGEXP.indexIn(url.toString()) == 0) {
        emit downloadRequest(QNetworkRequest(url));
    }
    else {
        const QString response = QString::fromUtf8(reply->readAll());
        
        if (WAIT_ERROR.indexIn(response) != -1) {
            const int hours = qMax(0, WAIT_ERROR.cap(2).toInt());
            const int mins = qMax(0, WAIT_ERROR.cap(5).toInt());
            const int secs = qMax(1, WAIT_ERROR.cap(8).toInt());
            emit waitRequest((hours * 3600000) + (mins * 60000) + (secs * 1000), true);
        }
        else if (response.contains(LIMIT_EXCEEDED_ERROR)) {
            emit waitRequest(600000, true);
        }
        else if (OTHER_ERROR.indexIn(response) != -1) {
            emit error(OTHER_ERROR.cap(1).trimmed());
        }
        else {
            emit error(tr("Unknown error"));
        }
    }

    reply->deleteLater();
}

void FileFactoryPlugin::submitFolderPassword(const QVariantMap &password) {
    if ((m_url.isEmpty()) || (m_passwordSlot.isEmpty())) {
        emit error(tr("Cannot submit folder password"));
        return;
    }
    
    const QByteArray p = password.value("password").toString().toUtf8();
    
    if (p.isEmpty()) {
        emit error(tr("Invalid password specified"));
    }
    else {
        m_redirects = 0;
        QNetworkRequest request(m_url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QNetworkReply *reply = networkAccessManager()->post(request, "Submit=Continue&password=" + p);
        connect(reply, SIGNAL(finished()), this, m_passwordSlot.constData());
        connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
    }
}

void FileFactoryPlugin::submitLogin(const QVariantMap &credentials) {
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
    
    fetchDownloadRequest(m_url);
    reply->deleteLater();
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

ServicePlugin* FileFactoryPluginFactory::createPlugin(QObject *parent) {
    return new FileFactoryPlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-filefactory, FileFactoryPluginFactory)
#endif
