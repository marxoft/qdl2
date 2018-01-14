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

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QVariantMap>

class Utils : public QObject
{
    Q_OBJECT

public:
    explicit Utils(QObject *parent = 0);

public Q_SLOTS:
    static QString createId();

    static QString formatBytes(qint64 bytes);
    static QString formatMSecs(qint64 secs);
    static QString formatSecs(qint64 secs);

    static bool isArchive(const QString &fileName);
    static bool isSplitArchive(const QString &fileName);
    static bool belongsToArchive(const QString &fileName, const QString &archiveFileName);

    static QString getSanitizedFileName(const QString &fileName);
    static QString getSaveDirectory(const QString &directory);
    static QString getSaveFileName(const QString &fileName, const QString &outputDirectory);

    static QList< QPair<QString, QString> > urlQueryItems(const QUrl &url);
    static QVariantMap urlQueryItemMap(const QUrl &url);
    static QString mapToUrlQuery(const QVariantMap &map);
    static QString urlQueryItemValue(const QUrl &url, const QString &queryItem,
                                     const QString &defaultValue = QString());
};

#endif // UTILS_H
