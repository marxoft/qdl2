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

#ifndef SETTING_H
#define SETTING_H

#include <QList>
#include <QVariant>

struct Setting
{
    enum ValueType {
        Boolean = 0,
        Group,
        Integer,
        List,
        Password,
        Text
    };
    
    Setting(ValueType t, const QString &l, const QString &k, const QVariant &v) :
        type(t),
        label(l),
        key(k),
        value(v)
    {
    }

    ValueType type;

    QString label;
    QString key;

    QVariant value;

    QList<Setting> settings; // For Group type
};

typedef QList<Setting> SettingList;

#endif // SETTING_H
