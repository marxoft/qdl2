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

#include "qdl.h"
#include "categories.h"
#include "decaptchapluginmanager.h"
#include "logger.h"
#include "pluginsettings.h"
#include "recaptchapluginmanager.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "transfermodel.h"
#include "urlcheckmodel.h"
#include "urlretrievalmodel.h"
#include <QCoreApplication>

Qdl* Qdl::self = 0;

static QModelIndex getTransferModelIndexById(const QString &id) {
    const QModelIndexList indexes =
    TransferModel::instance()->match(TransferModel::instance()->index(0, 0, QModelIndex()), TransferItem::IdRole,
                                     id, 1, Qt::MatchExactly | Qt::MatchRecursive);

    if (indexes.isEmpty()) {
        return QModelIndex();
    }

    return indexes.first();
}

static QModelIndex getUrlCheckModelIndexByUrl(const QString &url) {
    const QModelIndexList indexes =
    UrlCheckModel::instance()->match(UrlCheckModel::instance()->index(0, 0, QModelIndex()), UrlCheckModel::UrlRole,
                                     url, 1, Qt::MatchExactly | Qt::MatchWrap);

    if (indexes.isEmpty()) {
        return QModelIndex();
    }

    return indexes.first();
}

static QModelIndex getUrlRetrievalModelIndexByUrl(const QString &url) {
    const QModelIndexList indexes =
    UrlRetrievalModel::instance()->match(UrlRetrievalModel::instance()->index(0, 0, QModelIndex()),
                                         UrlRetrievalModel::UrlRole, url, 1, Qt::MatchExactly | Qt::MatchWrap);

    if (indexes.isEmpty()) {
        return QModelIndex();
    }

    return indexes.first();
}

static QVariantMap configToVariantMap(const DecaptchaPluginConfig *config) {
    QVariantMap map;
    map["displayName"] = config->displayName();
    map["filePath"] = config->filePath();
    map["iconFilePath"] = config->iconFilePath();
    map["id"] = config->id();
    map["pluginFilePath"] = config->pluginFilePath();
    map["pluginType"] = config->pluginType();
    map["version"] = config->version();
    return map;
}

static QVariantMap configToVariantMap(const RecaptchaPluginConfig *config) {
    QVariantMap map;
    map["displayName"] = config->displayName();
    map["filePath"] = config->filePath();
    map["iconFilePath"] = config->iconFilePath();
    map["id"] = config->id();
    map["pluginFilePath"] = config->pluginFilePath();
    map["pluginType"] = config->pluginType();
    map["version"] = config->version();
    return map;
}

static QVariantMap configToVariantMap(const ServicePluginConfig *config) {
    QVariantMap map;
    map["displayName"] = config->displayName();
    map["filePath"] = config->filePath();
    map["iconFilePath"] = config->iconFilePath();
    map["id"] = config->id();
    map["pluginFilePath"] = config->pluginFilePath();
    map["pluginType"] = config->pluginType();
    map["regExp"] = config->regExp().pattern();
    map["version"] = config->version();
    return map;
}

static void getConfigSetting(const PluginSettings &plugin, QVariantMap &setting, const QString &group = QString()) {
    QString key = setting.value("key").toString();

    if (key.isEmpty()) {
        return;
    }
    
    if (!group.isEmpty()) {
        key.prepend("/");
        key.prepend(group);
    }

    const QString type = setting.value("type").toString();

    if (type == "group") {
        QVariantList settings = setting.value("settings").toList();
        
        for (int i = 0; i < settings.size(); i++) {
            QVariantMap map = settings.at(i).toMap();
            getConfigSetting(plugin, map, key);
            settings[i] = map;
        }

        setting["settings"] = settings;
    }
    else {
        setting["value"] = plugin.value(key, setting.value("value"));
    }
}     

static QVariantList getConfigSettings(const DecaptchaPluginConfig *config) {
    QVariantList settings = config->settings();

    if (settings.isEmpty()) {
        return settings;
    }

    PluginSettings plugin(config->id());

    for (int i = 0; i < settings.size(); i++) {
        QVariantMap setting = settings.at(i).toMap();
        getConfigSetting(plugin, setting);
        settings[i] = setting;
    }

    return settings;
}

static QVariantList getConfigSettings(const RecaptchaPluginConfig *config) {
    QVariantList settings = config->settings();

    if (settings.isEmpty()) {
        return settings;
    }

    PluginSettings plugin(config->id());

    for (int i = 0; i < settings.size(); i++) {
        QVariantMap setting = settings.at(i).toMap();
        getConfigSetting(plugin, setting);
        settings[i] = setting;
    }

    return settings;
}

