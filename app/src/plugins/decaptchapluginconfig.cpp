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

#include "decaptchapluginconfig.h"
#include "definitions.h"
#include "json.h"
#include "logger.h"
#include <QFile>

DecaptchaPluginConfig::DecaptchaPluginConfig(QObject *parent) :
    QObject(parent)
{
}

QString DecaptchaPluginConfig::displayName() const {
    return m_displayName;
}

QString DecaptchaPluginConfig::filePath() const {
    return m_filePath;
}

QString DecaptchaPluginConfig::iconFilePath() const {
    return m_iconFilePath;
}

QString DecaptchaPluginConfig::id() const {
    return m_id;
}

QString DecaptchaPluginConfig::pluginFilePath() const {
    return m_pluginFilePath;
}

QString DecaptchaPluginConfig::pluginType() const {
    return m_pluginType;
}

QVariantList DecaptchaPluginConfig::settings() const {
    return m_settings;
}

bool DecaptchaPluginConfig::load(const QString &filePath) {
    m_filePath = filePath;
    QFile file(filePath);

    if ((!file.exists()) || (!file.open(QFile::ReadOnly))) {
        Logger::log("DecaptchaPluginConfig::load(): Unable to open config file: " + filePath);
        return false;
    }

    bool ok;
    const QVariant v = QtJson::Json::parse(QString::fromUtf8(file.readAll()), ok);
    file.close();

    if (!ok) {
        Logger::log("DecaptchaPluginConfig::load(): Error parsing config file: " + filePath);
        return false;
    }

    const QVariantMap config = v.toMap();
    
    if ((!config.contains("name")) || (!config.contains("file")) || (!config.contains("id"))) {
        Logger::log("DecaptchaPluginConfig::load(): Some parameters are missing");
        return false;
    }

    Logger::log("DecaptchaPluginConfig::load(): Config file loaded: " + filePath);
    m_displayName = config.value("name").toString();
    m_iconFilePath = config.contains("icon") ? PLUGIN_ICON_PATH + config.value("icon").toString() : DEFAULT_ICON;
    m_id = config.value("id").toString();
    m_pluginFilePath = DECAPTCHA_PLUGIN_PATH + config.value("file").toString();
    m_pluginType = config.value("type").toString();
    m_settings = config.value("settings").toList();
    emit changed();
    return true;
}
