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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <QString>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

// Plugins
static const QString DECAPTCHA_PLUGIN_PATH("/opt/qdl2/plugins/decaptcha/");
static const QString RECAPTCHA_PLUGIN_PATH("/opt/qdl2/plugins/recaptcha/");
static const QString SERVICE_PLUGIN_PATH("/opt/qdl2/plugins/services/");

// Icons
static const QString APP_ICON_PATH("/opt/qdl2/icons/");
static const QString PLUGIN_ICON_PATH("/opt/qdl2/plugins/icons/");
static const QString DEFAULT_ICON("/opt/qdl2/icons/qdl2.png");

// Config
#if QT_VERSION >= 0x050000
static const QString APP_CONFIG_PATH(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/qdl2/");
#else
static const QString APP_CONFIG_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                     + "/.config/qdl2/");
#endif
static const QString PLUGIN_CONFIG_PATH(APP_CONFIG_PATH + "plugins/");

// Downloads
#if QT_VERSION >= 0x050000
static const QString DOWNLOAD_PATH(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                   + "/Downloads/qdl2/");
#else
static const QString DOWNLOAD_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                   + "/Downloads/qdl2/");
#endif

// Captcha
static const int CAPTCHA_TIMEOUT = 120000;

// Network
static const int DOWNLOAD_BUFFER_SIZE = 128000;
static const int MAX_CONCURRENT_TRANSFERS = 4;
static const int MAX_REDIRECTS = 8;

// Web interface
static const QString WEB_INTERFACE_PATH("/opt/qdl2/webif/");

// Version
static const QString VERSION_NUMBER("2.0.0");

#endif // DEFINITIONS_H