static QVariantList getConfigSettings(const ServicePluginConfig *config) {
    QVariantList settings = config->settings();

    if (settings.isEmpty()) {
        return settings;
    }

    PluginSettings plugin(config->id());

    for (int i = 0; i < settings.size(); i++) {
        QVariantMap setting = settings.at(i).toMap();
        getConfigSetting(plugin, setting);
        settings[i] = setting;
    }

    return settings;
}

Qdl::Qdl() :
    QObject()
{
}

Qdl::~Qdl() {
    self = 0;
}

Qdl* Qdl::instance() {
    return self ? self : self = new Qdl;
}

void Qdl::quit() {
    Logger::log("Qdl::quit(): Quitting the application");
    QCoreApplication::instance()->quit();
}

void Qdl::addTransfer(const QString &url, const QString &requestMethod, const QVariantMap &requestHeaders,
                      const QString &postData) {
    TransferModel::instance()->append(url, requestMethod, requestHeaders, postData);
}

void Qdl::addTransfers(const QStringList &urls, const QString &requestMethod, const QVariantMap &requestHeaders,
                       const QString &postData) {
    TransferModel::instance()->append(urls, requestMethod, requestHeaders, postData);
}

QVariantMap Qdl::getTransfers(int offset, int limit, bool includeChildren) {
    if ((limit <= 0) || (limit > TransferModel::instance()->rowCount())) {
        limit = TransferModel::instance()->rowCount();
    }

    QVariantMap map;
    map["active"] = TransferModel::instance()->activeTransfers();
    map["speed"] = TransferModel::instance()->totalSpeed();
    map["speedString"] = TransferModel::instance()->totalSpeedString();
    map["total"] = TransferModel::instance()->rowCount();
    
    QVariantList transfers;
    
    for (int i = offset; i < limit; i++) {
        const QModelIndex parent = TransferModel::instance()->index(i, 0, QModelIndex());
        QVariantMap package = TransferModel::instance()->itemDataWithRoleNames(parent);

        if (!package.isEmpty()) {
            if (includeChildren) {
                const int transferCount = TransferModel::instance()->rowCount(parent);
                QVariantList children;
                
                for (int j = 0; j < transferCount; j++) {
                    const QModelIndex child = TransferModel::instance()->index(j, 0, parent);
                    const QVariantMap transfer = TransferModel::instance()->itemDataWithRoleNames(child);
                    
                    if (!transfer.isEmpty()) {
                        children << transfer;
                    }
                }
                
                package["children"] = children;
            }
            
            transfers << package;
        }
    }

    map["transfers"] = transfers;
    return map;
}

QVariantMap Qdl::getTransfer(const QString &id, bool includeChildren) {
    const QModelIndex index = getTransferModelIndexById(id);

    if (index.isValid()) {
        QVariantMap transfer = TransferModel::instance()->itemDataWithRoleNames(index);

        if (includeChildren) {
            QVariantList children;
            const int count = TransferModel::instance()->rowCount(index);
            
            for (int i = 0; i < count; i++) {
                children << TransferModel::instance()->itemDataWithRoleNames(TransferModel::instance()->index(i, 0, index));
            }

            transfer["children"] = children;
        }

        return transfer;
    }

    return QVariantMap();
}

QVariantList Qdl::searchTransfers(const QString &property, const QVariant &value, int hits, bool includeChildren) {
    QVariantList transfers;
    const QModelIndexList indexes =
    TransferModel::instance()->match(TransferModel::instance()->index(0, 0),
                                     TransferItem::roleNames().key(property.toUtf8()), value, hits,
                                     Qt::MatchExactly | Qt::MatchRecursive);
    
    foreach (const QModelIndex &index, indexes) {
        QVariantMap transfer = TransferModel::instance()->itemDataWithRoleNames(index);
        
        if (!transfer.isEmpty()) {
            if (includeChildren) {
                QVariantList children;
                const int count = TransferModel::instance()->rowCount(index);
                
                for (int i = 0; i < count; i++) {
                    children << TransferModel::instance()->itemDataWithRoleNames(TransferModel::instance()->index(i, 0, index));
                }
                
                transfer["children"] = children;
            }
            
            transfers << transfer;
        }
    }
    
    return transfers;
}

bool Qdl::setTransferProperty(const QString &id, const QString &property, const QVariant &value) {
    const QModelIndex index = getTransferModelIndexById(id);

    if (index.isValid()) {
        return TransferModel::instance()->setData(index, value, property.toUtf8());
    }

    return false;
}

bool Qdl::setTransferProperties(const QString &id, const QVariantMap &properties) {
    const QModelIndex index = getTransferModelIndexById(id);

    if (index.isValid()) {
        return TransferModel::instance()->setItemData(index, properties);
    }

    return false;
}

bool Qdl::startTransfer(const QString &id) {
    return setTransferProperty(id, "status", TransferItem::Queued);
}

