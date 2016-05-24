/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef ARCHIVEPASSWORDMODEL_H
#define ARCHIVEPASSWORDMODEL_H

#include "stringmodel.h"

class ArchivePasswordModel : public StringModel
{
    Q_OBJECT

public:
    explicit ArchivePasswordModel(QObject *parent = 0);
    
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    Q_INVOKABLE virtual void append(const QString &name);
    Q_INVOKABLE virtual void insert(int row, const QString &name);
    Q_INVOKABLE virtual bool remove(int row);

public:
    void reload();
};

#endif // ARCHIVEPASSWORDMODEL_H
