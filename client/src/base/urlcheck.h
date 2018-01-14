/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef URLCHECK_H
#define URLCHECK_H

#include <QList>
#include <QMetaType>

struct UrlCheck
{
    UrlCheck() :
        checked(false),
        ok(false)
    {
    }
    
    UrlCheck(const QString &u) :
        url(u),
        checked(false),
        ok(false)
    {
    }
    
    QString url;
    bool checked;
    bool ok;
};

typedef QList<UrlCheck> UrlCheckList;

Q_DECLARE_METATYPE(UrlCheck)
Q_DECLARE_METATYPE(UrlCheckList)

#endif // URLCHECK_H
