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

#include "transferitem.h"

class TransferItemRoleNames : public QHash<int, QByteArray>
{

public:
    TransferItemRoleNames() :
        QHash<int, QByteArray>()
    {
        insert(TransferItem::BytesTransferredRole, "bytesTransferred");
        insert(TransferItem::CanCancelRole, "canCancel");
        insert(TransferItem::CanPauseRole, "canPause");
        insert(TransferItem::CanStartRole, "canStart");
        insert(TransferItem::CaptchaDataRole, "captchaData");
        insert(TransferItem::CaptchaResponseRole, "captchaResponse");
        insert(TransferItem::CaptchaTimeoutRole, "captchaTimeout");
        insert(TransferItem::CaptchaTimeoutStringRole, "captchaTimeoutString");
        insert(TransferItem::CaptchaTypeRole, "captchaType");
        insert(TransferItem::CaptchaTypeStringRole, "captchaTypeString");
        insert(TransferItem::CategoryRole, "category");
        insert(TransferItem::CreateSubfolderRole, "createSubfolder");
        insert(TransferItem::CustomCommandRole, "customCommand");
        insert(TransferItem::CustomCommandOverrideEnabledRole, "customCommandOverrideEnabled");
        insert(TransferItem::DownloadPathRole, "downloadPath");
        insert(TransferItem::ErrorStringRole, "errorString");
        insert(TransferItem::ExpandedRole, "expanded");
        insert(TransferItem::FileNameRole, "fileName");
        insert(TransferItem::FilePathRole, "filePath");
        insert(TransferItem::IdRole, "id");
        insert(TransferItem::ItemTypeRole, "itemType");
        insert(TransferItem::ItemTypeStringRole, "itemTypeString");
        insert(TransferItem::NameRole, "name");
        insert(TransferItem::PluginIconPathRole, "pluginIconPath");
        insert(TransferItem::PluginIdRole, "pluginId");
        insert(TransferItem::PluginNameRole, "pluginName");
        insert(TransferItem::PostDataRole, "postData");
        insert(TransferItem::PriorityRole, "priority");
        insert(TransferItem::PriorityStringRole, "priorityString");
        insert(TransferItem::ProgressRole, "progress");
        insert(TransferItem::ProgressStringRole, "progressString");
        insert(TransferItem::RequestedSettingsRole, "requestedSettings");
        insert(TransferItem::RequestedSettingsTimeoutRole, "requestedSettingsTimeout");
        insert(TransferItem::RequestedSettingsTimeoutStringRole, "requestedSettingsTimeoutString");
        insert(TransferItem::RequestedSettingsTitleRole, "requestedSettingsTitle");
        insert(TransferItem::RequestHeadersRole, "requestHeaders");
        insert(TransferItem::RequestMethodRole, "requestMethod");
        insert(TransferItem::RowRole, "row");
        insert(TransferItem::RowCountRole, "count");
        insert(TransferItem::SizeRole, "size");
        insert(TransferItem::SpeedRole, "speed");
        insert(TransferItem::SpeedStringRole, "speedString");
        insert(TransferItem::StatusRole, "status");
        insert(TransferItem::StatusStringRole, "statusString");
        insert(TransferItem::SuffixRole, "suffix");
        insert(TransferItem::UrlRole, "url");
        insert(TransferItem::UsePluginsRole, "usePlugins");
        insert(TransferItem::WaitTimeRole, "waitTime");
        insert(TransferItem::WaitTimeStringRole, "waitTimeString");
    }
};

QHash<int, QByteArray> TransferItem::roles = TransferItemRoleNames();

TransferItem::TransferItem(QObject *parent) :
    QObject(parent),
    m_expanded(false),
    m_row(-1),
    m_parentItem(0)
{
}

QHash<int, QByteArray> TransferItem::roleNames() {
    return roles;
}

QString TransferItem::priorityString(TransferItem::Priority priority) {
    switch (priority) {
    case HighestPriority:
        return tr("Highest");
    case HighPriority:
        return tr("High");
    case NormalPriority:
        return tr("Normal");
    case LowPriority:
        return tr("Low");
    case LowestPriority:
        return tr("Lowest");
    default:
        return tr("Unknown");
    }
}

