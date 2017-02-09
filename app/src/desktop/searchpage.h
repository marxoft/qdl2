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

#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include "page.h"
#include "searchmodel.h"

class Browser;
class QHBoxLayout;
class QListView;
class QSplitter;

class SearchPage : public Page
{
    Q_OBJECT

public:
    explicit SearchPage(QWidget *parent = 0);
   
public Q_SLOTS:
    void search(const QString &query, const QString &pluginId);

private Q_SLOTS:
    void addUrl(const QModelIndex &index);
    void retrieveUrls(const QModelIndex &index);
    
    void showContextMenu(const QPoint &pos);
    
    void showItemDetails(const QModelIndex &index);
    
    void showPluginSettingsDialog(const QString &title, const QVariantList &settings);
    
    void onModelStatusChanged(SearchModel::Status status);

private:
    SearchModel *m_model;
    
    QSplitter *m_splitter;
    
    QListView *m_view;
    
    Browser *m_browser;
    
    QHBoxLayout *m_layout;
};

#endif // SEARCHPAGE_H
