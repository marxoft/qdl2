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

#ifndef PLUGINSETTINGSPAGE_H
#define PLUGINSETTINGSPAGE_H

#include "settingspage.h"
#include <QVariantList>

class QFormLayout;

class PluginSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit PluginSettingsPage(const QString &pluginId, QWidget *parent = 0);

public Q_SLOTS:
    virtual void save();

private Q_SLOTS:
    void setBooleanValue(bool value);
    void setIntegerValue(int value);
    void setListValue(int value);
    void setMultiListValue(QAction *action);
    void setTextValue(const QString &value);

    void onSettingsReady(const QString &pluginId, const QVariantList &settings);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    void addCheckBox(QFormLayout *layout, const QString &label, const QString &key, bool value);
    void addComboBox(QFormLayout *layout, const QString &label, const QString &key, const QVariantList &options,
                     const QVariant &value);
    void addGroupBox(QFormLayout *layout, const QString &label, const QString &key, const QVariantList &settings);
    void addLineEdit(QFormLayout *layout, const QString &label, const QString &key, const QString &value,
                     bool isPassword = false);
    void addMenuButton(QFormLayout *layout, const QString &label, const QString &key, const QVariantList &options,
                       const QVariantList &values);
    void addSpinBox(QFormLayout *layout, const QString &label, const QString &key, int minimum, int maximum,
                    int step, int value);
    void addWidget(QFormLayout *layout, const QVariantMap &setting, const QString &group = QString());

    QFormLayout *m_layout;

    QString m_pluginId;
    QVariantMap m_settings;
};

#endif // PLUGINSETTINGSPAGE_H
