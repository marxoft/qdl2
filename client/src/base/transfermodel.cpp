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

#include "transfermodel.h"
#include "definitions.h"
#include "package.h"
#include "request.h"
#include "transfer.h"
#include "utils.h"
#include <QDataStream>
#include <QMimeData>

TransferModel* TransferModel::self = 0;

const QString TransferModel::MIME_TYPE("application/x-qdl2transfermodeldatalist");

TransferModel::TransferModel() :
    QAbstractItemModel(),
    m_packages(new TransferItem(this)),
    m_autoReloadEnabled(false)
{
#if QT_VERSION < 0x050000
    setRoleNames(TransferItem::roleNames());
#endif
    connect(m_packages, SIGNAL(loaded(TransferItem*)), this, SLOT(onPackagesLoaded()));
}

TransferModel::~TransferModel() {
    self = 0;
}

TransferModel* TransferModel::instance() {
    return self ? self : self = new TransferModel;
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> TransferModel::roleNames() const {
    return TransferItem::roleNames();
}
#endif

Qt::DropActions TransferModel::supportedDropActions() const {
    return Qt::MoveAction;
}

Qt::ItemFlags TransferModel::flags(const QModelIndex &index) const {
    switch (data(index, TransferItem::ItemTypeRole).toInt()) {
    case TransferItem::ListType:
        return Qt::ItemIsDropEnabled;
    case TransferItem::PackageType:
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    case TransferItem::TransferType:
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
    default:
        break;
    }
    
    return Qt::NoItemFlags;
}

QStringList TransferModel::mimeTypes() const {
    return QStringList() << MIME_TYPE;
}

QMimeData* TransferModel::mimeData(const QModelIndexList &indexes) const {
    if (indexes.isEmpty()) {
        return 0;
    }

    QMimeData *data = new QMimeData();
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);

    for (int i = 0; i < indexes.size(); i++) {
        const QModelIndex &index = indexes.at(i);
        
        if (index.column() == 0) {
            stream << index.row() << (index.parent().isValid() ? index.parent().row() : -1);
        }
    }
    
    data->setData(MIME_TYPE, encoded);
    return data;
}

bool TransferModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int,
                                 const QModelIndex &parent) {
    if ((!data) || (!data->hasFormat(MIME_TYPE)) || (action != Qt::MoveAction)) {
        return false;
    }

    const int max = rowCount(parent);
    
    if ((row < 0) || (row > max)) {
        row = max;
    }

    QByteArray encoded = data->data(MIME_TYPE);
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    
    while (!stream.atEnd()) {
        int r;
        int pr;
        stream >> r >> pr;

        if (!moveRows(pr == -1 ? QModelIndex() : index(pr, 0), r, 1, parent, row)) {
            return false;
        }
    }
    
    return true;
}

int TransferModel::rowCount(const QModelIndex &parent) const {
    if (const TransferItem *item = get(parent)) {
        return item->rowCount();
    }

    return 0;
}

int TransferModel::columnCount(const QModelIndex &) const {
    return 6;
}

QModelIndex TransferModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (const TransferItem *parentItem = get(parent)) {
        if (TransferItem *child = parentItem->childItem(row)) {
            return createIndex(row, column, child);
        }
    }

    return QModelIndex();
}

QVariant TransferModel::modelIndex(int row, int column, const QVariant &parent) const {
    return QVariant::fromValue(index(row, column, parent.value<QModelIndex>()));
}

QModelIndex TransferModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }

    if (const TransferItem *item = get(child)) {
        if (TransferItem *parentItem = item->parentItem()) {
            if (parentItem != m_packages) {
                return createIndex(parentItem->row(), 0, parentItem);
            }
        }
    }

    return QModelIndex();
}

QVariant TransferModel::parentModelIndex(const QVariant &child) const {
    return QVariant::fromValue(parent(child.value<QModelIndex>()));
}

QVariant TransferModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
        return QVariant();
    }
    
    switch (section) {
    case 0:
        return tr("Name");
    case 1:
        return tr("Category");
    case 2:
        return tr("Priority");
    case 3:
        return tr("Progress");
    case 4:
        return tr("Speed");
    case 5:
        return tr("Status");
    default:
        return QVariant();
    }
}

