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

#include <QStringList>

// Home
static const QString ROOT_PATH("C:/");
static const QString HOME_PATH("E:/");

// Plugins
static const QStringList DECAPTCHA_PLUGIN_PATHS = QStringList() << QString(ROOT_PATH + "qdl2/plugins/decaptcha/")
                                                                << QString(HOME_PATH + "qdl2/plugins/decaptcha/");

static const QStringList RECAPTCHA_PLUGIN_PATHS = QStringList() << QString(ROOT_PATH + "qdl2/plugins/recaptcha/")
                                                                << QString(HOME_PATH + "qdl2/plugins/recaptcha/");

static const QStringList SERVICE_PLUGIN_PATHS = QStringList() << QString(ROOT_PATH + "qdl2/plugins/services/")
                                                              << QString(HOME_PATH + "qdl2/plugins/services/");

static const QString LIB_PREFIX();
static const QString LIB_SUFFIX(".qtplugin");

// Icons
static const QString DEFAULT_ICON(ROOT_PATH + "qdl2/icons/qdl2.png");

// Config
static const QString APP_CONFIG_PATH(ROOT_PATH + "qdl2/.config/");
static const QString PLUGIN_CONFIG_PATH(APP_CONFIG_PATH + "plugins/");

// Downloads
static const QString DOWNLOAD_PATH(HOME_PATH + "qdl2/");

// Captcha
static const int CAPTCHA_TIMEOUT = 120000;

// Network
static const int DOWNLOAD_BUFFER_SIZE = 512000;
static const int MAX_CONCURRENT_TRANSFERS = 4;
static const int MAX_REDIRECTS = 8;

// Version
static const QString VERSION_NUMBER("2.0.1");

// Appearance
static const QString ACTIVE_COLOR("#0881cb");

#endif // DEFINITIONS_H
