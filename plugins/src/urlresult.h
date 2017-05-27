/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef URLRESULT_H
#define URLRESULT_H

#include <QList>
#include <QMetaType>
#include <QString>

/*!
 * Defines the result of a url check.
 *
 * \sa ServicePlugin
 */
struct UrlResult
{
    /*!
     * Contructs a UrlResult with an empty url and fileName.
     */
    UrlResult() :
        url(QString()),
        fileName(QString())
    {
    }
    
    /*!
     * Contructs a UrlResult with url \a u and fileName \a f.
     */
    UrlResult(const QString &u, const QString &f) :
        url(u),
        fileName(f)
    {
    }
    
    /*!
     * Contructs a copy of \a other.
     */
    UrlResult(const UrlResult &other) :
        url(other.url),
        fileName(other.fileName)
    {
    }
    
    /*!
     * The url of the result.
     *
     * The default value is an empty string.
     */
    QString url;
    
    /*!
     * The filename of the result.
     *
     * The default value is an empty string.
     */
    QString fileName;
};

/*!
 * Typedef for QList<UrlResult>.
 */
typedef QList<UrlResult> UrlResultList;

Q_DECLARE_METATYPE(UrlResult)
Q_DECLARE_METATYPE(UrlResultList)

#endif // URLRESULT_H
