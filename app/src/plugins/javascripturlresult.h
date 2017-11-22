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

#ifndef JAVASCRIPTURLRESULT_H
#define JAVASCRIPTURLRESULT_H

#include "urlresult.h"
#include <QObject>
#include <QScriptable>

class JavaScriptUrlResult : public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)

public:
    explicit JavaScriptUrlResult(QObject *parent = 0);
    
    QString url() const;
    void setUrl(const QString &u);
    
    QString fileName() const;
    void setFileName(const QString &f);
};

Q_DECLARE_METATYPE(UrlResult*)

#endif // JAVASCRIPTURLRESULT_H
