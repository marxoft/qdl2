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

#include "transferserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "utils.h"

void TransferServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QString method = request->path().mid(request->path().lastIndexOf("/") + 1).toLower();

    if (method == "addtransfer") {
        // Add transfer
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
            const QString url = properties.value("url").toString();

            if (!url.isEmpty()) {
                // OK
                const QString method = properties.value("requestMethod", "GET").toString();
                const QVariantMap headers = properties.value("requestHeaders").toMap();
                const QString data = properties.value("postData").toString();
                const QString category = properties.value("category").toString();
                const bool createSubfolder = properties.value("createSubfolder", false).toBool();
                const int priority = properties.value("priority", TransferItem::NormalPriority).toInt();
                const QString customCommand = properties.value("customCommand").toString();
                const bool overrideGlobalCommand = properties.value("customCommandOverrideEnabled", false).toBool();
                const QVariantMap transfer = Qdl::addTransfer(url, method, headers, data, category, createSubfolder,
                        priority, customCommand, overrideGlobalCommand);
                const QByteArray json = QtJson::Json::serialize(transfer);
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
    else if (method == "addtransfers") {
        // Add transfers
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
            const QStringList urls = properties.value("urls").toStringList();

            if (!urls.isEmpty()) {
                // OK
                const QString method = properties.value("requestMethod", "GET").toString();
                const QVariantMap headers = properties.value("requestHeaders").toMap();
                const QString data = properties.value("postData").toString();
                const QString category = properties.value("category").toString();
                const bool createSubfolder = properties.value("createSubfolder", false).toBool();
                const int priority = properties.value("priority", TransferItem::NormalPriority).toInt();
                const QString customCommand = properties.value("customCommand").toString();
                const bool overrideGlobalCommand = properties.value("customCommandOverrideEnabled", false).toBool();
                const QVariantList transfers = Qdl::addTransfers(urls, method, headers, data, category,
                        createSubfolder, priority, customCommand, overrideGlobalCommand);
                const QByteArray json = QtJson::Json::serialize(transfers);
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
    else if (method == "gettransfers") {
        // Get transfers
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            const int offset = Utils::urlQueryItemValue(request->url(), "offset", "0").toInt();
            const int limit = Utils::urlQueryItemValue(request->url(), "limit", "-1").toInt();
            const QVariantList transfers = Qdl::getTransfers(offset, limit);
            const QByteArray json = QtJson::Json::serialize(transfers);
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
    else if (method == "gettransfersstatus") {
        // Get transfers status
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            const QVariantMap status = Qdl::getTransfersStatus();
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
    else if (method == "gettransfer") {
        // Get transfer
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");

            if (!id.isEmpty()) {
                const QVariantMap transfer = Qdl::getTransfer(id);

                if (!transfer.isEmpty()) {
                    // OK
                    const QByteArray json = QtJson::Json::serialize(transfer);
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
    else if (method == "searchtransfers") {
        // Search transfers
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString property = Utils::urlQueryItemValue(request->url(), "property");

            if (!property.isEmpty()) {
                // OK
                const QString value = Utils::urlQueryItemValue(request->url(), "value");
                const int offset = Utils::urlQueryItemValue(request->url(), "offset", "0").toInt();
                const int limit = Utils::urlQueryItemValue(request->url(), "limit", "-1").toInt();
                const QVariantList transfers = Qdl::searchTransfers(property, value, offset, limit);
                const QByteArray json = QtJson::Json::serialize(transfers);
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
    else if (method == "settransferproperties") {
        // Set transfer properties
        if (request->method() == QHttpRequest::HTTP_PUT) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");

            if (!id.isEmpty()) {
                const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();

                if (Qdl::setTransferProperties(id, properties)) {
                    // OK
                    const QVariantMap transfer = Qdl::getTransfer(id);
                    const QByteArray json = QtJson::Json::serialize(transfer);
                    response->setHeader("Content-Type", "application/json");
                    response->setHeader("Content-Length", QString::number(json.size()));
                    response->writeHead(QHttpResponse::STATUS_OK);
                    response->end(json);
                    return;
                }
            }

            // Bad request
            response->writeHead(QHttpResponse::STATUS_BAD_REQUEST);
            response->end();
        }
        else {
            // Method not allowed
            response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            response->end();
        }
    }
    else if (method == "starttransfer") {
        // Start transfer
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");

            if ((!id.isEmpty()) && (Qdl::startTransfer(id))) {
                // OK
                const QVariantMap transfer = Qdl::getTransfer(id);
                const QByteArray json = QtJson::Json::serialize(transfer);
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
    else if (method == "pausetransfer") {
        // Pause transfer
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");

            if ((!id.isEmpty()) && (Qdl::pauseTransfer(id))) {
                // OK
                const QVariantMap transfer = Qdl::getTransfer(id);
                const QByteArray json = QtJson::Json::serialize(transfer);
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
    else if (method == "removetransfer") {
        // Remove transfer
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");
            const bool deleteFiles = Utils::urlQueryItemValue(request->url(), "deleteFiles", "false") == "true";

            if ((!id.isEmpty()) && (Qdl::removeTransfer(id, deleteFiles))) {
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
    else if (method == "movetransfers") {
        // Move transfer
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString sourceParentId = Utils::urlQueryItemValue(request->url(), "sourceParentId");
            const QString destinationParentId = Utils::urlQueryItemValue(request->url(), "destinationParentId");
            const int sourceRow = Utils::urlQueryItemValue(request->url(), "sourceRow", "0").toInt();
            const int count = Utils::urlQueryItemValue(request->url(), "count", "1").toInt();
            const int destinationRow = Utils::urlQueryItemValue(request->url(), "destinationRow", "0").toInt();

            if (Qdl::moveTransfers(sourceParentId, sourceRow, count, destinationParentId, destinationRow)) {
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
    else if (method == "starttransfers") {
        // Start transfers
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            Qdl::startTransfers();
            response->writeHead(QHttpResponse::STATUS_OK);
            response->end();
        }
        else {
            // Method not allowed
            response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            response->end();
        }
    }
    else if (method == "pausetransfers") {
        // Pause transfers
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            Qdl::pauseTransfers();
            response->writeHead(QHttpResponse::STATUS_OK);
            response->end();
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
