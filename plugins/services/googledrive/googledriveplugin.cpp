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

#include "googledriveplugin.h"
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#else
#include <QtPlugin>
#endif

const QStringList GoogleDrivePlugin::VIDEO_FORMATS = QStringList() << "37" << "46" << "22" << "45" << "44" << "35"
                                                                   << "18" << "43" << "34" << "36" << "17";

const int GoogleDrivePlugin::MAX_REDIRECTS = 8;

GoogleDrivePlugin::GoogleDrivePlugin(QObject *parent) :
    ServicePlugin(parent),
    m_nam(0),
    m_redirects(0),
    m_ownManager(false)
{
}

QString GoogleDrivePlugin::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));
    
    if (redirect.startsWith("/")) {
        redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
    }
    
    return redirect;
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

void GoogleDrivePlugin::checkUrl(const QString &url, const QVariantMap &) {
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

    const QString redirect = getRedirect(reply);

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
    const QString fileName = response.section("itemprop=\"name\" content=\"", -1).section('"', 0, 0);
    
    if (fileName.isEmpty()) {
        emit error(tr("Unknown error"));
    }
    else {
        emit urlChecked(UrlResult(reply->request().url().toString(), fileName));
    }

    reply->deleteLater();
}

void GoogleDrivePlugin::getDownloadRequest(const QString &url, const QVariantMap &settings) {
    m_redirects = 0;
    m_settings = settings;
    
    if (m_settings.value("useYouTubeFormats", false).toBool()) {
        QNetworkReply *reply = networkAccessManager()->get(QNetworkRequest(QUrl::fromUserInput(url)));
        connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
        connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
    }
    else {
        getDownloadPage(url.section("/d/", -1).section('/', 0, 0));
    }
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

    const QString redirect = getRedirect(reply);
    
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
    const QString url = reply->url().toString();
    reply->deleteLater();
    
    if (response.contains("url_encoded_fmt_stream_map")) {
        // Treat as YouTube video if possible
        const QString formatMap = response.section("url_encoded_fmt_stream_map\",\"", 1, 1).section("\"]", 0, 0)
                                          .trimmed().replace("\\u0026", "&").replace("\\u003d", "=")
                                          .remove(QRegExp("itag=\\d+"));
        const QMap<QString, QUrl> urlMap = getYouTubeVideoUrlMap(formatMap);
        const QString format = m_settings.value("videoFormat", "18").toString();
        
        for (int i = VIDEO_FORMATS.indexOf(format); i < VIDEO_FORMATS.size(); i++) {
            const QUrl &videoUrl = urlMap.value(VIDEO_FORMATS.at(i));
                
            if (!url.isEmpty()) {
                emit downloadRequest(QNetworkRequest(videoUrl));
                return;
            }
        }
    }

    getDownloadPage(url.section("/d/", -1).section('/', 0, 0));
}

void GoogleDrivePlugin::getDownloadPage(const QString &id) {
    m_redirects = 0;
    QUrl url("https://docs.google.com/uc");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("id", id);
    query.addQueryItem("export", "download");
    url.setQuery(query);
#else
    url.addQueryItem("id", id);
    url.addQueryItem("export", "download");
#endif
    QNetworkReply *reply = networkAccessManager()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadPage()));
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

void GoogleDrivePlugin::checkDownloadPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit error(tr("Network error"));
        return;
    }

    const QString redirect = getRedirect(reply);
    
    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
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
        QUrl url = reply->url();
#if QT_VERSION >= 0x050000
        QUrlQuery query(url);
        query.addQueryItem("confirm", confirm);
        url.setQuery(query);
        const QString id = query.queryItemValue("id").toUtf8();
#else
        url.addQueryItem("confirm", confirm);
        const QString id = url.queryItemValue("id");
#endif
        // Set the cookie for the virus warning page. This relies on the QDL application using the same 
        // QNetworkCookieJar instance for the download request, which is always the case.
        QList<QNetworkCookie> cookies = networkAccessManager()->cookieJar()->cookiesForUrl(url);
        QNetworkCookie cookie("download_warning_13058876669334088843_" + id.toUtf8(), confirm.toUtf8());
        cookie.setDomain("google.com");
        cookie.setPath("/");
        cookie.setHttpOnly(true);
        
        if (!cookies.isEmpty()) {
            cookie.setExpirationDate(cookies.first().expirationDate());
        }
        
        cookies << cookie;
        networkAccessManager()->cookieJar()->setCookiesFromUrl(cookies, url);
        emit downloadRequest(QNetworkRequest(url));
    }

    reply->deleteLater();
}

QMap<QString, QUrl> GoogleDrivePlugin::getYouTubeVideoUrlMap(const QString &page) {
    QMap<QString, QUrl> urlMap;
    const QStringList parts = page.split(',', QString::SkipEmptyParts);

    foreach (QString part, parts) {
        part = unescape(part);
        part.replace(QRegExp("(^|&)sig="), "&signature=");
        const QStringList splitPart = part.split("url=");

        if (!splitPart.isEmpty()) {
            const QString urlString = splitPart.last();
            QStringList params = urlString.mid(urlString.indexOf('?') + 1).split('&', QString::SkipEmptyParts);
            params.removeDuplicates();

            QUrl url(urlString.left(urlString.indexOf('?')));
#if QT_VERSION >= 0x050000
            QUrlQuery query;

            foreach (const QString &param, params) {
                query.addQueryItem(param.section('=', 0, 0), param.section('=', -1));
            }

            if (!query.hasQueryItem("signature")) {
                query.addQueryItem("signature", splitPart.first().section("signature=", 1, 1).section('&', 0, 0));
            }

            url.setQuery(query);

            urlMap[query.queryItemValue("itag")] = url;
#else
            foreach (const QString &param, params) {
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

ServicePlugin* GoogleDrivePluginFactory::createPlugin(QObject *parent) {
    return new GoogleDrivePlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-googledrive, GoogleDrivePluginFactory)
#endif
