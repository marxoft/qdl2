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

#include "transferlistmodel.h"
#include "logger.h"
#include "transferitem.h"

TransferListModel::TransferListModel(QObject *parent) :
    QAbstractListModel(parent),
    m_rootItem(0)
{
#if QT_VERSION < 0x050000
    setRoleNames(TransferItem::roleNames());
#endif
}

TransferItem* TransferListModel::rootItem() const {
    return m_rootItem;
}

void TransferListModel::setRootItem(TransferItem *item) {
    if (item != rootItem()) {
        if (m_rootItem) {
            disconnect(m_rootItem, SIGNAL(dataChanged(TransferItem*, int)),
                       this, SLOT(onRootDataChanged(TransferItem*, int)));
        }
    
        m_rootItem = item;

        if (item) {
            Logger::log(QString("TransferListModel::setRootItem(). Item type: %1, ID: %2, Name: %3")
                               .arg(item->itemTypeString())
                               .arg(item->data(TransferItem::IdRole).toString())
                               .arg(item->data(TransferItem::NameRole).toString()), Logger::HighVerbosity);
            connect(m_rootItem, SIGNAL(dataChanged(TransferItem*, int)),
                    this, SLOT(onRootDataChanged(TransferItem*, int)));
        }

        reload();
        emit rootItemChanged();
    }
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> TransferListModel::roleNames() const {
    return TransferItem::roleNames();
}
#endif

int TransferListModel::rowCount(const QModelIndex &) const {
    return m_rootItem ? m_rootItem->rowCount() : 0;
}

QVariant TransferListModel::data(const QModelIndex &index, int role) const {
    if (TransferItem *item = get(index.row())) {
        return item->data(role);
    }

    return QVariant();
}

QVariant TransferListModel::data(int row, const QByteArray &role) const {
    if (TransferItem *item = get(row)) {
        return item->data(role);
    }

    return QVariant();
}

bool TransferListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (TransferItem *item = get(index.row())) {
        return item->setData(role, value);
    }

    return false;
}

bool TransferListModel::setData(int row, const QVariant &value, const QByteArray &role) {
    if (TransferItem *item = get(row)) {
        return item->setData(role, value);
    }

    return false;
}

QMap<int, QVariant> TransferListModel::itemData(const QModelIndex &index) const {
    if (TransferItem *item = get(index.row())) {
        return item->itemData();
    }

    return QMap<int, QVariant>();
}

QVariantMap TransferListModel::itemData(int row) const {
    if (TransferItem *item = get(row)) {
        return item->itemDataWithRoleNames();
    }

    return QVariantMap();
}

bool TransferListModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &data) {
    if (TransferItem *item = get(index.row())) {
        return item->setItemData(data);
    }

    return false;
}

bool TransferListModel::setItemData(int row, const QVariantMap &data) {
    if (TransferItem *item = get(row)) {
        return item->setItemData(data);
    }

    return false;
}

QModelIndexList TransferListModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                         Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int TransferListModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), roleNames().key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

TransferItem* TransferListModel::get(int row) const {
    return m_rootItem ? m_rootItem->childItem(row) : 0;
}

void TransferListModel::reload() {
    beginResetModel();
    
    if (m_rootItem) {
        for (int i = 0; i < m_rootItem->rowCount(); i++) {
            if (TransferItem *item = m_rootItem->childItem(i)) {
                connect(item, SIGNAL(dataChanged(TransferItem*, int)), this, SLOT(onItemDataChanged(TransferItem*)),
                        Qt::UniqueConnection);
            }
        }        
    }

    endResetModel();
}

void TransferListModel::onItemDataChanged(TransferItem *item) {
    const QModelIndex idx = index(item->row(), 0, QModelIndex());
    emit dataChanged(idx, idx);
}

void TransferListModel::onRootDataChanged(TransferItem*, int role) {
    if (role == TransferItem::RowCountRole) {
        reload();
        emit countChanged(rowCount());
    }
}
