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

#include "utils.h"
#include <QFile>
#include <QRegExp>
#include <QUuid>
#include <QUrl>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

static const double KB = 1024;
static const double MB = KB * 1024;
static const double GB = MB * 1024;

Utils::Utils(QObject *parent) :
    QObject(parent)
{
}

QString Utils::createId() {
    const QString uuid = QUuid::createUuid().toString();
    return uuid.mid(1, uuid.size() - 2);
}

QString Utils::formatBytes(qint64 bytes) {
    if (bytes <= 0) {
        return QString("0B");
    }
    
    if (bytes > GB) {
        return QString::number(bytes / GB, 'f', 2) + "GB";
    }
    
    if (bytes > MB) {
        return QString::number(bytes / MB, 'f', 2) + "MB";
    }
    
    if (bytes > KB) {
        return QString::number(bytes / KB, 'f', 2) + "KB";
    }
    
    return QString::number(bytes) + "B";
}

QString Utils::formatMSecs(qint64 msecs) {    
    return msecs > 0 ? formatSecs(msecs / 1000) : QString("--:--");
}

QString Utils::formatSecs(qint64 secs) {    
    return secs > 0 ? QString("%1:%2").arg(secs / 60, 2, 10, QChar('0')).arg(secs % 60, 2, 10, QChar('0'))
                    : QString("--:--");
}

bool Utils::isArchive(const QString &fileName) {
    const QString suffix = fileName.mid(fileName.lastIndexOf(".") + 1).toLower();
    return (suffix == "rar") || (suffix == "zip") || (suffix == "gz") || (suffix == "bz") || (suffix == "7z");
}

bool Utils::isSplitArchive(const QString &fileName) {
    return fileName.indexOf(QRegExp("\\.part\\d+\\.(rar|zip)$", Qt::CaseInsensitive)) != -1;
}

bool Utils::belongsToArchive(const QString &fileName, const QString &archiveFileName) {
    return fileName.indexOf(QRegExp(QString("%1\\.part\\d+\\.%2")
                           .arg(archiveFileName.left(archiveFileName.lastIndexOf(".part")))
                           .arg(archiveFileName.mid(archiveFileName.lastIndexOf(".") + 1)))) == 0;
}

QString Utils::getSanitizedFileName(const QString &fileName) {
    return QString(fileName).replace(QRegExp("[\\/\\\\\\|]"), "_");
}

QString Utils::getSaveFileName(const QString &fileName, const QString &outputDirectory) {
    QString newFileName = outputDirectory + fileName;

    if (!QFile::exists(newFileName)) {
        return newFileName;
    }
    
    int i = 1;
    int lastDot = fileName.lastIndexOf(".");

    if (lastDot == -1) {
        lastDot = fileName.size();
    }

    do {
        newFileName = QString("%1%2(%3)%4").arg(outputDirectory).arg(fileName.left(lastDot)).arg(i)
                                           .arg(fileName.mid(lastDot));
        i++;
    } while ((QFile::exists(newFileName)) && (i < 99));

    if (i == 100) {
        return QString();
    }

    return newFileName;
}

QList< QPair<QString, QString> > Utils::urlQueryItems(const QUrl &url) {
#if QT_VERSION >= 0x050000
    return QUrlQuery(url).queryItems();
#else
    return url.queryItems();
#endif
}

QVariantMap Utils::urlQueryItemMap(const QUrl &url) {
    QList< QPair<QString, QString> > queryItems = urlQueryItems(url);
    QVariantMap map;
    
    while (!queryItems.isEmpty()) {
        const QPair<QString, QString> &queryItem = queryItems.takeFirst();
        map[queryItem.first] = queryItem.second;
    }
    
    return map;
}

QString Utils::urlQueryItemValue(const QUrl &url, const QString &queryItem, const QString &defaultValue) {
#if QT_VERSION >= 0x050000
    const QUrlQuery query(url);
    return query.hasQueryItem(queryItem) ? query.queryItemValue(queryItem) : defaultValue;
#else
    return url.hasQueryItem(queryItem) ? url.queryItemValue(queryItem) : defaultValue;
#endif
}
