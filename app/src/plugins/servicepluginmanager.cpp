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

#include "servicepluginmanager.h"
#include "definitions.h"
#include "javascriptserviceplugin.h"
#include "logger.h"
#include <QDir>
#include <QNetworkAccessManager>
#include <QPluginLoader>

static bool displayNameLessThan(const ServicePluginConfig *config, const ServicePluginConfig *other) {
    return QString::localeAwareCompare(config->displayName(), other->displayName()) < 0;
}

ServicePluginManager* ServicePluginManager::self = 0;

ServicePluginManager::ServicePluginManager() :
    QObject(),
    m_nam(0)
{
}

ServicePluginManager::~ServicePluginManager() {
    self = 0;
}

ServicePluginManager* ServicePluginManager::instance() {
    return self ? self : self = new ServicePluginManager;
}

QList<ServicePluginConfig*> ServicePluginManager::configs() const {
    QList<ServicePluginConfig*> list = m_plugins.keys();
    qSort(list.begin(), list.end(), displayNameLessThan);
    return list;
}

QList<ServicePlugin*> ServicePluginManager::plugins() const {
    return m_plugins.values();
}

ServicePluginConfig* ServicePluginManager::getConfigById(const QString &id) const {
    foreach (ServicePluginConfig *config, m_plugins.keys()) {
        if (config->id() == id) {
            Logger::log("ServicePluginManager::getConfigById(). PluginFound: " + id);
            return config;
        }
    }
    
    Logger::log("ServicePluginManager::getConfigById(). No Plugin found");
    return 0;
}

ServicePluginConfig* ServicePluginManager::getConfigByUrl(const QString &url) const {
    foreach (ServicePluginConfig *config, m_plugins.keys()) {
        if (config->urlIsSupported(url)) {
            Logger::log("ServicePluginManager::getConfigByUrl(). Plugin found: " + config->id());
            return config;
        }
    }
    
    Logger::log("ServicePluginManager::getConfigByUrl(). No Plugin found");
    return 0;
}

ServicePlugin* ServicePluginManager::getPlugin(ServicePluginConfig *config) const {
    return m_plugins.value(config, 0);
}

ServicePlugin* ServicePluginManager::getPluginById(const QString &id) const {
    if (ServicePluginConfig *config = getConfigById(id)) {
        return getPlugin(config);
    }

    return 0;
}

ServicePlugin* ServicePluginManager::getPluginByUrl(const QString &url) const {
    if (ServicePluginConfig *config = getConfigByUrl(url)) {
        return getPlugin(config);
    }

    return 0;
}

ServicePlugin* ServicePluginManager::createPluginById(const QString &id, QObject *parent) const {
    if (ServicePlugin *plugin = getPluginById(id)) {
        return plugin->createPlugin(parent);
    }

    return 0;
}

ServicePlugin* ServicePluginManager::createPluginByUrl(const QString &url, QObject *parent) const {
    if (ServicePlugin *plugin = getPluginByUrl(url)) {
        return plugin->createPlugin(parent);
    }

    return 0;
}

bool ServicePluginManager::urlIsSupported(const QString &url) const {
    if (getConfigByUrl(url)) {
        return true;
    }

    return false;
}

void ServicePluginManager::load() {
    if (!m_plugins.isEmpty()) {
        return;
    }
    
    QDir dir(SERVICE_PLUGIN_PATH);

    foreach (const QString &fileName, dir.entryList(QStringList() << "*.json", QDir::Files)) {
        ServicePluginConfig *config = new ServicePluginConfig(this);

        if (config->load(dir.absoluteFilePath(fileName))) {
            if (config->pluginType() == "js") {
                JavaScriptServicePlugin *js = new JavaScriptServicePlugin(config->id(), config->filePath(), this);
                m_plugins.insert(config, js);
                Logger::log("ServicePluginManager::load(). JavaScript plugin loaded: " + config->id());
            }
            else {
                QPluginLoader loader(config->filePath());
                QObject *obj = loader.instance();

                if (obj) {
                    if (ServicePlugin *plugin = qobject_cast<ServicePlugin*>(obj)) {
                        plugin->setNetworkAccessManager(networkAccessManager());
                        m_plugins.insert(config, plugin);
                        Logger::log("ServicePluginManager::load(). Qt plugin loaded: " + config->id());
                    }
                    else {
                        loader.unload();
                        Logger::log("ServicePluginManager::load(). Error loading Qt plugin: " + config->id());
                    }
                }
                else {
                    Logger::log("ServicePluginManager::load(). Qt plugin is NULL: " + config->id());
                }
            }
        }
        else {
            delete config;
        }
    }
}

QNetworkAccessManager* ServicePluginManager::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
