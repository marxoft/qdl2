/*
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

#include "categoryserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"

bool CategoryServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.isEmpty()) || (parts.size() > 2) || (parts.first() != "categories")) {
        return false;
    }

    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(Qdl::getCategories()));
            return true;
        }

        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
            const QString name = properties.value("name").toString();
            const QString path = properties.value("path").toString();

            if ((name.isEmpty()) || (path.isEmpty()) || (!Qdl::addCategory(name, path))) {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_CREATED);
            }

            return true;
        }

        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }

    if (request->method() == QHttpRequest::HTTP_GET) {
        const QVariantMap category = Qdl::getCategory(parts.at(1));

        if (category.isEmpty()) {
            return false;
        }

        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(category));
        return true;
    }

    if (request->method() == QHttpRequest::HTTP_PUT) {
        const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
        const QString path = properties.value("path").toString();

        if ((path.isEmpty()) || (!Qdl::addCategory(parts.at(1), path))) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_OK);
        }

        return true;
    }

    if (request->method() == QHttpRequest::HTTP_DELETE) {
        const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
        const QString name = properties.value("name").toString();

        if ((name.isEmpty()) || (!Qdl::removeCategory(name))) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_OK);
        }

        return true;
    }

    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}