QString TransferItem::statusString(TransferItem::Status status) {
    switch (status) {
    case Null:
        return QString();
    case Paused:
        return tr("Paused");
    case Canceled:
    case CanceledAndDeleted:
        return tr("Canceled");
    case Failed:
        return tr("Failed");
    case Completed:
        return tr("Completed");
    case WaitingInactive:
        return tr("Waiting (inactive)");
    case Queued:
        return tr("Queued");
    case Connecting:
        return tr("Connecting");
    case WaitingActive:
        return tr("Waiting (active)");
    case RetrievingCaptchaChallenge:
        return tr("Retrieving captcha challenge");
    case AwaitingCaptchaResponse:
        return tr("Awaiting captcha response");
    case RetrievingCaptchaResponse:
        return tr("Retrieving captcha response");
    case SubmittingCaptchaResponse:
        return tr("Submitting captcha response");
    case ReportingCaptchaResponse:
        return tr("Reporting incorrect captcha response");
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
        return tr("Awaiting settings response");
    case SubmittingDecaptchaSettingsResponse:
    case SubmittingRecaptchaSettingsResponse:
    case SubmittingServiceSettingsResponse:
        return tr("Submitting settings response");
    case Downloading:
        return tr("Downloading");
    case Canceling:
        return tr("Canceling");
    case ExtractingArchive:
        return tr("Extracting archive");
    case MovingFiles:
        return tr("Moving files");
    case ExecutingCustomCommand:
        return tr("Executing custom command");
    default:
        return tr("Unknown");
    }
}

QVariant TransferItem::data(int role) const {
    switch (role) {
    case CanCancelRole:
        return canCancel();
    case CanPauseRole:
        return canPause();
    case CanStartRole:
        return canStart();
    case ExpandedRole:
        return expanded();
    case ItemTypeRole:
        return itemType();
    case ItemTypeStringRole:
        return itemTypeString();
    case RowRole:
        return row();
    case RowCountRole:
        return rowCount();
    default:
        return QVariant();
    }
}

QVariant TransferItem::data(const QByteArray &roleName) const {
    return data(roles.key(roleName));
}

bool TransferItem::setData(int role, const QVariant &value) {
    switch (role) {
    case ExpandedRole:
        setExpanded(value.toBool());
        return true;
    default:
        return false;
    }
}

bool TransferItem::setData(const QByteArray &roleName, const QVariant &value) {
    return setData(roles.key(roleName), value);
}

QMap<int, QVariant> TransferItem::itemData() const {
    QMap<int, QVariant> map;
    map[CanCancelRole] = canCancel();
    map[CanPauseRole] = canPause();
    map[CanStartRole] = canStart();
    map[ExpandedRole] = expanded();
    map[ItemTypeRole] = itemType();
    map[ItemTypeStringRole] = itemTypeString();
    map[RowRole] = row();
    map[RowCountRole] = rowCount();
    return map;
}

QVariantMap TransferItem::itemDataWithRoleNames() const {
    QVariantMap map;
    map[roles.value(CanCancelRole)] = canCancel();
    map[roles.value(CanPauseRole)] = canPause();
    map[roles.value(CanStartRole)] = canStart();
    map[roles.value(ExpandedRole)] = expanded();
    map[roles.value(ItemTypeRole)] = itemType();
    map[roles.value(ItemTypeStringRole)] = itemTypeString();
    map[roles.value(RowRole)] = row();
    map[roles.value(RowCountRole)] = rowCount();
    return map;
}

bool TransferItem::setItemData(const QMap<int, QVariant> &data) {
    if (data.isEmpty()) {
        return false;
    }
    
    QMapIterator<int, QVariant> iterator(data);

    while (iterator.hasNext()) {
        iterator.next();

        if (!setData(iterator.key(), iterator.value())) {
            return false;
        }
    }

    return true;
}

bool TransferItem::setItemData(const QVariantMap &data) {
    if (data.isEmpty()) {
        return false;
    }
    
    QMapIterator<QString, QVariant> iterator(data);

    while (iterator.hasNext()) {
        iterator.next();

        if (!setData(iterator.key().toUtf8(), iterator.value())) {
            return false;
        }
    }

    return true;
}

