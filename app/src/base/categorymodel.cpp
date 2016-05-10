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

#include "categorymodel.h"
#include "categories.h"

CategoryModel::CategoryModel(QObject *parent) :
    SelectionModel(parent)
{
    reload();
    connect(Categories::instance(), SIGNAL(changed()), this, SLOT(reload()));
}

QVariant CategoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
        return QVariant();
    }
    
    switch (section) {
    case 0:
        return tr("Name");
    case 1:
        return tr("Path");
    default:
        return QVariant();
    }
}

bool CategoryModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == NameRole) {
        return Categories::add(value.toString(), index.data(ValueRole).toString());
    }

    return Categories::add(index.data(NameRole).toString(), value.toString());
}

void CategoryModel::append(const QString &name, const QVariant &value) {
    Categories::add(name, value.toString());
}

void CategoryModel::insert(int, const QString &name, const QVariant &value) {
    Categories::add(name, value.toString());
}

bool CategoryModel::remove(int row) {
    return Categories::remove(data(row, "name").toString());
}

void CategoryModel::reload() {
    clear();

    const CategoryList categories = Categories::get();

    for (int i = 0; i < categories.size(); i++) {
        SelectionModel::append(categories.at(i).name, categories.at(i).path);
    }
}