QVariant TransferModel::data(const QModelIndex &index, int role) const {
    if (const TransferItem *item = get(index)) {
        switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
            case 0:
                return item->data(TransferItem::NameRole);
            case 1:
                return item->data(TransferItem::CategoryRole);
            case 2:
                return item->data(TransferItem::PriorityStringRole);
            case 3:
                return item->data(TransferItem::ProgressStringRole);
            case 4:
                return item->data(TransferItem::SpeedStringRole);
            case 5:
                return item->data(TransferItem::StatusStringRole);
            default:
                return QVariant();
            }
        case Qt::DecorationRole:
            switch (index.column()) {
            case 0:
                if (item->data(TransferItem::ItemTypeRole) == TransferItem::TransferType) {
                    return item->data(TransferItem::PluginIconRole);
                }

                return QVariant();
            default:
                return QVariant();
            }
        default:
            return item->data(role);
        }
    }

    return QVariant();
}

QVariant TransferModel::data(const QModelIndex &index, const QByteArray &roleName) const {
    if (const TransferItem *item = get(index)) {
        return item->data(roleName);
    }

    return QVariant();
}

QVariant TransferModel::data(const QVariant &index, const QByteArray &roleName) const {
    if (const TransferItem *item = get(index)) {
        return item->data(roleName);
    }

    return QVariant();
}

bool TransferModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (TransferItem *item = get(index)) {
        if (item->setData(role, value)) {
            item->save();
            return true;
        }
    }

    return false;
}

bool TransferModel::setData(const QModelIndex &index, const QVariant &value, const QByteArray &roleName) {
    if (TransferItem *item = get(index)) {
        if (item->setData(roleName, value)) {
            item->save();
            return true;
        }
    }

    return false;
}

bool TransferModel::setData(const QVariant &index, const QVariant &value, const QByteArray &roleName) {
    if (TransferItem *item = get(index)) {
        if (item->setData(roleName, value)) {
            item->save();
            return true;
        }
    }

    return false;
}

QMap<int, QVariant> TransferModel::itemData(const QModelIndex &index) const {
    if (const TransferItem *item = get(index)) {
        return item->itemData();
    }

    return QMap<int, QVariant>();
}

QVariantMap TransferModel::itemDataWithRoleNames(const QModelIndex &index) const {
    if (const TransferItem *item = get(index)) {
        return item->itemDataWithRoleNames();
    }

    return QVariantMap();
}

QVariantMap TransferModel::itemDataWithRoleNames(const QVariant &index) const {
    if (const TransferItem *item = get(index)) {
        return item->itemDataWithRoleNames();
    }

    return QVariantMap();
}

bool TransferModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &data) {
    if (TransferItem *item = get(index)) {
        if (item->setItemData(data)) {
            item->save();
            return true;
        }
    }

    return false;
}

bool TransferModel::setItemData(const QModelIndex &index, const QVariantMap &data) {
    if (TransferItem *item = get(index)) {
        if (item->setItemData(data)) {
            item->save();
            return true;
        }
    }

    return false;
}

bool TransferModel::setItemData(const QVariant &index, const QVariantMap &data) {
    if (TransferItem *item = get(index)) {
        if (item->setItemData(data)) {
            item->save();
            return true;
        }
    }

    return false;
}

