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

#ifndef TRANSFERLISTMODEL_H
#define TRANSFERLISTMODEL_H

#include <QAbstractListModel>
#include <QPointer>

class TransferItem;

class TransferListModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(TransferItem* rootItem READ rootItem WRITE setRootItem NOTIFY rootItemChanged)
    
public:
    explicit TransferListModel(QObject *parent = 0);

    TransferItem* rootItem() const;
    void setRootItem(TransferItem *item);
    
#if QT_VERSION >= 0x050000
    virtual QHash<int, QByteArray> roleNames() const;
#endif
    
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    
    virtual QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE virtual QVariant data(int row, const QByteArray &role) const;
    
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    Q_INVOKABLE virtual bool setData(int row, const QVariant &value, const QByteArray &role);
    
    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE virtual QVariantMap itemData(int row) const;
    
    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &data);
    Q_INVOKABLE virtual bool setItemData(int row, const QVariantMap &data);
    
    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
                                  Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;
    Q_INVOKABLE virtual int match(int start, const QByteArray &role, const QVariant &value,
                                  int flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;

    Q_INVOKABLE TransferItem* get(int row) const;

public Q_SLOTS:
    void reload();

private Q_SLOTS:
    void onItemDataChanged(TransferItem *item);
    void onRootDataChanged(TransferItem *item, int role);
    
Q_SIGNALS:
    void countChanged(int count);
    void rootItemChanged();
    
private:
    QPointer<TransferItem> m_rootItem;
};

#endif // URLCHECKMODEL_H
