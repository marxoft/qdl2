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

#include "urlretrievalserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"

void UrlRetrievalServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QString method = request->path().mid(request->path().lastIndexOf("/") + 1).toLower();

    if (method == "addretrievals") {
        // Add URL retrievals
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
            const QStringList urls = properties.value("urls").toStringList();

            if (!urls.isEmpty()) {
                // OK
                const QString pluginId = properties.value("pluginId").toString();
                Qdl::addUrlRetrievals(urls, pluginId);
                const QVariantList checks = Qdl::getUrlChecks();
                const QByteArray json = QtJson::Json::serialize(checks);
                response->setHeader("Content-Type", "application/json");
                response->setHeader("Content-Length", QString::number(json.size()));
                response->writeHead(QHttpResponse::STATUS_OK);
                response->end(json);
            }
            else {
                // Bad request
                response->writeHead(QHttpResponse::STATUS_BAD_REQUEST);
                response->end();
            }
        }
        else {
            // Method not allowed
            response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            response->end();
        }
    }
    else if (method == "clearretrievals") {
        // Clear URL retrievals
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            Qdl::clearUrlRetrievals();
            response->writeHead(QHttpResponse::STATUS_OK);
            response->end();
        }
        else {
            // Method not allowed
            response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            response->end();
        }
    }
    else if (method == "getretrievals") {
        // Get URL retrievals
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            const QVariantList retrievals = Qdl::getUrlRetrievals();
            const QByteArray json = QtJson::Json::serialize(retrievals);
            response->setHeader("Content-Type", "application/json");
            response->setHeader("Content-Length", QString::number(json.size()));
            response->writeHead(QHttpResponse::STATUS_OK);
            response->end(json);
        }
        else {
            // Method not allowed
            response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            response->end();
        }
    }
    else if (method == "getstatus") {
        // Get status
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            const QVariantMap status = Qdl::getUrlRetrievalsStatus();
            const QByteArray json = QtJson::Json::serialize(status);
            response->setHeader("Content-Type", "application/json");
            response->setHeader("Content-Length", QString::number(json.size()));
            response->writeHead(QHttpResponse::STATUS_OK);
            response->end(json);
        }
        else {
            // Method not allowed
            response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            response->end();
        }
    }
    else {
        // Bad request
        response->writeHead(QHttpResponse::STATUS_BAD_REQUEST);
        response->end();
    }
}
