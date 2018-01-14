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

#ifndef PLUGINSETTINGS_H
#define PLUGINSETTINGS_H

#include <QObject>
#include <QVariantList>

class Request;

class PluginSettings : public QObject
{
    Q_OBJECT

public:
    ~PluginSettings();

    static PluginSettings* instance();
    
public Q_SLOTS:
    void getSettings(const QString &pluginId);
    void setSettings(const QString &pluginId, const QVariantMap &settings);

private Q_SLOTS:
    void onRequestFinished(Request *request);

Q_SIGNALS:
    void error(const QString &errorString);
    void ready(const QString &pluginId, const QVariantList &settings);

private:
    PluginSettings();

    static PluginSettings *self;
};

#endif // PLUGINSETTINGS_H
