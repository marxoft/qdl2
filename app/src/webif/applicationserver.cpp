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

#include "applicationserver.h"
#include "qdl.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"
#include <QTimer>

bool ApplicationServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);

    if ((parts.size() != 2) || (parts.first() != "app")) {
        return false;
    }

    if (request->method() == QHttpRequest::HTTP_GET) {
        if (parts.at(1) == "showwindow") {
            Qdl::showWindow();
            writeResponse(response, QHttpResponse::STATUS_OK);
            return true;
        }

        if (parts.at(1) == "closewindow") {
            Qdl::closeWindow();
            writeResponse(response, QHttpResponse::STATUS_OK);
            return true;
        }

        if (parts.at(1) == "quit") {
            QTimer::singleShot(1000, Qdl::instance(), SLOT(quit()));
            writeResponse(response, QHttpResponse::STATUS_OK);
            return true;
        }

        return false;
    }

    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}
