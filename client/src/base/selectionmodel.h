/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef SELECTIONMODEL_H
#define SELECTIONMODEL_H

#include <QAbstractListModel>

class SelectionModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment
               NOTIFY textAlignmentChanged)
    
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ValueRole = Qt::UserRole
    };
    
    explicit SelectionModel(QObject *parent = 0);
        
#if QT_VERSION >= 0x050000
    virtual QHash<int, QByteArray> roleNames() const;
#endif
    
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    
    Qt::Alignment textAlignment() const;
    void setTextAlignment(Qt::Alignment align);

    virtual QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal,
                                int role = Qt::DisplayRole) const;
    
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    
    Q_INVOKABLE virtual QVariant data(int row, const QByteArray &role) const;
    Q_INVOKABLE virtual QVariantMap itemData(int row) const;
    Q_INVOKABLE virtual bool setData(int row, const QVariant &value, const QByteArray &role);
    Q_INVOKABLE virtual bool setItemData(int row, const QVariantMap &roles);
    
    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
                                  Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;
    Q_INVOKABLE virtual int match(int start, const QByteArray &role, const QVariant &value,
                                  int flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;    

public Q_SLOTS:
    virtual void append(const QString &name, const QVariant &value);
    virtual void insert(int row, const QString &name, const QVariant &value);
    virtual bool remove(int row);
    
    void clear();
    
Q_SIGNALS:
    void countChanged(int count);
    void textAlignmentChanged();
    
protected:
    QList< QPair<QString, QVariant> > m_items;
    
    QHash<int, QByteArray> m_roles;
    
    Qt::Alignment m_alignment;
};

#endif // SELECTIONMODEL_H
