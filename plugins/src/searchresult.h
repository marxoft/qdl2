/*!
 * \file searchresult.h
 *
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

#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QList>
#include <QMetaType>
#include <QString>

/*!
 * Defines the result of a content search.
 *
 * \sa SearchPlugin
 */
struct SearchResult
{
    /*!
     * Contructs a SearchResult with an empty name, description and url.
     */
    SearchResult() :
        name(QString()),
        description(QString()),
        url(QString()),
        iconFilePath(QString())
    {
    }
    
    /*!
     * Contructs a SearchResult with name \a n, description \a d and url \a u.
     */
    SearchResult(const QString &n, const QString &d, const QString &u) :
        name(n),
        description(d),
        url(u),
        iconFilePath(QString())
    {
    }
    
    /*!
     * Contructs a copy of \a other.
     */
    SearchResult(const SearchResult &other) :
        name(other.name),
        description(other.description),
        url(other.url),
        iconFilePath(other.iconFilePath)
    {
    }
    
    /*!
     * The display name of the search result.
     */
    QString name;
    
    /*!
     * A description of the search result.
     *
     * Can be plain text or HTML.
     */
    QString description;

    /*!
     * The url of the search result.
     */
    QString url;
    
    /*!
     * \internal
     *
     * For internal use.
     */
    mutable QString iconFilePath;
};

/*!
 * Typedef for QList<SearchResult>.
 */
typedef QList<SearchResult> SearchResultList;

Q_DECLARE_METATYPE(SearchResult)
Q_DECLARE_METATYPE(SearchResultList)

#endif // SEARCHRESULT_H
