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

#include "packagelist.h"

PackageList::PackageList(QObject *parent) :
    TransferItem(parent)
{
}

QVariant PackageList::data(int role) const {
    switch (role) {
    case CountRole:
        return count();
    case ItemTypeRole:
        return itemType();
    case ItemTypeStringRole:
        return itemTypeString();
    default:
        return QVariant();
    }
}

QMap<int, QVariant> PackageList::itemData() const {
    QMap<int, QVariant> map;
    map[CountRole] = count();
    map[ItemTypeRole] = itemType();
    map[ItemTypeStringRole] = itemTypeString();
    return map;
}

QVariantMap PackageList::itemDataWithRoleNames() const {
    QVariantMap map;
    map[roleNames().value(CountRole)] = count();
    map[roleNames().value(ItemTypeRole)] = itemType();
    map[roleNames().value(ItemTypeStringRole)] = itemTypeString();
    return map;
}

TransferItem::ItemType PackageList::itemType() const {
    return TransferItem::PackageListType;
}

int PackageList::count() const {
    return m_packages.size();
}

int PackageList::indexOf(Package *package) const {
    return m_packages.indexOf(package);
}

Package* PackageList::at(int i) const {
    return ((i >= 0) && (i < m_packages.size())) ? m_packages.at(i) : 0;
}

void PackageList::append(Package *package) {
    m_packages.append(package);
    package->setParent(this);
    emit dataChanged(this, CountRole);
}

bool PackageList::insert(int i, Package *package) {
    if ((i >= 0) && (i <= m_packages.size())) {
        m_packages.insert(i, package);
        package->setParent(this);
        emit dataChanged(this, CountRole);
        return true;
    }

    return false;
}

bool PackageList::move(int from, int to) {
    if ((from != to) && (from >= 0) && (from < m_packages.size()) && (to >= 0) && (to < m_packages.size())) {
        m_packages.move(from, to);
        return true;
    }

    return false;
}

bool PackageList::remove(Package *package) {
    if (m_packages.removeOne(package)) {
        emit dataChanged(this, CountRole);
        return true;
    }

    return false;
}

Package* PackageList::takeAt(int i) {
    if ((i >= 0) && (i < m_packages.size())) {
        Package *package = m_packages.takeAt(i);
        emit dataChanged(this, CountRole);
        return package;
    }

    return 0;
}
