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

#include "javascriptsearchresult.h"
#include <QScriptEngine>

JavaScriptSearchResult::JavaScriptSearchResult(QObject *parent) :
    QObject(parent)
{
}

QString JavaScriptSearchResult::name() const {
    if (const SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        return result->name;
    }
    
    return QString();
}

void JavaScriptSearchResult::setName(const QString &n) {
    if (SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        result->name = n;
    }
}

QString JavaScriptSearchResult::description() const {
    if (const SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        return result->description;
    }
    
    return QString();
}

void JavaScriptSearchResult::setDescription(const QString &d) {
    if (SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        result->description = d;
    }
}

QString JavaScriptSearchResult::url() const {
    if (const SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        return result->url;
    }
    
    return QString();
}

void JavaScriptSearchResult::setUrl(const QString &u) {
    if (SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        result->url = u;
    }
}