bool Qdl::pauseTransfer(const QString &id) {
    return setTransferProperty(id, "status", TransferItem::Paused);
}

bool Qdl::removeTransfer(const QString &id, bool deleteFiles) {
    return setTransferProperty(id, "status", deleteFiles ? TransferItem::CanceledAndDeleted : TransferItem::Canceled);
}

bool Qdl::moveTransfer(const QString &sourceId, const QString &destinationId, int destinationRow) {
    const QModelIndex sourceIndex = getTransferModelIndexById(sourceId);

    if (sourceIndex.isValid()) {
        const QModelIndex destinationParent = getTransferModelIndexById(destinationId);

        if (destinationRow == -1) {
            destinationRow = TransferModel::instance()->rowCount(destinationParent);
        }
        
        return TransferModel::instance()->moveRows(sourceIndex.parent(), sourceIndex.row(), 1,
                                                   destinationParent, destinationRow);
    }

    return false;
}

void Qdl::startTransfers() {
    TransferModel::instance()->queue();
}

void Qdl::pauseTransfers() {
    TransferModel::instance()->pause();
}

bool Qdl::addCategory(const QString &name, const QString &path) {
    return Categories::add(name, path);
}

QVariantList Qdl::getCategories() {
    QVariantList list;
    const CategoryList categories = Categories::get();

    for (int i = 0; i < categories.size(); i++) {
        QVariantMap category;
        category["name"] = categories.at(i).name;
        category["path"] = categories.at(i).path;
        list << category;
    }

    return list;
}

QVariantMap Qdl::getCategory(const QString &name) {
    const Category category = Categories::get(name);
    QVariantMap map;
    map["name"] = category.name;
    map["path"] = category.path;
    return map;
}

bool Qdl::removeCategory(const QString &name) {
    return Categories::remove(name);
}

QVariantList Qdl::getDecaptchaPlugins() {
    QVariantList list;

    foreach (const DecaptchaPluginPair &pair, DecaptchaPluginManager::instance()->plugins()) {
        list << configToVariantMap(pair.config);
    }

    return list;
}

QVariantMap Qdl::getDecaptchaPlugin(const QString &id) {
    if (const DecaptchaPluginConfig *config = DecaptchaPluginManager::instance()->getConfigById(id)) {
        return configToVariantMap(config);
    }

    return QVariantMap();
}

QVariantList Qdl::getDecaptchaPluginSettings(const QString &id) {
    if (const DecaptchaPluginConfig *config = DecaptchaPluginManager::instance()->getConfigById(id)) {
        return getConfigSettings(config);
    }

    return QVariantList();
}

bool Qdl::setDecaptchaPluginSettings(const QString &id, const QVariantMap &properties) {
    if (properties.isEmpty()) {
        return false;
    }
    
    if (const DecaptchaPluginConfig *config = DecaptchaPluginManager::instance()->getConfigById(id)) {
        PluginSettings settings(config->id());
        QMapIterator<QString, QVariant> iterator(properties);

        while (iterator.hasNext()) {
            iterator.next();
            settings.setValue(iterator.key(), iterator.value());
        }

        return true;
    }

    return false;
}

QVariantList Qdl::getRecaptchaPlugins() {
    QVariantList list;

    foreach (const RecaptchaPluginPair &pair, RecaptchaPluginManager::instance()->plugins()) {
        list << configToVariantMap(pair.config);
    }

    return list;
}

QVariantMap Qdl::getRecaptchaPlugin(const QString &id) {
    if (const RecaptchaPluginConfig *config = RecaptchaPluginManager::instance()->getConfigById(id)) {
        return configToVariantMap(config);
    }

    return QVariantMap();
}

QVariantList Qdl::getRecaptchaPluginSettings(const QString &id) {
    if (const RecaptchaPluginConfig *config = RecaptchaPluginManager::instance()->getConfigById(id)) {
        return getConfigSettings(config);
    }

    return QVariantList();
}

bool Qdl::setRecaptchaPluginSettings(const QString &id, const QVariantMap &properties) {
    if (properties.isEmpty()) {
        return false;
    }
    
    if (const RecaptchaPluginConfig *config = RecaptchaPluginManager::instance()->getConfigById(id)) {
        PluginSettings settings(config->id());
        QMapIterator<QString, QVariant> iterator(properties);

        while (iterator.hasNext()) {
            iterator.next();
            settings.setValue(iterator.key(), iterator.value());
        }

        return true;
    }

    return false;
}

QVariantList Qdl::getServicePlugins() {
    QVariantList list;

    foreach (const ServicePluginPair &pair, ServicePluginManager::instance()->plugins()) {
        list << configToVariantMap(pair.config);
    }

    return list;
}

