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

#ifndef PACKAGELIST_H
#define PACKAGELIST_H

#include "package.h"

class PackageList : public TransferItem
{
    Q_OBJECT

    Q_PROPERTY(int count READ count)

public:
    explicit PackageList(QObject *parent = 0);

    virtual QVariant data(int role) const;
    virtual QMap<int, QVariant> itemData() const;
    virtual QVariantMap itemDataWithRoleNames() const;

    virtual ItemType itemType() const;

    int count() const;
    int indexOf(Package *package) const;
    Package* at(int i) const;
    void append(Package *package);
    bool insert(int i, Package *package);
    bool move(int from, int to);
    bool remove(Package *package);
    Package* takeAt(int i);

private:
    QList<Package*> m_packages;
};

#endif // PACKAGELIST_H
