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

#include "decaptchapluginconfigmodel.h"
#include "decaptchapluginmanager.h"

DecaptchaPluginConfigModel::DecaptchaPluginConfigModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_roles[DisplayNameRole] = "displayName";
    m_roles[IconRole] = "icon";
    m_roles[FilePathRole] = "filePath";
    m_roles[IconFilePathRole] = "iconFilePath";
    m_roles[IdRole] = "id";
    m_roles[PluginFilePathRole] = "pluginFilePath";
    m_roles[PluginTypeRole] = "pluginType";
    m_roles[VersionRole] = "version";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
    reload();
    connect(DecaptchaPluginManager::instance(), SIGNAL(countChanged(int)), this, SLOT(reload()));
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> DecaptchaPluginConfigModel::roleNames() const {
    return m_roles;
}
#endif

int DecaptchaPluginConfigModel::rowCount(const QModelIndex &) const {
    return DecaptchaPluginManager::instance()->plugins().size();
}

QVariant DecaptchaPluginConfigModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    const DecaptchaPluginConfig *config = DecaptchaPluginManager::instance()->plugins().at(index.row());

    if (!config) {
        return QVariant();
    }
    
    switch (role) {
    case DisplayNameRole:
        return config->displayName();
    case IconRole:
        return config->icon();
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
    case VersionRole:
        return config->version();
    default:
        return QVariant();
    }
}

QMap<int, QVariant> DecaptchaPluginConfigModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant DecaptchaPluginConfigModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap DecaptchaPluginConfigModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList DecaptchaPluginConfigModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                                Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int DecaptchaPluginConfigModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void DecaptchaPluginConfigModel::clear() {
    foreach (const DecaptchaPluginConfig *config, DecaptchaPluginManager::instance()->plugins()) {
        disconnect(config, SIGNAL(changed(DecaptchaPluginConfig*)), this, SLOT(onConfigChanged(DecaptchaPluginConfig*)));
    }
}

void DecaptchaPluginConfigModel::reload() {
    clear();
    beginResetModel();

    foreach (const DecaptchaPluginConfig *config, DecaptchaPluginManager::instance()->plugins()) {
        connect(config, SIGNAL(changed(DecaptchaPluginConfig*)), this, SLOT(onConfigChanged(DecaptchaPluginConfig*)));
    }

    endResetModel();
    emit countChanged(rowCount());
}

void DecaptchaPluginConfigModel::onConfigChanged(DecaptchaPluginConfig *config) {
    const QModelIndex idx = index(DecaptchaPluginManager::instance()->plugins().indexOf(config), 0, QModelIndex());
    emit dataChanged(idx, idx);
}
