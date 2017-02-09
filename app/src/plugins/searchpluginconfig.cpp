/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "searchpluginconfig.h"
#include "definitions.h"
#include "json.h"
#include "logger.h"
#include <QFile>

SearchPluginConfig::SearchPluginConfig(QObject *parent) :
    QObject(parent),
    m_version(1)
{
}

QString SearchPluginConfig::displayName() const {
    return m_displayName;
}

QString SearchPluginConfig::filePath() const {
    return m_filePath;
}

QString SearchPluginConfig::iconFilePath() const {
    return m_iconFilePath;
}

QString SearchPluginConfig::id() const {
    return m_id;
}

QString SearchPluginConfig::pluginFilePath() const {
    return m_pluginFilePath;
}

QString SearchPluginConfig::pluginType() const {
    return m_pluginType;
}

QVariantList SearchPluginConfig::settings() const {
    return m_settings;
}

int SearchPluginConfig::version() const {
    return m_version;
}

bool SearchPluginConfig::load(const QString &filePath) {
    m_filePath = filePath;
    QFile file(filePath);

    if ((!file.exists()) || (!file.open(QFile::ReadOnly))) {
        Logger::log("SearchPluginConfig::load(): Unable to open config file: " + filePath);
        return false;
    }

    bool ok;
    const QVariant v = QtJson::Json::parse(QString::fromUtf8(file.readAll()), ok);
    file.close();

    if (!ok) {
        Logger::log("SearchPluginConfig::load(): Error parsing config file: " + filePath);
        return false;
    }

    const QVariantMap config = v.toMap();

    if (!config.contains("name")) {
        Logger::log("SearchPluginConfig::load(): 'name' parameter is missing");
        return false;
    }

    Logger::log("SearchPluginConfig::load(): Config file loaded: " + filePath, Logger::MediumVerbosity);    
    const int slash = filePath.lastIndexOf("/");
    const QString fileName = filePath.mid(slash + 1);
    const int dot = fileName.lastIndexOf(".");
    m_displayName = config.value("name").toString();
    m_iconFilePath = config.contains("icon") ? QString("%1/icons/%2").arg(filePath.section("/", 0, -3))
                                                                     .arg(config.value("icon").toString())
                                             : DEFAULT_ICON;
    m_id = fileName.left(dot);
    m_pluginType = config.value("type").toString();
    m_settings = config.value("settings").toList();
    m_version = qMax(1, config.value("version").toInt());
    
    if (m_pluginType == "js") {
        m_pluginFilePath = filePath.left(slash + 1) + m_id + ".js";
    }
    else {
        m_pluginFilePath = filePath.left(slash + 1) + LIB_PREFIX + m_id + LIB_SUFFIX;
    }
    
    emit changed();
    return true;
}
