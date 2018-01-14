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

#include "fileserver.h"
#include "definitions.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include <QFile>

void FileServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    QString filePath = request->path();
    const QString dir = filePath.left(filePath.lastIndexOf("/") + 1);

    if (!WEB_INTERFACE_ALLOWED_PATHS.contains(dir)) {
        filePath = filePath.mid(filePath.indexOf("/") + 1);
        filePath.prepend(WEB_INTERFACE_PATH);
    }

    if (QFile::exists(filePath)) {
        QFile file(filePath);

        if (file.open(QFile::ReadOnly)) {
            // OK
            response->setHeader("Content-Length", QString::number(file.size()));
            response->writeHead(QHttpResponse::STATUS_OK);
            response->end(file.readAll());
            file.close();
        }
        else {
            // Internal server error
            response->writeHead(QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
            response->end();
        }
    }
    else {
        // Not found
        response->writeHead(QHttpResponse::STATUS_NOT_FOUND);
        response->end();
    }
}
