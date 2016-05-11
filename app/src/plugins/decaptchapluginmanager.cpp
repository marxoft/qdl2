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
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QPluginLoader>

static bool displayNameLessThan(const DecaptchaPluginPair &pair, const DecaptchaPluginPair &other) {
    return QString::localeAwareCompare(pair.config->displayName(), other.config->displayName()) < 0;
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

DecaptchaPluginList DecaptchaPluginManager::plugins() const {
    return m_plugins;
}

DecaptchaPluginConfig* DecaptchaPluginManager::getConfigById(const QString &id) const {
    foreach (const DecaptchaPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("DecaptchaPluginManager::getConfigById(). PluginFound: " + id);
            return pair.config;
        }
    }
    
    Logger::log("DecaptchaPluginManager::getConfigById(). No Plugin found");
    return 0;
}

DecaptchaPluginConfig* DecaptchaPluginManager::getConfigByFilePath(const QString &filePath) const {
    foreach (const DecaptchaPluginPair &pair, m_plugins) {
        if (pair.config->filePath() == filePath) {
            Logger::log("DecaptchaPluginManager::getConfigByFilePath(). PluginFound: " + pair.config->id());
            return pair.config;
        }
    }
    
    Logger::log("DecaptchaPluginManager::getConfigByFilePath(). No Plugin found");
    return 0;
}

DecaptchaPlugin* DecaptchaPluginManager::getPluginById(const QString &id) const {
    foreach (const DecaptchaPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("DecaptchaPluginManager::getPluginById(). PluginFound: " + id);
            return pair.plugin;
        }
    }
    
    Logger::log("DecaptchaPluginManager::getPluginById(). No Plugin found");
    return 0;
}

DecaptchaPlugin* DecaptchaPluginManager::createPluginById(const QString &id, QObject *parent) const {
    if (DecaptchaPlugin *plugin = getPluginById(id)) {
        return plugin->createPlugin(parent);
    }

    return 0;
}

QNetworkAccessManager* DecaptchaPluginManager::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}

int DecaptchaPluginManager::load() {
    Logger::log("DecaptchaPluginManager::load(): Loading plugins modified since "
                + m_lastLoaded.toString(Qt::ISODate));
    int count = 0;
    QDir dir(DECAPTCHA_PLUGIN_PATH);
    
    foreach (const QFileInfo &info, dir.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Time)) {
        if (info.lastModified() > m_lastLoaded) {
            DecaptchaPluginConfig *config = getConfigByFilePath(info.absoluteFilePath());

            if (!config) {
                config = new DecaptchaPluginConfig(this);
                
                if (config->load(info.absoluteFilePath())) {
                    if (config->pluginType() == "js") {
                        JavaScriptDecaptchaPlugin *js =
                        new JavaScriptDecaptchaPlugin(config->id(), config->pluginFilePath(), this);
                        m_plugins << DecaptchaPluginPair(config, js);
                        ++count;
                        Logger::log("DecaptchaPluginManager::load(). JavaScript plugin loaded: " + config->id());
                    }
                    else {
                        QPluginLoader loader(config->pluginFilePath());
                        QObject *obj = loader.instance();
                        
                        if (obj) {
                            if (DecaptchaPlugin *plugin = qobject_cast<DecaptchaPlugin*>(obj)) {
                                plugin->setNetworkAccessManager(networkAccessManager());
                                m_plugins << DecaptchaPluginPair(config, plugin);
                                ++count;
                                Logger::log("DecaptchaPluginManager::load(). Qt Plugin loaded: " + config->id());
                            }
                            else {
                                loader.unload();
                                Logger::log("DecaptchaPluginManager::load(). Error loading Qt plugin: "
                                            + config->id());
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
        else {
            break;
        }
    }

    Logger::log(QString("DecaptchaPluginManager::load() %1 new plugins loaded").arg(count));

    if (count > 0) {
        qSort(m_plugins.begin(), m_plugins.end(), displayNameLessThan);
    }

    m_lastLoaded = QDateTime::currentDateTime();
    return count;
}
