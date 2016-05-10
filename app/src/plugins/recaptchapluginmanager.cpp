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

#include "recaptchapluginmanager.h"
#include "definitions.h"
#include "javascriptrecaptchaplugin.h"
#include "logger.h"
#include <QDir>
#include <QNetworkAccessManager>
#include <QPluginLoader>

static bool displayNameLessThan(const RecaptchaPluginConfig *config, const RecaptchaPluginConfig *other) {
    return QString::localeAwareCompare(config->displayName(), other->displayName()) < 0;
}

RecaptchaPluginManager* RecaptchaPluginManager::self = 0;

RecaptchaPluginManager::RecaptchaPluginManager() :
    QObject(),
    m_nam(0)
{
}

RecaptchaPluginManager::~RecaptchaPluginManager() {
    self = 0;
}

RecaptchaPluginManager* RecaptchaPluginManager::instance() {
    return self ? self : self = new RecaptchaPluginManager;
}

QList<RecaptchaPluginConfig*> RecaptchaPluginManager::configs() const {
    QList<RecaptchaPluginConfig*> list = m_plugins.keys();
    qSort(list.begin(), list.end(), displayNameLessThan);
    return list;
}

QList<RecaptchaPlugin*> RecaptchaPluginManager::plugins() const {
    return m_plugins.values();
}

RecaptchaPluginConfig* RecaptchaPluginManager::getConfigById(const QString &id) const {
    foreach (RecaptchaPluginConfig *config, m_plugins.keys()) {
        if (config->id() == id) {
            Logger::log("RecaptchaPluginManager::getConfigById(). PluginFound: " + id);
            return config;
        }
    }
    
    Logger::log("RecaptchaPluginManager::getConfigById(). No Plugin found");
    return 0;
}

RecaptchaPlugin* RecaptchaPluginManager::getPlugin(RecaptchaPluginConfig *config) const {
    return m_plugins.value(config, 0);
}

RecaptchaPlugin* RecaptchaPluginManager::getPluginById(const QString &id) const {
    if (RecaptchaPluginConfig *config = getConfigById(id)) {
        return getPlugin(config);
    }

    return 0;
}

RecaptchaPlugin* RecaptchaPluginManager::createPluginById(const QString &id, QObject *parent) const {
    if (RecaptchaPlugin *plugin = getPluginById(id)) {
        return plugin->createPlugin(parent);
    }

    return 0;
}

void RecaptchaPluginManager::load() {
    if (!m_plugins.isEmpty()) {
        return;
    }
    
    QDir dir(RECAPTCHA_PLUGIN_PATH);

    foreach (const QString &fileName, dir.entryList(QStringList() << "*.json", QDir::Files)) {
        RecaptchaPluginConfig *config = new RecaptchaPluginConfig(this);

        if (config->load(dir.absoluteFilePath(fileName))) {
            if (config->pluginType() == "js") {
                JavaScriptRecaptchaPlugin *js = new JavaScriptRecaptchaPlugin(config->id(), config->filePath(), this);
                m_plugins.insert(config, js);
                Logger::log("RecaptchaPluginManager::load(). JavaScript plugin loaded: " + config->id());
            }
            else {
                QPluginLoader loader(config->filePath());
                QObject *obj = loader.instance();

                if (obj) {
                    if (RecaptchaPlugin *plugin = qobject_cast<RecaptchaPlugin*>(obj)) {
                        plugin->setNetworkAccessManager(networkAccessManager());
                        m_plugins.insert(config, plugin);
                        Logger::log("RecaptchaPluginManager::load(). Qt Plugin loaded: " + config->id());
                    }
                    else {
                        loader.unload();
                        Logger::log("RecaptchaPluginManager::load(). Error loading Qt plugin: " + config->id());
                    }
                }
                else {
                    Logger::log("RecaptchaPluginManager::load(). Qt plugin is NULL: " + config->id());
                }
            }
        }
        else {
            delete config;
        }
    }
}

QNetworkAccessManager* RecaptchaPluginManager::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
