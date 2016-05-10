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

#include "stringmodel.h"

StringModel::StringModel(QObject *parent) :
    QAbstractListModel(parent),
    m_alignment(Qt::AlignLeft | Qt::AlignVCenter)
{
    m_roles[NameRole] = "name";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> StringModel::roleNames() const {
    return m_roles;
}
#endif

int StringModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

Qt::Alignment StringModel::textAlignment() const {
    return m_alignment;
}

void StringModel::setTextAlignment(Qt::Alignment align) {
    if (align != textAlignment()) {
        m_alignment = align;
        emit textAlignmentChanged();
        
        if (!m_items.isEmpty()) {
            emit dataChanged(index(0), index(m_items.size() - 1));
        }
    }
}

QVariant StringModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
        return QVariant();
    }
    
    switch (section) {
    case 0:
        return tr("Name");
    default:
        return QVariant();
    }
}

QVariant StringModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    
    switch (role) {
    case NameRole:
        return m_items.at(index.row());
    case Qt::TextAlignmentRole:
        return QVariant(textAlignment());
    default:
        return QVariant();
    }
}

QMap<int, QVariant> StringModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    map[NameRole] = data(index, NameRole);
    
    return map;
}

bool StringModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()) {
        return false;
    }
    
    switch (role) {
    case NameRole:
        m_items.replace(index.row(), value.toString());
        break;
    default:
        return false;
    }
    
    emit dataChanged(index, index);
    return true;
}

bool StringModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) {
    if (roles.isEmpty()) {
        return false;
    }
    
    QMapIterator<int, QVariant> iterator(roles);
    
    while (iterator.hasNext()) {
        iterator.next();
        
        if (!setData(index, iterator.value(), iterator.key())) {
            return false;
        }
    }
    
    return true;
}

QVariant StringModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap StringModel::itemData(int row) const {
    QVariantMap map;
    map["name"] = data(row, "name");
    
    return map;
}

bool StringModel::setData(int row, const QVariant &value, const QByteArray &role) {
    return setData(index(row), value, m_roles.key(role));
}

bool StringModel::setItemData(int row, const QVariantMap &roles) {
    if (roles.isEmpty()) {
        return false;
    }
    
    QMapIterator<QString, QVariant> iterator(roles);
    
    while (iterator.hasNext()) {
        iterator.next();
        
        if (!setData(row, iterator.value(), iterator.key().toUtf8())) {
            return false;
        }
    }
    
    return true;
}

QModelIndexList StringModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                      Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int StringModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void StringModel::append(const QString &name) {
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items << name;
    endInsertRows();
    emit countChanged(rowCount());
}

void StringModel::insert(int row, const QString &name) {
    if ((row < 0) || (row >= m_items.size())) {
        append(name);
    }
    else {
        beginInsertRows(QModelIndex(), row, row);
        m_items.insert(row, name);
        endInsertRows();
        emit countChanged(rowCount());
    }
}

bool StringModel::remove(int row) {
    if ((row >= 0) && (row < m_items.size())) {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.removeAt(row);
        endRemoveRows();
        emit countChanged(rowCount());
        
        return true;
    }
    
    return false;
}

void StringModel::clear() {
    if (!m_items.isEmpty()) {
        beginResetModel();
        m_items.clear();
        endResetModel();
        emit countChanged(0);
    }
}