bool TransferModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                             const QModelIndex &destinationParent, int destinationChild) {
    if ((sourceRow < 0) || (sourceRow + count > rowCount(sourceParent))
        || (destinationChild < 0) || (destinationChild > rowCount(destinationParent))) {
        return false;
    }
    
    const int sourceParentType = data(sourceParent, TransferItem::ItemTypeRole).toInt();
    const int destinationParentType = data(destinationParent, TransferItem::ItemTypeRole).toInt();

    if ((sourceParentType != destinationParentType) || (sourceParentType == TransferItem::TransferType)
        || (destinationParentType == TransferItem::TransferType)) {
        return false;
    }

    if (TransferItem *oldParent = get(sourceParent)) {
        if (TransferItem *newParent = get(destinationParent)) {
            if (beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild)) {
                if (oldParent == newParent) {
                    for (int i = sourceRow; i < sourceRow + count; i++) {
                        oldParent->moveRow(sourceRow, destinationChild);
                    }
                }
                else {
                    for (int i = sourceRow; i < sourceRow + count; i++) {
                        if (TransferItem *item = oldParent->takeRow(sourceRow)) {
                            newParent->insertRow(destinationChild++, item);
                        }
                    }
                }

                endMoveRows();
                QVariantMap params;
                params["sourceParentId"] = oldParent->data(TransferItem::IdRole);
                params["sourceRow"] = sourceRow;
                params["count"] = count;
                params["destinationParentId"] = newParent->data(TransferItem::IdRole);
                params["destinationRow"] = destinationChild;
                Request *request = new Request(this);
                request->get("/transfers/moveTransfers", params);
                connect(request, SIGNAL(finished(Request*)), this, SLOT(onMoveRequestFinished(Request*)));
                return true;
            }
        }
    }

    return false;
}

bool TransferModel::moveRows(const QVariant &sourceParent, int sourceRow, int count,
                             const QVariant &destinationParent, int destinationChild) {
    return moveRows(sourceParent.value<QModelIndex>(), sourceRow, count, destinationParent.value<QModelIndex>(),
                    destinationChild);
}

int TransferModel::activeTransfers() const {
    return m_activeTransfers.size();
}

bool TransferModel::autoReloadEnabled() const {
    return m_autoReloadEnabled;
}

void TransferModel::setAutoReloadEnabled(bool enabled) {
    if (enabled != autoReloadEnabled()) {
        m_autoReloadEnabled = enabled;
        emit autoReloadEnabledChanged(enabled);

        for (int i = 0; i < m_packages->rowCount(); i++) {
            if (const TransferItem *package = m_packages->childItem(i)) {
                for (int j = 0; j < package->rowCount(); j++) {
                    if (TransferItem *transfer = package->childItem(j)) {
                        transfer->setAutoReloadEnabled((enabled)
                                && (transfer->data(TransferItem::StatusRole).toInt() >= TransferItem::WaitingInactive));
                    }
                }
            }
        }
    }
}

int TransferModel::totalSpeed() const {
    int speed = 0;

    foreach (TransferItem *transfer, m_activeTransfers) {
        speed += transfer->data(TransferItem::SpeedRole).toInt();
    }

    return speed;
}

QString TransferModel::totalSpeedString() const {
    return Utils::formatBytes(totalSpeed()) + "/s";
}

TransferItem* TransferModel::get(const QModelIndex &index) const {
    return index.isValid() ? static_cast<TransferItem*>(index.internalPointer()) : m_packages;
}

TransferItem* TransferModel::get(const QVariant &index) const {
    return get(index.value<QModelIndex>());
}

void TransferModel::append(const QString &url, const QString &requestMethod, const QVariantMap &requestHeaders,
                           const QString &postData, const QString &category, bool createSubfolder, int priority,
                           const QString &customCommand, bool overrideGlobalCommand, bool startAutomatically) {
    QVariantMap data;
    data["url"] = url;
    data["requestMethod"] = requestMethod;
    data["requestHeaders"] = requestHeaders;
    data["postData"] = postData;
    data["category"] = category;
    data["createSubfolder"] = createSubfolder;
    data["priority"] = priority;
    data["customCommand"] = customCommand;
    data["customCommandOverrideEnabled"] = overrideGlobalCommand;
    data["startAutomatically"] = startAutomatically;
    Request *request = new Request(this);
    request->post("/transfers/addTransfer", data);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onAppendRequestFinished(Request*)));
}

void TransferModel::append(const QStringList &urls, const QString &requestMethod, const QVariantMap &requestHeaders,
                           const QString &postData, const QString &category, bool createSubfolder, int priority,
                           const QString &customCommand, bool overrideGlobalCommand, bool startAutomatically) {
    QVariantMap data;
    data["urls"] = urls;
    data["requestMethod"] = requestMethod;
    data["requestHeaders"] = requestHeaders;
    data["postData"] = postData;
    data["category"] = category;
    data["createSubfolder"] = createSubfolder;
    data["priority"] = priority;
    data["customCommand"] = customCommand;
    data["customCommandOverrideEnabled"] = overrideGlobalCommand;
    data["startAutomatically"] = startAutomatically;
    Request *request = new Request(this);
    request->post("/transfers/addTransfers", data);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onAppendRequestFinished(Request*)));
}

