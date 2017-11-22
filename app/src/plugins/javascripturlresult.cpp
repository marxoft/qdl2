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

#include "javascripturlresult.h"
#include <QScriptEngine>

JavaScriptUrlResult::JavaScriptUrlResult(QObject *parent) :
    QObject(parent)
{
}

QString JavaScriptUrlResult::url() const {
    if (const UrlResult* result = qscriptvalue_cast<UrlResult*>(thisObject())) {
        return result->url;
    }
    
    return QString();
}

void JavaScriptUrlResult::setUrl(const QString &u) {
    if (UrlResult* result = qscriptvalue_cast<UrlResult*>(thisObject())) {
        result->url = u;
    }
}

QString JavaScriptUrlResult::fileName() const {
    if (const UrlResult* result = qscriptvalue_cast<UrlResult*>(thisObject())) {
        return result->fileName;
    }
    
    return QString();
}

void JavaScriptUrlResult::setFileName(const QString &f) {
    if (UrlResult* result = qscriptvalue_cast<UrlResult*>(thisObject())) {
        result->fileName = f;
    }
}