TransferItem::ItemType TransferItem::itemType() const {
    return ListType;
}

QString TransferItem::itemTypeString() const {
    switch (itemType()) {
    case ListType:
        return tr("List");
    case PackageType:
        return tr("Package");
    case TransferType:
        return tr("Transfer");
    default:
        return tr("Unknown");
    }
}

bool TransferItem::expanded() const {
    return m_expanded;
}

void TransferItem::setExpanded(bool enabled) {
    if (enabled != expanded()) {
        m_expanded = enabled;
        emit dataChanged(this, ExpandedRole);
    }
}

int TransferItem::row() const {
    return m_row;
}

void TransferItem::setRow(int r) {
    m_row = r;
}

int TransferItem::rowCount() const {
    return m_childItems.size();
}

TransferItem* TransferItem::parentItem() const {
    return m_parentItem;
}

void TransferItem::setParentItem(TransferItem *item) {
    setParent(item);
    m_parentItem = item;
}

TransferItem* TransferItem::childItem(int i) const {
    return ((i >= 0) && (i < m_childItems.size())) ? m_childItems.at(i) : 0;
}

void TransferItem::appendRow(TransferItem *item) {
    item->setParentItem(this);
    item->setRow(m_childItems.size());
    m_childItems.append(item);
    connectItemSignals(item);
    emit dataChanged(this, RowCountRole);
}

bool TransferItem::insertRow(int i, TransferItem *item) {
    if ((i < 0) || (i > m_childItems.size())) {
        return false;
    }

    item->setParentItem(this);
    item->setRow(i);
    m_childItems.insert(i, item);
    connectItemSignals(item);

    for (int j = i + 1; j < m_childItems.size(); j++) {
        m_childItems.at(j)->setRow(j);
    }

    emit dataChanged(this, RowCountRole);
    return true;
}

bool TransferItem::moveRow(int from, int to) {
    if ((from == to) || (from < 0) || (from >= m_childItems.size()) || (to < 0) || (to >= m_childItems.size())) {
        return false;
    }

    m_childItems.at(from)->setRow(to);
    m_childItems.at(to)->setRow(from);
    m_childItems.move(from, to);
    return true;
}

bool TransferItem::removeRow(int i) {
    if (TransferItem *item = takeRow(i)) {
        item->deleteLater();
        return true;
    }

    return false;
}

TransferItem* TransferItem::takeRow(int i) {
    if ((i < 0) || (i >= m_childItems.size())) {
        return 0;
    }

    TransferItem *item = m_childItems.takeAt(i);
    item->setParentItem(0);
    item->setRow(-1);
    disconnectItemSignals(item);

    for (int j = i; j < m_childItems.size(); j++) {
        m_childItems.at(j)->setRow(j);
    }

    emit dataChanged(this, RowCountRole);
    return item;
}

bool TransferItem::canStart() const {
    return true;
}

bool TransferItem::canPause() const {
    return true;
}

bool TransferItem::canCancel() const {
    return true;
}

bool TransferItem::queue() {
    if (!canStart()) {
        return false;
    }
    
    foreach (TransferItem *child, m_childItems) {
        child->queue();
    }

    return true;
}

bool TransferItem::start() {
    foreach (TransferItem *child, m_childItems) {
        child->start();
    }

    return true;
}

bool TransferItem::pause() {
    foreach (TransferItem *child, m_childItems) {
        child->pause();
    }

    return true;
}

bool TransferItem::cancel(bool deleteFiles) {
    foreach (TransferItem *child, m_childItems) {
        child->cancel(deleteFiles);
    }

    return true;
}

void TransferItem::restore(const QSettings &) {}

void TransferItem::save(QSettings &) {}

void TransferItem::childItemFinished(TransferItem*) {}

void TransferItem::connectItemSignals(TransferItem *item) {
    connect(item, SIGNAL(finished(TransferItem*)), this, SLOT(childItemFinished(TransferItem*)));
}

void TransferItem::disconnectItemSignals(TransferItem *item) {
    disconnect(item, SIGNAL(finished(TransferItem*)), this, SLOT(childItemFinished(TransferItem*)));
}
