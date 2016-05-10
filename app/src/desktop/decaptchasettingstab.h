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

#ifndef DECAPTCHASETTINGSTAB_H
#define DECAPTCHASETTINGSTAB_H

#include "settingstab.h"

class DecaptchaPluginConfigModel;
class QCheckBox;
class QListView;
class QScrollArea;
class QSplitter;
class QVBoxLayout;

class DecaptchaSettingsTab : public SettingsTab
{
    Q_OBJECT

public:
    explicit DecaptchaSettingsTab(QWidget *parent = 0);

public Q_SLOTS:
    virtual void save();

private Q_SLOTS:
    void setCurrentPlugin(const QModelIndex &index);

private:
    DecaptchaPluginConfigModel *m_model;

    QListView *m_view;

    QWidget *m_container;

    QCheckBox *m_checkBox;
    
    QScrollArea *m_scrollArea;

    QVBoxLayout *m_vbox;

    QSplitter *m_splitter;
    
    QVBoxLayout *m_layout;

    QString m_pluginId;
};

#endif // DECAPTCHASETTINGSTAB_H
