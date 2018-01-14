/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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
#include "imagecache.h"
#include "logger.h"

DecaptchaPluginConfig::DecaptchaPluginConfig(QObject *parent) :
    QObject(parent),
    m_version(1)
{
}

QString DecaptchaPluginConfig::displayName() const {
    return m_displayName;
}

QString DecaptchaPluginConfig::filePath() const {
    return m_filePath;
}

QIcon DecaptchaPluginConfig::icon() const {
    const QPixmap icon = ImageCache::instance()->image(iconFilePath());

    if (icon.isNull()) {
        connect(ImageCache::instance(), SIGNAL(imageReady(QString)), this, SLOT(onImageReady(QString)));
        return QPixmap(DEFAULT_ICON);
    }

    return icon;
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

int DecaptchaPluginConfig::version() const {
    return m_version;
}

void DecaptchaPluginConfig::load(const QVariantMap &config) {
    m_displayName = config.value("displayName").toString();
    m_iconFilePath = config.value("iconFilePath", DEFAULT_ICON).toString();
    m_id = config.value("id").toString();
    m_pluginFilePath = config.value("pluginFilePath").toString();
    m_pluginType = config.value("pluginType").toString();
    m_version = qMax(1, config.value("version").toInt());
    Logger::log("DecaptchaPluginConfig::load(): Config loaded: " + id(), Logger::MediumVerbosity);
    emit changed(this);
}

void DecaptchaPluginConfig::onImageReady(const QString &path) {
    if (path == iconFilePath()) {
        emit changed(this);
        disconnect(ImageCache::instance(), SIGNAL(imageReady(QString)), this, SLOT(onImageReady(QString)));
    }
}
