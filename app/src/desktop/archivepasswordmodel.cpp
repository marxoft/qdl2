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

#include "archivepasswordmodel.h"
#include "settings.h"

ArchivePasswordModel::ArchivePasswordModel(QObject *parent) :
    StringModel(parent)
{
    reload();
}

bool ArchivePasswordModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (StringModel::setData(index, value, role)) {
        Settings::setArchivePasswords(m_items);
        return true;
    }

    return false;
}

void ArchivePasswordModel::append(const QString &name) {
    StringModel::append(name);
    Settings::setArchivePasswords(m_items);
}

void ArchivePasswordModel::insert(int row, const QString &name) {
    StringModel::insert(row, name);
    Settings::setArchivePasswords(m_items);
}

bool ArchivePasswordModel::remove(int row) {
    if (StringModel::remove(row)) {
        Settings::setArchivePasswords(m_items);
        return true;
    }

    return false;
}

void ArchivePasswordModel::reload() {
    clear();
    
    foreach (const QString &password, Settings::archivePasswords()) {
        StringModel::append(password);
    }
}
