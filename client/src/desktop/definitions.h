/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

// Home
#if QT_VERSION >= 0x050000
static const QString HOME_PATH(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
#else
static const QString HOME_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
#endif

// Icons
static const QString DEFAULT_ICON("/usr/share/qdl2-client/icons/qdl2-client.png");
static const QString OK_ICON("dialog-yes");
static const QString ERROR_ICON("dialog-no");

// Config
static const QString APP_CONFIG_PATH(HOME_PATH + "/.config/qdl2-client/");

// Timers
static const int CAPTCHA_TIMEOUT = 300000;
static const int RELOAD_INTERVAL = 3000;

// Network
static const int MAX_CONCURRENT_TRANSFERS = 4;
static const int MAX_REDIRECTS = 8;
static const QByteArray USER_AGENT("Mozilla/5.0 (X11; Linux x86_64; rv:53.0) Gecko/20100101 Firefox/53.0");

// Version
static const QString VERSION_NUMBER("2.7.0");

#endif // DEFINITIONS_H
