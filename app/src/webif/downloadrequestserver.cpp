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

#include "downloadrequestserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"

void DownloadRequestServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QString method = request->path().mid(request->path().lastIndexOf("/") + 1).toLower();

    if (method == "addrequests") {
        // Add download requests
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QStringList urls = QtJson::Json::parse(request->body()).toStringList();

            if (!urls.isEmpty()) {
                // OK
                Qdl::addDownloadRequests(urls);
                const QVariantList requests = Qdl::getDownloadRequests();
                const QByteArray json = QtJson::Json::serialize(requests);
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
    else if (method == "clearrequests") {
        // Clear download requests
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            Qdl::clearDownloadRequests();
            response->writeHead(QHttpResponse::STATUS_OK);
            response->end();
        }
        else {
            // Method not allowed
            response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            response->end();
        }
    }
    else if (method == "getrequests") {
        // Get download requests
        if (request->method() == QHttpRequest::HTTP_GET) {
            // OK
            const QVariantList requests = Qdl::getDownloadRequests();
            const QByteArray json = QtJson::Json::serialize(requests);
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
            const QVariantMap status = Qdl::getDownloadRequestsStatus();
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
    else if (method == "submitcaptcharesponse") {
        // Submit captcha response
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QString captcha = QString::fromUtf8(request->body());

            if (Qdl::submitDownloadRequestCaptchaResponse(captcha)) {
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
    else if (method == "submitsettingsresponse") {
        // Submit settings response
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantMap settings = QtJson::Json::parse(request->body()).toMap();

            if (Qdl::submitDownloadRequestSettingsResponse(settings)) {
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
