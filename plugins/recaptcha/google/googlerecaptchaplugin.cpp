/*
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

#include "googlerecaptchaplugin.h"
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

GoogleRecaptchaPlugin::GoogleRecaptchaPlugin(QObject *parent) :
    RecaptchaPlugin(parent),
    m_nam(0),
    m_ownManager(false)
{
}

RecaptchaPlugin* GoogleRecaptchaPlugin::createPlugin(QObject *parent) {
    return new GoogleRecaptchaPlugin(parent);
}

QNetworkAccessManager* GoogleRecaptchaPlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void GoogleRecaptchaPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool GoogleRecaptchaPlugin::cancelCurrentOperation() {
    emit currentOperationCanceled();
    return true;
}

void GoogleRecaptchaPlugin::getCaptcha(const QString &captchaKey) {
    QUrl url("http://www.google.com/recaptcha/api/challenge?k=" + captchaKey);
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaDownloaded()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GoogleRecaptchaPlugin::onCaptchaDownloaded() {
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

    const QString response = QString::fromUtf8(reply->readAll());
    const QString challenge = response.section("challenge : '", 1, 1).section("'", 0, 0);

    if (challenge.isEmpty()) {
        emit error(tr("No captcha challenge found"));
    }
    else {
        downloadCaptchaImage(challenge);
    }

    reply->deleteLater();
}

void GoogleRecaptchaPlugin::downloadCaptchaImage(const QString &challenge) {
    m_challenge = challenge;
    QUrl url("http://www.google.com/recaptcha/api/image?c=" + challenge);
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaImageDownloaded()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GoogleRecaptchaPlugin::onCaptchaImageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

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

    const QImage image = QImage::fromData(reply->readAll());

    if (image.isNull()) {
        emit error(tr("Invalid captcha image"));
    }
    else {
        emit captcha(m_challenge, image);
    }

    reply->deleteLater();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(googlerecaptcha, GoogleRecaptchaPlugin)
#endif
