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

#include "googledriveplugin.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#include <QUrlQuery>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

#if QT_VERSION >= 0x050000
QString GoogleDrivePlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                       + "/.config/qdl2/plugins/googledrive");
#else
QString GoogleDrivePlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                       + "/.config/qdl2/plugins/googledrive");
#endif
QStringList GoogleDrivePlugin::VIDEO_FORMATS = QStringList() << "37" << "46" << "22" << "45" << "44" << "35" << "18"
                                                             << "43" << "34" << "36" << "17";

int GoogleDrivePlugin::MAX_REDIRECTS = 8;

GoogleDrivePlugin::GoogleDrivePlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_redirects(0),
    m_ownManager(false)
{
}

ServicePlugin* GoogleDrivePlugin::createPlugin(QObject *parent) {
    return new GoogleDrivePlugin(parent);
}

QNetworkAccessManager* GoogleDrivePlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void GoogleDrivePlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

bool GoogleDrivePlugin::cancelCurrentOperation() {
    m_redirects = 0;
    emit currentOperationCanceled();
    return true;
}

void GoogleDrivePlugin::checkUrl(const QString &url) {
    m_redirects = 0;
    QNetworkRequest request(QUrl::fromUserInput(url));
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GoogleDrivePlugin::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            if (redirect.host().isEmpty()) {
                redirect.setScheme(reply->url().scheme());
                redirect.setHost(reply->url().host());
            }
            
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
    const QString fileName = response.section("itemprop=\"name\" content=\"", -1).section('"', 0, 0);
    
    if (fileName.isEmpty()) {
        emit error(tr("Unknown error"));
    }
    else {
        emit urlChecked(UrlResult(reply->request().url().toString(), fileName));
    }

    reply->deleteLater();
}

void GoogleDrivePlugin::getDownloadRequest(const QString &url) {
    m_redirects = 0;
    QNetworkRequest request(QUrl::fromUserInput(url));
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GoogleDrivePlugin::followRedirect(const QUrl &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GoogleDrivePlugin::checkDownloadRequest() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            if (redirect.host().isEmpty()) {
                redirect.setScheme(reply->url().scheme());
                redirect.setHost(reply->url().host());
            }
            
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
    
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);

    if ((response.contains("url_encoded_fmt_stream_map"))
        && (settings.value("Google Drive/useYouTubeForVideos", false).toBool())) {
        // Treat as YouTube video if possible
        const QString formatMap = response.section("url_encoded_fmt_stream_map\",\"", 1, 1).section("\"]", 0, 0)
                                          .trimmed().replace("\\u0026", "&").replace("\\u003d", "=")
                                          .remove(QRegExp("itag=\\d+"));
        const QMap<QString, QUrl> urlMap = getYouTubeVideoUrlMap(formatMap);
        const QString format = settings.value("videoFormat", "18").toString();

        for (int i = VIDEO_FORMATS.indexOf(format); i < VIDEO_FORMATS.size(); i++) {
            const QUrl url = urlMap.value(VIDEO_FORMATS.at(i));

            if (!url.isEmpty()) {
                emit downloadRequest(QNetworkRequest(url));
                return;
            }
        }
        
        getDownloadPage(QUrl(QString("https://docs.google.com/uc?id=%1&export=download")
                                    .arg(reply->url().toString().section("/d/", -1).section('/', 0, 0))));
    }
}

void GoogleDrivePlugin::getDownloadPage(const QUrl &url) {
    m_redirects = 0;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadPage()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GoogleDrivePlugin::checkDownloadPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            if (redirect.host().isEmpty()) {
                redirect.setScheme(reply->url().scheme());
                redirect.setHost(reply->url().host());
            }
            
            followRedirect(redirect, SLOT(checkDownloadPage()));
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
    const QString confirm = response.contains("confirm=") ? response.section("confirm=", -1).section('&', 0, 0)
                                                          : "";

    if (confirm.isEmpty()) {
        emit error(tr("Unknown error"));
    }
    else {
        QUrl url = reply->request().url();
#if QT_VERSION >= 0x050000
        QUrlQuery query(url);
        query.addQueryItem("confirm", confirm);
        url.setQuery(query);
#else
        url.addQueryItem("confirm", confirm);
#endif
        emit downloadRequest(QNetworkRequest(url));
    }

    reply->deleteLater();
}

QMap<QString, QUrl> GoogleDrivePlugin::getYouTubeVideoUrlMap(const QString &page) {
    QMap<QString, QUrl> urlMap;
    QStringList parts = page.split(',', QString::SkipEmptyParts);

    foreach (QString part, parts) {
        part = unescape(part);
        part.replace(QRegExp("(^|&)sig="), "&signature=");
        QStringList splitPart = part.split("url=");

        if (!splitPart.isEmpty()) {
            QString urlString = splitPart.last();
            QStringList params = urlString.mid(urlString.indexOf('?') + 1).split('&', QString::SkipEmptyParts);
            params.removeDuplicates();

            QUrl url(urlString.left(urlString.indexOf('?')));
#if QT_VERSION >= 0x050000
            QUrlQuery query;

            foreach (QString param, params) {
                query.addQueryItem(param.section('=', 0, 0), param.section('=', -1));
            }

            if (!query.hasQueryItem("signature")) {
                query.addQueryItem("signature", splitPart.first().section("signature=", 1, 1).section('&', 0, 0));
            }

            url.setQuery(query);

            urlMap[query.queryItemValue("itag")] = url;
#else
            foreach (QString param, params) {
                url.addQueryItem(param.section('=', 0, 0), param.section('=', -1));
            }

            if (!url.hasQueryItem("signature")) {
                url.addQueryItem("signature", splitPart.first().section("signature=", 1, 1).section('&', 0, 0));
            }

            urlMap[url.queryItemValue("itag")] = url;
#endif
        }
    }

    return urlMap;
}

QString GoogleDrivePlugin::unescape(const QString &s) {
    int unescapes = 0;
    QByteArray us = s.toUtf8();

    while ((us.contains('%')) && (unescapes < 10)) {
        us = QByteArray::fromPercentEncoding(us);
        unescapes++;
    }

    return QString(us);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(googledrive, GoogleDrivePlugin)
#endif
