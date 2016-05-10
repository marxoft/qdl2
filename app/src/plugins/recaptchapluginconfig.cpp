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

#include "recaptchapluginconfig.h"
#include "definitions.h"
#include "json.h"
#include "logger.h"
#include <QFile>

RecaptchaPluginConfig::RecaptchaPluginConfig(QObject *parent) :
    QObject(parent)
{
}

QString RecaptchaPluginConfig::displayName() const {
    return m_displayName;
}

QString RecaptchaPluginConfig::filePath() const {
    return m_filePath;
}

QString RecaptchaPluginConfig::iconFilePath() const {
    return m_iconFilePath;
}

QString RecaptchaPluginConfig::id() const {
    return m_id;
}

QString RecaptchaPluginConfig::pluginType() const {
    return m_pluginType;
}

QVariantList RecaptchaPluginConfig::settings() const {
    return m_settings;
}

bool RecaptchaPluginConfig::load(const QString &configFileName) {
    QFile file(configFileName);

    if ((!file.exists()) || (!file.open(QFile::ReadOnly))) {
        Logger::log("RecaptchaPluginConfig::load(): Unable to open config file: " + configFileName);
        return false;
    }

    bool ok;
    const QVariant v = QtJson::Json::parse(QString::fromUtf8(file.readAll()), ok);
    file.close();

    if (!ok) {
        Logger::log("RecaptchaPluginConfig::load(): Error parsing config file: " + configFileName);
        return false;
    }

    const QVariantMap config = v.toMap();

    if ((!config.contains("name")) || (!config.contains("file")) || (!config.contains("id"))) {
        Logger::log("RecaptchaPluginConfig::load(): Some parameters are missing");
        return false;
    }

    Logger::log("RecaptchaPluginConfig::load(): Config file loaded: " + configFileName);
    m_displayName = config.value("name").toString();
    m_filePath = RECAPTCHA_PLUGIN_PATH + config.value("file").toString();
    m_iconFilePath = config.contains("icon") ? PLUGIN_ICON_PATH + config.value("icon").toString() : DEFAULT_ICON;
    m_id = config.value("id").toString();
    m_pluginType = config.value("type").toString();
    m_settings = config.value("settings").toList();
    emit changed();
    return true;
}
