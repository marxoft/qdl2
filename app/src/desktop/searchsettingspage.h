/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef SEARCHSETTINGSPAGE_H
#define SEARCHSETTINGSPAGE_H

#include "settingspage.h"

class SearchPluginConfigModel;
class QListView;
class QScrollArea;
class QSplitter;
class QVBoxLayout;

class SearchSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit SearchSettingsPage(QWidget *parent = 0);

public Q_SLOTS:
    virtual void save();

private Q_SLOTS:
    void setCurrentPlugin(const QModelIndex &index);

private:
    SearchPluginConfigModel *m_model;

    QListView *m_view;

    QScrollArea *m_scrollArea;

    QSplitter *m_splitter;

    QVBoxLayout *m_layout;
};

#endif // SEARCHSETTINGSPAGE_H
