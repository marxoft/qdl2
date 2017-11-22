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

#include "googlerecaptchaplugin.h"
#include "captchatype.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

const QString GoogleRecaptchaPlugin::CAPTCHA_CHALLENGE_URL("https://www.google.com/recaptcha/api/challenge?k=");
const QString GoogleRecaptchaPlugin::CAPTCHA_IMAGE_URL("https://www.google.com/recaptcha/api/image?c=");
const QString GoogleRecaptchaPlugin::NO_CAPTCHA_HTML("<html><title>Google reCaptcha</title><body><script>function recaptchaCallback(response) { document.title = \"response=\" + response; }</script><form action=\"\" method=\"post\"><div class=\"g-recaptcha\" data-sitekey=\"%1\" data-callback=\"recaptchaCallback\"></div></form><script type=\"text/javascript\" src=\"https://www.google.com/recaptcha/api.js\"></script></body></html>");

GoogleRecaptchaPlugin::GoogleRecaptchaPlugin(QObject *parent) :
    RecaptchaPlugin(parent),
    m_nam(0),
    m_ownManager(false)
{
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

void GoogleRecaptchaPlugin::getCaptcha(int captchaType, const QString &captchaKey, const QVariantMap &) {
    switch (captchaType) {
        case CaptchaType::Image:
            break;
        case CaptchaType::NoCaptcha:
            emit captcha(captchaType, NO_CAPTCHA_HTML.arg(captchaKey).toUtf8());
            return;
        default:
            emit error(tr("Captcha type %1 not supported").arg(captchaType));
            return;
    }

    QUrl url(CAPTCHA_CHALLENGE_URL + captchaKey);
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
    QUrl url(CAPTCHA_IMAGE_URL + challenge);
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaImageDownloaded()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GoogleRecaptchaPlugin::onCaptchaImageDownloaded() {
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

    emit captcha(CaptchaType::Image, QByteArray(m_challenge.toUtf8() + "\n" + reply->readAll().toBase64()));
    reply->deleteLater();
}

RecaptchaPlugin* GoogleRecaptchaPluginFactory::createPlugin(QObject *parent) {
    return new GoogleRecaptchaPlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-googlerecaptcha, GoogleRecaptchaPluginFactory)
#endif
