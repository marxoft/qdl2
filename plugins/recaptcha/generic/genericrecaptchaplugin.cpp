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

#include "genericrecaptchaplugin.h"
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

int GenericRecaptchaPlugin::MAX_REDIRECTS = 8;

GenericRecaptchaPlugin::GenericRecaptchaPlugin(QObject *parent) :
    RecaptchaPlugin(parent),
    m_nam(0),
    m_redirects(0),
    m_ownManager(false)
{
}

RecaptchaPlugin* GenericRecaptchaPlugin::createPlugin(QObject *parent) {
    return new GenericRecaptchaPlugin(parent);
}

QNetworkAccessManager* GenericRecaptchaPlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void GenericRecaptchaPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool GenericRecaptchaPlugin::cancelCurrentOperation() {
    emit currentOperationCanceled();
    return true;
}

void GenericRecaptchaPlugin::getCaptcha(const QString &imageUrl) {
    m_redirects = 0;
    m_url = imageUrl;
    QNetworkRequest request(imageUrl);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaImage()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GenericRecaptchaPlugin::followRedirect(const QUrl &url) {
    m_redirects++;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaImage()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GenericRecaptchaPlugin::checkCaptchaImage() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            if (redirect.startsWith("/")) {
                redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
            }
            
            followRedirect(redirect);
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

    const QImage image = QImage::fromData(reply->readAll());

    if (image.isNull()) {
        emit error(tr("Invalid captcha image"));
    }
    else {
        emit captcha(m_url, image);
    }

    reply->deleteLater();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-genericrecaptcha, GenericRecaptchaPlugin)
#endif
