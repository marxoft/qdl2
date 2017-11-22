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

#ifndef SERVERRESPONSE_H
#define SERVERRESPONSE_H

#include "qhttpresponse.h"

inline void writeResponse(QHttpResponse *response, int responseCode, const QByteArray &data = QByteArray(),
        const QByteArray &contentType = QByteArray()) {
    if (!contentType.isEmpty()) {
        response->setHeader("Content-Type", contentType);
    }

    response->setHeader("Content-Length", QByteArray::number(data.size()));
    response->writeHead(responseCode);
    response->end(data);
}

#endif // SERVERRESPONSE_H