void TransferModel::reload() {
    m_activeTransfers.clear();
    m_packages->reload();
}

void TransferModel::queue() {
    m_packages->queue();
}

void TransferModel::pause() {
    m_packages->pause();
}

bool TransferModel::reload(const QModelIndex &index) {
    if (TransferItem *item = get(index)) {
        return item->reload();
    }

    return false;
}

bool TransferModel::queue(const QModelIndex &index) {
    if (TransferItem *item = get(index)) {
        return item->queue();
    }

    return false;
}

bool TransferModel::pause(const QModelIndex &index) {
    if (TransferItem *item = get(index)) {
        return item->pause();
    }

    return false;
}

bool TransferModel::cancel(const QModelIndex &index, bool deleteFiles) {
    if (TransferItem *item = get(index)) {
        return item->cancel(deleteFiles);
    }

    return false;
}

void TransferModel::addActiveTransfer(TransferItem *transfer) {
    if (!m_activeTransfers.contains(transfer)) {
        m_activeTransfers.append(transfer);
        emit activeTransfersChanged(activeTransfers());
        emit totalSpeedChanged(totalSpeed());
    }
}

void TransferModel::removeActiveTransfer(TransferItem *transfer) {
    m_activeTransfers.removeOne(transfer);
    emit activeTransfersChanged(activeTransfers());
    emit totalSpeedChanged(totalSpeed());
}

void TransferModel::onPackageDataChanged(TransferItem *package, int role) {
    int column = 3;
        
    switch (role) {
    case TransferItem::RowCountRole:
    case TransferItem::ProgressRole:
        break;
    case TransferItem::StatusRole:
        onPackageStatusChanged(package);
        column = 5;
        break;
    case TransferItem::ExpandedRole:
    case TransferItem::NameRole:
        column = 0;
        break;
    case TransferItem::CategoryRole:
        column = 1;
        break;
    case TransferItem::PriorityRole:
        column = 2;
        break;
    default:
        return;
    }

    const QModelIndex idx = index(package->row(), column, QModelIndex());
    emit dataChanged(idx, idx);
}

void TransferModel::onTransferDataChanged(TransferItem *transfer, int role) {
    int column = 3;
        
    switch (role) {
    case TransferItem::BytesTransferredRole:
    case TransferItem::ProgressRole:
    case TransferItem::SizeRole:
        break;
    case TransferItem::SpeedRole:
        column = 4;
        emit totalSpeedChanged(totalSpeed());
        break;
    case TransferItem::CaptchaTimeoutRole:
    case TransferItem::RequestedSettingsTimeoutRole:
    case TransferItem::WaitTimeRole:
        column = 5;
        break;
    case TransferItem::StatusRole:
        onTransferStatusChanged(transfer);
        column = 5;
        break;
    case TransferItem::ExpandedRole:
    case TransferItem::FileNameRole:
    case TransferItem::NameRole:
    case TransferItem::PluginIconRole:
    case TransferItem::PluginIconPathRole:
        column = 0;
        break;
    case TransferItem::PriorityRole:
        column = 2;
        break;
    default:
        return;
    }

    const TransferItem *package = transfer->parentItem();
    const QModelIndex idx = index(transfer->row(), column,
                                  package ? index(package->row(), 0, QModelIndex()) : QModelIndex());
    emit dataChanged(idx, idx);
}

void TransferModel::onPackageStatusChanged(TransferItem *package) {
    switch (package->data(TransferItem::StatusRole).toInt()) {
    case TransferItem::Completed:
    case TransferItem::Canceled:
    case TransferItem::CanceledAndDeleted:
        break;
    default:
        return;
    }
    
    const int row = package->row();
    beginRemoveRows(QModelIndex(), row, row);
    m_packages->removeRow(row);
    endRemoveRows();
}

