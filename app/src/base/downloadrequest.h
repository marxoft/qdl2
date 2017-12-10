/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DOWNLOADREQUEST_H
#define DOWNLOADREQUEST_H

#include <QList>
#include <QMetaType>
#include <QNetworkRequest>

struct DownloadRequest
{
    DownloadRequest()
    {
    }

    DownloadRequest(const QString &f, const QNetworkRequest &r, const QByteArray &m = QByteArray("GET"),
            const QByteArray &d = QByteArray()) :
        fileName(f),
        request(r),
        method(m),
        data(d)
    {
    }
    
    QString fileName;
    QNetworkRequest request;
    QByteArray method;
    QByteArray data;
};

typedef QList<DownloadRequest> DownloadRequestList;

Q_DECLARE_METATYPE(DownloadRequest)
Q_DECLARE_METATYPE(DownloadRequestList)

#endif // DOWNLOADREQUEST_H
