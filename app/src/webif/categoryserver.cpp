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

#include "categoryserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "utils.h"

void CategoryServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QString method = request->path().mid(request->path().lastIndexOf("/") + 1).toLower();

    if (method == "addcategory") {
        // Add category
        if (request->method() == QHttpRequest::HTTP_POST) {
            QVariantMap category = QtJson::Json::parse(request->body()).toMap();
            const QString name = category.value("name").toString();
            const QString path = category.value("path").toString();

            if ((!name.isEmpty()) && (!path.isEmpty())) {
                // OK
                category = Qdl::addCategory(name, path);
                const QByteArray json = QtJson::Json::serialize(category);
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
    else if (method == "getcategories") {
        // Get categories
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            const QVariantList categories = Qdl::getCategories();
            const QByteArray json = QtJson::Json::serialize(categories);
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
    else if (method == "getcategory") {
        // Get category
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString name = Utils::urlQueryItemValue(request->url(), "name");

            if (!name.isEmpty()) {
                const QVariantMap category = Qdl::getCategory(name);

                if (!category.isEmpty()) {
                    // OK
                    const QByteArray json = QtJson::Json::serialize(category);
                    response->setHeader("Content-Type", "application/json");
                    response->setHeader("Content-Length", QString::number(json.size()));
                    response->writeHead(QHttpResponse::STATUS_OK);
                    response->end(json);
                }
                else {
                    // Not found
                    response->writeHead(QHttpResponse::STATUS_NOT_FOUND);
                    response->end();
                }
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
    else if (method == "removecategory") {
        // Remove category
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString name = Utils::urlQueryItemValue(request->url(), "name");

            if ((!name.isEmpty()) && (Qdl::removeCategory(name))) {
                // OK
                response->writeHead(QHttpResponse::STATUS_OK);
                response->end();
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
    else {
        // Bad request
        response->writeHead(QHttpResponse::STATUS_BAD_REQUEST);
        response->end();
    }
}
