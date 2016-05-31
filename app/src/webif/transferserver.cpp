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

#include "transferserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"
#include "transfermodel.h"
#include "utils.h"

bool TransferServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.isEmpty()) || (parts.size() > 2) || (parts.first() != "transfers")) {
        return false;
    }
    
    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const bool includeChildren = Utils::urlQueryItemValue(request->url(), "includeChildren") == "true";
            const int offset = qMax(0, Utils::urlQueryItemValue(request->url(), "offset").toInt());
            const int limit = Utils::urlQueryItemValue(request->url(), "limit").toInt();            
            writeResponse(response, QHttpResponse::STATUS_OK,
                          QtJson::Json::serialize(Qdl::getTransfers(offset, limit, includeChildren)));
            return true;
        }
        
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
            const QStringList urls = properties.value("urls").toStringList();
            const QString method = properties.value("requestMethod", "GET").toString();
            const QVariantMap headers = properties.value("requestHeaders").toMap();
            const QString data = properties.value("postData").toString();
            
            if (urls.isEmpty()) {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            else {
                Qdl::addTransfers(urls, method, headers, data);
                writeResponse(response, QHttpResponse::STATUS_CREATED);
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "search") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString property = Utils::urlQueryItemValue(request->url(), "property");
            const QString value = Utils::urlQueryItemValue(request->url(), "value");
            
            if ((property.isEmpty()) || (value.isEmpty())) {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_OK,
                QtJson::Json::serialize(Qdl::searchTransfers(property, value,
                                        Utils::urlQueryItemValue(request->url(), "hits", "1").toInt(),
                                        Utils::urlQueryItemValue(request->url(), "includeChildren") == "true")));
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "start") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");

            if (id.isEmpty()) {
                TransferModel::instance()->queue();
                writeResponse(response, QHttpResponse::STATUS_OK);
                return true;
            }
            
            if (Qdl::startTransfer(id)) {
                writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(Qdl::getTransfer(id)));
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "pause") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");

            if (id.isEmpty()) {
                TransferModel::instance()->pause();
                writeResponse(response, QHttpResponse::STATUS_OK);
                return true;
            }
            
            if (Qdl::pauseTransfer(id)) {
                writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(Qdl::getTransfer(id)));
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "remove") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");
            const bool deleteFiles = Utils::urlQueryItemValue(request->url(), "deleteFiles") == "true";

            if ((!id.isEmpty()) && (Qdl::removeTransfer(id, deleteFiles))) {
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

    if (parts.at(1) == "move") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString sourceId = Utils::urlQueryItemValue(request->url(), "sourceId");

            if ((!sourceId.isEmpty()) && (Qdl::moveTransfer(sourceId, Utils::urlQueryItemValue(request->url(),
                "destinationId"), Utils::urlQueryItemValue(request->url(), "destinationRow").toInt()))) {
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
    
    if (request->method() == QHttpRequest::HTTP_GET) {
        const QVariantMap data = Qdl::getTransfer(parts.at(1), Utils::urlQueryItemValue(request->url(),
                                                  "includeChildren") == "true");
        
        if (!data.isEmpty()) {
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(data));
            return true;
        }
        
        return false;
    }
    
    if (request->method() == QHttpRequest::HTTP_PUT) {
        const QString &id = parts.at(1);        
        const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
        
        if ((properties.isEmpty()) || (!Qdl::setTransferProperties(id, properties))) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(Qdl::getTransfer(id)));
        }

        return true;
    }
    
    if (request->method() == QHttpRequest::HTTP_DELETE) {
        const QString &id = parts.at(1);
        const bool deleteFiles = Utils::urlQueryItemValue(request->url(), "deleteFiles") == "true";

        if (Qdl::removeTransfer(id, deleteFiles)) {
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
