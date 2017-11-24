/**
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "deathbycaptchaplugin.h"
#include "captchatype.h"
#include "json.h"
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

const QString DeathByCaptchaPlugin::CAPTCHA_URL("http://api.dbcapi.me/api/captcha");
const QString DeathByCaptchaPlugin::REPORT_URL("http://api.dbcapi.me/api/captcha/%1/report");

using namespace QtJson;

DeathByCaptchaPlugin::DeathByCaptchaPlugin(QObject *parent) :
    DecaptchaPlugin(parent),
    m_nam(0),
    m_ownManager(false)
{
}

QNetworkAccessManager* DeathByCaptchaPlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void DeathByCaptchaPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool DeathByCaptchaPlugin::cancelCurrentOperation() {
    emit currentOperationCanceled();
    return true;
}

void DeathByCaptchaPlugin::getCaptchaResponse(int captchaType, const QByteArray &captchaData,
        const QVariantMap &settings) {
    if (captchaType != CaptchaType::Image) {
        emit error(tr("Captcha type %1 not supported").arg(captchaType));
        return;
    }

    m_imageData = QByteArray::fromBase64(captchaData);
    m_username = settings.value("Account/username").toString();
    m_password = settings.value("Account/password").toString();
    
    if ((m_username.isEmpty()) || (m_password.isEmpty())) {
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
        emit settingsRequest(tr("Login"), list, "setLogin");
    }
    else {
        fetchCaptchaResponse(m_imageData);
        m_imageData.clear();
    }
}

void DeathByCaptchaPlugin::reportCaptchaResponse(const QString &captchaId) {
    m_captchaId = captchaId;
    const QString postData = QString("username=%1&password=%2").arg(m_username).arg(m_password);
    QNetworkRequest request(REPORT_URL.arg(captchaId));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkAccessManager()->post(request, postData.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaReport()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void DeathByCaptchaPlugin::setLogin(const QVariantMap &login) {
    m_username = login.value("username").toString();
    m_password = login.value("password").toString();

    if ((m_username.isEmpty()) || (m_password.isEmpty())) {
        emit error(tr("Invalid login provided"));
    }
    else {
        fetchCaptchaResponse(m_imageData);
    }

    m_imageData.clear();
}

void DeathByCaptchaPlugin::fetchCaptchaResponse(const QByteArray &imageData) {
    qsrand(QDateTime::currentDateTime().toTime_t());
    QByteArray boundary("---------------------------" + QByteArray::number(qrand()) + QByteArray::number(qrand())
                                                      + QByteArray::number(qrand()));
    const QByteArray crlf("\r\n");
    const QByteArray endBoundary(crlf + "--" + boundary + "--" + crlf);
    const QByteArray contentType("multipart/form-data; boundary=" + boundary);
    boundary.prepend("--");
    boundary.append(crlf);
    
    QByteArray bond = boundary;
    QByteArray send = bond;
    boundary.prepend(crlf);
    bond = boundary;
    // Add username field
    send.append("Content-Disposition: form-data; name=\"username\"");
    send.append(crlf);
    send.append("Content-Transfer-Encoding: 8bit");
    send.append(crlf);
    send.append(crlf);
    send.append(m_username.toUtf8());
    // Add password field
    send.append(bond);
    send.append("Content-Disposition: form-data; name=\"password\"");
    send.append(crlf);
    send.append("Content-Transfer-Encoding: 8bit");
    send.append(crlf);
    send.append(crlf);
    send.append(m_password.toUtf8());
    // Add captcha image data
    send.append(bond);
    send.append("Content-Disposition: form-data; name=\"captchafile\"; filename=\"captcha.jpg\"");
    send.append(crlf);
    send.append("Content-Type: image/jpeg");
    send.append(crlf);
    send.append(crlf);
    send.append(imageData);
    send.append(endBoundary);

    QNetworkRequest request(CAPTCHA_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    request.setHeader(QNetworkRequest::ContentLengthHeader, send.size());
    QNetworkReply *reply = networkAccessManager()->post(request, send);
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaResponse()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void DeathByCaptchaPlugin::checkCaptchaResponse() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
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

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 303) {
        m_statusUrl = reply->header(QNetworkRequest::LocationHeader).toString();

        if (m_statusUrl.isValid()) {
            QTimer::singleShot(10000, this, SLOT(checkCaptchaStatus()));
        }
        else {
            emit error(tr("Unknown error"));
        }
    }
    else {
        emit error(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
    }

    reply->deleteLater();
}

void DeathByCaptchaPlugin::checkCaptchaStatus() {
    QNetworkRequest request(m_statusUrl);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaStatusResponse()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void DeathByCaptchaPlugin::checkCaptchaStatusResponse() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
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
    m_captchaId = map.value("captcha").toString();
    const bool success = map.value("is_correct").toBool();
    
    if (!success) {
        emit error(tr("Captcha could not be solved"));
    }
    else {
        const QString text = map.value("text").toString();
        
        if (!text.isEmpty()) {
            emit captchaResponse(m_captchaId, text);
        }
        else {
            QTimer::singleShot(5000, this, SLOT(checkCaptchaStatus()));
        }
    }
    
    reply->deleteLater();
}

void DeathByCaptchaPlugin::checkCaptchaReport() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
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

    emit captchaResponseReported(m_captchaId);
    reply->deleteLater();
}

DecaptchaPlugin* DeathByCaptchaPluginFactory::createPlugin(QObject *parent) {
    return new DeathByCaptchaPlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-deathbycaptcha, DeathByCaptchaPluginFactory)
#endif
