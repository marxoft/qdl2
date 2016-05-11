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

#include "recaptchapluginconfigmodel.h"
#include <QIcon>

RecaptchaPluginConfigModel::RecaptchaPluginConfigModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_roles[DisplayNameRole] = "displayName";
    m_roles[FilePathRole] = "filePath";
    m_roles[IconFilePathRole] = "iconFilePath";
    m_roles[IdRole] = "id";
    m_roles[PluginFilePathRole] = "pluginFilePath";
    m_roles[PluginTypeRole] = "pluginType";
    m_roles[SettingsRole] = "settings";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
    reload();
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> RecaptchaPluginConfigModel::roleNames() const {
    return m_roles;
}
#endif

int RecaptchaPluginConfigModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

QVariant RecaptchaPluginConfigModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    const RecaptchaPluginConfig *config = m_items.at(index.row()).config;

    if (!config) {
        return QVariant();
    }
    
    switch (role) {
    case Qt::DecorationRole:
        if (!config->iconFilePath().isEmpty()) {
            return QIcon(config->iconFilePath());
        }

        return QVariant();
    case DisplayNameRole:
        return config->displayName();
    case FilePathRole:
        return config->filePath();
    case IconFilePathRole:
        return config->iconFilePath();
    case IdRole:
        return config->id();
    case PluginFilePathRole:
        return config->pluginFilePath();
    case PluginTypeRole:
        return config->pluginType();
    case SettingsRole:
        return config->settings();
    default:
        return QVariant();
    }
}

QMap<int, QVariant> RecaptchaPluginConfigModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant RecaptchaPluginConfigModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap RecaptchaPluginConfigModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList RecaptchaPluginConfigModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                                Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int RecaptchaPluginConfigModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void RecaptchaPluginConfigModel::clear() {
    if (!m_items.isEmpty()) {
        beginResetModel();
        m_items.clear();
        endResetModel();
        emit countChanged(0);
    }
}

void RecaptchaPluginConfigModel::reload() {
    clear();
    beginResetModel();
    m_items = RecaptchaPluginManager::instance()->plugins();
    endResetModel();
    emit countChanged(rowCount());
}
