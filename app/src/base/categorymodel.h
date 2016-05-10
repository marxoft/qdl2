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

#ifndef CATEGORYMODEL_H
#define CATEGORYMODEL_H

#include "selectionmodel.h"

class CategoryModel : public SelectionModel
{
    Q_OBJECT

public:
    explicit CategoryModel(QObject *parent = 0);

    virtual QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal,
                                int role = Qt::DisplayRole) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    Q_INVOKABLE virtual void append(const QString &name, const QVariant &value);
    Q_INVOKABLE virtual void insert(int row, const QString &name, const QVariant &value);
    Q_INVOKABLE virtual bool remove(int row);

public Q_SLOTS:
    void reload();
};
    
#endif // CATEGORYMODEL_H
