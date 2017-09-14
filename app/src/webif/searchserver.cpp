/*
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "searchserver.h"
#include "json.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "searchpluginmanager.h"
#include "serverresponse.h"
#include "utils.h"

static QVariantList searchResultsToVariantList(const SearchResultList &results) {
    QVariantList list;

    foreach (const SearchResult &result, results) {
        QVariantMap m;
        m["name"] = result.name;
        m["description"] = result.description;
        m["url"] = result.url;
        list << m;
    }

    return list;
}

SearchServer::SearchServer(QObject *parent) :
    QObject(parent)
{
}

bool SearchServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.size() != 1) || (parts.first() != "search")) {
        return false;
    }

    const QString pluginId = Utils::urlQueryItemValue(request->url(), "pluginId");
    const QString method = Utils::urlQueryItemValue(request->url(), "method", "search");

    SearchPlugin *plugin = SearchPluginManager::instance()->getPluginById(pluginId);

    if (plugin) {
        if (method == "search") {
            if (request->method() == QHttpRequest::HTTP_GET) {
                SearchPlugin *sp = plugin->createPlugin(this);
                addResponse(sp, response);
                sp->search();
            }
            else if (request->method() == QHttpRequest::HTTP_POST) {
                SearchPlugin *sp = plugin->createPlugin(this);
                addResponse(sp, response);
                sp->fetchMore(QtJson::Json::parse(request->body()).toMap());
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            }
        }
        else if (request->method() == QHttpRequest::HTTP_POST) {
            SearchPlugin *sp = plugin->createPlugin(this);

            if (QMetaObject::invokeMethod(sp, method.toUtf8(), Qt::QueuedConnection, Q_ARG(QVariantMap,
                            QtJson::Json::parse(request->body()).toMap()))) {
                addResponse(sp, response);
            }
            else {
                sp->deleteLater();
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        }
    }
    else {
        writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
    }

    return true;
}

void SearchServer::addResponse(SearchPlugin *plugin, QHttpResponse *response) {
    m_hash.insert(plugin, response);
    connect(plugin, SIGNAL(error(QString)), this, SLOT(onSearchError(QString)));
    connect(plugin, SIGNAL(searchCompleted(SearchResultList)),
            this, SLOT(onSearchCompleted(SearchResultList)));
    connect(plugin, SIGNAL(searchCompleted(SearchResultList, QVariantMap)),
            this, SLOT(onSearchCompleted(SearchResultList, QVariantMap)));
    connect(plugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
            this, SLOT(onSettingsRequest(QString, QVariantList, QByteArray)));
    connect(response, SIGNAL(done()), this, SLOT(onResponseDone()));
}

QHttpResponse* SearchServer::getResponse(SearchPlugin *plugin) {
    return m_hash.value(plugin);
}

void SearchServer::removeResponse(QHttpResponse *response) {
    if (SearchPlugin *plugin = m_hash.key(response)) {
        m_hash.remove(plugin);
        plugin->deleteLater();
        disconnect(response, 0, this, 0);
    }
}

void SearchServer::onSearchCompleted(const SearchResultList &results) {
    SearchPlugin *plugin = qobject_cast<SearchPlugin*>(sender());
    QHttpResponse *response = getResponse(plugin);

    if (response) {
        QVariantMap result;
        result["result"] = "searchCompleted";
        result["items"] = searchResultsToVariantList(results);
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(result));
    }
}

void SearchServer::onSearchCompleted(const SearchResultList &results, const QVariantMap &nextParams) {
    SearchPlugin *plugin = qobject_cast<SearchPlugin*>(sender());
    QHttpResponse *response = getResponse(plugin);

    if (response) {
        QVariantMap result;
        result["result"] = "searchCompleted";
        result["items"] = searchResultsToVariantList(results);

        if (!nextParams.isEmpty()) {
            result["next"] = nextParams;
        }

        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(result));
    }
}

void SearchServer::onSearchError(const QString &errorString) {
    SearchPlugin *plugin = qobject_cast<SearchPlugin*>(sender());
    QHttpResponse *response = getResponse(plugin);

    if (response) {
        QVariantMap result;
        result["result"] = "error";
        result["error"] = errorString;
        writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR, QtJson::Json::serialize(result));
    }
}

void SearchServer::onSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback) {
    SearchPlugin *plugin = qobject_cast<SearchPlugin*>(sender());
    QHttpResponse *response = getResponse(plugin);

    if (response) {
        QVariantMap result;
        result["result"] = "settingsRequest";
        result["title"] = title;
        result["settings"] = settings;
        result["method"] = callback;
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(result));
    }
}

void SearchServer::onResponseDone() {
    if (QHttpResponse *response = qobject_cast<QHttpResponse*>(sender())) {
        removeResponse(response);
    }
}
