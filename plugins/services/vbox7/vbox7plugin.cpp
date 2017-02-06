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

#include "vbox7plugin.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

#if QT_VERSION >= 0x050000
const QString Vbox7Plugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                       + "/.config/qdl2/plugins/qdl2-vbox7");
#else
const QString Vbox7Plugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                       + "/.config/qdl2/plugins/qdl2-vbox7");
#endif
const QString Vbox7Plugin::BASE_URL("http://vbox7.com");

const QByteArray Vbox7Plugin::USER_AGENT("Wget/1.13.4 (linux-gnu)");

const QRegExp Vbox7Plugin::NO_JS_ERROR("/show:missjavascript\\?[^']+");

const int Vbox7Plugin::MAX_REDIRECTS = 8;

Vbox7Plugin::Vbox7Plugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_redirects(0),
    m_ownManager(false)
{
}

ServicePlugin* Vbox7Plugin::createPlugin(QObject *parent) {
    return new Vbox7Plugin(parent);
}

QNetworkAccessManager* Vbox7Plugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void Vbox7Plugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool Vbox7Plugin::cancelCurrentOperation() {
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void Vbox7Plugin::checkUrl(const QString &url) {
    m_redirects = 0;
    m_url = QUrl::fromUserInput(url).toString();
    QNetworkRequest request(m_url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void Vbox7Plugin::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QString redirect = getRedirect(reply);

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
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
    redirect = getRedirect(response, m_url);
    
    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkUrlIsValid()));
        }
        else {
            emit error(tr("Maximum redirects reached"));
        }
        
        return;
    }
    
    const QString fileName = response.section("name=\"title\" content=\"", 1, 1).section("\"", 0, 0);
    
    if (fileName.isEmpty()) {
        emit error(tr("Not found"));
    }
    else {
        emit urlChecked(UrlResult(reply->request().url().toString(), fileName + ".mp4"));
    }

    reply->deleteLater();
}

void Vbox7Plugin::getDownloadRequest(const QString &url) {
    m_redirects = 0;
    m_url = QUrl::fromUserInput(url).toString();
    QNetworkRequest request(m_url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void Vbox7Plugin::checkDownloadRequest() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QString redirect = getRedirect(reply);
    
    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
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
    reply->deleteLater();
    redirect = getRedirect(response, m_url);
    
    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkDownloadRequest()));
        }
        else {
            emit error(tr("Maximum redirects reached"));
        }
        
        return;
    }
    
    const QString options = response.section("var options =", 1, 1).section(";\n", 0, 0);
    QString url = options.section("src: '", 1, 1).section("'", 0, 0);
    
    if (url.isEmpty()) {
        emit error(tr("Not found"));
        return;
    }
    
    if (url.startsWith("//")) {
        url.prepend("http:");
    }
    
    const bool isHD = options.section("videoIsHD: ", 1, 1).section(",", 0, 0).toInt() == 1;
    const bool hasLD = options.section("videoHasLD: ", 1, 1).section(",", 0, 0).toInt() == 1;
    const int dot = url.lastIndexOf(".");
    const QString ldUrl = QString("%1_480p%2").arg(url.left(dot)).arg(url.mid(dot));
    
    if ((isHD) && (hasLD)) {
        const QSettings settings(CONFIG_FILE, QSettings::IniFormat);
        
        if (settings.value("useDefaultVideoFormat", true).toBool()) {
            if (settings.value("videoFormat", "hd").toString() == "hd") {
                emit downloadRequest(QNetworkRequest(url));
            }
            else {
                emit downloadRequest(QNetworkRequest(ldUrl));
            }
        }
        else {
            QVariantList settingsList;
            QVariantList optionsList;
            QVariantMap list;
            list["type"] = "list";
            list["label"] = tr("Video format");
            list["key"] = "videoFormat";
            list["value"] = "hd";
            QVariantMap hd;
            hd["label"] = "720P";
            hd["value"] = url;
            QVariantMap ld;
            ld["label"] = "480P";
            ld["value"] = ldUrl;
            optionsList << hd << ld;
            list["options"] = optionsList;
            settingsList << list;
            emit settingsRequest(tr("Video format"), settingsList, "submitVideoFormat");
        }
    }
    else {
        emit downloadRequest(QNetworkRequest(url));
    }
}

void Vbox7Plugin::submitVideoFormat(const QVariantMap &format) {
    const QUrl url(format.value("videoFormat").toString());

    if (url.isEmpty()) {
        emit error(tr("Invalid video format chosen"));
    }
    else {
        emit downloadRequest(QNetworkRequest(url));
    }
}

QString Vbox7Plugin::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(QByteArray::fromPercentEncoding(reply->rawHeader("Location")));
    
    if (redirect.startsWith("/")) {
        redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
    }
    
    return redirect;
}

QString Vbox7Plugin::getRedirect(const QString &response, const QString &url) {
    if (response == "1") {
        return url;
    }
    
    if (NO_JS_ERROR.indexIn(response) != -1) {
        return BASE_URL + NO_JS_ERROR.cap();
    }
    
    return QString();
}

void Vbox7Plugin::followRedirect(const QString &url, const char *slot) {
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-vbox7, Vbox7Plugin)
#endif
