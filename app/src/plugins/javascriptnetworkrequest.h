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

#ifndef JAVASCRIPTNETWORKREQUEST_H
#define JAVASCRIPTNETWORKREQUEST_H

#include <QObject>
#include <QScriptable>
#include <QNetworkRequest>

class JavaScriptNetworkRequest : public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(QVariantMap headers READ headers WRITE setHeaders)
    
public:
    explicit JavaScriptNetworkRequest(QObject *parent = 0);
    
    QString url() const;
    void setUrl(const QString &u);
    
    QVariantMap headers() const;
    void setHeaders(const QVariantMap &h);

public Q_SLOTS:
    QVariant header(const QString &name) const;
    void setHeader(const QString &name, const QVariant &value);
};

Q_DECLARE_METATYPE(QNetworkRequest*)

#endif // JAVASCRIPTNETWORKREQUEST_H
