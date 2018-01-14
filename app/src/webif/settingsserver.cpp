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

#include "settingsserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"

void SettingsServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
     const QString method = request->path().mid(request->path().lastIndexOf("/") + 1).toLower();   

     if (method == "getsettings") {
         // Get settings
         if (request->method() == QHttpRequest::HTTP_GET) {
             // OK
             const QVariantMap settings = Qdl::getSettings();
             const QByteArray json = QtJson::Json::serialize(settings);
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
     else if (method == "setsettings") {
         // Set settings
         if (request->method() == QHttpRequest::HTTP_PUT) {
             const QVariantMap settings = QtJson::Json::parse(request->body()).toMap();

             if ((!settings.isEmpty()) && (Qdl::setSettings(settings))) {
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
