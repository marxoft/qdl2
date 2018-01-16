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

#ifndef TRANSFERMODEL_H
#define TRANSFERMODEL_H

#include "transferitem.h"
#include <QAbstractItemModel>

class TransferModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(int activeTransfers READ activeTransfers NOTIFY activeTransfersChanged)
    Q_PROPERTY(bool autoReloadEnabled READ autoReloadEnabled WRITE setAutoReloadEnabled
               NOTIFY autoReloadEnabledChanged)
    Q_PROPERTY(int totalSpeed READ totalSpeed NOTIFY totalSpeedChanged)
    Q_PROPERTY(QString totalSpeedString READ totalSpeedString NOTIFY totalSpeedChanged)

public:
    ~TransferModel();

    static TransferModel* instance();

#if QT_VERSION >= 0x050000
    virtual QHash<int, QByteArray> roleNames() const;
#endif

    virtual Qt::DropActions supportedDropActions() const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual QStringList mimeTypes() const;

    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                              const QModelIndex &parent);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE QVariant modelIndex(int row, int column, const QVariant &parent = QVariant()) const;
    
    virtual QModelIndex parent(const QModelIndex &child) const;
    Q_INVOKABLE QVariant parentModelIndex(const QVariant &child) const;

    Q_INVOKABLE virtual QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal,
                                            int role = Qt::DisplayRole) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, const QByteArray &roleName) const;
    Q_INVOKABLE virtual QVariant data(const QVariant &index, const QByteArray &roleName) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual bool setData(const QModelIndex &index, const QVariant &value, const QByteArray &roleName);
    Q_INVOKABLE virtual bool setData(const QVariant &index, const QVariant &value, const QByteArray &roleName);

    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    virtual QVariantMap itemDataWithRoleNames(const QModelIndex &index) const;
    Q_INVOKABLE virtual QVariantMap itemDataWithRoleNames(const QVariant &index) const;

    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &data);
    virtual bool setItemData(const QModelIndex &index, const QVariantMap &data);
    Q_INVOKABLE virtual bool setItemData(const QVariant &index, const QVariantMap &data);

    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                          const QModelIndex &destinationParent, int destinationRow);
    Q_INVOKABLE virtual bool moveRows(const QVariant &sourceParent, int sourceRow, int count,
                                      const QVariant &destinationParent, int destinationRow);

    int activeTransfers() const;

    bool autoReloadEnabled() const;

    int totalSpeed() const;
    QString totalSpeedString() const;

    TransferItem* get(const QModelIndex &index) const;
    Q_INVOKABLE TransferItem *get(const QVariant &index) const;

public Q_SLOTS:
    void setAutoReloadEnabled(bool enabled);

    void append(const QString &url, const QString &requestMethod = QString("GET"),
                const QVariantMap &requestHeaders = QVariantMap(), const QString &postData = QString(),
                const QString &category = QString(), bool createSubfolder = false,
                int priority = TransferItem::NormalPriority, const QString &customCommand = QString(),
                bool overrideGlobalCommand = false, bool startAutomatically = false);
    void append(const QStringList &urls, const QString &requestMethod = QString("GET"),
                const QVariantMap &requestHeaders = QVariantMap(), const QString &postData = QString(),
                const QString &category = QString(), bool createSubfolder = false,
                int priority = TransferItem::NormalPriority, const QString &customCommand = QString(),
                bool overrideGlobalCommand = false, bool startAutomatically = false);
    
    void reload();
    void queue();
    void pause();

    bool reload(const QModelIndex &index);
    bool queue(const QModelIndex &index);
    bool pause(const QModelIndex &index);
    bool cancel(const QModelIndex &index, bool deleteFiles = false);

private Q_SLOTS:
    void onPackageDataChanged(TransferItem *package, int role);
    void onTransferDataChanged(TransferItem *transfer, int role);

    void onPackageStatusChanged(TransferItem *package);
    void onTransferStatusChanged(TransferItem *transfer);

    void onPackageError(TransferItem *package, const QString &errorString);
    void onTransferError(TransferItem *transfer, const QString &errorString);

    void onPackagesLoaded();
    void onPackageLoaded(TransferItem *package);
    
    void onAppendRequestFinished(Request *request);
    void onMoveRequestFinished(Request *request);

Q_SIGNALS:
    void activeTransfersChanged(int active);
    void autoReloadEnabledChanged(bool enabled);
    void totalSpeedChanged(int speed);

    void captchaRequest(TransferItem *transfer);
    void settingsRequest(TransferItem *transfer);

    void error(const QString &errorString);

private:
    TransferModel();

    void addActiveTransfer(TransferItem *transfer);
    void removeActiveTransfer(TransferItem *transfer);

    static TransferModel *self;

    static const QString MIME_TYPE;

    TransferItem *m_packages;

    QList<TransferItem*> m_activeTransfers;

    bool m_autoReloadEnabled;
};

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(QModelIndex)
#endif

#endif // TRANSFERMODEL_H
