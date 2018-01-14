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

class QHBoxLayout;
class QListView;
class QSplitter;
class QWebView;

class SearchPage : public Page
{
    Q_OBJECT

public:
    explicit SearchPage(QWidget *parent = 0);
    ~SearchPage();

    virtual QString errorString() const;

    virtual Status status() const;
    virtual QString statusString() const;
   
public Q_SLOTS:
    void search(const QString &pluginId);

protected:
    virtual void closeEvent(QCloseEvent *event);

private Q_SLOTS:
    void showContextMenu(const QPoint &pos);
    void showBrowserContextMenu(const QPoint &pos);
    
    void showItemDetails(const QModelIndex &index);
    
    void showPluginSettingsDialog(const QString &title, const QVariantList &settings);
    
    void onModelStatusChanged();

private:
    void addUrl(const QString &url);
    void copyUrl(const QString &url);
    void retrieveUrls(const QString &url);
    void fetchDownloadRequests(const QString &url);

    SearchModel *m_model;
    
    QSplitter *m_splitter;
    
    QListView *m_view;
    
    QWebView *m_browser;
    
    QHBoxLayout *m_layout;

    static const QByteArray STYLE_SHEET;
};

#endif // SEARCHPAGE_H
