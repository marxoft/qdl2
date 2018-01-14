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

#ifndef RECAPTCHASETTINGSPAGE_H
#define RECAPTCHASETTINGSPAGE_H

#include "settingspage.h"

class RecaptchaPluginConfigModel;
class QListView;
class QScrollArea;
class QSplitter;
class QVBoxLayout;

class RecaptchaSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit RecaptchaSettingsPage(QWidget *parent = 0);

public Q_SLOTS:
    virtual void save();

private Q_SLOTS:
    void setCurrentPlugin(const QModelIndex &index);

private:
    RecaptchaPluginConfigModel *m_model;

    QListView *m_view;

    QScrollArea *m_scrollArea;

    QSplitter *m_splitter;

    QVBoxLayout *m_layout;
};

#endif // RECAPTCHASETTINGSPAGE_H
