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

#include "settingsserver.h"
#include "json.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"
#include "utils.h"

bool SettingsServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    if (request->path() != "/settings") {
        return false;
    }
    
    if (request->method() == QHttpRequest::HTTP_GET) {
        const QStringList settings =
        Utils::urlQueryItemValue(request->url(), "settings").split(",", QString::SkipEmptyParts);

        if (settings.isEmpty()) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(Qdl::getSettings(settings)));
        }
        
        return true;
    }
    
    if (request->method() == QHttpRequest::HTTP_PUT) {
        const QVariantMap settings = QtJson::Json::parse(QString::fromUtf8(request->body())).toMap();
        
        if ((settings.isEmpty()) || (!Qdl::setSettings(settings))) {
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