void TransferModel::onTransferStatusChanged(TransferItem *transfer) {
    switch (transfer->data(TransferItem::StatusRole).toInt()) {
    case TransferItem::Queued:
        transfer->setAutoReloadEnabled(autoReloadEnabled());
        break;
    case TransferItem::WaitingInactive:
        transfer->setAutoReloadEnabled(autoReloadEnabled());
        removeActiveTransfer(transfer);
        break;
    case TransferItem::Paused:
    case TransferItem::Failed:
    case TransferItem::Completed:
        transfer->setAutoReloadEnabled(false);
        removeActiveTransfer(transfer);
        break;
    case TransferItem::Canceled:
    case TransferItem::CanceledAndDeleted:
        transfer->setAutoReloadEnabled(false);
        removeActiveTransfer(transfer);
        
        if (TransferItem *package = transfer->parentItem()) {
            switch (package->data(TransferItem::StatusRole).toInt()) {
            case TransferItem::Canceling:
            case TransferItem::Canceled:
                return;
            default:
                break;
            }
            
            const int row = transfer->row();
            beginRemoveRows(index(package->row(), 0), row, row);
            package->removeRow(row);
            endRemoveRows();
        }

        break;
    case TransferItem::AwaitingCaptchaResponse:
        transfer->setAutoReloadEnabled(autoReloadEnabled());
        addActiveTransfer(transfer);
        emit captchaRequest(transfer);
        break;
    case TransferItem::AwaitingSettingsResponse:
        transfer->setAutoReloadEnabled(autoReloadEnabled());
        addActiveTransfer(transfer);
        emit settingsRequest(transfer);
        break;
    default:
        transfer->setAutoReloadEnabled(autoReloadEnabled());
        addActiveTransfer(transfer);
        break;
    }
}

void TransferModel::onPackageError(TransferItem*, const QString &errorString) {
    emit error(errorString);
}

void TransferModel::onTransferError(TransferItem*, const QString &errorString) {
    emit error(errorString);
}

void TransferModel::onPackagesLoaded() {
    beginResetModel();

    for (int i = 0; i < m_packages->rowCount(); i++) {
        if (TransferItem *package = m_packages->childItem(i)) {
            connect(package, SIGNAL(dataChanged(TransferItem*, int)),
                    this, SLOT(onPackageDataChanged(TransferItem*, int)), Qt::UniqueConnection);
            connect(package, SIGNAL(error(TransferItem*, QString)),
                    this, SLOT(onPackageError(TransferItem*, QString)), Qt::UniqueConnection);
            connect(package, SIGNAL(loaded(TransferItem*)), this, SLOT(onPackageLoaded(TransferItem*)),
                    Qt::UniqueConnection);
            onPackageStatusChanged(package);

            for (int j = 0; j < package->rowCount(); j++) {
                if (TransferItem *transfer = package->childItem(j)) {
                    connect(transfer, SIGNAL(dataChanged(TransferItem*, int)),
                            this, SLOT(onTransferDataChanged(TransferItem*, int)), Qt::UniqueConnection);
                    connect(transfer, SIGNAL(error(TransferItem*, QString)),
                            this, SLOT(onTransferError(TransferItem*, QString)), Qt::UniqueConnection);
                    onTransferStatusChanged(transfer);
                }
            }
        }
    }

    endResetModel();
}

void TransferModel::onPackageLoaded(TransferItem *package) {
    for (int i = 0; i < package->rowCount(); i++) {
        if (TransferItem *transfer = package->childItem(i)) {
            connect(transfer, SIGNAL(dataChanged(TransferItem*, int)),
                    this, SLOT(onTransferDataChanged(TransferItem*, int)), Qt::UniqueConnection);
            connect(transfer, SIGNAL(error(TransferItem*, QString)),
                    this, SLOT(onTransferError(TransferItem*, QString)), Qt::UniqueConnection);
            onTransferStatusChanged(transfer);
        }
    }
}

void TransferModel::onAppendRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void TransferModel::onMoveRequestFinished(Request *request) {
    if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}
