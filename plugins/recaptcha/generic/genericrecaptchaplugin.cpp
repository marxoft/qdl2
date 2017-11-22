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

#include "genericrecaptchaplugin.h"
#include "captchatype.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

const int GenericRecaptchaPlugin::MAX_REDIRECTS = 8;

GenericRecaptchaPlugin::GenericRecaptchaPlugin(QObject *parent) :
    RecaptchaPlugin(parent),
    m_nam(0),
    m_redirects(0),
    m_ownManager(false)
{
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

void GenericRecaptchaPlugin::getCaptcha(int captchaType, const QString &captchaKey, const QVariantMap &) {
    switch (captchaType) {
        case CaptchaType::Image:
        case CaptchaType::NoCaptcha:
            break;
        default:
            emit error(tr("Captcha type %1 not supported").arg(captchaType));
            return;
    }

    m_redirects = 0;
    m_captchaKey = captchaKey;
    m_captchaType = captchaType;
    QNetworkRequest request(captchaKey);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaResponse()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GenericRecaptchaPlugin::followRedirect(const QUrl &url) {
    m_redirects++;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaResponse()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GenericRecaptchaPlugin::checkCaptchaResponse() {
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

    if (m_captchaType == CaptchaType::NoCaptcha) {
        emit captcha(m_captchaType, reply->readAll());
    }
    else {
        emit captcha(m_captchaType, QByteArray(m_captchaKey.toUtf8() + "\n" + reply->readAll().toBase64()));
    }

    reply->deleteLater();
}

RecaptchaPlugin* GenericRecaptchaPluginFactory::createPlugin(QObject *parent) {
    return new GenericRecaptchaPlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-genericrecaptcha, GenericRecaptchaPluginFactory)
#endif
