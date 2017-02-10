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

#include "searchpage.h"
#include "addurlsdialog.h"
#include "urlcheckdialog.h"
#include "browser.h"
#include "pluginsettingsdialog.h"
#include "retrieveurlsdialog.h"
#include "transfermodel.h"
#include <QHBoxLayout>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QSplitter>

SearchPage::SearchPage(QWidget *parent) :
    Page(parent),
    m_model(new SearchModel(this)),
    m_splitter(new QSplitter(Qt::Horizontal, this)),
    m_view(new QListView(m_splitter)),
    m_browser(new Browser(m_splitter)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Search"));
    
    m_splitter->addWidget(m_view);
    m_splitter->addWidget(m_browser);
    
    m_view->setModel(m_model);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setUniformItemSizes(true);
    
    m_layout->addWidget(m_splitter);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    connect(m_model, SIGNAL(settingsRequest(QString, QVariantList)),
            this, SLOT(showPluginSettingsDialog(QString, QVariantList)));
    connect(m_model, SIGNAL(statusChanged(SearchModel::Status)), this, SLOT(onModelStatusChanged(SearchModel::Status)));
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(showItemDetails(QModelIndex)));
}

void SearchPage::search(const QString &query, const QString &pluginId) {
    m_model->search(query, pluginId);
}

void SearchPage::addUrl(const QModelIndex &index) {
    AddUrlsDialog addDialog(this);
    addDialog.setText(index.data(SearchModel::UrlRole).toString());

    if (addDialog.exec() == QDialog::Accepted) {
        const QStringList urls = addDialog.urls();

        if (!urls.isEmpty()) {
            if (addDialog.usePlugins()) {
                UrlCheckDialog checkDialog(this);
                checkDialog.addUrls(urls);
                checkDialog.exec();
            }
            else {
                TransferModel::instance()->append(urls, addDialog.requestMethod(), addDialog.requestHeaders(),
                                                  addDialog.postData());
            }
        }
    }
}

void SearchPage::retrieveUrls(const QModelIndex &index) {
    RetrieveUrlsDialog retrieveDialog(this);
    retrieveDialog.setText(index.data(SearchModel::UrlRole).toString());

    if (retrieveDialog.exec() == QDialog::Accepted) {
        const QStringList results = retrieveDialog.results();
        retrieveDialog.clear();

        if (!results.isEmpty()) {
            AddUrlsDialog addDialog(this);
            addDialog.setUrls(results);
            
            if (addDialog.exec() == QDialog::Accepted) {
                const QStringList urls = addDialog.urls();
                
                if (!urls.isEmpty()) {
                    if (addDialog.usePlugins()) {
                        UrlCheckDialog checkDialog(this);
                        checkDialog.addUrls(urls);
                        checkDialog.exec();
                    }
                    else {
                        TransferModel::instance()->append(urls, addDialog.requestMethod(), addDialog.requestHeaders(),
                                                          addDialog.postData());
                    }
                }
            }
        }
    }
}

void SearchPage::showContextMenu(const QPoint &pos) {
    const QModelIndex index = m_view->currentIndex();
    
    if (!index.isValid()) {
        return;
    }
    
    QMenu menu(this);
    QAction *addAction = menu.addAction(QIcon::fromTheme("list-add"), tr("&Add URL"));
    QAction *retrieveAction = menu.addAction(QIcon::fromTheme("folder-remote"), tr("&Retrieve URLs"));
    QAction *action = menu.exec(m_view->mapToGlobal(pos));
    
    if (!action) {
        return;
    }
    
    if (action == addAction) {
        addUrl(index);
    }
    else if (action == retrieveAction) {
        retrieveUrls(index);
    }
}

void SearchPage::showItemDetails(const QModelIndex &index) {
    m_browser->setHtml(index.data(SearchModel::DescriptionRole).toString());
}

void SearchPage::showPluginSettingsDialog(const QString &title, const QVariantList &settings) {
    PluginSettingsDialog dialog(settings, this);
    dialog.setWindowTitle(title);
    
    switch (dialog.exec()) {
    case QDialog::Accepted:
        m_model->submitSettingsResponse(dialog.settings());
        break;
    case QDialog::Rejected:
        m_model->submitSettingsResponse(QVariantMap());
        break;
    default:
        break;
    }
}

void SearchPage::onModelStatusChanged(SearchModel::Status status) {
    switch (status) {
    case SearchModel::Error:
        QMessageBox::critical(this, tr("Search error"), m_model->errorString());
        break;
    default:
        break;
    }
}