QVariantMap Qdl::getServicePlugin(const QString &id) {
    if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigById(id)) {
        return configToVariantMap(config);
    }

    return QVariantMap();
}

QVariantList Qdl::getServicePluginSettings(const QString &id) {
    if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigById(id)) {
        return getConfigSettings(config);
    }

    return QVariantList();
}

bool Qdl::setServicePluginSettings(const QString &id, const QVariantMap &properties) {
    if (properties.isEmpty()) {
        return false;
    }
    
    if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigById(id)) {
        PluginSettings settings(config->id());
        QMapIterator<QString, QVariant> iterator(properties);

        while (iterator.hasNext()) {
            iterator.next();
            settings.setValue(iterator.key(), iterator.value());
        }

        return true;
    }

    return false;
}

QVariantMap Qdl::getSettings(const QStringList &settings) {
    QVariantMap map;

    foreach (const QString &setting, settings) {
        map[setting] = Settings::instance()->property(setting.toUtf8());
    }

    return map;
}

bool Qdl::setSettings(const QVariantMap &settings) {
    QMapIterator<QString, QVariant> iterator(settings);

    while (iterator.hasNext()) {
        iterator.next();

        if (!Settings::instance()->setProperty(iterator.key().toUtf8(), iterator.value())) {
            return false;
        }
    }

    return true;
}

void Qdl::addUrlChecks(const QStringList &urls, const QString &category) {
    Settings::setDefaultCategory(category);
    UrlCheckModel::instance()->append(urls);
}

void Qdl::clearUrlChecks() {
    UrlCheckModel::instance()->clear();
}

QVariantMap Qdl::getUrlChecks() {
    QVariantMap map;
    map["count"] = UrlCheckModel::instance()->rowCount();
    map["progress"] = UrlCheckModel::instance()->progress();
    map["requestedSettings"] = UrlCheckModel::instance()->requestedSettings();
    map["requestedSettingsTimeout"] = UrlCheckModel::instance()->requestedSettingsTimeout();
    map["requestedSettingsTimeoutString"] = UrlCheckModel::instance()->requestedSettingsTimeoutString();
    map["requestedSettingsTitle"] = UrlCheckModel::instance()->requestedSettingsTitle();
    map["status"] = UrlCheckModel::instance()->status();
    map["statusString"] = UrlCheckModel::instance()->statusString();
    
    QVariantList checks;
    
    for (int i = 0; i < UrlCheckModel::instance()->rowCount(); i++) {
        checks << UrlCheckModel::instance()->itemData(i);
    }
    
    map["checks"] = checks;
    return map;
}

QVariantMap Qdl::getUrlCheck(const QString &url) {
    const QModelIndex index = getUrlCheckModelIndexByUrl(url);

    if (!index.isValid()) {
        return QVariantMap();
    }

    return UrlCheckModel::instance()->itemData(index.row());
}

bool Qdl::removeUrlCheck(const QString &url) {
    const QModelIndex index = getUrlCheckModelIndexByUrl(url);

    if (!index.isValid()) {
        return false;
    }

    return UrlCheckModel::instance()->removeRow(index.row());
}

bool Qdl::submitUrlCheckSettingsResponse(const QVariantMap &settings) {
    return UrlCheckModel::instance()->submitSettingsResponse(settings);
}

QVariantMap Qdl::getUrlRetrievals() {
    QVariantMap map;
    map["count"] = UrlRetrievalModel::instance()->rowCount();
    map["progress"] = UrlRetrievalModel::instance()->progress();
    map["status"] = UrlRetrievalModel::instance()->status();
    map["statusString"] = UrlRetrievalModel::instance()->statusString();
    
    QVariantList retrievals;
    
    for (int i = 0; i < UrlRetrievalModel::instance()->rowCount(); i++) {
        retrievals << UrlRetrievalModel::instance()->itemData(i);
    }
    
    map["retrievals"] = retrievals;
    return map;
}

void Qdl::addUrlRetrievals(const QStringList &urls, const QString &pluginId) {
    Settings::setDefaultServicePlugin(pluginId);
    UrlRetrievalModel::instance()->append(urls, pluginId);
}

void Qdl::clearUrlRetrievals() {
    UrlRetrievalModel::instance()->clear();
}

QVariantMap Qdl::getUrlRetrieval(const QString &url) {
    const QModelIndex index = getUrlRetrievalModelIndexByUrl(url);

    if (!index.isValid()) {
        return QVariantMap();
    }

    return UrlRetrievalModel::instance()->itemData(index.row());
}

bool Qdl::removeUrlRetrieval(const QString &url) {
    const QModelIndex index = getUrlRetrievalModelIndexByUrl(url);

    if (!index.isValid()) {
        return false;
    }

    return UrlRetrievalModel::instance()->removeRow(index.row());
}
