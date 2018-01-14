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

#include "servicepluginconfig.h"
#include "definitions.h"
#include "imagecache.h"
#include "logger.h"

ServicePluginConfig::ServicePluginConfig(QObject *parent) :
    QObject(parent),
    m_version(1)
{
}

QString ServicePluginConfig::displayName() const {
    return m_displayName;
}

QString ServicePluginConfig::filePath() const {
    return m_filePath;
}

QIcon ServicePluginConfig::icon() {
    const QPixmap icon = ImageCache::instance()->image(iconFilePath());

    if (icon.isNull()) {
        connect(ImageCache::instance(), SIGNAL(imageReady(QString)), this, SLOT(onImageReady(QString)));
        return QPixmap(DEFAULT_ICON);
    }

    return icon;
}

QString ServicePluginConfig::iconFilePath() const {
    return m_iconFilePath;
}

QString ServicePluginConfig::id() const {
    return m_id;
}

QString ServicePluginConfig::pluginFilePath() const {
    return m_pluginFilePath;
}

QString ServicePluginConfig::pluginType() const {
    return m_pluginType;
}

QRegExp ServicePluginConfig::regExp() const {
    return m_regExp;
}

int ServicePluginConfig::version() const {
    return m_version;
}

void ServicePluginConfig::load(const QVariantMap &config) {
    m_displayName = config.value("displayName").toString();
    m_iconFilePath = config.value("iconFilePath", DEFAULT_ICON).toString();
    m_id = config.value("id").toString();
    m_pluginFilePath = config.value("pluginFilePath").toString();
    m_pluginType = config.value("pluginType").toString();
    m_regExp = QRegExp(config.value("regExp").toString());
    m_version = qMax(1, config.value("version").toInt());
    Logger::log("ServicePluginConfig::load(): Config loaded: " + id(), Logger::MediumVerbosity);    
    emit changed(this);
}

bool ServicePluginConfig::urlIsSupported(const QString &url) const {
    return url.indexOf(regExp()) == 0;
}

void ServicePluginConfig::onImageReady(const QString &path) {
    if (path == iconFilePath()) {
        emit changed(this);
        disconnect(ImageCache::instance(), SIGNAL(imageReady(QString)), this, SLOT(onImageReady(QString)));
    }
}
