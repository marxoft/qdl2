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

#include "urlcheckserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"

bool UrlCheckServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.isEmpty()) || (parts.size() > 2) || (parts.first() != "urlcheck")) {
        return false;
    }

    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(Qdl::getUrlChecks()));
            return true;
        }

        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
            const QStringList urls = properties.value("urls").toStringList();
            
            if (urls.isEmpty()) {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            else {
                Qdl::addUrlChecks(urls, properties.value("category").toString());
                writeResponse(response, QHttpResponse::STATUS_CREATED);
            }

            return true;
        }

        if (request->method() == QHttpRequest::HTTP_DELETE) {
            Qdl::clearUrlChecks();
            writeResponse(response, QHttpResponse::STATUS_OK);
            return true;
        }
    }

    if (request->method() == QHttpRequest::HTTP_GET) {
        const QVariantMap data = Qdl::getUrlCheck(parts.at(1));

        if (!data.isEmpty()) {
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(data));
            return true;
        }

        return false;
    }

    if (request->method() == QHttpRequest::HTTP_DELETE) {
        if (Qdl::removeUrlCheck(parts.at(1))) {
            writeResponse(response, QHttpResponse::STATUS_OK);
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        }

        return true;
    }

    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}
