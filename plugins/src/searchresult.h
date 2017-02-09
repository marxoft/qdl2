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
#include <QString>

/*!
 * Defines the result of a url search.
 *
 * \sa SearchPlugin
 */
struct SearchResult
{
    SearchResult(const QString &n, const QString &d, const QString &u) :
        name(n),
        description(d),
        url(u)
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
};

/*!
 * Typedef for QList<SearchResult>.
 */
typedef QList<SearchResult> SearchResultList;

#endif // SEARCHRESULT_H
