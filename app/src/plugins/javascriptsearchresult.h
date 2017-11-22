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

#ifndef JAVASCRIPTSEARCHRESULT_H
#define JAVASCRIPTSEARCHRESULT_H

#include "searchresult.h"
#include <QObject>
#include <QScriptable>

class JavaScriptSearchResult : public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QString url READ url WRITE setUrl)

public:
    explicit JavaScriptSearchResult(QObject *parent = 0);
    
    QString name() const;
    void setName(const QString &n);
    
    QString description() const;
    void setDescription(const QString &d);
    
    QString url() const;
    void setUrl(const QString &u);
};

Q_DECLARE_METATYPE(SearchResult*)

#endif // JAVASCRIPTSEARCHRESULT
