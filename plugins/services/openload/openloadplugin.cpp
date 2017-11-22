/**
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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
 * along with plugin.program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "openloadplugin.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWebElement>
#include <QWebFrame>
#include <QWebPage>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

const QString OpenloadPlugin::EMBED_URL("https://openload.co/embed/");
const QString OpenloadPlugin::STREAM_URL("https://openload.co/stream/");

const int OpenloadPlugin::MAX_REDIRECTS = 8;

OpenloadPlugin::OpenloadPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_page(0),
    m_nam(0),
    m_redirects(0),
    m_ownManager(false)
{
}

QString OpenloadPlugin::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));
    
    if (redirect.startsWith("/")) {
        redirect.prepend(reply->url().scheme() + "://" + reply->url().authority());
    }
    
    return redirect;
}

QNetworkAccessManager* OpenloadPlugin::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownManager = true;
    }

    return m_nam;
}

void OpenloadPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
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

QWebPage* OpenloadPlugin::webPage() {
    if (!m_page) {
        m_page = new QWebPage(this);
        connect(m_page, SIGNAL(loadFinished(bool)), this, SLOT(checkDownloadRequest(bool)));
    }

    return m_page;
}

bool OpenloadPlugin::cancelCurrentOperation() {
    if (m_page) {
        m_page->mainFrame()->load(QUrl());
    }

    emit currentOperationCanceled();
    return true;
}

void OpenloadPlugin::checkUrl(const QString &url, const QVariantMap &) {
    const QString id = url.section(QRegExp("/(f|embed)/"), 1, 1);

    if (id.isEmpty()) {
        error(tr("No video ID found"));
        return;
    }

    m_redirects = 0;
    QNetworkRequest request(EMBED_URL + id);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
}

void OpenloadPlugin::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

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
    const QString fileName = response.section("name=\"og:title\" content=\"", 1, 1).section("\"", 0, 0);

    if (fileName.isEmpty()) {
        emit error(tr("File not found"));
    }
    else {
        emit urlChecked(UrlResult(reply->url().toString(), fileName));
    }

    reply->deleteLater();
}

void OpenloadPlugin::getDownloadRequest(const QString &url, const QVariantMap &) {
    webPage()->mainFrame()->load(url);
}

void OpenloadPlugin::checkDownloadRequest(bool ok) {
    if (!ok) {
        emit error(tr("Network error"));
        return;
    }

    const QString url = webPage()->mainFrame()->findFirstElement("#streamurl").toPlainText();

    if (url.isEmpty()) {
        emit error(tr("No video stream URL found"));
    }
    else {
        emit downloadRequest(QNetworkRequest(STREAM_URL + url));
    }
}

void OpenloadPlugin::followRedirect(const QString &url, const char* slot) {
    m_redirects++;
    QNetworkRequest request(url);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(currentOperationCanceled()), reply, SLOT(deleteLater()));
}

ServicePlugin* OpenloadPluginFactory::createPlugin(QObject *parent) {
    return new OpenloadPlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-openload, OpenloadPluginFactory)
#endif
