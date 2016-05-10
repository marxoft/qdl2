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

#include "servicepluginconfigserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"

bool ServicePluginConfigServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.isEmpty()) || (parts.size() > 3) || (parts.first() != "services")) {
        return false;
    }

    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            writeResponse(response, QHttpResponse::STATUS_OK,
                          QtJson::Json::serialize(Qdl::getServicePlugins()));
            return true;
        }
    }

    if (parts.size() == 2) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QVariantMap config = Qdl::getServicePlugin(parts.at(1));
            
            if (!config.isEmpty()) {
                writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(config));
                return true;
            }
            
            return false;
        }
    }

    if (parts.last() == "settings") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            writeResponse(response, QHttpResponse::STATUS_OK,
                          QtJson::Json::serialize(Qdl::getServicePluginSettings(parts.at(1))));
            return true;
        }

        if (request->method() == QHttpRequest::HTTP_PUT) {
            const QVariantMap properties = QtJson::Json::parse(QString::fromUtf8(request->body())).toMap();
            
            if ((properties.isEmpty()) || (!Qdl::setServicePluginSettings(parts.at(1), properties))) {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_OK);
            }
            
            return true;
        }
    }

    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}
