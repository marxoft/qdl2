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

#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "javascriptdecaptchaplugin.h"
#include "logger.h"
#include <QDir>
#include <QNetworkAccessManager>
#include <QPluginLoader>

static bool displayNameLessThan(const DecaptchaPluginConfig *config, const DecaptchaPluginConfig *other) {
    return QString::localeAwareCompare(config->displayName(), other->displayName()) < 0;
}

DecaptchaPluginManager* DecaptchaPluginManager::self = 0;

DecaptchaPluginManager::DecaptchaPluginManager() :
    QObject(),
    m_nam(0)
{
}

DecaptchaPluginManager::~DecaptchaPluginManager() {
    self = 0;
}

DecaptchaPluginManager* DecaptchaPluginManager::instance() {
    return self ? self : self = new DecaptchaPluginManager;
}

QList<DecaptchaPluginConfig*> DecaptchaPluginManager::configs() const {
    QList<DecaptchaPluginConfig*> list =  m_plugins.keys();
    qSort(list.begin(), list.end(), displayNameLessThan);
    return list;
}

QList<DecaptchaPlugin*> DecaptchaPluginManager::plugins() const {
    return m_plugins.values();
}

DecaptchaPluginConfig* DecaptchaPluginManager::getConfigById(const QString &id) const {
    foreach (DecaptchaPluginConfig *config, m_plugins.keys()) {
        if (config->id() == id) {
            Logger::log("DecaptchaPluginManager::getConfigById(). PluginFound: " + id);
            return config;
        }
    }
    
    Logger::log("DecaptchaPluginManager::getConfigById(). No Plugin found");
    return 0;
}

DecaptchaPlugin* DecaptchaPluginManager::getPlugin(DecaptchaPluginConfig *config) const {
    return m_plugins.value(config, 0);
}

DecaptchaPlugin* DecaptchaPluginManager::getPluginById(const QString &id) const {
    if (DecaptchaPluginConfig *config = getConfigById(id)) {
        return getPlugin(config);
    }

    return 0;
}

DecaptchaPlugin* DecaptchaPluginManager::createPluginById(const QString &id, QObject *parent) const {
    if (DecaptchaPlugin *plugin = getPluginById(id)) {
        return plugin->createPlugin(parent);
    }

    return 0;
}

void DecaptchaPluginManager::load() {
    if (!m_plugins.isEmpty()) {
        return;
    }
    
    QDir dir(DECAPTCHA_PLUGIN_PATH);
    
    foreach (const QString &fileName, dir.entryList(QStringList() << "*.json", QDir::Files)) {
        DecaptchaPluginConfig *config = new DecaptchaPluginConfig(this);

        if (config->load(dir.absoluteFilePath(fileName))) {
            if (config->pluginType() == "js") {
                JavaScriptDecaptchaPlugin *js = new JavaScriptDecaptchaPlugin(config->id(), config->filePath(), this);
                m_plugins.insert(config, js);
                Logger::log("DecaptchaPluginManager::load(). JavaScript plugin loaded: " + config->id());
            }
            else {
                QPluginLoader loader(config->filePath());
                QObject *obj = loader.instance();

                if (obj) {
                    if (DecaptchaPlugin *plugin = qobject_cast<DecaptchaPlugin*>(obj)) {
                        plugin->setNetworkAccessManager(networkAccessManager());
                        m_plugins.insert(config, plugin);
                        Logger::log("DecaptchaPluginManager::load(). Qt Plugin loaded: " + config->id());
                    }
                    else {
                        loader.unload();
                        Logger::log("DecaptchaPluginManager::load(). Error loading Qt plugin: " + config->id());
                    }
                }
                else {
                    Logger::log("DecaptchaPluginManager::load(). Qt plugin is NULL: " + config->id());
                }
            }
        }
        else {
            delete config;
        }
    }
}

QNetworkAccessManager* DecaptchaPluginManager::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
