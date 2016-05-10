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

#include "urlretriever.h"
#include "definitions.h"
#include "logger.h"
#include "servicepluginmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

UrlRetriever::UrlRetriever(QObject *parent) :
    QObject(parent),
    m_processor(new UrlProcessor),
    m_nam(new QNetworkAccessManager(this)),
    m_reply(0),
    m_status(Idle),
    m_redirects(0)
{
    connect(m_processor, SIGNAL(finished(QStringList)), this, SLOT(onProcessorFinished(QStringList)));
}

UrlRetriever::~UrlRetriever() {
    delete m_processor;
    m_processor = 0;
}

QString UrlRetriever::url() const {
    return m_url;
}

void UrlRetriever::setUrl(const QString &u) {
    if (u != url()) {
        m_url = u;
        emit urlChanged(u);
    }
}

QString UrlRetriever::pluginId() const {
    return m_pluginId;
}

void UrlRetriever::setPluginId(const QString &id) {
    if (id != pluginId()) {
        m_pluginId = id;
        emit pluginIdChanged(id);
    }
}

UrlRetriever::Status UrlRetriever::status() const {
    return m_status;
}

void UrlRetriever::setStatus(UrlRetriever::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString UrlRetriever::statusString() const {
    switch (status()) {
    case Active:
        return tr("Retrieving URLs");
    case Completed:
        return tr("Completed");
    case Canceled:
        return tr("Canceled");
    case Failed:
        return tr("Failed: %1").arg(errorString());
    default:
        return QString();
    }
}

QString UrlRetriever::errorString() const {
    return m_errorString;
}

void UrlRetriever::setErrorString(const QString &e) {
    m_errorString = e;
}

QStringList UrlRetriever::results() const {
    return m_results;
}

void UrlRetriever::setResults(const QStringList &r) {
    m_results = r;
}

void UrlRetriever::start(const QString &url, const QString &pluginId) {
    if (status() == Active) {
        return;
    }

    Logger::log("UrlRetriever::start(): " + url + " " + pluginId);
    setStatus(Active);
    setUrl(url);
    setPluginId(pluginId);
    m_redirects = 0;
    m_reply = m_nam->get(QNetworkRequest(QUrl::fromUserInput(url)));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}

void UrlRetriever::cancel() {
    if ((m_reply) && (m_reply->isRunning())) {
        m_reply->abort();
    }
}

void UrlRetriever::followRedirect(const QUrl &url) {
    Logger::log("UrlRetriever::followRedirect(): " + url.toString());
    setStatus(Active);
    m_redirects++;
    m_reply = m_nam->get(QNetworkRequest(url));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}

void UrlRetriever::onReplyFinished() {
    const QNetworkReply::NetworkError error = m_reply->error();
    const QString errorString = m_reply->errorString();
    const QVariant redirect = m_reply->header(QNetworkRequest::LocationHeader);
    const QString baseUrl = QString("%1://%2").arg(m_reply->url().scheme()).arg(m_reply->url().host());
    const QString response = QString::fromUtf8(m_reply->readAll());
    m_reply->deleteLater();
    m_reply = 0;

    if (!redirect.isNull()) {
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect.toUrl());
        }
        else {
            setErrorString(tr("Maximum redirects reached"));
            setStatus(Failed);
            emit finished(this);
        }
        
        return;
    }

    switch (error) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        setStatus(Canceled);
        emit finished(this);
        return;
    default:
        setErrorString(errorString);
        setStatus(Failed);
        emit finished(this);
        return;
    }

    m_processor->processUrls(baseUrl, response, pluginId());
}

void UrlRetriever::onProcessorFinished(const QStringList &urls) {
    setResults(urls);
    setStatus(Completed);
    emit finished(this);
}

UrlProcessor::UrlProcessor() :
    QObject()
{
    moveToThread(new QThread);
    thread()->start(QThread::LowestPriority);
}

UrlProcessor::~UrlProcessor() {
    thread()->quit();
    thread()->deleteLater();
}

void UrlProcessor::processUrls(const QString &baseUrl, const QString &response, const QString &pluginId) {
    Logger::log("UrlProcessor::processUrls(): " + baseUrl + " " + pluginId);
    QStringList urls;
    QRegExp re("(http(s|):/|href=\"|src=\")/[^'\"<\\s]+");
    int pos = 0;

    if (!pluginId.isEmpty()) {
        ServicePluginConfig *config = ServicePluginManager::instance()->getConfigById(pluginId);

        if (!config) {
            emit finished(urls);
            return;
        }

        while ((pos = re.indexIn(response, pos)) != -1) {
            QString url = re.cap(0);
            const int quote = url.indexOf('"');

            if (quote != -1) {
                url = url.mid(quote + 1);

                if (url.startsWith("//")) {
                    url.prepend(baseUrl.left(baseUrl.indexOf("/")));
                }
                else {
                    url.prepend(baseUrl);
                }
            }

            Logger::log("UrlProcessor::processUrls(): Checking URL " + url);

            if (config->urlIsSupported(url)) {
                urls << url;
            }
            
            pos += re.matchedLength();
        }
    }
    else {
        while ((pos = re.indexIn(response, pos)) != -1) {
            QString url = re.cap(0);
            const int quote = url.indexOf('"');

            if (quote != -1) {
                url = url.mid(quote + 1);

                if (url.startsWith("//")) {
                    url.prepend(baseUrl.left(baseUrl.indexOf("/")));
                }
                else {
                    url.prepend(baseUrl);
                }
            }

            Logger::log("UrlProcessor::processUrls(): Checking URL " + url);

            if (ServicePluginManager::instance()->urlIsSupported(url)) {
                urls << url;
            }
        
            pos += re.matchedLength();
        }
    }

    urls.removeDuplicates();
    emit finished(urls);
}
